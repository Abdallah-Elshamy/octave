// defun-dld.h                                           -*- C++ -*-
/*

Copyright (C) 1992, 1993, 1994, 1995 John W. Eaton

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

#if !defined (octave_defun_dld_h)
#define octave_defun_dld_h 1

#if defined (octave_defun_h)
#error defun.h and defun-dld.h both included in same file!
#endif

#include "defun-int.h"

// Define a builtin function that may be loaded dynamically at run
// time.
//
// If Octave is not configured for dynamic linking of builtin
// functions, this is exactly like DEFUN.

#if defined (WITH_DLD) && defined (OCTAVE_LITE) && defined (MAKE_BUILTINS)
#define DEFUN_DLD_BUILTIN(name, fname, sname, nargin_max, nargout_max, doc) \
  BEGIN_INSTALL_BUILTIN \
    DEFINE_FUN_STRUCT (name, 0, sname, nargin_max, nargout_max, 0, doc); \
    install_builtin_function (&sname); \
  END_INSTALL_BUILTIN
#else
#define DEFUN_DLD_BUILTIN(name, fname, sname, nargin_max, nargout_max, doc) \
  DEFUN_INTERNAL (name, fname, sname, nargin_max, nargout_max, 0, doc)
#endif

// Define a function that may be loaded dynamically at run time.
//
// If Octave is not configured for dynamic linking of builtin
// functions, this won't do anything useful.
//
// The forward declaration is for the struct, the second is for the
// definition of the function.

#if ! defined (MAKE_BUILTINS)
#define DEFUN_DLD(name, fname, sname, fsname, nargin_max, nargout_max, doc) \
  DECLARE_FUN (fname); \
  DEFINE_FUN_STRUCT (name, fname, sname, nargin_max, nargout_max, 0, doc); \
  DEFINE_FUN_STRUCT_FUN (sname, fsname) \
  DECLARE_FUN (fname)
#endif

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
