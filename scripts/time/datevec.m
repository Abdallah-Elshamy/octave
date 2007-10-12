## Copyright (C) 2000, 2001, 2004, 2005 Paul Kienzle
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
## @deftypefn {Function File} {@var{v} =} datevec (@var{date})
## @deftypefnx {Function File} {@var{v} =} datevec (@var{date}, @var{f})
## @deftypefnx {Function File} {@var{v} =} datevec (@var{date}, @var{p})
## @deftypefnx {Function File} {@var{v} =} datevec (@var{date}, @var{f}, @var{p})
## @deftypefnx {Function File} {[@var{y}, @var{m}, @var{d}, @var{h}, @var{mi}, @var{s}] =} datevec (@dots{})
## Convert a serial date number (see @code{datenum}) or date string (see
## @code{datestr}) into a date vector.
##
## A date vector is a row vector with six members, representing the year,
## month, day, hour, minute, and seconds respectively.
##
## @var{f} is the format string used to interpret date strings
## (see @code{datestr}).
##
## @var{p} is the year at the start of the century in which two-digit years
## are to be interpreted in. If not specified, it defaults to the current
## year minus 50.
## @seealso{datenum, datestr, date, clock, now}
## @end deftypefn

## Algorithm: Peter Baum (http://vsg.cape.com/~pbaum/date/date0.htm)

## Author: pkienzle <pkienzle@users.sf.net>
## Modified: bdenney <bill@givebillmoney.com>
## Created: 10 October 2001 (CVS)
## Adapted-By: William Poetra Yoga Hadisoeseno <williampoetra@gmail.com>

## The function __date_str2vec__ is based on datesplit by Bill Denney.

function [y, m, d, h, mi, s] = datevec (date, varargin)

  persistent std_formats nfmt;

  if (isempty (std_formats))
    std_formats = cell ();
    nfmt = 0;
    std_formats{++nfmt} = "dd-mmm-yyyy HH:MM:SS";   # 0
    std_formats{++nfmt} = "dd-mmm-yyyy";            # 1
    std_formats{++nfmt} = "mm/dd/yy";               # 2
    std_formats{++nfmt} = "mm/dd";                  # 6
    std_formats{++nfmt} = "HH:MM:SS";               # 13
    std_formats{++nfmt} = "HH:MM:SS PM";            # 14
    std_formats{++nfmt} = "HH:MM";                  # 15
    std_formats{++nfmt} = "HH:MM PM";               # 16
    std_formats{++nfmt} = "mm/dd/yyyy";             # 23
    std_formats{++nfmt} = "mmm-dd-yyyy HH:MM:SS"; 
    std_formats{++nfmt} = "mmm-dd-yyyy";
    std_formats{++nfmt} = "dd mmm yyyy HH:MM:SS";
    std_formats{++nfmt} = "dd mmm yyyy";
    std_formats{++nfmt} = "mmm dd yyyy HH:MM:SS";
    std_formats{++nfmt} = "mmm dd yyyy";
    std_formats{++nfmt} = "dd.mmm.yyyy HH:MM:SS";
    std_formats{++nfmt} = "dd.mmm.yyyy";
    std_formats{++nfmt} = "mmm.dd.yyyy HH:MM:SS";
    std_formats{++nfmt} = "mmm.dd.yyyy";

   # custom formats
    std_formats{++nfmt} = "mmmyy";                  # 12
    std_formats{++nfmt} = "mm/dd/yyyy HH:MM";
  endif

  if (nargin < 1 || nargin > 3)
    print_usage ();
  endif

  switch (nargin)
  case 1
    f = [];
    p = [];
  case 2
    if (ischar (varargin{1}))
      f = varargin{1};
      p = [];
    else
      f = [];
      p = varargin{1};
    endif
  case 3
      f = varargin{1};
      p = varargin{2};
  endswitch

  if (isempty (f))
    f = -1;
  endif

  if (isempty (p))
    p = (localtime (time)).year + 1900 - 50;
  endif

  if (ischar (date))
    t = date;
    date = cell (1);
    date{1} = t;
  endif

  if (iscell (date))

    nd = numel (date);

    y = m = d = h = mi = s = zeros (nd, 1);

    if (f == -1)
      for k = 1:nd
        found = false;
        for l = 1:nfmt
          [found y(k) m(k) d(k) h(k) mi(k) s(k)] = __date_str2vec__ (date{k}, std_formats{l}, p);
          if (found)
            break;
          endif
        endfor
        if (! found)
          error ("datevec: none of the standard formats match the date string");
        endif
      endfor
    else
      for k = 1:nd
        [found y(k) m(k) d(k) h(k) mi(k) s(k)] = __date_str2vec__ (date{k}, f, p);
        if (! found)
          error ("datevec: date not parsed correctly with given format");
        endif
      endfor
    endif

  else

    date = date(:);

    ## Move day 0 from midnight -0001-12-31 to midnight 0000-3-1
    z = floor (date) - 60;
    ## Calculate number of centuries; K1 = 0.25 is to avoid rounding problems.
    a = floor ((z - 0.25) / 36524.25);
    ## Days within century; K2 = 0.25 is to avoid rounding problems.
    b = z - 0.25 + a - floor (a / 4);
    ## Calculate the year (year starts on March 1).
    y = floor (b / 365.25);
    ## Calculate day in year.
    c = fix (b - floor (365.25 * y)) + 1;
    ## Calculate month in year.
    m = fix ((5 * c + 456) / 153);
    d = c - fix ((153 * m - 457) / 5);
    ## Move to Jan 1 as start of year.
    ++y(m > 12);
    m(m > 12) -= 12;

    ## Convert hour-minute-seconds.  Attempt to account for precision of
    ## datenum format.

    fracd = date - floor (date);
    tmps = abs (eps*86400*date);
    tmps(tmps == 0) = 1;
    srnd = 2 .^ floor (- log2 (tmps));
    s = round (86400 * fracd .* srnd) ./ srnd;
    h = floor (s / 3600);
    s = s - 3600 * h;
    mi = floor (s / 60);
    s = s - 60 * mi;

  endif

  if (nargout <= 1)
    y = [y, m, d, h, mi, s];
  endif

### endfunction

function [found, y, m, d, h, mi, s] = __date_str2vec__ (ds, f, p)

  # Play safe with percent signs
  f = strrep(f, "%", "%%");

  ## dates to lowercase (note: we cannot convert MM to mm)
  f = strrep (f, "YYYY", "yyyy");
  f = strrep (f, "YY", "yy");
  f = strrep (f, "QQ", "qq");
  f = strrep (f, "MMMM", "mmmm");
  f = strrep (f, "MMM", "mmm");
  f = strrep (f, "DDDD", "dddd");
  f = strrep (f, "DDD", "ddd");
  f = strrep (f, "DD", "dd");
  ## times to uppercase (also cannot convert mm to MM)
  f = strrep (f, "hh", "HH");
  f = strrep (f, "ss", "SS");
  f = strrep (f, "pm", "PM");
  f = strrep (f, "am", "AM");

  ## right now, the format string may only contain these tokens:
  ## yyyy   4 digit year
  ## yy     2 digit year
  ## mmmm   month name, full
  ## mmm    month name, abbreviated
  ## mm     month number
  ## dddd   weekday name, full
  ## ddd    weekday name, abbreviated
  ## dd     date
  ## HH     hour
  ## MM     minutes
  ## SS     seconds
  ## PM     AM/PM
  ## AM     AM/PM

  if (! isempty (strfind (f, "PM")) || ! isempty (strfind (f, "AM")))
    ampm = true;
  else
    ampm = false;
  endif

  # date part
  f = strrep (f, "yyyy", "%Y");
  f = strrep (f, "yy", "%y");
  f = strrep (f, "mmmm", "%B");
  f = strrep (f, "mmm", "%b");
  f = strrep (f, "mm", "%m");
  f = strrep (f, "dddd", "%A");
  f = strrep (f, "ddd", "%a");
  f = strrep (f, "dd", "%d");

  # time part
  if (ampm)
    f = strrep (f, "HH", "%I");
    f = strrep (f, "PM", "%p");
    f = strrep (f, "AM", "%p");
  else
    f = strrep (f, "HH", "%H");
  endif
  f = strrep (f, "MM", "%M");
  f = strrep (f, "SS", "%S");

  [tm, nc] = strptime (ds, f);

  if (nc == length (ds) + 1)
    y = tm.year + 1900; m = tm.mon + 1; d = tm.mday;
    h = tm.hour; mi = tm.min; s = tm.sec + tm.usec / 1e6;
    found = true;
    rY = rindex (f, "%Y");
    ry = rindex (f, "%y");
    if (rY < ry)
      if (y > 1999)
        y -= 2000;
      else
        y -= 1900;
      endif
      y += p - mod (p, 100);
      if (y < p)
        y += 100;
      endif
    endif
    # check whether we need to give default values
    # possible error when string contains "%%"
    fy = rY || ry;
    fm = index (f, "%m") || index (f, "%b") || index (f, "%B");
    fd = index (f, "%d") || index (f, "%a") || index (f, "%A");
    fh = index (f, "%H") || index (f, "%I");
    fmi = index (f, "%M");
    fs = index (f, "%S");
    if (! fy && ! fm && ! fd)
      tvm = localtime (time ());  ## tvm: this very moment
      y = tvm.year + 1900;
      m = tvm.mon + 1;
      d = tvm.mday;
    elseif (! fy && fm && fd)
      tvm = localtime (time ());  ## tvm: this very moment
      y = tvm.year + 1900;
    elseif (fy && fm && ! fd)
      tvm = localtime (time ());  ## tvm: this very moment
      d = 1;
    endif
    if (! fh && ! fmi && ! fs)
      h = mi = s = 0;
    elseif (fh && fmi && ! fs)
      s = 0;
    endif
  else
    y = m = d = h = mi = s = 0;
    found = false;
  endif

### endfunction

%!shared nowvec
%! nowvec = datevec (now); # Some tests could fail around midnight!
# tests for standard formats: 0, 1, 2, 6, 13, 14, 15, 16, 23
%!assert(datevec("07-Sep-2000 15:38:09"),[2000,9,7,15,38,9]);
%!assert(datevec("07-Sep-2000"),[2000,9,7,0,0,0]);
%!assert(datevec("09/07/00"),[2000,9,7,0,0,0]);
%!assert(datevec("09/13"),[nowvec(1),9,13,0,0,0]);
%!assert(datevec("15:38:09"),[nowvec(1:3),15,38,9]);
%!assert(datevec("3:38:09 PM"),[nowvec(1:3),15,38,9]);
%!assert(datevec("15:38"),[nowvec(1:3),15,38,0]);
%!assert(datevec("03:38 PM"),[nowvec(1:3),15,38,0]);
%!assert(datevec("03/13/1962"),[1962,3,13,0,0,0]);
# other tests
%!assert(all(datenum(datevec([-1e4:1e4]))==[-1e4:1e4]'))
%!test
%! t = linspace (-2e5, 2e5, 10993);
%! assert (all (abs (datenum (datevec (t)) - t') < 1e-5));
# demos
%!demo
%! datevec (now ())
