## Copyright (C) 1995, 1996, 1997  Kurt Hornik
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this file.  If not, write to the Free Software Foundation,
## 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

## usage:  gamma_rnd (a, b [, r, c])
##
## gamma_rnd (a, b) returns a matrix of random samples from the Gamma
## distribution with parameters a and b.  The size of the matrix is the
## common size of a and b.
##
## gamma_rnd (a, b, r, c) returns an r by c matrix of random samples
## from the Gamma distribution with parameters a and b.  Both a and b
## must be scalar or of size r by c.

## Author:  KH <Kurt.Hornik@ci.tuwien.ac.at>
## Description:  Random deviates from the Gamma distribution

function rnd = gamma_rnd (a, b, r, c)

  if (nargin == 4)
    if ( !(is_scalar (r) && (r > 0) && (r == round (r))) )
      error ("gamma_rnd:  r must be a positive integer");
    endif
    if ( !(is_scalar (c) && (c > 0) && (c == round (c))) )
      error ("gamma_rnd:  c must be a positive integer");
    endif
    [retval, a, b] = common_size (a, b, zeros (r, c));
    if (retval > 0)
      error (strcat("gamma_rnd:  ",
                    "a and b must be scalar or of size ",
                    sprintf ("%d by %d", r, c)));
    endif
  elseif (nargin == 2)
    [retval, a, b] = common_size (a, b);
    if (retval > 0)
      error (["gamma_rnd:  ", ...
              "a and b must be of common size or scalar"]);
    endif
  else
    usage ("gamma_rnd (a, b [, r, c])");
  endif

  [r, c] = size (a);
  s = r * c;
  a = reshape (a, 1, s);
  b = reshape (b, 1, s);
  rnd = zeros (1, s);

  k = find (!(a > 0) | !(a < Inf) | !(b > 0) | !(b < Inf));
  if (any (k))
    rnd(k) = NaN * ones (1, length (k));
  endif
  k = find ((a > 0) & (a < Inf) & (b > 0) & (b < Inf));
  if (any (k))
    rnd(k) = gamma_inv (rand (1, length (k)), a(k), b(k));
  endif

  rnd = reshape (rnd, r, c);

endfunction