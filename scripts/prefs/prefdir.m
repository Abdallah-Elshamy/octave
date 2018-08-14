## Copyright (C) 2013-2018 John Donoghue
##
## This file is part of Octave.
##
## Octave is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <https://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn  {} {} prefdir
## @deftypefnx {} {} prefdir (1)
## @deftypefnx {} {@var{dir} =} prefdir
## Return the directory that holds the preferences for Octave.
##
## Examples:
##
## Display the preferences directory
##
## @example
## prefdir
## @end example
##
## Change to the preferences folder
##
## @example
## cd (prefdir)
## @end example
##
## If called with an argument, the preferences directory is created if it
## doesn't already exist.
## @seealso{getpref, setpref, addpref, rmpref, ispref}
## @end deftypefn

## Author: John Donoghue

function dir = prefdir ()

  dir = get_home_directory ();

  if (nargin > 0)
    if (! isfolder (dir))
      mkdir (dir);
    endif
  endif

endfunction
