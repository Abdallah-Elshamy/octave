/*

Copyright (C) 1996, 1997, 1998, 1999, 2000, 2002, 2003, 2005, 2006,
              2007 John W. Eaton

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>

#include <iomanip>
#include <iostream>
#include <sstream>

#include "dNDArray.h"
#include "NLEqn.h"

#include "defun-dld.h"
#include "error.h"
#include "gripes.h"
#include "oct-obj.h"
#include "ov-fcn.h"
#include "ov-cell.h"
#include "pager.h"
#include "unwind-prot.h"
#include "utils.h"
#include "variables.h"

#include "NLEqn-opts.cc"

// Global pointer for user defined function required by hybrd1.
static octave_function *fsolve_fcn;

// Global pointer for optional user defined jacobian function.
static octave_function *fsolve_jac;

// Original dimensions of X0.
static dim_vector x_dims;

// Have we warned about imaginary values returned from user function?
static bool warned_fcn_imaginary = false;
static bool warned_jac_imaginary = false;

// Is this a recursive call?
static int call_depth = 0;

octave_idx_type
hybrd_info_to_fsolve_info (octave_idx_type info)
{
  switch (info)
    {
    case -1:
      break;

    case 0:
      info = -2;
      break;

    case 1:
      break;

    case 2:
      info = 4;
      break;

    case 3:
    case 4:
    case 5:
      info = 3;
      break;

    default:
      {
	std::ostringstream buf;
	buf << "fsolve: unrecognized value of INFO from MINPACK (= "
	    << info << ")";
	std::string msg = buf.str ();
	warning (msg.c_str ());
      }
      break;
    }

  return info;
}

ColumnVector
fsolve_user_function (const ColumnVector& x)
{
  ColumnVector retval;

  octave_idx_type n = x.length ();

  octave_value_list args;
  args.resize (1);

  if (n > 1)
    {
      NDArray m (ArrayN<double> (x, x_dims));
      octave_value vars (m);
      args(0) = vars;
    }
  else
    {
      double d = x (0);
      octave_value vars (d);
      args(0) = vars;
    }

  if (fsolve_fcn)
    {
      octave_value_list tmp = fsolve_fcn->do_multi_index_op (1, args);

      if (tmp.length () > 0 && tmp(0).is_defined ())
	{
	  if (! warned_fcn_imaginary && tmp(0).is_complex_type ())
	    {
	      warning ("fsolve: ignoring imaginary part returned from user-supplied function");
	      warned_fcn_imaginary = true;
	    }

	  retval = ColumnVector (tmp(0).vector_value (false, true));

	  if (error_state || retval.length () <= 0)
	    gripe_user_supplied_eval ("fsolve");
	  else if (retval.length () != x.length ())
	    error ("fsolve: unable to solve non-square systems");
	}
      else
	gripe_user_supplied_eval ("fsolve");
    }

  return retval;
}

Matrix
fsolve_user_jacobian (const ColumnVector& x)
{
  Matrix retval;

  octave_idx_type n = x.length ();

  octave_value_list args;
  args.resize (1);

  if (n > 1)
    {
      NDArray m (ArrayN<double> (x, x_dims));
      octave_value vars (m);
      args(0) = vars;
    }
  else
    {
      double d = x(0);
      octave_value vars (d);
      args(0) = vars;
    }

  if (fsolve_fcn)
    {
      octave_value_list tmp = fsolve_jac->do_multi_index_op (1, args);

      if (tmp.length () > 0 && tmp(0).is_defined ())
	{
	  if (! warned_fcn_imaginary && tmp(0).is_complex_type ())
	    {
	      warning ("fsolve: ignoring imaginary part returned from user-supplied jacobian function");
	      warned_fcn_imaginary = true;
	    }

	  retval = tmp(0).matrix_value ();

	  if (error_state || retval.length () <= 0)
	    gripe_user_supplied_eval ("fsolve");
	  else if (! (retval.rows () == x.length ()
		      && retval.columns () == x.length ()))
	    error ("fsolve: invalid Jacobian matrix dimensions");
	}
      else
	gripe_user_supplied_eval ("fsolve");
    }

  return retval;
}

#define FSOLVE_ABORT() \
  do \
    { \
      unwind_protect::run_frame ("Ffsolve"); \
      return retval; \
    } \
  while (0)

#define FSOLVE_ABORT1(msg) \
  do \
    { \
      ::error ("fsolve: " msg); \
      FSOLVE_ABORT (); \
    } \
  while (0)

#define FSOLVE_ABORT2(fmt, arg) \
  do \
    { \
      ::error ("fsolve: " fmt, arg); \
      FSOLVE_ABORT (); \
    } \
  while (0)

DEFUN_DLD (fsolve, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {[@var{x}, @var{fval}, @var{info}] =} fsolve (@var{fcn}, @var{x0})\n\
Given @var{fcn}, the name of a function of the form @code{f (@var{x})}\n\
and an initial starting point @var{x0}, @code{fsolve} solves the set of\n\
equations such that @code{f(@var{x}) == 0}.\n\
\n\
On return, @var{fval} contains the value of the function @var{fcn}\n\
evaluated at @var{x}, and @var{info} may be one of the following values:\n\
\n\
@table @asis\n\
\n\
@item -2\n\
Invalid input parameters.\n\
@item -1\n\
Error in user-supplied function.\n\
@item 1\n\
Relative error between two consecutive iterates is at most the\n\
specified tolerance (see @code{fsolve_options}).\n\
@item 3\n\
Algorithm failed to converge.\n\
@item 4\n\
Limit on number of function calls reached.\n\
@end table\n\
\n\
If @var{fcn} is a two-element string array, or a two element cell array\n\
containing either the function name or inline or function handle. The\n\
first element names the function @math{f} described above, and the second\n\
element names a function of the form @code{j (@var{x})} to compute the\n\
Jacobian matrix with elements\n\
@tex\n\
$$ J = {\\partial f_i \\over \\partial x_j} $$\n\
@end tex\n\
@ifinfo\n\
\n\
@example\n\
           df_i\n\
jac(i,j) = ----\n\
           dx_j\n\
@end example\n\
@end ifinfo\n\
\n\
You can use the function @code{fsolve_options} to set optional\n\
parameters for @code{fsolve}.\n\
@end deftypefn")
{
  octave_value_list retval;

  warned_fcn_imaginary = false;
  warned_jac_imaginary = false;

  unwind_protect::begin_frame ("Ffsolve");

  unwind_protect_int (call_depth);
  call_depth++;

  if (call_depth > 1)
    FSOLVE_ABORT1 ("invalid recursive call");

  int nargin = args.length ();

  if (nargin == 2 && nargout < 4)
    {
      std::string fcn_name, fname, jac_name, jname;
      fsolve_fcn = 0;
      fsolve_jac = 0;

      octave_value f_arg = args(0);

      if (f_arg.is_cell ())
  	{
	  Cell c = f_arg.cell_value ();
	  if (c.length() == 1)
	    f_arg = c(0);
	  else if (c.length() == 2)
	    {
	      if (c(0).is_function_handle () || c(0).is_inline_function ())
		fsolve_fcn = c(0).function_value ();
	      else
		{
		  fcn_name = unique_symbol_name ("__fsolve_fcn__");
		  fname = "function y = ";
		  fname.append (fcn_name);
		  fname.append (" (x) y = ");
		  fsolve_fcn = extract_function
		    (c(0), "fsolve", fcn_name, fname, "; endfunction");
		}
	      
	      if (fsolve_fcn)
		{
		  if (c(1).is_function_handle () || c(1).is_inline_function ())
		    fsolve_jac = c(1).function_value ();
		  else
		    {
		      jac_name = unique_symbol_name ("__fsolve_jac__");
		      jname = "function y = ";
		      jname.append (jac_name);
		      jname.append (" (x) jac = ");
		      fsolve_jac = extract_function
			(c(1), "fsolve", jac_name, jname, "; endfunction");

		      if (!fsolve_jac)
			{
			  if (fcn_name.length())
			    clear_function (fcn_name);
			  fsolve_fcn = 0;
			}
		    }
		}
	    }
	  else
	    FSOLVE_ABORT1 ("incorrect number of elements in cell array");
	}

      if (!fsolve_fcn && ! f_arg.is_cell())
	{
	  if (f_arg.is_function_handle () || f_arg.is_inline_function ())
	    fsolve_fcn = f_arg.function_value ();
	  else
	    {
	      switch (f_arg.rows ())
		{
		case 1:
		  do
		    {
		      fcn_name = unique_symbol_name ("__fsolve_fcn__");
		      fname = "function y = ";
		      fname.append (fcn_name);
		      fname.append (" (x) y = ");
		      fsolve_fcn = extract_function
			(f_arg, "fsolve", fcn_name, fname, "; endfunction");
		    }
		  while (0);
		  break;

		case 2:
		  {
		    string_vector tmp = f_arg.all_strings ();

		    if (! error_state)
		      {
			fcn_name = unique_symbol_name ("__fsolve_fcn__");
			fname = "function y = ";
			fname.append (fcn_name);
			fname.append (" (x) y = ");
			fsolve_fcn = extract_function
			  (tmp(0), "fsolve", fcn_name, fname, "; endfunction");

			if (fsolve_fcn)
			  {
			    jac_name = unique_symbol_name ("__fsolve_jac__");
			    jname = "function y = ";
			    jname.append (jac_name);
			    jname.append (" (x) jac = ");
			    fsolve_jac = extract_function
			      (tmp(1), "fsolve", jac_name, jname, 
			       "; endfunction");

			    if (!fsolve_jac)
			      {
				if (fcn_name.length())
				  clear_function (fcn_name);
				fsolve_fcn = 0;
			      }
			  }
		      }
		  }
		}
	    }
	}

      if (error_state || ! fsolve_fcn)
	FSOLVE_ABORT ();

      NDArray xa = args(1).array_value ();
      x_dims = xa.dims ();
      ColumnVector x (xa);

      if (error_state)
	FSOLVE_ABORT1 ("expecting vector as second argument");

      if (nargin > 3)
	warning ("fsolve: ignoring extra arguments");

      if (nargout > 3)
	warning ("fsolve: can't compute path output yet");

      NLFunc nleqn_fcn (fsolve_user_function);
      if (fsolve_jac)
	nleqn_fcn.set_jacobian_function (fsolve_user_jacobian);

      NLEqn nleqn (x, nleqn_fcn);
      nleqn.set_options (fsolve_opts);

      octave_idx_type info;
      ColumnVector soln = nleqn.solve (info);

      if (fcn_name.length())
	clear_function (fcn_name);
      if (jac_name.length())
	clear_function (jac_name);

      if (! error_state)
	{
	  retval(2) = static_cast<double> (hybrd_info_to_fsolve_info (info));
	  retval(1) = nleqn.function_value ();
	  retval(0) = NDArray (ArrayN<double> (soln.reshape (x_dims)));

	  if (! nleqn.solution_ok () && nargout < 2)
	    {
	      std::string msg = nleqn.error_message ();
	      error ("fsolve: %s", msg.c_str ());
	    }
	}
    }
  else
    print_usage ();

  unwind_protect::run_frame ("Ffsolve");

  return retval;
}

/*
%!function retval = f (p) 
%!  x = p(1);
%!  y = p(2);
%!  z = p(3);
%!  retval = zeros (3, 1);
%!  retval(1) = sin(x) + y**2 + log(z) - 7;
%!  retval(2) = 3*x + 2**y -z**3 + 1;
%!  retval(3) = x + y + z - 5;
%!test
%! x_opt = [ 0.599054;
%! 2.395931;
%! 2.005014 ];
%! tol = 1.0e-5;
%! [x, fval, info] = fsolve ("f", [ 0.5; 2.0; 2.5 ]);
%! info_bad = (info != 1);
%! solution_bad = sum (abs (x - x_opt) > tol);
%! value_bad = sum (abs (fval) > tol);
%! if (info_bad)
%!   printf_assert ("info bad\n");
%! else
%!   printf_assert ("info good\n");
%! endif
%! if (solution_bad)
%!   printf_assert ("solution bad\n");
%! else
%!   printf_assert ("solution good\n");
%! endif
%! if (value_bad)
%!   printf_assert ("value bad\n");
%! else
%!   printf_assert ("value good\n");
%! endif
%! assert(prog_output_assert("info good\nsolution good\nvalue good"));

%!function retval = f (p)
%!  x = p(1);
%!  y = p(2);
%!  z = p(3);
%!  w = p(4);
%!  retval = zeros (4, 1);
%!  retval(1) = 3*x + 4*y + exp (z + w) - 1.007;
%!  retval(2) = 6*x - 4*y + exp (3*z + w) - 11;
%!  retval(3) = x^4 - 4*y^2 + 6*z - 8*w - 20;
%!  retval(4) = x^2 + 2*y^3 + z - w - 4;
%!test
%! x_opt = [ -0.767297326653401, 0.590671081117440, 1.47190018629642, -1.52719341133957 ];
%! tol = 1.0e-5;
%! [x, fval, info] = fsolve ("f", [-1, 1, 2, -1]);
%! info_bad = (info != 1);
%! solution_bad = sum (abs (x - x_opt) > tol);
%! value_bad = sum (abs (fval) > tol);
%! if (info_bad)
%!   printf_assert ("info bad\n");
%! else
%!   printf_assert ("info good\n");
%! endif
%! if (solution_bad)
%!   printf_assert ("solution bad\n");
%! else
%!   printf_assert ("solution good\n");
%! endif
%! if (value_bad)
%!   printf_assert ("value bad\n");
%! else
%!   printf_assert ("value good\n");
%! endif
%! assert(prog_output_assert("info good\nsolution good\nvalue good"));

%!test
%! fsolve_options ("tolerance", eps);
%! assert(fsolve_options ("tolerance") == eps);

%!error <Invalid call to fsolve_options.*> fsolve_options ("foo", 1, 2);
*/

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
