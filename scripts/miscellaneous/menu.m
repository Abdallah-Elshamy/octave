# Copyright (C) 1993 John W. Eaton
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

function s = menu (t, ...)

# usage: menu (title, opt1, ...)
#
# See also: disp, printf, input

  if (nargin < 2)
    error ("usage: menu (title, opt1, ...)");
  endif

# Force pending output to appear before the menu.

  fflush (stdout);

# Don't send the menu through the pager since doing that can cause
# major confusion.

  save_page_screen_output = page_screen_output;
  page_screen_output = "false";

  if (! isempty (t))
    disp (t);
    printf ("\n");
  endif

  nopt = nargin - 1;

  s = 0;
  while (1)
    page_screen_output = "false";
    va_start ();
    for i = 1:nopt
      printf ("  [%2d] ", i);
      disp (va_arg ());
    endfor
    printf ("\n");
    page_screen_output = save_page_screen_output;
    s = input ("pick a number, any number: ");
    if (s < 1 || s > nopt)
      printf ("\nerror: input out of range\n\n");
    else
      break;
    endif
  endwhile

  page_screen_output = save_page_screen_output;

endfunction
