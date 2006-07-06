/*

Copyright (C) 1996, 1997 John W. Eaton

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
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#if !defined (octave_oct_map_h)
#define octave_oct_map_h 1

#include <map>

#include "Cell.h"
#include "oct-obj.h"

class string_vector;

class
Octave_map
{
 public:

  typedef std::map<std::string, Cell>::iterator iterator;
  typedef std::map<std::string, Cell>::const_iterator const_iterator;

  typedef std::list<std::string>::iterator key_list_iterator;
  typedef std::list<std::string>::const_iterator const_key_list_iterator;

  // Warning!  You should always use at least two dimensions.

  Octave_map (const dim_vector& dv = dim_vector (0, 0),
	      const string_vector& key_list_arg = string_vector ());

  Octave_map (const std::string& k, const octave_value& value)
    : map (), key_list (), dimensions (1, 1)
  {
    map[k] = value;
    key_list.push_back (k);
  }

  Octave_map (const std::string& k, const Cell& vals)
    : map (), key_list (), dimensions (vals.dims ())
  {
    map[k] = vals;
    key_list.push_back (k);
  }

  Octave_map (const std::string& k, const octave_value_list& val_list)
    : map (), key_list (), dimensions (1, val_list.length ())
  {
    map[k] = val_list;
    key_list.push_back (k);
  }

  Octave_map (const Octave_map& m)
    : map (m.map), key_list (m.key_list), dimensions (m.dimensions) { }

  Octave_map& operator = (const Octave_map& m)
    {
      if (this != &m)
	{
	  map = m.map;
	  key_list = m.key_list;
	  dimensions = m.dimensions;
	}

      return *this;
    }

  ~Octave_map (void) { }

  // This is the number of keys.
  octave_idx_type length (void) const { return map.size (); }

  int empty (void) const { return map.empty (); }

  void del (const std::string& k)
    {
      iterator p = map.find (k);
      if (p != map.end ())
	map.erase (p);

      key_list_iterator q = find (key_list.begin (), key_list.end (), k);
      if (q != key_list.end ())
	key_list.erase (q);
    }

  iterator begin (void) { return iterator (map.begin ()); }
  const_iterator begin (void) const { return const_iterator (map.begin ()); }

  iterator end (void) { return iterator (map.end ()); }
  const_iterator end (void) const { return const_iterator (map.end ()); }

  std::string key (const_iterator p) const { return p->first; }

  Cell& contents (const std::string& k);
  Cell contents (const std::string& k) const;

  Cell& contents (const_iterator p)
    { return contents (key(p)); }

  Cell contents (const_iterator p) const
    { return contents (key(p)); }

  int intfield (const std::string& k, int def_val = 0) const;

  std::string stringfield (const std::string& k,
			   const std::string& def_val = std::string ()) const;

  iterator seek (const std::string& k) { return map.find (k); }
  const_iterator seek (const std::string& k) const { return map.find (k); }

  bool contains (const std::string& k) const
    { return (seek (k) != map.end ()); }

  void clear (void) { map.clear (); }

  string_vector keys (void) const;

  octave_idx_type rows (void) const { return dimensions(0); }

  octave_idx_type columns (void) const { return dimensions(1); }

  dim_vector dims (void) const { return dimensions; }

  int ndims (void) const { return dimensions.length (); }

  Octave_map transpose (void) const;

  Octave_map reshape (const dim_vector& new_dims) const;

  void resize (const dim_vector& dv, bool fill = false);

  octave_idx_type numel (void) const;

  Octave_map concat (const Octave_map& rb, const Array<octave_idx_type>& ra_idx);

  Octave_map& maybe_delete_elements (const octave_value_list& idx);

  Octave_map& assign (const octave_value_list& idx, const Octave_map& rhs);

  Octave_map& assign (const octave_value_list& idx, const std::string& k,
		      const Cell& rhs);

  Octave_map& assign (const std::string& k, const octave_value& rhs);

  Octave_map& assign (const std::string& k, const Cell& rhs);

  Octave_map index (const octave_value_list& idx);

private:

  // The map of names to values.
  std::map<std::string, Cell> map;

  // An extra list of keys, so we can keep track of the order the keys
  // are added for compatibility with you know what.
  std::list<std::string> key_list;

  // The current size.
  mutable dim_vector dimensions;

  void maybe_add_to_key_list (const std::string& k)
  {
    key_list_iterator p = find (key_list.begin (), key_list.end (), k);

    if (p == key_list.end ())
      key_list.push_back (k);
  }
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
