## Copyright (C) 1996, 1997, 2007, 2008 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {} polyint (@var{c}, @var{k})
## Return the coefficients of the integral of the polynomial whose
## coefficients are represented by the vector @var{c}. The variable
## @var{k} is the constant of integration, which by default is set to zero.
## @seealso{poly, polyderiv, polyreduce, roots, conv, deconv, residue,
## filter, polyval, polyvalm}
## @end deftypefn

## Author: Tony Richardson <arichard@stark.cc.oh.us>
## Created: June 1994
## Adapted-By: jwe

function p = polyint (p, k)

  if (nargin < 1 || nargin > 2)
    print_usage ();
  endif

  if (nargin == 1)
    k = 0;
  elseif (! isscalar (k))
    error ("polyint: the constant of integration must be a scalar");
  endif

  if (! (isvector (p) || isempty (p)))
    error ("argument must be a vector");
  endif

  lp = length (p);

  if (lp == 0)
    p = [];
    return;
  endif

  if (rows (p) > 1)
    ## Convert to column vector
    p = p.';
  endif

  p = [(p ./ [lp:-1:1]), k];

endfunction
