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
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#if !defined (octave_LU_h)
#define octave_LU_h 1

#include "base-lu.h"
#include "dMatrix.h"

class
OCTAVE_API
LU : public base_lu <Matrix, double, Matrix, double>
{
public:

  LU (void) : base_lu <Matrix, double, Matrix, double> () { }

  LU (const Matrix& a);

  LU (const LU& a) : base_lu <Matrix, double, Matrix, double> (a) { }

  LU& operator = (const LU& a)
    {
      if (this != &a)
	base_lu <Matrix, double, Matrix, double> :: operator = (a);

      return *this;
    }

  ~LU (void) { }
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
