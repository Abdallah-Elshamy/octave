// Map-tc.cc                                            -*- C++ -*-
/*

Copyright (C) 1993, 1994, 1995 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// Instantiate Maps of tree_constants.

#include "Map.h"
#include "Map.cc"

#include "pt-const.h"

template class Map<tree_constant>;
template class CHNode<tree_constant>;
template class CHMap<tree_constant>;

template static int goodCHptr (CHNode<tree_constant> *t);
template static int CHptr_to_index (CHNode<tree_constant> *t);

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
