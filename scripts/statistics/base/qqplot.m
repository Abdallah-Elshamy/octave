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
## @deftypefn {Function File} {[@var{q}, @var{s}] =} qqplot (@var{x}, @var{dist}, @var{params})
## Perform a QQ-plot (quantile plot).
##
## If F is the CDF of the distribution @var{dist} with parameters
## @var{params} and G its inverse, and @var{x} a sample vector of length
## @var{n}, the QQ-plot graphs ordinate @var{s}(@var{i}) = @var{i}-th
## largest element of x versus abscissa @var{q}(@var{i}f) = G((@var{i} -
## 0.5)/@var{n}).
##
## If the sample comes from F except for a transformation of location
## and scale, the pairs will approximately follow a straight line.
##
## The default for @var{dist} is the standard normal distribution.  The
## optional argument @var{params} contains a list of parameters of
## @var{dist}.  For example, for a quantile plot of the uniform
## distribution on [2,4] and @var{x}, use
##
## @example
## qqplot (x, "uniform", 2, 4)
## @end example
##
## @noindent
## @var{dist} can be any string for which a function @var{dist_inv}
## that calculates the inverse CDF of distribution @var{dist} exists.
##
## If no output arguments are given, the data are plotted directly.
## @end deftypefn

## Author: KH <Kurt.Hornik@wu-wien.ac.at>
## Description: Perform a QQ-plot (quantile plot)

function [q, s] = qqplot (x, dist, varargin)

  if (nargin < 1)
    print_usage ();
  endif

  if (! (isvector(x)))
    error ("qqplot: x must be a vector");
  endif

  s = sort (x);
  n = length (x);
  t = ((1 : n)' - .5) / n;
  if (nargin == 1)
    f = @stdnormal_inv;
  else
    f = str2func (sprintf ("%s_inv", dist));
  endif;
  if (nargin <= 2)
    q = feval (f, t);
    q_label = func2str (f);
  else
    q = feval (f, t, varargin{:});
    if (nargin > 3)
      tmp = sprintf (", %g", varargin{2:end});
    else 
      tmp = "";
    endif
    q_label = sprintf ("%s with parameter(s) %g%s", func2str (f),
		       varargin{1}, tmp);
  endif

  if (nargout == 0)
    plot (q, s);
    xlabel (q_label);
    ylabel ("sample points");
  endif

endfunction
