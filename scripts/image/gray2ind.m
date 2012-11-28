## Copyright (C) 1994-2012 John W. Eaton
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
## @deftypefn  {Function File} {@var{img} =} gray2ind (@var{I})
## @deftypefnx {Function File} {@var{img} =} gray2ind (@var{I}, @var{n})
## @deftypefnx {Function File} {@var{img} =} gray2ind (@var{BW})
## @deftypefnx {Function File} {@var{img} =} gray2ind (@var{BW}, @var{n})
## @deftypefnx {Function File} {[@var{img}, @var{map}] =} gray2ind (@dots{})
## Convert a grayscale or binary intensity image to an indexed image.
## 
## The indexed image will consist of @var{n} different intensity values.
## If not given @var{n} defaults to 64 for grayscale images or 2 for
## binary black and white images.
##
## The output @var{img} is of class uint8 if @var{n} is less than or equal to
## 256.  Otherwise, the output is of class uint16.
## @seealso{ind2gray, rgb2ind} 
## @end deftypefn

## Author: Tony Richardson <arichard@stark.cc.oh.us>
## Created: July 1994
## Adapted-By: jwe

function [img, map] = gray2ind (I, n = 64)

  if (nargin < 1 || nargin > 2)
    print_usage ();
  elseif (! isreal (I) || issparse (I))
    error ("gray2ind: I must be a grayscale or binary image");
  elseif (! isscalar (n) || n < 1 || n > 65536)
    error ("gray2ind: N must be a positive integer in the range [1, 65536]");
  endif
  cls = class (I);
  if (! any (isa (I, {"logical", "uint8", "uint16", "int16", "single", "double"})))
    error ("gray2ind: invalid data type '%s'", cls);
  elseif (isfloat (I) && (any (I(:) < 0) || any (I(:) > 1)))
    error ("gray2ind: floating point images may only contain values between 0 and 1");
  endif

  ## Create grayscale colormap
  if (nargin == 1 && islogical (I))
    n = 2;
  endif
  map = gray (n);

  ## Set up scale factor
  if (isinteger (I))
    low = intmin (cls);
    if (low != 0)
      I -= low;  # shift to zero-based indexing
    endif
    scale = double (intmax (cls)) - double (low);
  else
    scale = 1;  # floating point doesn't need scaling
  endif

  ## Scale image
  img = I * ((n-1)/scale);

  ## Round and convert to appropriate output type.
  ## Note: no separate call to round () necessary because
  ##       type conversion does that automatically.
  if (n <= 256)
    img = uint8 (img) + 1;
  else
    img = uint16 (img) + 1;
  endif

endfunction


%!assert (gray2ind ([0 0.25 0.5 1]), uint8 ([1 17 33 64]))
%!assert (gray2ind ([0 0.25 0.5 1], 400), uint16 ([1 101 201 400]))
%!assert (gray2ind (logical ([1 0 0 1])), uint8 ([2 1 1 2]))
%!assert (gray2ind (uint8 ([0 64 128 192 255])), uint8 ([1 17 33 48 64]))

%!test
%! i2g = ind2gray (1:100, gray (100));
%! g2i = gray2ind (i2g, 100);
%! assert (g2i, uint8 (1:100));

%% Test input validation
%!error gray2ind ()
%!error gray2ind (1,2,3)
%!error <I must be a grayscale or binary image> gray2ind ({1})
%!error <I must be a grayscale or binary image> gray2ind ([1+i])
%!error <I must be a grayscale or binary image> gray2ind (sparse ([1]))
%!error <N must be a positive integer> gray2ind (1, ones (2,2))
%!error <N must be a positive integer> gray2ind (1, 0)
%!error <N must be a positive integer> gray2ind (1, 65537)
%!error <invalid data type> gray2ind (uint32 (1))
%!error <values between 0 and 1> gray2ind (-1)
%!error <values between 0 and 1> gray2ind (2)

