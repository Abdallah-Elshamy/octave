## Copyright (C) 1996, 1997 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, write to the Free
## Software Foundation, 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {} prepad (@var{x}, @var{l}, @var{c})
## @deftypefnx {Function File} {} postpad (@var{x}, @var{l}, @var{c})
##
## Prepends (appends) the scalar value @var{c} to the vector @var{x}
## until it is of length @var{l}.  If the third argument is not
## supplied, a value of 0 is used.
##
## If @code{length (@var{x}) > @var{l}}, elements from the beginning (end) of
## @var{x} are removed until a vector of length @var{l} is obtained.
##
## If @var{x} is a matrix, elements are prepended or removed from each row.
## @end deftypefn

## Author: Tony Richardson <arichard@stark.cc.oh.us>
## Created: June 1994

function y = prepad (x, l, c, dim)

  if (nargin < 2 || nargin > 4)
    usage ("prepad (x, l, [c, [dim]])");
  endif

  if (nargin < 3 || isempty (c))
    c = 0;
  else
    if (! isscalar (c))
      error ("prepad: third argument must be empty or a scalar");
    endif
  endif

  nd = ndims (x);
  sz = size (x);
  if (nargin < 4)
    %% Find the first non-singleton dimension
    dim  = 1;
    while (dim < nd + 1 && sz (dim) == 1)
      dim = dim + 1;
    endwhile
    if (dim > nd)
      dim = 1;
    endif
  else
    if (! (isscalar (dim) && dim == round (dim)) && dim > 0 && 
	dim < (nd + 1))
      error ("prepad: dim must be an integer and valid dimension");
    endif
  endif

  if (! ismatrix (x))
    error ("first argument must be a vector or matrix");
  elseif (! isscalar (l) || l < 0)
    error ("second argument must be a positive scaler");
  endif

  d = sz (dim);

  if (d >= l)
    idx = cell ();
    for i = 1:nd
      idx {i} = 1:sz(i);
    endfor
    idx {dim} = d-l+1:d;
    y = x (idx {:});
  else
    sz (dim) = l - d;
    y = cat (dim, c * ones (sz), x);
  endif

endfunction
