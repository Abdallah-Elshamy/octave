## Copyright (C) 2013 Julien Bect
## Copyright (C) 1996-2012 John W. Eaton
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
## @deftypefn  {Function File} {} skewness (@var{x})
## @deftypefnx {Function File} {} skewness (@var{x}, @var{flag})
## @deftypefnx {Function File} {} skewness (@var{x}, @var{flag}, @var{dim})
## Compute the sample skewness of the elements of @var{x}:
## @tex
## $$
## {\rm skewness} (@var{x}) = {{{1\over N}\,
##          \sum_{i=1}^N (@var{x}_i - \bar{@var{x}})^3} \over \sigma^3},
## $$
## where $N$ is the length of @var{x}, $\bar{@var{x}}$ its mean and $\sigma$
## its (uncorrected) standard deviation.
## @end tex
## @ifnottex
##
## @example
##                mean ((@var{x} - mean (@var{x})).^3)
## skewness (@var{X}) = ------------------------.
##                     std (@var{x}, 1).^3
## @end example
##
## @end ifnottex
##
## @noindent
## The optional argument @var{flag} controls which normalization is used.
## If @var{flag} is equal to 1 (default value, used when @var{flag} is omitted
## or empty), return the sample skewness as defined above. If @var{flag} is
## equal to 0, return the adjusted skewness coefficient instead:
## @tex
## $$
## {\rm skewness} (@var{x}) = {{{N \over (N - 1) (N - 2)}\,
##          \sum_{i=1}^N (@var{x}_i - \bar{@var{x}})^3} \over @var{s}^3},
## $$
## where @var{s} is the corrected standard deviation of @var{x}.
## @end tex
## @ifnottex
##
## @example
##                         N^2        mean ((@var{x} - mean (@var{x})).^3)
## skewness (@var{X}, 0) = -------------- * ------------------------.
##                   (N - 1)(N - 2)        std (@var{x}, 0).^3
## @end example
##
## @end ifnottex
## The adjusted skewness coefficient is obtained by replacing the sample second
## and third central moments by their bias-corrected versions.
##
## If @var{x} is a matrix, or more generally a multidimensional array, return
## the skewness along the first non-singleton dimension.  If the optional
## @var{dim} argument is given, operate along this dimension.
##
## @seealso{var, kurtosis, moment}
## @end deftypefn

## Author: KH <Kurt.Hornik@wu-wien.ac.at>
## Created: 29 July 1994
## Adapted-By: jwe

function y = skewness (x, flag, dim)

  if (nargin < 1) || (nargin > 3)
    print_usage ();
  endif

  if (! isnumeric (x)) || islogical (x)
    error ("skewness: X must be a numeric vector or matrix");
  endif

  if (nargin < 2) || isempty (flag)
    flag = 1;  # default: do not use the "bias corrected" version
  else
    flag = double (flag);
    if (~ isequal (flag, 0)) && (~ isequal (flag, 1))
      error ("skewness: FLAG must be 0 or 1");
    end
  endif

  nd = ndims (x);
  sz = size (x);
  if nargin < 3
    ## Find the first non-singleton dimension.
    (dim = find (sz > 1, 1)) || (dim = 1);
  else
    if (!(isscalar (dim) && dim == fix (dim)) || !(1 <= dim && dim <= nd))
      error ("skewness: DIM must be an integer and a valid dimension");
    endif
  endif

  n = sz(dim);
  sz(dim) = 1;

  ## In the following sequence of computations, if x is of class single,
  ## then the result y is also of class single
  x = center (x, dim);
  s = std (x, flag, dim);  # use the biased estimator of the variance
  y = sum (x .^ 3, dim);
  y = y ./ (n * s .^ 3);
  y(s <= 0) = nan;

  ## Apply bias correction to the third central sample moment
  if flag == 0
    if n > 2
      y = y * (n * n ) / ((n - 1) * (n - 2));
    else
      y(:) = nan;
    end
  endif

endfunction


%!assert (skewness ([-1, 0, 1]), 0)
%!assert (skewness ([-2, 0, 1]) < 0)
%!assert (skewness ([-1, 0, 2]) > 0)
%!assert (skewness ([-3, 0, 1]) == -1 * skewness ([-1, 0, 3]))
%!assert (skewness (ones (3, 5)), nan (1, 5))

%!test
%! x = [0; 0; 0; 1];
%! y = [x, 2*x];
%! assert (skewness (y),  1.154700538379251 * [1 1], 1e-13)

%!assert (skewness ([1:5 10; 1:5 10],  0, 2), 1.439590274527954 * [1; 1], 1e-15)
%!assert (skewness ([1:5 10; 1:5 10],  1, 2), 1.051328089232020 * [1; 1], 1e-15)
%!assert (skewness ([1:5 10; 1:5 10], [], 2), 1.051328089232020 * [1; 1], 1e-15)

## Test behaviour on single input
%! assert (skewness (single ([1:5 10])), single (1.0513283))
%! assert (skewness (single ([1 2]), 0), single (nan))

## Test input validation
%!error skewness ()
%!error skewness (1, 2, 3)
%!error skewness (['A'; 'B'])
%!error skewness (1, ones (2,2))
%!error skewness (1, 1.5)
%!error skewness (1, [], 0)
%!error skewness (1, 3)

