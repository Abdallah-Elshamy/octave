## Copyright (C) 1995, 1996, 1997  Kurt Hornik
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
## Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {} betainv (@var{x}, @var{a}, @var{b})
## For each component of @var{x}, compute the quantile (the inverse of
## the CDF) at @var{x} of the Beta distribution with parameters @var{a}
## and @var{b}.
## @end deftypefn

## Author: KH <Kurt.Hornik@wu-wien.ac.at>
## Description: Quantile function of the Beta distribution

function inv = betainv (x, a, b)

  if (nargin != 3)
    usage ("betainv (x, a, b)");
  endif

  if (!isscalar (a) || !isscalar(b))
    [retval, x, a, b] = common_size (x, a, b);
    if (retval > 0)
      error ("betainv: x, a and b must be of common size or scalars");
    endif
  endif
  
  sz = size (x);
  inv = zeros (sz);

  k = find ((x < 0) | (x > 1) | !(a > 0) | !(b > 0) | isnan (x));
  if (any (k))
    inv (k) = NaN;
  endif

  k = find ((x == 1) & (a > 0) & (b > 0));
  if (any (k))
    inv (k) = 1;
  endif

  k = find ((x > 0) & (x < 1) & (a > 0) & (b > 0));
  if (any (k))
    if (!isscalar(a) || !isscalar(b))
      a = a (k);
      b = b (k);
      y = a ./ (a + b);
    else
      y = a / (a + b) * ones (size (k));
    endif
    x = x (k);
    l = find (y < eps);
    if (any (l))
      y(l) = sqrt (eps) * ones (length (l), 1);
    endif
    l = find (y > 1 - eps);
    if (any (l))
      y(l) = 1 - sqrt (eps) * ones (length (l), 1);
    endif

    y_old = y;
    for i = 1 : 10000
      h     = (betacdf (y_old, a, b) - x) ./ betapdf (y_old, a, b);
      y_new = y_old - h;
      ind   = find (y_new <= eps);
      if (any (ind))
        y_new (ind) = y_old (ind) / 10;
      endif
      ind = find (y_new >= 1 - eps);
      if (any (ind))
        y_new (ind) = 1 - (1 - y_old (ind)) / 10;
      endif
      h = y_old - y_new;
      if (max (abs (h)) < sqrt (eps))
        break;
      endif
      y_old = y_new;
    endfor

    inv (k) = y_new;
  endif

endfunction
