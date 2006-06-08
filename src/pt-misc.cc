/*

Copyright (C) 1996, 1997 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Cell.h"

#include "defun.h"
#include "error.h"
#include "ov.h"
#include "oct-lvalue.h"
#include "pt-id.h"
#include "pt-idx.h"
#include "pt-misc.h"
#include "pt-walk.h"
#include "utils.h"

// Parameter lists.

tree_parameter_list::~tree_parameter_list (void)
{
  while (! empty ())
    {
      iterator p = begin ();
      delete *p;
      erase (p);
    }
}

void
tree_parameter_list::mark_as_formal_parameters (void)
{
  for (iterator p = begin (); p != end (); p++)
    {
      tree_identifier *elt = *p;
      elt->mark_as_formal_parameter ();
    }
}

void
tree_parameter_list::initialize_undefined_elements (const std::string& warnfor,
						    int nargout,
						    const octave_value& val)
{
  bool warned = false;

  int count = 0;

  for (iterator p = begin (); p != end (); p++)
    {
      if (++count > nargout)
	break;

      tree_identifier *elt = *p;

      if (! elt->is_defined ())
	{
	  if (! warned)
	    {
	      warned = true;

	      warning_with_id
		("Octave:undefined-return-values",
		 "%s: some elements in list of return values are undefined",
		 warnfor.c_str ());
	    }

	  octave_lvalue tmp = elt->lvalue ();

	  tmp.assign (octave_value::op_asn_eq, val);
	}
    }
}

void
tree_parameter_list::define_from_arg_vector (const octave_value_list& args)
{
  int nargin = args.length ();

  if (nargin <= 0)
    return;

  int expected_nargin = length ();

  iterator p = begin ();

  for (int i = 0; i < expected_nargin; i++)
    {
      tree_identifier *elt = *p++;

      octave_lvalue ref = elt->lvalue ();

      if (i < nargin)
	{
	  if (args(i).is_defined () && args(i).is_magic_colon ())
	    {
	      ::error ("invalid use of colon in function argument list");
	      return;
	    }

	  ref.assign (octave_value::op_asn_eq, args(i));
	}
      else
	ref.assign (octave_value::op_asn_eq, octave_value ());
    }
}

void
tree_parameter_list::undefine (void)
{
  int len = length ();

  iterator p = begin ();

  for (int i = 0; i < len; i++)
    {
      tree_identifier *elt = *p++;

      octave_lvalue ref = elt->lvalue ();

      ref.assign (octave_value::op_asn_eq, octave_value ());
    }
}

octave_value_list
tree_parameter_list::convert_to_const_vector (const Cell& varargout)
{
  octave_idx_type vlen = varargout.numel ();

  int nout = length () + vlen;

  octave_value_list retval (nout, octave_value ());

  int i = 0;

  for (iterator p = begin (); p != end (); p++)
    {
      tree_identifier *elt = *p;

      retval(i++) = elt->is_defined () ? elt->rvalue () : octave_value ();
    }

  for (octave_idx_type j = 0; j < vlen; j++)
    retval(i++) = varargout(j);

  return retval;
}

bool
tree_parameter_list::is_defined (void)
{
  bool status = true;

  for (iterator p = begin (); p != end (); p++)
    {
      tree_identifier *elt = *p;

      if (! elt->is_defined ())
	{
	  status = false;
	  break;
	}
    }

  return status;
}

void
tree_parameter_list::accept (tree_walker& tw)
{
  tw.visit_parameter_list (*this);
}

// Return lists.

tree_return_list::~tree_return_list (void)
{
  while (! empty ())
    {
      iterator p = begin ();
      delete *p;
      erase (p);
    }
}

void
tree_return_list::accept (tree_walker& tw)
{
  tw.visit_return_list (*this);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
