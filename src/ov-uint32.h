/*

Copyright (C) 2004 John W. Eaton

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

#if !defined (octave_uint32_h)
#define octave_uint32_h 1

#define OCTAVE_VALUE_INT_MATRIX_T octave_uint32_matrix
#define OCTAVE_INT_NDARRAY_T uint32NDArray
#define OCTAVE_VALUE_INT_NDARRAY_EXTRACTOR_FUNCTION uint32_array_value

#define OCTAVE_VALUE_INT_SCALAR_T octave_uint32_scalar
#define OCTAVE_INT_T octave_uint32
#define OCTAVE_VALUE_INT_SCALAR_EXTRACTOR_FUNCTION uint32_scalar_value

#include "ov-intx.h"

#undef OCTAVE_VALUE_INT_MATRIX_T
#undef OCTAVE_INT_NDARRAY_T
#undef OCTAVE_VALUE_INT_NDARRAY_EXTRACTOR_FUNCTION

#undef OCTAVE_VALUE_INT_SCALAR_T
#undef OCTAVE_INT_T
#undef OCTAVE_VALUE_INT_SCALAR_EXTRACTOR_FUNCTION

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/

