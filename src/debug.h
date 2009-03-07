/*

Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Ben Sapp

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

#if !defined (octave_debug_h)
#define octave_debug_h 1

#include <map>
#include "ov.h"
#include "dRowVector.h"

class octave_value_list;
class octave_user_code;

// Interface to breakpoints,.

class bp_table
{
private:

  bp_table (void) { }

  ~bp_table (void) { }

public:

  typedef std::map<int, int> intmap;

  typedef intmap::const_iterator const_intmap_iterator;
  typedef intmap::iterator intmap_iterator;

  typedef std::map <std::string, intmap> fname_line_map;

  typedef fname_line_map::const_iterator const_fname_line_map_iterator;
  typedef fname_line_map::iterator fname_line_map_iterator;

  static bool instance_ok (void)
  {
    bool retval = true;

    if (! instance)
      instance = new bp_table ();

    if (! instance)
      {
        ::error ("unable to create breakpoint table!");
        retval = false;
      }
    
    return retval;
  }

  // Add a breakpoint at the nearest executable line.
  static intmap add_breakpoint (const std::string& fname = "", 
				const intmap& lines = intmap ())
  {
    return instance_ok ()
      ? instance->do_add_breakpoint (fname, lines) : intmap ();
  }

  // Remove a breakpoint from a line in file.
  static int remove_breakpoint (const std::string& fname = "", 
				const intmap& lines = intmap ())
  {
    return instance_ok ()
      ? instance->do_remove_breakpoint (fname, lines) : 0;
  }

  // Remove all the breakpoints in a specified file.
  static intmap remove_all_breakpoints_in_file (const std::string& fname,
						bool silent = false)
  {
    return instance_ok ()
      ? instance->do_remove_all_breakpoints_in_file (fname, silent) : intmap ();
  }
  
  // Remove all the breakpoints registered with octave.
  static void remove_all_breakpoints (void)
  {
    if (instance_ok ())
      instance->do_remove_all_breakpoints ();
  }
  
  // Return all breakpoints.  Each element of the map is a vector
  // containing the breakpoints corresponding to a given function name.
  static fname_line_map
  get_breakpoint_list (const octave_value_list& fname_list)
  {
    return instance_ok ()
      ? instance->do_get_breakpoint_list (fname_list) : fname_line_map ();
  }

  static bool
  have_breakpoints (void)
  {
    return instance_ok () ? instance->do_have_breakpoints () : 0;
  }

private:

  // Map from function names to function objects for functions
  // containing at least one breakpoint.
  typedef std::map<std::string, octave_user_code *> breakpoint_map;

  typedef breakpoint_map::const_iterator const_breakpoint_map_iterator;
  typedef breakpoint_map::iterator breakpoint_map_iterator;

  breakpoint_map bp_map;

  static bp_table *instance;

  intmap do_add_breakpoint (const std::string& fname, const intmap& lines);

  int do_remove_breakpoint (const std::string&, const intmap& lines);

  intmap do_remove_all_breakpoints_in_file (const std::string& fname, 
					    bool silent);

  void do_remove_all_breakpoints (void);

  fname_line_map do_get_breakpoint_list (const octave_value_list& fname_list);

  bool do_have_breakpoints (void) { return (! bp_map.empty ()); }
};

std::string get_file_line (const std::string& fname, size_t line);

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
