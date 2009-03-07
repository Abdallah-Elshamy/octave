/*

Copyright (C) 1994, 1995, 1996, 1997, 1999, 2000, 2002, 2003, 2004,
              2005, 2007, 2008 John W. Eaton

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

#include <iostream>

#include "floatSVD.h"
#include "f77-fcn.h"

extern "C"
{
  F77_RET_T
  F77_FUNC (sgesvd, SGESVD) (F77_CONST_CHAR_ARG_DECL,
			     F77_CONST_CHAR_ARG_DECL,
			     const octave_idx_type&, const octave_idx_type&, float*,
			     const octave_idx_type&, float*, float*,
			     const octave_idx_type&, float*, const octave_idx_type&,
			     float*, const octave_idx_type&, octave_idx_type&
			     F77_CHAR_ARG_LEN_DECL
			     F77_CHAR_ARG_LEN_DECL);
}

FloatMatrix
FloatSVD::left_singular_matrix (void) const
{
  if (type_computed == SVD::sigma_only)
    {
      (*current_liboctave_error_handler)
	("FloatSVD: U not computed because type == SVD::sigma_only");
      return FloatMatrix ();
    }
  else
    return left_sm;
}

FloatMatrix
FloatSVD::right_singular_matrix (void) const
{
  if (type_computed == SVD::sigma_only)
    {
      (*current_liboctave_error_handler)
	("FloatSVD: V not computed because type == SVD::sigma_only");
      return FloatMatrix ();
    }
  else
    return right_sm;
}

octave_idx_type
FloatSVD::init (const FloatMatrix& a, SVD::type svd_type)
{
  octave_idx_type info;

  octave_idx_type m = a.rows ();
  octave_idx_type n = a.cols ();

  FloatMatrix atmp = a;
  float *tmp_data = atmp.fortran_vec ();

  octave_idx_type min_mn = m < n ? m : n;

  char jobu = 'A';
  char jobv = 'A';

  octave_idx_type ncol_u = m;
  octave_idx_type nrow_vt = n;
  octave_idx_type nrow_s = m;
  octave_idx_type ncol_s = n;

  switch (svd_type)
    {
    case SVD::economy:
      jobu = jobv = 'S';
      ncol_u = nrow_vt = nrow_s = ncol_s = min_mn;
      break;

    case SVD::sigma_only:

      // Note:  for this case, both jobu and jobv should be 'N', but
      // there seems to be a bug in dgesvd from Lapack V2.0.  To
      // demonstrate the bug, set both jobu and jobv to 'N' and find
      // the singular values of [eye(3), eye(3)].  The result is
      // [-sqrt(2), -sqrt(2), -sqrt(2)].
      //
      // For Lapack 3.0, this problem seems to be fixed.

      jobu = 'N';
      jobv = 'N';
      ncol_u = nrow_vt = 1;
      break;

    default:
      break;
    }

  type_computed = svd_type;

  if (! (jobu == 'N' || jobu == 'O'))
    left_sm.resize (m, ncol_u);

  float *u = left_sm.fortran_vec ();

  sigma.resize (nrow_s, ncol_s);
  float *s_vec  = sigma.fortran_vec ();

  if (! (jobv == 'N' || jobv == 'O'))
    right_sm.resize (nrow_vt, n);

  float *vt = right_sm.fortran_vec ();

  // Ask DGESVD what the dimension of WORK should be.

  octave_idx_type lwork = -1;

  Array<float> work (1);

  F77_XFCN (sgesvd, SGESVD, (F77_CONST_CHAR_ARG2 (&jobu, 1),
			     F77_CONST_CHAR_ARG2 (&jobv, 1),
			     m, n, tmp_data, m, s_vec, u, m, vt,
			     nrow_vt, work.fortran_vec (), lwork, info
			     F77_CHAR_ARG_LEN (1)
			     F77_CHAR_ARG_LEN (1)));

  lwork = static_cast<octave_idx_type> (work(0));
  work.resize (lwork);

  F77_XFCN (sgesvd, SGESVD, (F77_CONST_CHAR_ARG2 (&jobu, 1),
			     F77_CONST_CHAR_ARG2 (&jobv, 1),
			     m, n, tmp_data, m, s_vec, u, m, vt,
			     nrow_vt, work.fortran_vec (), lwork, info
			     F77_CHAR_ARG_LEN (1)
			     F77_CHAR_ARG_LEN (1)));

  if (! (jobv == 'N' || jobv == 'O'))
    right_sm = right_sm.transpose ();

  return info;
}

std::ostream&
operator << (std::ostream& os, const FloatSVD& a)
{
  os << a.left_singular_matrix () << "\n";
  os << a.singular_values () << "\n";
  os << a.right_singular_matrix () << "\n";

  return os;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
