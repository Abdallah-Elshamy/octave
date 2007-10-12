/*

Copyright (C) 1996, 1997 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gripes.h"
#include "oct-obj.h"
#include "ov.h"
#include "ov-struct.h"
#include "ov-typeinfo.h"
#include "ops.h"

// struct ops.

DEFUNOP (transpose, cell)
{
  CAST_UNOP_ARG (const octave_struct&);

  if (v.ndims () > 2)
    {
      error ("transpose not defined for N-d objects");
      return octave_value ();
    }
  else
    return octave_value (v.map_value().transpose ());
}

DEFNDCATOP_FN (struct_struct, struct, struct, map, map, concat)

void
install_struct_ops (void)
{
  INSTALL_UNOP (op_transpose, octave_struct, transpose);
  INSTALL_UNOP (op_hermitian, octave_struct, transpose);

  INSTALL_CATOP (octave_struct, octave_struct, struct_struct);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
