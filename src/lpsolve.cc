// f-lpsolve.cc                                          -*- C++ -*-
/*

Copyright (C) 1993, 1994 John W. Eaton

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
Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "LPsolve.h"

#include "tree-const.h"
#include "error.h"
#include "f-lpsolve.h"

#ifdef WITH_DLD
Octave_object
builtin_lpsolve_2 (const Octave_object& args, int nargin, int nargout)
{
  return lpsolve (args, nargin, nargout);
}

Octave_object
builtin_lpsolve_options_2 (const Octave_object& args, int nargin, int nargout) 
{
  return lpsolve_options (args, nargin, nargout);
}
#endif

Octave_object
lpsolve (const Octave_object& args, int nargin, int nargout)
{
// Assumes that we have been given the correct number of arguments.

  Octave_object retval;
  error ("lpsolve: not implemented yet");
  return retval;
}

Octave_object
lpsolve_options (const Octave_object& args, int nargin, int nargout)
{
// Assumes that we have been given the correct number of arguments.

  Octave_object retval;
  error ("lpsolve_options: not implemented yet");
  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
