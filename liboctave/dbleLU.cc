//                                        -*- C++ -*-
/*

Copyright (C) 1996 John W. Eaton

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

#include "dbleLU.h"
#include "f77-fcn.h"
#include "lo-error.h"
#include "mx-inlines.cc"

extern "C"
{
  int F77_FCN (dgesv, DGESV) (const int&, const int&, double*,
			      const int&, int*, double&, const int&,
			      int&);
}

LU::LU (const Matrix& a)
{
  int a_nr = a.rows ();
  int a_nc = a.cols ();

  if (a_nr == 0 || a_nc == 0 || a_nr != a_nc)
    {
      (*current_liboctave_error_handler) ("LU requires square matrix");
      return;
    }

  int n = a_nr;

  Array<int> ipvt (n);
  int *pipvt = ipvt.fortran_vec ();

  Matrix A_fact = a;
  double *tmp_data = A_fact.fortran_vec ();

  int info = 0;
  double dummy = 0;

  F77_XFCN (dgesv, DGESV, (n, 0, tmp_data, n, pipvt, dummy, n, info));

  if (f77_exception_encountered)
    (*current_liboctave_error_handler) ("unrecoverable error in dgesv");
  else
    {
      Array<int> pvt (n);

      for (int i = 0; i < n; i++)
	{
	  ipvt.elem (i) -= 1;
	  pvt.elem (i) = i;
	}

      for (int i = 0; i < n - 1; i++)
	{
	  int k = ipvt.elem (i);
	  if (k != i)
	    {
	      int tmp = pvt.elem (k);
	      pvt.elem (k) = pvt.elem (i);
	      pvt.elem (i) = tmp;
	    }
	}

      l.resize (n, n, 0.0);
      u.resize (n, n, 0.0);
      p.resize (n, n, 0.0);

      for (int i = 0; i < n; i++)
	{
	  p.elem (i, pvt.elem (i)) = 1.0;

	  l.elem (i, i) = 1.0;
	  for (int j = 0; j < i; j++)
	    l.elem (i, j) = A_fact.elem (i, j);

	  for (int j = i; j < n; j++)
	    u.elem (i, j) = A_fact.elem (i, j);
	}
    }
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
