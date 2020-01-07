/*

Copyright (C) 1996-2020 The Octave Project Developers

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

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <sstream>

#include "file-info.h"
#include "file-stat.h"
#include "str-vec.h"

#include "builtin-defun-decls.h"
#include "defaults.h"
#include "Cell.h"
#include "defun.h"
#include "error.h"
#include "errwarn.h"
#include "input.h"
#include "ovl.h"
#include "ov-usr-fcn.h"
#include "ov.h"
#include "pager.h"
#include "pt-eval.h"
#include "pt-jit.h"
#include "pt-jump.h"
#include "pt-misc.h"
#include "pt-pr-code.h"
#include "pt-stmt.h"
#include "pt-walk.h"
#include "symtab.h"
#include "interpreter-private.h"
#include "interpreter.h"
#include "unwind-prot.h"
#include "utils.h"
#include "parse.h"
#include "profiler.h"
#include "variables.h"
#include "ov-fcn-handle.h"

// Whether to optimize subsasgn method calls.
static bool Voptimize_subsasgn_calls = true;

octave_user_code::~octave_user_code (void)
{
  // This function is no longer valid, so remove the pointer to it from
  // the corresponding scope.
  // FIXME: would it be better to use shared/weak pointers for this job
  // instead of storing a bare pointer in the scope object?
  m_scope.set_user_code (nullptr);

  // FIXME: shouldn't this happen automatically when deleting cmd_list?
  if (cmd_list)
    {
      octave::event_manager& evmgr
        = octave::__get_event_manager__ ("octave_user_code::~octave_user_code");

      cmd_list->remove_all_breakpoints (evmgr, file_name);
    }

  delete cmd_list;
  delete m_file_info;
}

void
octave_user_code::get_file_info (void)
{
  m_file_info = new octave::file_info (file_name);

  octave::sys::file_stat fs (file_name);

  if (fs && (fs.mtime () > time_parsed ()))
    warning ("function file '%s' changed since it was parsed",
             file_name.c_str ());
}

std::string
octave_user_code::get_code_line (size_t line)
{
  if (! m_file_info)
    get_file_info ();

  return m_file_info->get_line (line);
}

std::deque<std::string>
octave_user_code::get_code_lines (size_t line, size_t num_lines)
{
  if (! m_file_info)
    get_file_info ();

  return m_file_info->get_lines (line, num_lines);
}

void
octave_user_code::cache_function_text (const std::string& text,
                                       const octave::sys::time& timestamp)
{
  if (m_file_info)
    delete m_file_info;

  if (timestamp > time_parsed ())
    warning ("help text for function is newer than function");

  m_file_info = new octave::file_info (text, timestamp);
}

std::map<std::string, octave_value>
octave_user_code::subfunctions (void) const
{
  return std::map<std::string, octave_value> ();
}

octave_value
octave_user_code::dump (void) const
{
  std::map<std::string, octave_value> m
    = {{ "scope_info", m_scope ? m_scope.dump () : "0x0" },
       { "file_name", file_name },
       { "time_parsed", t_parsed },
       { "time_checked", t_checked }};

  return octave_value (m);
}


// User defined scripts.

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_user_script,
                                     "user-defined script",
                                     "user-defined script");

octave_user_script::octave_user_script (void)
  : octave_user_code ()
{ }

octave_user_script::octave_user_script
  (const std::string& fnm, const std::string& nm,
   const octave::symbol_scope& scope, octave::tree_statement_list *cmds,
   const std::string& ds)
  : octave_user_code (fnm, nm, scope, cmds, ds)
{
  if (cmd_list)
    cmd_list->mark_as_script_body ();
}

octave_user_script::octave_user_script
  (const std::string& fnm, const std::string& nm,
   const octave::symbol_scope& scope, const std::string& ds)
    : octave_user_code (fnm, nm, scope, nullptr, ds)
{ }

octave_value_list
octave_user_script::call (octave::tree_evaluator& tw, int nargout,
                          const octave_value_list& args)
{
  return tw.execute_user_script (*this, nargout, args);
}

void
octave_user_script::accept (octave::tree_walker& tw)
{
  tw.visit_octave_user_script (*this);
}

// User defined functions.

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_user_function,
                                     "user-defined function",
                                     "user-defined function");

// Ugh.  This really needs to be simplified (code/data?
// extrinsic/intrinsic state?).

octave_user_function::octave_user_function
  (const octave::symbol_scope& scope, octave::tree_parameter_list *pl,
   octave::tree_parameter_list *rl, octave::tree_statement_list *cl,
   const local_vars_map& lviv)
  : octave_user_code ("", "", scope, cl, ""),
    param_list (pl), ret_list (rl),
    m_local_var_init_vals (lviv),
    lead_comm (), trail_comm (),
    location_line (0), location_column (0),
    parent_name (), system_fcn_file (false),
    num_named_args (param_list ? param_list->length () : 0),
    subfunction (false), inline_function (false),
    anonymous_function (false), nested_function (false),
    class_constructor (none), class_method (none)
#if defined (HAVE_LLVM)
    , jit_info (0)
#endif
{
  if (cmd_list)
    cmd_list->mark_as_function_body ();
}

octave_user_function::~octave_user_function (void)
{
  delete param_list;
  delete ret_list;
  delete lead_comm;
  delete trail_comm;

#if defined (HAVE_LLVM)
  delete jit_info;
#endif
}

octave_user_function *
octave_user_function::define_ret_list (octave::tree_parameter_list *t)
{
  ret_list = t;

  return this;
}

// If there is no explicit end statement at the end of the function,
// relocate the no_op that was generated for the end of file condition
// to appear on the next line after the last statement in the file, or
// the next line after the function keyword if there are no statements.
// More precisely, the new location should probably be on the next line
// after the end of the parameter list, but we aren't tracking that
// information (yet).

void
octave_user_function::maybe_relocate_end_internal (void)
{
  if (cmd_list && ! cmd_list->empty ())
    {
      octave::tree_statement *last_stmt = cmd_list->back ();

      if (last_stmt && last_stmt->is_end_of_fcn_or_script ()
          && last_stmt->is_end_of_file ())
        {
          octave::tree_statement_list::reverse_iterator
            next_to_last_elt = cmd_list->rbegin ();

          next_to_last_elt++;

          int new_eof_line;
          int new_eof_col;

          if (next_to_last_elt == cmd_list->rend ())
            {
              new_eof_line = beginning_line ();
              new_eof_col = beginning_column ();
            }
          else
            {
              octave::tree_statement *next_to_last_stmt = *next_to_last_elt;

              new_eof_line = next_to_last_stmt->line ();
              new_eof_col = next_to_last_stmt->column ();
            }

          last_stmt->set_location (new_eof_line + 1, new_eof_col);
        }
    }
}

void
octave_user_function::maybe_relocate_end (void)
{
  std::map<std::string, octave_value> fcns = subfunctions ();

  if (! fcns.empty ())
    {
      for (auto& nm_fnval : fcns)
        {
          octave_user_function *f = nm_fnval.second.user_function_value ();

          if (f)
            f->maybe_relocate_end_internal ();
        }
    }

  maybe_relocate_end_internal ();
}

void
octave_user_function::stash_parent_fcn_scope (const octave::symbol_scope& ps)
{
  m_scope.set_parent (ps);
}

std::string
octave_user_function::profiler_name (void) const
{
  std::ostringstream result;

  if (is_anonymous_function ())
    result << "anonymous@" << fcn_file_name ()
           << ':' << location_line << ':' << location_column;
  else if (is_subfunction ())
    result << parent_fcn_name () << '>' << name ();
  else if (is_class_method ())
    result << '@' << dispatch_class () << '/' << name ();
  else if (is_class_constructor () || is_classdef_constructor ())
    result << '@' << name ();
  else if (is_inline_function ())
    result << "inline@" << fcn_file_name ()
           << ':' << location_line << ':' << location_column;
  else
    result << name ();

  return result.str ();
}

void
octave_user_function::mark_as_system_fcn_file (void)
{
  if (! file_name.empty ())
    {
      // We really should stash the whole path to the file we found,
      // when we looked it up, to avoid possible race conditions...
      // FIXME
      //
      // We probably also don't need to get the library directory
      // every time, but since this function is only called when the
      // function file is parsed, it probably doesn't matter that
      // much.

      std::string ff_name = octave::fcn_file_in_path (file_name);

      std::string fcn_file_dir = octave::config::fcn_file_dir ();
      if (fcn_file_dir == ff_name.substr (0, fcn_file_dir.length ()))
        system_fcn_file = true;
    }
  else
    system_fcn_file = false;
}

void
octave_user_function::erase_subfunctions (void)
{
  m_scope.erase_subfunctions ();
}

bool
octave_user_function::takes_varargs (void) const
{
  return (param_list && param_list->takes_varargs ());
}

bool
octave_user_function::takes_var_return (void) const
{
  return (ret_list && ret_list->takes_varargs ());
}

void
octave_user_function::mark_as_private_function (const std::string& cname)
{
  m_scope.mark_subfunctions_in_scope_as_private (cname);

  octave_function::mark_as_private_function (cname);
}

void
octave_user_function::lock_subfunctions (void)
{
  m_scope.lock_subfunctions ();
}

void
octave_user_function::unlock_subfunctions (void)
{
  m_scope.unlock_subfunctions ();
}

std::map<std::string, octave_value>
octave_user_function::subfunctions (void) const
{
  return m_scope.subfunctions ();
}

// Find definition of final subfunction in list of subfuns:
//
//  sub1>sub2>...>subN

octave_value
octave_user_function::find_subfunction (const std::string& subfuns_arg) const
{
  std::string subfuns = subfuns_arg;

  std::string first_fun = subfuns;

  size_t pos = subfuns.find ('>');

  if (pos == std::string::npos)
    subfuns = "";
  else
    {
      first_fun = subfuns.substr (0, pos-1);
      subfuns = subfuns.substr (pos+1);
    }

  octave_value ov_fcn = m_scope.find_subfunction (first_fun);

  if (subfuns.empty ())
    return ov_fcn;

  octave_user_function *fcn = ov_fcn.user_function_value ();

  return fcn->find_subfunction (subfuns);
}

bool
octave_user_function::has_subfunctions (void) const
{
  return m_scope.has_subfunctions ();
}

void
octave_user_function::stash_subfunction_names (const std::list<std::string>& names)
{
  m_scope.stash_subfunction_names (names);
}

std::list<std::string>
octave_user_function::subfunction_names (void) const
{
  return m_scope.subfunction_names ();
}

octave_value_list
octave_user_function::all_va_args (const octave_value_list& args)
{
  octave_value_list retval;

  octave_idx_type n = args.length () - num_named_args;

  if (n > 0)
    retval = args.slice (num_named_args, n);

  return retval;
}

octave_value_list
octave_user_function::call (octave::tree_evaluator& tw, int nargout,
                            const octave_value_list& args,
                            octave::stack_frame *closure_frames)
{
  return tw.execute_user_function (*this, nargout, args, closure_frames);
}

void
octave_user_function::accept (octave::tree_walker& tw)
{
  tw.visit_octave_user_function (*this);
}

octave::tree_expression *
octave_user_function::special_expr (void)
{
  assert (is_special_expr ());
  assert (cmd_list->length () == 1);

  octave::tree_statement *stmt = cmd_list->front ();
  return stmt->expression ();
}

bool
octave_user_function::subsasgn_optimization_ok (void)
{
  bool retval = false;
  if (Voptimize_subsasgn_calls
      && param_list && ret_list
      && param_list->length () > 0 && ! param_list->varargs_only ()
      && ret_list->length () == 1 && ! ret_list->takes_varargs ())
    {
      octave::tree_identifier *par1 = param_list->front ()->ident ();
      octave::tree_identifier *ret1 = ret_list->front ()->ident ();
      retval = par1->name () == ret1->name ();
    }

  return retval;
}

std::string
octave_user_function::ctor_type_str (void) const
{
  std::string retval;

  switch (class_constructor)
    {
    case none:
      retval = "none";
      break;

    case legacy:
      retval = "legacy";
      break;

    case classdef:
      retval = "classdef";
      break;

    default:
      retval = "unrecognized enum value";
      break;
    }

  return retval;
}

std::string
octave_user_function::method_type_str (void) const
{
  std::string retval;

  switch (class_method)
    {
    case none:
      retval = "none";
      break;

    case legacy:
      retval = "legacy";
      break;

    case classdef:
      retval = "classdef";
      break;

    default:
      retval = "unrecognized enum value";
      break;
    }

  return retval;
}

octave_value
octave_user_function::dump (void) const
{
  std::map<std::string, octave_value> m
    = {{ "user_code", octave_user_code::dump () },
       { "line", location_line },
       { "col", location_column },
       { "end_line", end_location_line },
       { "end_col", end_location_column },
       { "parent_name", parent_name },
       { "system_fcn_file", system_fcn_file },
       { "num_named_args", num_named_args },
       { "subfunction", subfunction },
       { "inline_function", inline_function },
       { "anonymous_function", anonymous_function },
       { "nested_function", nested_function },
       { "ctor_type", ctor_type_str () },
       { "class_method", class_method }};

  return octave_value (m);
}

void
octave_user_function::print_code_function_header (const std::string& prefix)
{
  octave::tree_print_code tpc (octave_stdout, prefix);

  tpc.visit_octave_user_function_header (*this);
}

void
octave_user_function::print_code_function_trailer (const std::string& prefix)
{
  octave::tree_print_code tpc (octave_stdout, prefix);

  tpc.visit_octave_user_function_trailer (*this);
}

void
octave_user_function::restore_warning_states (void)
{
  octave::interpreter& interp
    = octave::__get_interpreter__ ("octave_user_function::restore_warning_states");

  octave::tree_evaluator& tw = interp.get_evaluator ();

  octave_value val
    = tw.get_auto_fcn_var (octave::stack_frame::SAVED_WARNING_STATES);

  if (val.is_defined ())
    {
      // Fail spectacularly if SAVED_WARNING_STATES is not an
      // octave_map (or octave_scalar_map) object.

      if (! val.isstruct ())
        panic_impossible ();

      octave_map m = val.map_value ();

      Cell ids = m.contents ("identifier");
      Cell states = m.contents ("state");

      for (octave_idx_type i = 0; i < m.numel (); i++)
        Fwarning (interp, ovl (states(i), ids(i)));
    }
}

DEFMETHOD (nargin, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn  {} {} nargin ()
@deftypefnx {} {} nargin (@var{fcn})
Report the number of input arguments to a function.

Called from within a function, return the number of arguments passed to the
function.  At the top level, return the number of command line arguments
passed to Octave.

If called with the optional argument @var{fcn}---a function name or
handle---return the declared number of arguments that the function can
accept.

If the last argument to @var{fcn} is @var{varargin} the returned value is
negative.  For example, the function @code{union} for sets is declared as

@example
@group
function [y, ia, ib] = union (a, b, varargin)

and

nargin ("union")
@result{} -3
@end group
@end example

Programming Note: @code{nargin} does not work on compiled functions
(@file{.oct} files) such as built-in or dynamically loaded functions.
@seealso{nargout, narginchk, varargin, inputname}
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin > 1)
    print_usage ();

  octave_value retval;

  if (nargin == 1)
    {
      octave_value func = args(0);

      if (func.is_string ())
        {
          octave::symbol_table& symtab = interp.get_symbol_table ();

          std::string name = func.string_value ();
          func = symtab.find_function (name);
          if (func.is_undefined ())
            error ("nargin: invalid function name: %s", name.c_str ());
        }

      octave_function *fcn_val = func.function_value (true);
      if (! fcn_val)
        error ("nargin: FCN must be a string or function handle");

      octave_user_function *fcn = fcn_val->user_function_value (true);

      if (! fcn)
        {
          // Matlab gives up for histc, so maybe it's ok that we
          // give up sometimes too?

          std::string type = fcn_val->type_name ();
          error ("nargin: number of input arguments unavailable for %s objects",
                 type.c_str ());
        }

      octave::tree_parameter_list *param_list = fcn->parameter_list ();

      retval = (param_list ? param_list->length () : 0);
      if (fcn->takes_varargs ())
        retval = -1 - retval;
    }
  else
    {
      octave::tree_evaluator& tw = interp.get_evaluator ();

      retval = tw.get_auto_fcn_var (octave::stack_frame::NARGIN);

      if (retval.is_undefined ())
        retval = 0;
    }

  return retval;
}

DEFMETHOD (nargout, interp,args, ,
           doc: /* -*- texinfo -*-
@deftypefn  {} {} nargout ()
@deftypefnx {} {} nargout (@var{fcn})
Report the number of output arguments from a function.

Called from within a function, return the number of values the caller
expects to receive.  At the top level, @code{nargout} with no argument is
undefined and will produce an error.

If called with the optional argument @var{fcn}---a function name or
handle---return the number of declared output values that the function can
produce.

If the final output argument is @var{varargout} the returned value is
negative.

For example,

@example
f ()
@end example

@noindent
will cause @code{nargout} to return 0 inside the function @code{f} and

@example
[s, t] = f ()
@end example

@noindent
will cause @code{nargout} to return 2 inside the function @code{f}.

In the second usage,

@example
nargout (@@histc)   # or nargout ("histc") using a string input
@end example

@noindent
will return 2, because @code{histc} has two outputs, whereas

@example
nargout (@@imread)
@end example

@noindent
will return -2, because @code{imread} has two outputs and the second is
@var{varargout}.

Programming Note.  @code{nargout} does not work for built-in functions and
returns -1 for all anonymous functions.
@seealso{nargin, varargout, isargout, nthargout}
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin > 1)
    print_usage ();

  octave_value retval;

  if (nargin == 1)
    {
      octave_value func = args(0);

      if (func.is_string ())
        {
          octave::symbol_table& symtab = interp.get_symbol_table ();

          std::string name = func.string_value ();
          func = symtab.find_function (name);
          if (func.is_undefined ())
            error ("nargout: invalid function name: %s", name.c_str ());
        }

      if (func.is_inline_function ())
        return ovl (1);

      if (func.is_function_handle ())
        {
          octave_fcn_handle *fh = func.fcn_handle_value ();
          std::string fh_nm = fh->fcn_name ();

          if (fh_nm == octave_fcn_handle::anonymous)
            return ovl (-1);
        }

      octave_function *fcn_val = func.function_value (true);
      if (! fcn_val)
        error ("nargout: FCN must be a string or function handle");

      octave_user_function *fcn = fcn_val->user_function_value (true);

      if (! fcn)
        {
          // Matlab gives up for histc, so maybe it's ok that we
          // give up sometimes too?

          std::string type = fcn_val->type_name ();
          error ("nargout: number of output arguments unavailable for %s objects",
                 type.c_str ());
        }

      octave::tree_parameter_list *ret_list = fcn->return_list ();

      retval = (ret_list ? ret_list->length () : 0);

      if (fcn->takes_var_return ())
        retval = -1 - retval;
    }
  else
    {
      if (interp.at_top_level ())
        error ("nargout: invalid call at top level");

      octave::tree_evaluator& tw = interp.get_evaluator ();

      retval = tw.get_auto_fcn_var (octave::stack_frame::NARGOUT);

      if (retval.is_undefined ())
        retval = 0;
    }

  return retval;
}

DEFUN (optimize_subsasgn_calls, args, nargout,
       doc: /* -*- texinfo -*-
@deftypefn  {} {@var{val} =} optimize_subsasgn_calls ()
@deftypefnx {} {@var{old_val} =} optimize_subsasgn_calls (@var{new_val})
@deftypefnx {} {} optimize_subsasgn_calls (@var{new_val}, "local")
Query or set the internal flag for @code{subsasgn} method call
optimizations.

If true, Octave will attempt to eliminate the redundant copying when calling
the @code{subsasgn} method of a user-defined class.

When called from inside a function with the @qcode{"local"} option, the
variable is changed locally for the function and any subroutines it calls.
The original variable value is restored when exiting the function.
@seealso{subsasgn}
@end deftypefn */)
{
  return SET_INTERNAL_VARIABLE (optimize_subsasgn_calls);
}

static bool val_in_table (const Matrix& table, double val)
{
  if (table.isempty ())
    return false;

  octave_idx_type i = table.lookup (val, ASCENDING);
  return (i > 0 && table(i-1) == val);
}

static bool isargout1 (int nargout, const Matrix& ignored, double k)
{
  if (k != octave::math::fix (k) || k <= 0)
    error ("isargout: K must be a positive integer");

  return (k == 1 || k <= nargout) && ! val_in_table (ignored, k);
}

DEFMETHOD (isargout, interp, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {} isargout (@var{k})
Within a function, return a logical value indicating whether the argument
@var{k} will be assigned to a variable on output.

If the result is false, the argument has been ignored during the function
call through the use of the tilde (~) special output argument.  Functions
can use @code{isargout} to avoid performing unnecessary calculations for
outputs which are unwanted.

If @var{k} is outside the range @code{1:max (nargout)}, the function returns
false.  @var{k} can also be an array, in which case the function works
element-by-element and a logical array is returned.  At the top level,
@code{isargout} returns an error.
@seealso{nargout, varargout, nthargout}
@end deftypefn */)
{
  if (args.length () != 1)
    print_usage ();

  if (interp.at_top_level ())
    error ("isargout: invalid call at top level");

  octave::tree_evaluator& tw = interp.get_evaluator ();

  octave_value tmp;

  int nargout1 = 0;
  tmp = tw.get_auto_fcn_var (octave::stack_frame::NARGOUT);
  if (tmp.is_defined ())
    nargout1 = tmp.int_value ();

  Matrix ignored;
  tmp = tw.get_auto_fcn_var (octave::stack_frame::IGNORED);
  if (tmp.is_defined ())
    ignored = tmp.matrix_value ();

  if (args(0).is_scalar_type ())
    {
      double k = args(0).double_value ();

      return ovl (isargout1 (nargout1, ignored, k));
    }
  else if (args(0).isnumeric ())
    {
      const NDArray ka = args(0).array_value ();

      boolNDArray r (ka.dims ());
      for (octave_idx_type i = 0; i < ka.numel (); i++)
        r(i) = isargout1 (nargout1, ignored, ka(i));

      return ovl (r);
    }
  else
    err_wrong_type_arg ("isargout", args(0));

  return ovl ();
}

/*
%!function [x, y] = try_isargout ()
%!  if (isargout (1))
%!    if (isargout (2))
%!      x = 1; y = 2;
%!    else
%!      x = -1;
%!    endif
%!  else
%!    if (isargout (2))
%!      y = -2;
%!    else
%!      error ("no outputs requested");
%!    endif
%!  endif
%!endfunction
%!
%!function [a, b] = try_isargout2 (x, y)
%!  a = y;
%!  b = {isargout(1), isargout(2), x};
%!endfunction
%!
%!test
%! [x, y] = try_isargout ();
%! assert ([x, y], [1, 2]);
%!
%!test
%! [x, ~] = try_isargout ();
%! assert (x, -1);
%!
%!test
%! [~, y] = try_isargout ();
%! assert (y, -2);
%!
%!error [~, ~] = try_isargout ()
%!
## Check to see that isargout isn't sticky:
%!test
%! [x, y] = try_isargout ();
%! assert ([x, y], [1, 2]);
%!
## It should work without ():
%!test
%! [~, y] = try_isargout;
%! assert (y, -2);
%!
## It should work in function handles, anonymous functions, and cell
## arrays of handles or anonymous functions.
%!test
%! fh = @try_isargout;
%! af = @() try_isargout;
%! c = {fh, af};
%! [~, y] = fh ();
%! assert (y, -2);
%! [~, y] = af ();
%! assert (y, -2);
%! [~, y] = c{1}();
%! assert (y, -2);
%! [~, y] = c{2}();
%! assert (y, -2);
%!
## Nesting, anyone?
%!test
%! [~, b] = try_isargout2 (try_isargout, rand);
%! assert (b, {0, 1, -1});
%!test
%! [~, b] = try_isargout2 ({try_isargout, try_isargout}, rand);
%! assert (b, {0, 1, {-1, -1}});
*/
