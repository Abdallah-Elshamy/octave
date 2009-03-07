/*

Copyright (C) 2007, 2008, 2009 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if !defined (octave_class_h)
#define octave_class_h 1

#include <cstdlib>

#include <iostream>
#include <string>

#include "mx-base.h"
#include "str-vec.h"

#include "error.h"
#include "oct-alloc.h"
#include "oct-map.h"
#include "ov-base.h"
#include "ov-typeinfo.h"

class octave_value_list;

class tree_walker;

// Data structures.

class
octave_class : public octave_base_value
{
public:

  octave_class (void)
    : octave_base_value () { }

  octave_class (const Octave_map& m, const std::string& id)
    : octave_base_value (), map (m), c_name (id) { }

  octave_class (const octave_class& s)
    : octave_base_value (s), map (s.map), c_name (s.c_name) { }

  ~octave_class (void) { }

  octave_base_value *clone (void) const { return new octave_class (*this); }

  octave_base_value *empty_clone (void) const
  {
    return new octave_class (Octave_map (map.keys ()), class_name ());
  }

  Cell dotref (const octave_value_list& idx);

  octave_value subsref (const std::string& type,
			const std::list<octave_value_list>& idx)
    {
      octave_value_list tmp = subsref (type, idx, 1);
      return tmp.length () > 0 ? tmp(0) : octave_value ();
    }

  octave_value_list subsref (const std::string& type,
			     const std::list<octave_value_list>& idx,
			     int nargout);

  static octave_value numeric_conv (const Cell& val,
				    const std::string& type);

  octave_value subsasgn (const std::string& type,
			 const std::list<octave_value_list>& idx,
			 const octave_value& rhs);

  idx_vector index_vector (void) const;

  dim_vector dims (void) const { return map.dims (); }

  size_t byte_size (void) const;

  // This is the number of elements in each field.  The total number
  // of elements is numel () * nfields ().
  octave_idx_type numel (void) const
  {
    dim_vector dv = dims ();
    return dv.numel ();
  }

  octave_idx_type nfields (void) const { return map.nfields (); }

  octave_value reshape (const dim_vector& new_dims) const
    { return map.reshape (new_dims); }

  octave_value resize (const dim_vector& dv, bool = false) const
    { Octave_map tmap = map; tmap.resize (dv); return tmap; }

  bool is_defined (void) const { return true; }

  bool is_map (void) const { return false; }

  bool is_object (void) const { return true; }

  Octave_map map_value (void) const { return map; }

  string_vector map_keys (void) const;

  void print (std::ostream& os, bool pr_as_read_syntax = false) const;

  void print_raw (std::ostream& os, bool pr_as_read_syntax = false) const;

  bool print_name_tag (std::ostream& os, const std::string& name) const;

  void print_with_name (std::ostream& os, const std::string& name, 
			bool print_padding = true) const;

  bool save_ascii (std::ostream& os);

  bool load_ascii (std::istream& is);

  bool save_binary (std::ostream& os, bool& save_as_floats);

  bool load_binary (std::istream& is, bool swap, 
		    oct_mach_info::float_format fmt);

#if defined (HAVE_HDF5)
  bool save_hdf5 (hid_t loc_id, const char *name, bool save_as_floats);

  bool load_hdf5 (hid_t loc_id, const char *name, bool have_h5giterate_bug);
#endif

  mxArray *as_mxArray (void) const;

private:

  Octave_map map;

  DECLARE_OCTAVE_ALLOCATOR

public:
  int type_id (void) const { return t_id; }
  std::string type_name (void) const { return t_name; }
  std::string class_name (void) const { return c_name; }

  static int static_type_id (void) { return t_id; }
  static std::string static_type_name (void) { return t_name; }
  static std::string static_class_name (void) { return "<unknown>"; }
  static void register_type (void);

private:
  static int t_id;

  static const std::string t_name;
  std::string c_name;

  bool in_class_method (void) const;
};

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
