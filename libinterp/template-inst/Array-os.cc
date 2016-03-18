/*

Copyright (C) 1996-2015 John W. Eaton

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

// Instantiate Arrays of octave_stream objects.

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "Array.h"
#include "Array.cc"

extern template class OCTAVE_API Array<bool>;
extern template class OCTAVE_API Array<octave_idx_type>;

#include "oct-stream.h"

NO_INSTANTIATE_ARRAY_SORT (octave_stream);
INSTANTIATE_ARRAY (octave_stream, OCTINTERP_API);
