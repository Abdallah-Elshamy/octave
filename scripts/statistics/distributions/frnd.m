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
## @deftypefn {Function File} {} frnd (@var{m}, @var{n}, @var{r}, @var{c})
## @deftypefnx {Function File} {} frnd (@var{m}, @var{n}, @var{sz})
## Return an @var{r} by @var{c} matrix of random samples from the F
## distribution with @var{m} and @var{n} degrees of freedom.  Both
## @var{m} and @var{n} must be scalar or of size @var{r} by @var{c}.
## If @var{sz} is a vector the random samples are in a matrix of 
## size @var{sz}.
##
## If @var{r} and @var{c} are omitted, the size of the result matrix is
## the common size of @var{m} and @var{n}.
## @end deftypefn

## Author: KH <Kurt.Hornik@wu-wien.ac.at>
## Description: Random deviates from the F distribution

function rnd = frnd (m, n, r, c)

  if (nargin > 1)
    if (!isscalar(m) || !isscalar(n)) 
      [retval, m, n] = common_size (m, n);
      if (retval > 0)
	error ("frnd: m and n must be of common size or scalar");
      endif
    endif
  endif


  if (nargin == 4)
    if (! (isscalar (r) && (r > 0) && (r == round (r))))
      error ("frnd: r must be a positive integer");
    endif
    if (! (isscalar (c) && (c > 0) && (c == round (c))))
      error ("frnd: c must be a positive integer");
    endif
    sz = [r, c];

    if (any (size (m) != 1) && 
	((length (size (m)) != length (sz)) || any (size (m) != sz)))
      error ("frnd: m and n must be scalar or of size [r,c]");
    endif
  elseif (nargin == 3)
    if (isscalar (r) && (r > 0))
      sz = [r, r];
    elseif (isvector(r) && all (r > 0))
      sz = r(:)';
    else
      error ("frnd: r must be a postive integer or vector");
    endif

    if (any (size (m) != 1) && 
	((length (size (m)) != length (sz)) || any (size (m) != sz)))
      error ("frnd: m and n must be scalar or of size sz");
    endif
  elseif (nargin == 2)
    sz = size(a);
  else
    usage ("frnd (m, n, r, c)");
  endif


  if (isscalar (m) && isscalar (n))
    if ((m > 0) && (m < Inf) && (n > 0) && (n < Inf))
      rnd =  f_inv (rand (sz), m, n);
    else
      rnd = NaN * ones (sz);
    endif
  else
    rnd = zeros (sz);

    k = find (!(m > 0) | !(m < Inf) |
              !(n > 0) | !(n < Inf));
    if (any (k))
      rnd(k) = NaN;
    endif

    k = find ((m > 0) & (m < Inf) &
              (n > 0) & (n < Inf));
    if (any (k))
      rnd(k) = finv (rand (size (k)), m(k), n(k));
    endif
  endif

endfunction
