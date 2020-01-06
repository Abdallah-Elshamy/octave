/*

Copyright (C) 2009-2019 The Octave Project Developers

See the file COPYRIGHT.md in the top-level directory of this distribution
or <https://octave.org/COPYRIGHT.html/>.


This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Octave is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

#if ! defined (octave_ls_ascii_helper_h)
#define octave_ls_ascii_helper_h 1

#include "octave-config.h"

#include <iosfwd>
#include <string>

extern OCTINTERP_API void
skip_until_newline (std::istream& is, bool keep_newline = false);

extern OCTINTERP_API void
skip_preceeding_newline (std::istream& is);

extern OCTINTERP_API std::string
read_until_newline (std::istream& is, bool keep_newline = false);

#endif
