# Copyright (C) 1995 John W. Eaton
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
# Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

function p = polyreduce (p)

# usage: polyreduce(c)
#
# Reduces a polynomial coefficient vector to a minimum number of terms,
# i.e. it strips off any leading zeros.
#
# SEE ALSO: poly, roots, conv, deconv, residue, filter, polyval, polyvalm,
#           polyderiv, polyinteg

# Written by Tony Richardson (amr@mpl.ucsd.edu) June 1994.

  index = find (p == 0);

  if (length (index) != 0)

    index = find (index == 1:length (index));

    if (length (index) != 0)

      if (length (p) > 1)
	p = p (index (length (index))+1:length (p));
      endif

      if (length (p) == 0)
	p = 0;
      endif

    endif

  endif

endfunction
