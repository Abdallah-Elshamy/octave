// oct-hist.cc                                        -*- C++ -*-
/*

Copyright (C) 1992, 1993, 1994, 1995 John W. Eaton

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

The functions listed below were adapted from similar functions from
GNU Bash, the Bourne Again SHell, copyright (C) 1987, 1989, 1991 Free
Software Foundation, Inc.

  do_history         edit_history_readline
  do_edit_history    edit_history_add_hist

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <csignal>
#include <cstdlib>
#include <cstring>

#include <string>

#include <fstream.h>
#include <strstream.h>

#ifdef HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#endif

#include <fcntl.h>

#include <readline/history.h>

#include "str-vec.h"

#include "defun.h"
#include "error.h"
#include "file-ops.h"
#include "input.h"
#include "oct-hist.h"
#include "oct-obj.h"
#include "pager.h"
#include "sighandlers.h"
#include "sysdep.h"
#include "toplev.h"
#include "unwind-prot.h"
#include "user-prefs.h"
#include "utils.h"

// Nonzero means input is coming from temporary history file.
int input_from_tmp_history_file = 0;

// The number of hisory lines we read from the history file.
static int history_lines_in_file = 0;

// The number of history lines we've saved so far.
static int history_lines_this_session = 0;

// Get some default values, possibly reading them from the
// environment.

int
default_history_size (void)
{
  int size = 1024;
  char *env_size = getenv ("OCTAVE_HISTSIZE");
  if (env_size)
    {
      int val;
      if (sscanf (env_size, "%d", &val) == 1)
	size = val > 0 ? val : 0;
    }
  return size;
}

string
default_history_file (void)
{
  string file;

  char *env_file = getenv ("OCTAVE_HISTFILE");

  if (env_file)
    {
      fstream f (env_file, (ios::in | ios::out));

      if (f)
	{
	  file = env_file;
	  f.close ();
	}
    }

  if (file.empty ())
    {
      if (! home_directory.empty ())
	file = home_directory.append ("/.octave_hist");
      else
	file = ".octave_hist";
    }

  return file;
}

// Prime the history list.

void
initialize_history (void)
{
  string file = oct_tilde_expand (user_pref.history_file);

  read_history (file.c_str ());

  using_history ();

  history_lines_in_file = where_history ();
}

void
clean_up_history (void)
{
  stifle_history (user_pref.history_size);

  string file = oct_tilde_expand (user_pref.history_file);

  if (user_pref.saving_history)
    write_history (file.c_str ());
}

void
maybe_save_history (const string& s)
{
  if (user_pref.saving_history && ! input_from_startup_file)
    {
      add_history (s.c_str ());
      history_lines_this_session++;
    }
}

// Display, save, or load history.  Stolen and modified from bash.
//
// Arg of -w FILENAME means write file, arg of -r FILENAME
// means read file, arg of -q means don't number lines.  Arg of N
// means only display that many items. 

static void
do_history (int argc, const string_vector& argv)
{
  HIST_ENTRY **hlist;

  int numbered_output = 1;

  int i;
  for (i = 1; i < argc; i++)
    {
      if (argv[i][0] == '-' && argv[i].length () == 2
	  && (argv[i][1] == 'r' || argv[i][1] == 'w'
	      || argv[i][1] == 'a' || argv[i][1] == 'n'))
	{
	  int result = 0;

	  string file;

	  if (i < argc - 1)
	    file = oct_tilde_expand (argv[i+1]);
	  else
	    file = oct_tilde_expand (user_pref.history_file);

	  switch (argv[i][1])
	    {
	    case 'a':		// Append `new' lines to file.
	      {
		if (history_lines_this_session)
		  {
		    if (history_lines_this_session < where_history ())
		      {
			// Create file if it doesn't already exist.

			file_stat fs (file);

			if (! fs)
			  {
			    int tem;

			    tem = open (file.c_str (), O_CREAT, 0666);
			    close (tem);
			  }

			result
			  = append_history (history_lines_this_session,
					    file.c_str ());

			history_lines_in_file += history_lines_this_session;
			history_lines_this_session = 0;
		      }
		  }
	      }
	      break;

	    case 'w':		// Write entire history.
	      result = write_history (file.c_str ());
	      break;

	    case 'r':		// Read entire file.
	      result = read_history (file.c_str ());
	      break;

	    case 'n':		// Read `new' history from file.

	      // Read all of the lines in the file that we haven't
	      // already read.

	      using_history ();
	      result = read_history_range (file.c_str (),
					   history_lines_in_file, -1);
	      using_history ();
	      history_lines_in_file = where_history ();
	      break;
	    }
	  return;
	}
      else if (argv[i] == "-q")
	numbered_output = 0;
      else if (argv[i] == "--")
	{
	  i++;
	  break;
	}
      else
	break;
    }

  int limited = 0;
  int limit = 0;

  if (i < argc)
    {
      limited = 1;
      if (sscanf (argv[i].c_str (), "%d", &limit) != 1)
        {
	  if (argv[i][0] == '-')
	    error ("history: unrecognized option `%s'", argv[i].c_str ());
	  else
	    error ("history: bad non-numeric arg `%s'", argv[i].c_str ());
	  return;
        }
    }

  hlist = history_list ();

  if (hlist)
    {
      int i = 0;

      for (i = 0; hlist[i]; i++)
	; // Do nothing.

      if (limit < 0)
	limit = -limit;

      if (!limited)
	i = 0;
      else
	if ((i -= limit) < 0)
	  i = 0;

      ostrstream output_buf;

      while (hlist[i])
	{
//	  QUIT;  // in bash: (interrupt_state) throw_to_top_level ();

	  if (numbered_output)
	    output_buf.form ("%5d%c", i + history_base,
			     hlist[i]->data ? '*' : ' '); 
	  output_buf << hlist[i]->line << "\n";
	  i++;
	}

      output_buf << ends;
      maybe_page_output (output_buf);
    }
}

// Read the edited history lines from STREAM and return them
// one at a time.  This can read unlimited length lines.  The
//  caller should free the storage.

static char *
edit_history_readline (fstream& stream)
{
  char c;
  int line_len = 128;
  int lindex = 0;
  char *line = new char [line_len];
  line[0] = '\0';

  while (stream.get (c))
    {
      if (lindex + 2 >= line_len)
	{
	  char *tmp_line = new char [line_len += 128];
	  strcpy (tmp_line, line);
	  delete [] line;
	  line = tmp_line;
	}

      if (c == '\n')
	{
	  line[lindex++] = '\n';
	  line[lindex++] = '\0';
	  return line;
	}
      else
	line[lindex++] = c;
    }

  if (! lindex)
    {
      delete [] line;
      return 0;
    }

  if (lindex + 2 >= line_len)
    {
      char *tmp_line = new char [lindex+3];
      strcpy (tmp_line, line);
      delete [] line;
      line = tmp_line;
    }

  // Finish with newline if none in file.

  line[lindex++] = '\n';
  line[lindex++] = '\0';
  return line;
}

// Use `command' to replace the last entry in the history list, which,
// by this time, is `run_history blah...'.  The intent is that the
// new command become the history entry, and that `fc' should never
// appear in the history list.  This way you can do `run_history' to
// your heart's content.

static void
edit_history_repl_hist (char *command)
{
  if (! command || ! *command)
    return;

  HIST_ENTRY **hlist = history_list ();

  if (! hlist)
    return;

  int i = 0;

  for (i = 0; hlist[i]; i++)
    ; // Count 'em.
  i--;

  // History_get () takes a parameter that should be offset by history_base.

  // Don't free this.
  HIST_ENTRY *histent = history_get (history_base + i);
  if (! histent)
    return;

  char *data = 0;
  if (histent->data)
    {
      int len = strlen (histent->data);
      data = (char *) malloc (len);
      strcpy (data, histent->data);
    }

  int n = strlen (command);

  if (command[n - 1] == '\n')
    command[n - 1] = '\0';

  if (command && *command)
    {
      HIST_ENTRY *discard = replace_history_entry (i, command, data);
      if (discard)
	{
	  if (discard->line)
	    free (discard->line);

	  free ((char *) discard);
	}
    }
}

static void
edit_history_add_hist (char *line)
{
  if (line)
    {
      int len = strlen (line);
      if (len > 0 && line[len-1] == '\n')
	line[len-1] = '\0';

      if (line[0] != '\0')
	add_history (line);
    }
}

#define histline(i) (hlist[(i)]->line)

static string
mk_tmp_hist_file (int argc, const string_vector& argv,
		  int insert_curr, char *warn_for) 
{
  HIST_ENTRY **hlist;

  hlist = history_list ();

  int hist_count = 0;

  while (hlist[hist_count++])
    ; // Find the number of items in the history list.

  // The current command line is already part of the history list by
  // the time we get to this point.  Delete it from the list.

  hist_count -= 2;
  if (! insert_curr)
    remove_history (hist_count);
  hist_count--;

  // If no numbers have been specified, the default is to edit the
  // last command in the history list.

  int hist_end = hist_count;
  int hist_beg = hist_count;
  int reverse = 0;

  // Process options.

  int usage_error = 0;
  if (argc == 3)
    {
      if (sscanf (argv[1].c_str (), "%d", &hist_beg) != 1
	  || sscanf (argv[2].c_str (), "%d", &hist_end) != 1)
	usage_error = 1;
      else
	{
	  hist_beg--;
	  hist_end--;
	}
    }
  else if (argc == 2)
    {
      if (sscanf (argv[1].c_str (), "%d", &hist_beg) != 1)
	usage_error = 1;
      else
	{
	  hist_beg--;
	  hist_end = hist_beg;
	}
    }

  if (hist_beg < 0 || hist_end < 0 || hist_beg > hist_count
      || hist_end > hist_count)
    {
      error ("%s: history specification out of range", warn_for);
      return 0;
    }

  if (usage_error)
    {
      usage ("%s [first] [last]", warn_for);
      return 0;
    }

  if (hist_end < hist_beg)
    {
      int t = hist_end;
      hist_end = hist_beg;
      hist_beg = t;
      reverse = 1;
    }

  string name = octave_tmp_file_name ();

  fstream file (name.c_str (), ios::out);

  if (! file)
    {
      error ("%s: couldn't open temporary file `%s'", warn_for,
	     name.c_str ());
      return 0;
    }

  if (reverse)
    {
      for (int i = hist_end; i >= hist_beg; i--)
	file << histline (i) << "\n";
    }
  else
    {
      for (int i = hist_beg; i <= hist_end; i++)
	file << histline (i) << "\n";
    }

  file.close ();

  return name;
}

static void
do_edit_history (int argc, const string_vector& argv)
{
  string name = mk_tmp_hist_file (argc, argv, 0, "edit_history");

  if (name.empty ())
    return;

  // Call up our favorite editor on the file of commands.

  ostrstream buf;
  buf << user_pref.editor << " " << name << ends;
  char *cmd = buf.str ();

  // Ignore interrupts while we are off editing commands.  Should we
  // maybe avoid using system()?

  volatile sig_handler *saved_sigint_handler
    = octave_set_signal_handler (SIGINT, SIG_IGN);

  system (cmd);

  octave_set_signal_handler (SIGINT, saved_sigint_handler);

  // Write the commands to the history file since parse_and_execute
  // disables command line history while it executes.

  fstream file (name.c_str (), ios::in);

  char *line;
  int first = 1;
  while ((line = edit_history_readline (file)) != 0)
    {
      // Skip blank lines.

      if (line[0] == '\n')
	{
	  delete [] line;
	  continue;
	}

      if (first)
	{
	  first = 0;
	  edit_history_repl_hist (line);
	}
      else
	edit_history_add_hist (line);
    }

  file.close ();

  // Turn on command echo, so the output from this will make better
  // sense.

  begin_unwind_frame ("do_edit_history");
  unwind_protect_int (user_pref.echo_executing_commands);
  unwind_protect_int (input_from_tmp_history_file);
  user_pref.echo_executing_commands = ECHO_CMD_LINE;
  input_from_tmp_history_file = 1;

  parse_and_execute (name, 1);

  run_unwind_frame ("do_edit_history");

  // Delete the temporary file.  Should probably be done with an
  // unwind_protect.

  unlink (name.c_str ());
}

static void
do_run_history (int argc, const string_vector& argv)
{
  string name = mk_tmp_hist_file (argc, argv, 1, "run_history");

  if (name.empty ())
    return;

  // Turn on command echo, so the output from this will make better
  // sense.

  begin_unwind_frame ("do_run_history");
  unwind_protect_int (user_pref.echo_executing_commands);
  unwind_protect_int (input_from_tmp_history_file);
  user_pref.echo_executing_commands = ECHO_CMD_LINE;
  input_from_tmp_history_file = 1;

  parse_and_execute (name, 1);

  run_unwind_frame ("do_run_history");

  // Delete the temporary file.  Should probably be done with an
  // unwind_protect.

  unlink (name.c_str ());
}

int
current_history_number (void)
{
  using_history ();

  if (user_pref.history_size > 0)
    return history_base + where_history ();
  else
    return -1;

}

DEFUN_TEXT ("edit_history", Fedit_history, Sedit_history, 10,
  "edit_history [first] [last]\n\
\n\
edit commands from the history list")
{
  Octave_object retval;

  int argc = args.length () + 1;

  string_vector argv = make_argv (args, "edit_history");

  if (error_state)
    return retval;

  do_edit_history (argc, argv);

  return retval;
}

DEFUN_TEXT ("history", Fhistory, Shistory, 10,
  "history [N] [-w file] [-r file] [-q]\n\
\n\
display, save, or load command history")
{
  Octave_object retval;

  int argc = args.length () + 1;

  string_vector argv = make_argv (args, "history");

  if (error_state)
    return retval;

  do_history (argc, argv);

  return retval;
}

DEFUN_TEXT ("run_history", Frun_history, Srun_history, 10,
  "run_history [first] [last]\n\
\n\
run commands from the history list")
{
  Octave_object retval;

  int argc = args.length () + 1;

  string_vector argv = make_argv (args, "run_history");

  if (error_state)
    return retval;

  do_run_history (argc, argv);

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
