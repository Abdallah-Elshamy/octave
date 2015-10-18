## Copyright (C) 2014, Jacopo Corno <jacopo.corno@gmail.com>
## Copyright (C) 2013, Roberto Porcu' <roberto.porcu@polimi.it>
## Copyright (C) 2006-2012, Thomas Treichl <treichl@users.sourceforge.net>
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn  {Function File} {[@var{t}, @var{y}] =} ode45 (@var{fun}, @var{trange}, @var{init})
## @deftypefnx {Function File} {[@var{t}, @var{y}] =} ode45 (@var{fun}, @var{trange}, @var{init}, @var{ode_opt})
## @deftypefnx {Function File} {[@var{t}, @var{y}] =} ode45 (@dots{}, @var{par1}, @var{par2}, @dots{})
## @deftypefnx {Function File} {[@var{t}, @var{y}, @var{te}, @var{ye}, @var{ie}] =} ode45 (@dots{})
## @deftypefnx {Function File} {@var{solution} =} ode45 (@dots{})
##
## Solve a set of non-stiff Ordinary Differential Equations (non-stiff ODEs)
## with the well known explicit Dormand-Prince method of order 4.
##
## @var{fun} is a function handle, inline function, or string containing the
## name of the function that defines the ODE: @code{y' = f(t,y)}.  The function
## must accept two inputs where the first is time @var{t} and the second is a
## column vector of unknowns @var{y}.
##
## @var{trange} specifies the time interval over which the ODE will be
## evaluated.  Typically, it is a two-element vector specifying the initial and
## final times (@code{[tinit, tfinal]}).  If there are more than two elements
## then the solution will also be evaluated at these intermediate time
## instances unless the integrate function specified is
## @command{integrate_n_steps}.
##
## By default, @code{ode45} uses an adaptive timestep with the
## @code{integrate_adaptive} algorithm.  The tolerance for the timestep
## computation may be changed by using the option @qcode{"Tau"}, that has a
## default value of @math{1e-6}.  If the ODE option @qcode{"TimeStepSize"} is
## not empty, then the stepper called will be @code{integrate_const}.  If, in
## addition, the option @qcode{"TimeStepNumber"} is also specified then the
## integrate function @code{integrate_n_steps} will be used.
##
## @var{init} contains the initial value for the unknowns.  If it is a row
## vector then the solution @var{y} will be a matrix in which each column is
## the solution for the corresponding initial value in @var{init}.
##
## The optional fourth argument @var{ode_opt} specifies non-default options to
## the ODE solver.  It is a structure generated by @code{odeset}.
##
## The function typically returns two outputs.  Variable @var{t} is a
## column vector and contains the times where the solution was found.  The
## output @var{y} is a matrix in which each column refers to a different
## unknown of the problem and each row corresponds to a time in @var{t}.
##
## The output can also be returned as a structure @var{solution} which
## has field @var{x} containing the time where the solution was evaluated and
## field @var{y} containing the solution matrix for the times in @var{x}.
## Use @code{fieldnames (@var{solution})} to see the other fields and additional
## information returned.
##
## If using the @qcode{"Events"} option then three additional outputs may
## be returned.  @var{te} holds the time when an Event function returned a
## zero.  @var{ye} holds the value of the solution at time @var{te}.  @var{ie}
## contains an index indicating which Event function was triggered in the case
## of multiple Event functions.
##
## Example: Solve the Van der Pol equation
##
## @example
## @group
## fvdp = @@(@var{t},@var{y}) [@var{y}(2); (1 - @var{y}(1)^2) * @var{y}(2) - @var{y}(1)];
## [@var{t},@var{y}] = ode45 (fvdp, [0 20], [2 0]);
## @end group
## @end example
## @seealso{odeset, odeget}
## @end deftypefn

function varargout = ode45 (fun, trange, init, varargin)

  if (nargin < 3)
    print_usage ();
  endif

  order = 5;  # runge_kutta_45_dorpri uses local extrapolation
  solver = "ode45";

  if (nargin >= 4)
    if (! isstruct (varargin{1}))
      ## varargin{1:len} are parameters for fun
      odeopts = odeset ();
      odeopts.funarguments = varargin;
    elseif (length (varargin) > 1)
      ## varargin{1} is an ODE options structure vopt
      odeopts = ode_struct_value_check ("ode45", varargin{1}, "ode45");
      odeopts.funarguments = {varargin{2:length(varargin)}};
    else  # if (isstruct (varargin{1}))
      odeopts = ode_struct_value_check ("ode45", varargin{1}, "ode45");
      odeopts.funarguments = {};
    endif
  else  # nargin == 3
    odeopts = odeset ();
    odeopts.funarguments = {};
  endif

  if (! isvector (trange) || ! isnumeric (trange))
    error ("Octave:invalid-input-arg",
           "ode45: TRANGE must be a numeric vector");
  endif

  TimeStepNumber = odeget (odeopts, "TimeStepNumber", [], "fast");
  TimeStepSize = odeget (odeopts, "TimeStepSize", [], "fast");
  if (length (trange) < 2
      && (isempty (TimeStepSize) || isempty (TimeStepNumber)))
    error ("Octave:invalid-input-arg",
           "ode45: TRANGE must contain at least 2 elements");
  elseif (trange(1) == trange(2))
    error ("Octave:invalid-input-arg",
           "ode45: invalid time span, TRANGE(1) == TRANGE(2)");
  else
    odeopts.direction = sign (trange(2) - trange(1));
  endif
  trange = trange(:);

  if (! isvector (init) || ! isnumeric (init))
    error ("Octave:invalid-input-arg",
           "ode45: INIT must be a numeric vector");
  endif
  init = init(:);

  if (ischar (fun))
    try
      fun = str2func (fun);
    catch
      warning (lasterr);
    end_try_catch
  endif
  if (! isa (fun, "function_handle"))
    error ("Octave:invalid-input-arg",
           "ode45: FUN must be a valid function handle");
  endif

  ## Start preprocessing, have a look which options are set in odeopts,
  ## check if an invalid or unused option is set
  if (isempty (TimeStepNumber) && isempty (TimeStepSize))
    integrate_func = "adaptive";
    odeopts.stepsizefixed = false;
  elseif (! isempty (TimeStepNumber) && ! isempty (TimeStepSize))
    integrate_func = "n_steps";
    odeopts.stepsizefixed = true;
    if (sign (TimeStepSize) != odeopts.direction)
      warning ("Octave:invalid-input-arg",
               ["ode45: option 'TimeStepSize' has the wrong sign, ", ...
                "but will be corrected automatically\n"]);
      TimeStepSize = -TimeStepSize;
    endif
  elseif (isempty (TimeStepNumber) && ! isempty (TimeStepSize))
    integrate_func = "const";
    odeopts.stepsizefixed = true;
    if (sign (TimeStepSize) != odeopts.direction)
      warning ("Octave:invalid-input-arg",
               ["ode45: option 'TimeStepSize' has the wrong sign, ",
                "but will be corrected automatically\n"]);
      TimeStepSize = -TimeStepSize;
    endif
  else
    warning ("Octave:invalid-input-arg",
             "ode45: assuming an adaptive integrate function\n");
    integrate_func = "adaptive";
  endif

  if (isempty (odeopts.RelTol) && ! odeopts.stepsizefixed)
    odeopts.RelTol = 1e-3;
    warning ("Octave:invalid-input-arg",
             "ode45: option 'RelTol' not set, new value %f will be used\n",
             odeopts.RelTol);
  elseif (! isempty (odeopts.RelTol) && odeopts.stepsizefixed)
    warning ("Octave:invalid-input-arg",
             ["ode45: option 'RelTol' is ignored", ...
              " when fixed time stamps are given\n"]);
  endif

  if (isempty (odeopts.AbsTol) && ! odeopts.stepsizefixed)
    odeopts.AbsTol = 1e-6;
    warning ("Octave:invalid-input-arg",
             "ode45: option 'AbsTol' not set, new value %f will be used\n",
             odeopts.AbsTol);
  elseif (! isempty (odeopts.AbsTol) && odeopts.stepsizefixed)
    warning ("Octave:invalid-input-arg",
             ["ode45: option 'AbsTol' is ignored", ...
              " when fixed time stamps are given\n"]);
  else
    odeopts.AbsTol = odeopts.AbsTol(:);  # Create column vector
  endif

  odeopts.normcontrol = strcmp (odeopts.NormControl, "on");

  if (! isempty (odeopts.NonNegative))
    if (isempty (odeopts.Mass))
      odeopts.havenonnegative = true;
    else
      odeopts.havenonnegative = false;
      warning ("Octave:invalid-input-arg",
               ["ode45: option 'NonNegative' is ignored", ...
                " when mass matrix is set\n"]);
    endif
  else
    odeopts.havenonnegative = false;
  endif

  if (isempty (odeopts.OutputFcn) && nargout == 0)
    odeopts.OutputFcn = @odeplot;
    odeopts.haveoutputfunction = true;
  else
    odeopts.haveoutputfunction = ! isempty (odeopts.OutputFcn);
  endif

  odeopts.haveoutputselection = ! isempty (odeopts.OutputSel);

  if (odeopts.Refine > 0)
    odeopts.haverefine = true;
  else
    odeopts.haverefine = false;
  endif

  if (isempty (odeopts.InitialStep) && strcmp (integrate_func, "adaptive"))
    odeopts.InitialStep = ...
      odeopts.direction * starting_stepsize (order, fun, trange(1),
                                                  init,
                                                  odeopts.AbsTol,
                                                  odeopts.RelTol,
                                                  odeopts.normcontrol);
    warning ("Octave:invalid-input-arg",
             ["ode45: option 'InitialStep' not set,", ...
              " estimated value %f will be used\n"],
             odeopts.InitialStep);
  elseif (isempty (odeopts.InitialStep))
    odeopts.InitialStep = TimeStepSize;
  endif

  if (isempty (odeopts.MaxStep) && ! odeopts.stepsizefixed)
    odeopts.MaxStep = abs (trange(end) - trange(1)) / 10;
    warning ("Octave:invalid-input-arg",
             "ode45: option 'MaxStep' not set, new value %f will be used\n",
             odeopts.MaxStep);
  endif

  odeopts.haveeventfunction = ! isempty (odeopts.Events);

  ## The options "Jacobian", "JPattern" and "Vectorized" will be ignored
  ## by this solver because this solver uses an explicit Runge-Kutta method
  ## and therefore no Jacobian calculation is necessary.
  if (! isempty (odeopts.Jacobian))
    warning ("Octave:invalid-input-arg",
             "ode45: option 'Jacobian' is ignored by this solver\n");
  endif

  if (! isempty (odeopts.JPattern))
    warning ("Octave:invalid-input-arg",
             "ode45: option 'JPattern' is ignored by this solver\n");
  endif

  if (! isempty (odeopts.Vectorized))
    warning ("Octave:invalid-input-arg",
             "ode45: option 'Vectorized' is ignored by this solver\n");
  endif

  if (! isempty (odeopts.Mass) && isnumeric (odeopts.Mass))
    havemasshandle = false;
    mass = odeopts.Mass;  # constant mass
  elseif (isa (odeopts.Mass, "function_handle"))
    havemasshandle = true;    # mass defined by a function handle
  else  # no mass matrix - creating a diag-matrix of ones for mass
    havemasshandle = false;   # mass = diag (ones (length (init), 1), 0);
  endif

  massdependence = ! strcmp (odeopts.MStateDependence, "none");

  ## Other options that are not used by this solver.
  if (! isempty (odeopts.MvPattern))
    warning ("Octave:invalid-input-arg",
             "ode45: option 'MvPattern' is ignored by this solver\n");
  endif

  if (! isempty (odeopts.MassSingular))
    warning ("Octave:invalid-input-arg",
             "ode45: option 'MassSingular' is ignored by this solver\n");
  endif

  if (! isempty (odeopts.InitialSlope))
    warning ("Octave:invalid-input-arg",
             "ode45: option 'InitialSlope' is ignored by this solver\n");
  endif

  if (! isempty (odeopts.MaxOrder))
    warning ("Octave:invalid-input-arg",
             "ode45: option 'MaxOrder' is ignored by this solver\n");
  endif

  if (! isempty (odeopts.BDF))
    warning ("Octave:invalid-input-arg",
             "ode45: option 'BDF' is ignored by this solver\n");
  endif

  ## Starting the initialisation of the core solver ode45

  if (havemasshandle)   # Handle only the dynamic mass matrix,
    if (massdependence) # constant mass matrices have already
      mass = @(t,x) odeopts.Mass (t, x, odeopts.funarguments{:});
      fun = @(t,x) mass (t, x, odeopts.funarguments{:}) ...
             \ fun (t, x, odeopts.funarguments{:});
    else                 # if (massdependence == false)
      mass = @(t) odeopts.Mass (t, odeopts.funarguments{:});
      fun = @(t,x) mass (t, odeopts.funarguments{:}) ...
             \ fun (t, x, odeopts.funarguments{:});
    endif
  endif

  switch (integrate_func)
    case "adaptive"
      solution = integrate_adaptive (@runge_kutta_45_dorpri,
                                     order, fun, trange, init, odeopts);
    case "n_steps"
      solution = integrate_n_steps (@runge_kutta_45_dorpri,
                                    fun, trange(1), init,
                                    TimeStepSize, TimeStepNumber, odeopts);
    case "const"
      solution = integrate_const (@runge_kutta_45_dorpri,
                                  fun, trange, init,
                                  TimeStepSize, odeopts);
  endswitch

  ## Postprocessing, do whatever when terminating integration algorithm
  if (odeopts.haveoutputfunction)  # Cleanup plotter
    feval (odeopts.OutputFcn, solution.t(end),
           solution.x(end,:)', "done", odeopts.funarguments{:});
  endif
  if (odeopts.haveeventfunction)   # Cleanup event function handling
    ode_event_handler (odeopts.Events, solution.t(end),
                         solution.x(end,:)', "done",
                         odeopts.funarguments{:});
  endif

  ## Print additional information if option Stats is set
  if (strcmp (odeopts.Stats, "on"))
    havestats = true;
    nsteps    = solution.cntloop-2;                 # cntloop from 2..end
    nfailed   = (solution.cntcycles-1)-(nsteps)+1;  # cntcycl from 1..end
    nfevals   = 6 * (solution.cntcycles-1) + 1;     # number of ode evaluations
    ndecomps  = 0;  # number of LU decompositions
    npds      = 0;  # number of partial derivatives
    nlinsols  = 0;  # no. of linear systems solutions
    ## Print cost statistics if no output argument is given
    if (nargout == 0)
      printf ("Number of successful steps: %d\n", nsteps);
      printf ("Number of failed attempts:  %d\n", nfailed);
      printf ("Number of function calls:   %d\n", nfevals);
    endif
  else
    havestats = false;
  endif

  if (nargout == 2)
    varargout{1} = solution.t;      # Time stamps are first output argument
    varargout{2} = solution.x;      # Results are second output argument
  elseif (nargout == 1)
    varargout{1}.x = solution.t;    # Time stamps are saved in field x
    varargout{1}.y = solution.x;    # Results are saved in field y
    varargout{1}.solver = solver;   # Solver name is saved in field solver
    if (odeopts.haveeventfunction)
      varargout{1}.ie = solution.event{2};  # Index info which event occurred
      varargout{1}.xe = solution.event{3};  # Time info when an event occurred
      varargout{1}.ye = solution.event{4};  # Results when an event occurred
    endif
    if (havestats)
      varargout{1}.stats = struct ();
      varargout{1}.stats.nsteps   = nsteps;
      varargout{1}.stats.nfailed  = nfailed;
      varargout{1}.stats.nfevals  = nfevals;
      varargout{1}.stats.npds     = npds;
      varargout{1}.stats.ndecomps = ndecomps;
      varargout{1}.stats.nlinsols = nlinsols;
    endif
  elseif (nargout == 5)
    varargout = cell (1,5);
    varargout{1} = solution.t;
    varargout{2} = solution.x;
    if (odeopts.haveeventfunction)
      varargout{3} = solution.event{3};  # Time info when an event occurred
      varargout{4} = solution.event{4};  # Results when an event occurred
      varargout{5} = solution.event{2};  # Index info which event occurred
    endif
  endif

endfunction


## We are using the "Van der Pol" implementation for all tests that are done
## for this function.
## For further tests we also define a reference solution (computed at high
## accuracy)
%!function ydot = fpol (t, y)  # The Van der Pol
%! ydot = [y(2); (1 - y(1)^2) * y(2) - y(1)];
%!endfunction
%!function ref = fref ()         # The computed reference solution
%! ref = [0.32331666704577, -1.83297456798624];
%!endfunction
%!function jac = fjac (t, y, varargin) # its Jacobian
%! jac = [0, 1; -1 - 2 * y(1) * y(2), 1 - y(1)^2];
%!endfunction
%!function jac = fjcc (t, y, varargin) # sparse type
%! jac = sparse ([0, 1; -1 - 2 * y(1) * y(2), 1 - y(1)^2])
%!endfunction
%!function [val, trm, dir] = feve (t, y, varargin)
%! val = fpol (t, y, varargin);  # We use the derivatives
%! trm = zeros (2,1);              # that's why component 2
%! dir = ones (2,1);               # seems to not be exact
%!endfunction
%!function [val, trm, dir] = fevn (t, y, varargin)
%! val = fpol (t, y, varargin);    # We use the derivatives
%! trm = ones (2,1);               # that's why component 2
%! dir = ones (2,1);               # seems to not be exact
%!endfunction
%!function mas = fmas (t, y, varargin)
%! mas = [1, 0; 0, 1];            # Dummy mass matrix for tests
%!endfunction
%!function mas = fmsa (t, y, varargin)
%! mas = sparse ([1, 0; 0, 1]);   # A sparse dummy matrix
%!endfunction
%!function out = fout (t, y, flag, varargin)
%! if (regexp (char (flag), 'init') == 1)
%!   if (any (size (t) != [2, 1])) error ('"fout" step "init"'); endif
%! elseif (isempty (flag))
%!   if (any (size (t) != [1, 1])) error ('"fout" step "calc"'); endif
%!   out = false;
%! elseif (regexp (char (flag), 'done') == 1)
%!   if (any (size (t) != [1, 1])) error ('"fout" step "done"'); endif
%! else
%!   error ('"fout" invalid flag');
%! endif
%!endfunction
%!
%! ## Turn off output of warning messages for all tests,
%! ## turn them on again after the last test is called
%!test
%! warning ("off", "Octave:invalid-input-arg", "local");
%!error  # first input arg is not a function
%! [t, y] = ode45 (1, [0 25], [3 15 1]);
%!error  # input argument number one as name of non existing function
%! [t, y] = ode45 ("non-existing-function", [0 25], [3 15 1]);
%!error  # input argument number two
%! [t, y] = ode45 (@fpol, 1, [3 15 1]);
%!test  # two output arguments
%! [t, y] = ode45 (@fpol, [0 2], [2 0]);
%! assert ([t(end), y(end,:)], [2, fref], 1e-2);
%!test  # not too many steps
%! [t, y] = ode45 (@fpol, [0 2], [2 0]);
%! assert (size (t) < 20);
%!test  # anonymous function instead of real function
%! fvdb = @(t,y) [y(2); (1 - y(1)^2) * y(2) - y(1)];
%! [t, y] = ode45 (fvdb, [0 2], [2 0]);
%! assert ([t(end), y(end,:)], [2, fref], 1e-2);
%!test  # string instead of function
%! [t, y] = ode45 ("fpol", [0 2], [2 0]);
%! assert ([t(end), y(end,:)], [2, fref], 1e-2);
%!test  # extra input arguments passed through
%! [t, y] = ode45 (@fpol, [0 2], [2 0], 12, 13, "KL");
%! assert ([t(end), y(end,:)], [2, fref], 1e-2);
%!test  # empty ODEOPT structure *but* extra input arguments
%! opt = odeset;
%! [t, y] = ode45 (@fpol, [0 2], [2 0], opt, 12, 13, "KL");
%! assert ([t(end), y(end,:)], [2, fref], 1e-2);
%!error  # strange ODEOPT structure
%! opt = struct ("foo", 1);
%! [t, y] = ode45 (@fpol, [0 2], [2 0], opt);
%!test  # Solve vdp in fixed step sizes
%! opt = odeset("TimeStepSize", 0.1);
%! [t, y] = ode45 (@fpol, [0,2], [2 0], opt);
%! assert (t(:), [0:0.1:2]', 1e-2);
%!test  # Solve another anonymous function below zero
%! vref = [0, 14.77810590694212];
%! [t, y] = ode45 (@(t,y) y, [-2 0], 2);
%! assert ([t(end), y(end,:)], vref, 1e-1);
%!test  # InitialStep option
%! opt = odeset ("InitialStep", 1e-8);
%! [t, y] = ode45 (@fpol, [0 0.2], [2 0], opt);
%! assert ([t(2)-t(1)], [1e-8], 1e-9);
%!test  # MaxStep option
%! opt = odeset ("MaxStep", 1e-3);
%! sol = ode45 (@fpol, [0 0.2], [2 0], opt);
%! assert ([sol.x(5)-sol.x(4)], [1e-3], 1e-3);
%!test  # Solve with intermidiate step
%! sol = ode45 (@fpol, [0 1 2], [2 0]);
%! assert (any((sol.x-1) == 0));
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-3);
%!test  # Solve in backward direction starting at t=0
%! vref = [-1.205364552835178, 0.951542399860817];
%! sol = ode45 (@fpol, [0 -2], [2 0]);
%! assert ([sol.x(end), sol.y(end,:)], [-2, vref], 1e-2);
%!test  # Solve in backward direction starting at t=2
%! vref = [-1.205364552835178, 0.951542399860817];
%! sol = ode45 (@fpol, [2 -2], fref);
%! assert ([sol.x(end), sol.y(end,:)], [-2, vref], 1e-2);
%!test  # Solve in backward direction starting at t=2, with intermidiate step
%! vref = [-1.205364552835178, 0.951542399860817];
%! sol = ode45 (@fpol, [2 0 -2], fref);
%! idx = find(sol.x < 0, 1, "first") - 1;
%! assert ([sol.x(idx), sol.y(idx,:)], [0 2 0], 1e-2);
%! assert ([sol.x(end), sol.y(end,:)], [-2, vref], 1e-2);
%!test  # Solve another anonymous function in backward direction
%! vref = [-1, 0.367879437558975];
%! sol = ode45 (@(t,y) y, [0 -1], 1);
%! assert ([sol.x(end), sol.y(end,:)], vref, 1e-3);
%!test  # Solve another anonymous function below zero
%! vref = [0, 14.77810590694212];
%! sol = ode45 (@(t,y) y, [-2 0], 2);
%! assert ([sol.x(end), sol.y(end,:)], vref, 1e-3);
%!test  # Solve in backward direction starting at t=0 with MaxStep option
%! vref = [-1.205364552835178, 0.951542399860817];
%! opt = odeset ("MaxStep", 1e-3);
%! sol = ode45 (@fpol, [0 -2], [2 0], opt);
%! assert ([abs(sol.x(8)-sol.x(7))], [1e-3], 1e-3);
%! assert ([sol.x(end), sol.y(end,:)], [-2, vref], 1e-3);
%!test  # AbsTol option
%! opt = odeset ("AbsTol", 1e-5);
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-3);
%!test  # AbsTol and RelTol option
%! opt = odeset ("AbsTol", 1e-8, "RelTol", 1e-8);
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-3);
%!test  # RelTol and NormControl option -- higher accuracy
%! opt = odeset ("RelTol", 1e-8, "NormControl", "on");
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-5);
%!test  # Keeps initial values while integrating
%! opt = odeset ("NonNegative", 2);
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, 2, 0], 0.5);
%!test  # Details of OutputSel and Refine can't be tested
%! opt = odeset ("OutputFcn", @fout, "OutputSel", 1, "Refine", 5);
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%!test  # Stats must add further elements in sol
%! opt = odeset ("Stats", "on");
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert (isfield (sol, "stats"));
%! assert (isfield (sol.stats, "nsteps"));
%!test  # Events option add further elements in sol
%! opt = odeset ("Events", @feve);
%! sol = ode45 (@fpol, [0 10], [2 0], opt);
%! assert (isfield (sol, "ie"));
%! assert (sol.ie(1), 2);
%! assert (isfield (sol, "xe"));
%! assert (isfield (sol, "ye"));
%!test  # Events option, now stop integration
%! opt = odeset ("Events", @fevn, "NormControl", "on");
%! sol = ode45 (@fpol, [0 10], [2 0], opt);
%! assert ([sol.ie, sol.xe, sol.ye],
%!         [2.0, 2.496110, -0.830550, -2.677589], 6e-1);
%!test  # Events option, five output arguments
%! opt = odeset ("Events", @fevn, "NormControl", "on");
%! [t, y, vxe, ye, vie] = ode45 (@fpol, [0 10], [2 0], opt);
%! assert ([vie, vxe, ye],
%!         [2.0, 2.496110, -0.830550, -2.677589], 6e-1);
%!test  # Jacobian option
%! opt = odeset ("Jacobian", @fjac);
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-3);
%!test  # Jacobian option and sparse return value
%! opt = odeset ("Jacobian", @fjcc);
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-3);

## test for JPattern option is missing
## test for Vectorized option is missing

%!test  # Mass option as function
%! opt = odeset ("Mass", @fmas);
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-3);
%!test  # Mass option as matrix
%! opt = odeset ("Mass", eye (2,2));
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-3);
%!test  # Mass option as sparse matrix
%! opt = odeset ("Mass", sparse (eye (2,2)));
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-3);
%!test  # Mass option as function and sparse matrix
%! opt = odeset ("Mass", @fmsa);
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-3);
%!test  # Mass option as function and MStateDependence
%! opt = odeset ("Mass", @fmas, "MStateDependence", "strong");
%! sol = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-3);
%!test  # Set BDF option to something other than default
%! opt = odeset ("BDF", "on");
%! [t, y] = ode45 (@fpol, [0 2], [2 0], opt);
%! assert ([t(end), y(end,:)], [2, fref], 1e-3);

## test for MvPattern option is missing
## test for InitialSlope option is missing
## test for MaxOrder option is missing

%!test
%!
%! #warning ("on", "Octave:invalid-input-arg");

