# Copyright (C) 1993, 1994, 1995 John W. Eaton
# 
# This file is part of Octave.
# 
# Octave is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
# 
# Octave is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Octave; see the file COPYING.  If not, write to the Free
# Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

function y = trace (x)

# usage: trace (x)
#
# Returns the trace (the sum of the diagonal elements) of x.

  if (nargin != 1)
    usage ("trace (x)");
  endif

  [nr, nc] = size (x);
  if (nr == 1 || nc == 1)
    y = x(1);
  else
    y = sum (diag (x));
  endif

endfunction
