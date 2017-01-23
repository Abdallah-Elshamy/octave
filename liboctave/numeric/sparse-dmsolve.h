/*

Copyright (C) 2016 John W. Eaton
Copyright (C) 2006-2016 David Bateman

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if ! defined (octave_sparse_dmsolve_h)
#define octave_sparse_dmsolve_h 1

#include "octave-config.h"

template <typename RT, typename ST, typename T>
RT
dmsolve (const ST& a, const T& b, octave_idx_type& info);

#endif
