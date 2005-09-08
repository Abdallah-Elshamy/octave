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
## Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {} bottom_title (@var{string})
## See top_title.
## @end deftypefn

## Author: Vinayak Dutt <Dutt.Vinayak@mayo.EDU>
## Adapted-By: jwe

function bottom_title (text)

  if (nargin != 1)
    usage ("bottom_title (text)");
  endif

  if (ischar (text))
    __gnuplot_raw__ ("set top_title;\n");
    __gnuplot_raw__ ("set title;\n");
    __gnuplot_raw__ (sprintf ("set bottom_title \"%s\";\n",
			      undo_string_escapes (text)));
  else
    error ("bottom_title: text must be a string");
  endif

endfunction
