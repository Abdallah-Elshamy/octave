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
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#if defined (__GNUG__)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream.h>
#include <strstream.h>

#include "defun.h"
#include "error.h"
#include "input.h"
#include "oct-obj.h"
#include "oct-lvalue.h"
#include "pager.h"
#include "ov.h"
#include "pt-arg-list.h"
#include "pt-assign.h"
#include "pt-pr-code.h"
#include "pt-walk.h"
#include "utils.h"

// TRUE means print the right hand side of an assignment instead of
// the left.
static bool Vprint_rhs_assign_val;

// Simple assignment expressions.

tree_simple_assignment::~tree_simple_assignment (void)
{
  if (! preserve)
    delete lhs;

  delete rhs;
}

octave_value_list
tree_simple_assignment::rvalue (int nargout)
{
  octave_value_list retval;

  if (nargout > 1)
    error ("invalid number of output arguments for expression X = RHS");
  else
    retval = rvalue ();

  return retval;
}

// XXX FIXME XXX -- this works, but it would look a little better if
// it were broken up into a couple of separate functions.

octave_value
tree_simple_assignment::rvalue (void)
{
  octave_value rhs_val;

  if (error_state)
    return rhs_val;

  if (rhs)
    {
      octave_value_list tmp = rhs->rvalue ();

      if (! (error_state || tmp.empty ()))
	{
	  rhs_val = tmp(0);

	  if (rhs_val.is_undefined ())
	    {
	      error ("value on right hand side of assignment is undefined");
	      eval_error ();
	    }
	  else
	    {
	      octave_lvalue ult = lhs->lvalue ();

	      if (error_state)
		eval_error ();
	      else
		{
		  ult.assign (etype, rhs_val);

		  // We clear any index here so that we can get the
		  // new value of the referenced object below, instead
		  // of the indexed value (which should be the same as
		  // the right hand side value).

		  ult.clear_index ();

		  if (error_state)
		    eval_error ();
		  else if (! Vprint_rhs_assign_val)
		    {
		      octave_value lhs_val = ult.value ();

		      if (! error_state && print_result ())
			{
			  if (Vprint_rhs_assign_val)
			    {
			      ostrstream buf;

			      tree_print_code tpc (buf);

			      lhs->accept (tpc);

			      buf << ends;

			      const char *tag = buf.str ();

			      rhs_val.print_with_name (octave_stdout, tag);

			      delete [] tag;
			    }
			  else
			    lhs_val.print_with_name (octave_stdout,
						     lhs->name ());
			}
		    }
		}
	    }
	}
      else
	eval_error ();
    }

  return rhs_val;
}

void
tree_simple_assignment::eval_error (void)
{
  if (error_state > 0)
    {
      int l = line ();
      int c = column ();

      if (l != -1 && c != -1)
	::error ("evaluating assignment expression near line %d, column %d",
		 l, c);
    }
}

string
tree_simple_assignment::oper (void) const
{
  return octave_value::assign_op_as_string (etype);
}

void
tree_simple_assignment::accept (tree_walker& tw)
{
  tw.visit_simple_assignment (*this);
}

// Multi-valued assignment expressions.

tree_multi_assignment::~tree_multi_assignment (void)
{
  if (! preserve)
    delete lhs;

  delete rhs;
}

octave_value
tree_multi_assignment::rvalue (void)
{
  octave_value retval;

  octave_value_list tmp = rvalue (1);

  if (! tmp.empty ())
    retval = tmp(0);

  return retval;
}

// XXX FIXME XXX -- this works, but it would look a little better if
// it were broken up into a couple of separate functions.

octave_value_list
tree_multi_assignment::rvalue (int)
{
  octave_value_list rhs_val;

  if (error_state)
    return rhs_val;

  if (rhs)
    {
      int n_out = lhs->length ();

      rhs_val = rhs->rvalue (n_out);

      if (! (error_state || rhs_val.empty ()))
	{
	  if (rhs_val.empty ())
	    {
	      error ("value on right hand side of assignment is undefined");
	      eval_error ();
	    }
	  else
	    {
	      int k = 0;

	      int n = rhs_val.length ();

	      for (Pix p = lhs->first (); p != 0; lhs->next (p))
		{
		  tree_expression *lhs_elt = lhs->operator () (p);

		  if (lhs_elt)
		    {
		      octave_lvalue ult = lhs_elt->lvalue ();

		      if (error_state)
			eval_error ();
		      else
			{
			  octave_value tmp = k < n
			    ? rhs_val(k++) : octave_value ();

			  if (tmp.is_defined ())
			    {
			      // XXX FIXME XXX -- handle other assignment ops.
			      ult.assign (octave_value::asn_eq, tmp);

			      // We clear any index here so that we
			      // can get the new value of the
			      // referenced object below, instead of
			      // the indexed value (which should be
			      // the same as the right hand side
			      // value).

			      ult.clear_index ();
			    }
			  else
			    error ("element number %d undefined in return list", k);

			  if (error_state)
			    eval_error ();
			  else if (! Vprint_rhs_assign_val)
			    {
			      octave_value lhs_val = ult.value ();

			      if (! error_state && print_result ())
				{
				  if (Vprint_rhs_assign_val)
				    {
				      ostrstream buf;

				      tree_print_code tpc (buf);

				      lhs_elt->accept (tpc);

				      buf << ends;

				      const char *tag = buf.str ();

				      tmp.print_with_name
					(octave_stdout, tag);

				      delete [] tag;
				    }
				  else
				    lhs_val.print_with_name (octave_stdout,
							     lhs_elt->name ());
				}
			    }
			}
		    }

		  if (error_state)
		    break;
		}
	    }
	}
      else
	eval_error ();
    }

  return rhs_val;
}

void
tree_multi_assignment::eval_error (void)
{
  if (error_state > 0)
    {
      int l = line ();
      int c = column ();

      if (l != -1 && c != -1)
	::error ("evaluating assignment expression near line %d, column %d",
		 l, c);
    }
}

void
tree_multi_assignment::accept (tree_walker& tw)
{
  tw.visit_multi_assignment (*this);
}

static int
print_rhs_assign_val (void)
{
  Vprint_rhs_assign_val = check_preference ("print_rhs_assign_val");

  return 0;
}

void
symbols_of_pt_assign (void)
{
  DEFVAR (print_rhs_assign_val, 0.0, 0, print_rhs_assign_val,
    "if TRUE, print the right hand side of assignments instead of the left");
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
