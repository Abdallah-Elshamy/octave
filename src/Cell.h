/*

Copyright (C) 1999 John W. Eaton

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

#if !defined (Cell_h)
#define Cell_h 1

#include <string>

#include "ArrayN.h"
#include "oct-alloc.h"
#include "str-vec.h"

#include "oct-obj.h"

class
Cell : public ArrayN<octave_value>
{
public:

  Cell (void)
    : ArrayN<octave_value> (dim_vector (0, 0)) { }

  Cell (const octave_value& val)
    : ArrayN<octave_value> (dim_vector (1, 1), val) { }

  Cell (const octave_value_list& ovl)
    : ArrayN<octave_value> (dim_vector (ovl.length (), 1))
    {
      for (octave_idx_type i = 0; i < ovl.length (); i++)
	elem (i) = ovl (i);
    }

  Cell (octave_idx_type n, octave_idx_type m, const octave_value& val = resize_fill_value ())
    : ArrayN<octave_value> (dim_vector (n, m), val) { }

  Cell (const dim_vector& dv, const octave_value& val = resize_fill_value ())
    : ArrayN<octave_value> (dv, val) { }

  Cell (const ArrayN<octave_value>& c)
    : ArrayN<octave_value> (c) { }

  Cell (const Array<octave_value>& c)
    : ArrayN<octave_value> (c) { }

  Cell (const Array<octave_value>& c, octave_idx_type nr, octave_idx_type nc)
    : ArrayN<octave_value> (c, dim_vector (nr, nc)) { }

  Cell (const string_vector& sv, bool trim = false);

  Cell (const Cell& c)
    : ArrayN<octave_value> (c) { }

  Cell index (const octave_value_list& idx, bool resize_ok = false) const;

  Cell index (idx_vector& i, int resize_ok = 0,
	      const octave_value& rfv = resize_fill_value ()) const
    { return Cell (ArrayN<octave_value>::index (i, resize_ok, rfv)); }

  Cell index (idx_vector& i, idx_vector& j, int resize_ok = 0,
	      const octave_value& rfv = resize_fill_value ()) const
    { return Cell (ArrayN<octave_value>::index (i, j, resize_ok, rfv)); }

  Cell index (Array<idx_vector>& ra_idx, int resize_ok = 0,
	      const octave_value& rfv = resize_fill_value ()) const
    { return Cell (ArrayN<octave_value>::index (ra_idx, resize_ok, rfv)); }

  Cell& assign (const octave_value_list& idx, const Cell& rhs,
		const octave_value& fill_val = octave_value ());

  Cell reshape (const dim_vector& new_dims) const
    { return ArrayN<octave_value>::reshape (new_dims); }

  octave_idx_type nnz (void) const;

  Cell column (octave_idx_type i) const;

  // FIXME
  boolMatrix all (int /* dim */ = 0) const { return boolMatrix (); }

  // FIXME
  boolMatrix any (int /* dim */ = 0) const { return boolMatrix (); }

  Cell concat (const Cell& rb, const Array<octave_idx_type>& ra_idx);

  Cell& insert (const Cell& a, octave_idx_type r, octave_idx_type c);
  Cell& insert (const Cell& a, const Array<octave_idx_type>& ra_idx);

  // FIXME
  bool is_true (void) const { return false; }

  static octave_value resize_fill_value (void) { return Matrix (); }
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
