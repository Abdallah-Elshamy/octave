## Copyright (C) 1996, 1997 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, write to the Free
## Software Foundation, 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {} saveimage (@var{file}, @var{x}, @var{fmt}, @var{map})
## Save the matrix @var{x} to @var{file} in image format @var{fmt}.  Valid
## values for @var{fmt} are
##
## @table @code
## @item "img"
## Octave's image format.  The current colormap is also saved in the file.
##
## @item "ppm"
## Portable pixmap format.
##
## @item "ps"
## PostScript format.  Note that images saved in PostScript format can not
## be read back into Octave with loadimage.
## @end table
##
## If the fourth argument is supplied, the specified colormap will also be
## saved along with the image.
##
## Note: if the colormap contains only two entries and these entries are
## black and white, the bitmap ppm and PostScript formats are used.  If the
## image is a gray scale image (the entries within each row of the colormap
## are equal) the gray scale ppm and PostScript image formats are used,
## otherwise the full color formats are used.
## @end deftypefn

## The conversion to PostScript is based on pbmtolps.c, which was
## written by
##
##   George Phillips <phillips@cs.ubc.ca>
##   Department of Computer Science
##   University of British Columbia
##
## and is part of the portable bitmap utilities,
## @seealso{loadimage, save, load, and colormap}

## Author: Tony Richardson <arichard@stark.cc.oh.us>
## Created: July 1994
## Adapted-By: jwe

## Rewritten by jwe to avoid using octoppm and pbm routines so that
## people who don't have the the pbm stuff installed can still use this
## function.
##
## The conversion to PostScript is based on pnmtops.c, which is part of
## the portable bitmap utilties and bears this copyright notice:
##
## Copyright (C) 1989 by Jef Poskanzer.
##
## Permission to use, copy, modify, and distribute this software and its
## documentation for any purpose and without fee is hereby granted, provided
## that the above copyright notice appear in all copies and that both that
## copyright notice and this permission notice appear in supporting
## documentation.  This software is provided "as is" without express or
## implied warranty.

function saveimage (filename, img, img_form, map)

  if (nargin < 2 || nargin > 4)
    usage ("saveimage (filename, matrix, [format, [colormap]])");
  endif

  if (nargin < 4)
    map = colormap ();
  endif

  [map_nr, map_nc] = size (map);

  if (map_nc != 3)
    error ("colormap should be an N x 3 matrix");
  endif

  if (nargin < 3)
    img_form = "img";
  elseif (! isstr (img_form))
    error ("image format specification must be a string");
  elseif (! (strcmp (img_form, "img")
             || strcmp (img_form, "ppm")
             || strcmp (img_form, "ps")))
    error ("unsupported image format specification");
  endif

  if (! is_matrix (img))
    warning ("image variable is not a matrix");
  endif

  if (! isstr (filename))
    error ("file name must be a string");
  endif

  ## If we just want Octave image format, save and return.

  if (strcmp (img_form, "img"))
    eval (strcat ("save -ascii ", filename, " map img"));
    return;
  endif

  ## Convert to another format if requested.

  grey = all (map(:,1) == map(:,2) && map(:,1) == map (:,3));

  pbm = pgm = ppm = 0;

  map_sz = map_nr * map_nc;

  map = reshape (map, map_sz, 1);

  idx = find (map > 1);
  map (idx) = ones (size (idx));

  idx = find (map < 0);
  map (idx) = zeros (size (idx));

  map = round (255 * map);

  bw = (map_nr == 2
        && ((map(1,1) == 0 && map(2,1) == 255)
            || (map(1,1) == 255 && map(2,1) == 0)));

  img = round (img');
  [img_nr, img_nc] = size (img);

  img_sz = img_nr * img_nc;
  img = reshape (img, img_sz, 1);

  idx = find (img > map_nr);
  img (idx) = ones (size (idx)) * map_nr;

  idx = find (img <= 0);
  img (idx) = ones (size (idx));

  if (strcmp (img_form, "ppm"))

    ## Would be nice to make this consistent with the line used by the
    ## load/save functions, but we need a good way to get username and
    ## hostname information.

    time_string = ctime (time ());
    time_string = time_string (1:length (time_string)-1);
    tagline = sprintf ("# Created by Octave %s, %s",
		       __OCTAVE_VERSION__, time_string);

    if (grey && bw)

      if (map(1) != 0)
        map = [0; 1];
      else
        map = [1; 0];
      endif

      n_long = rem (img_nc, 8);
      tmp = zeros (ceil (img_nc/8), img_nr);

      k = ceil (img_nr/8);
      tmp = zeros (k, img_nc);

      ## Append columns with zeros to original image so that
      ## mod (cols, 8) = 0.

      bwimg = postpad (reshape (map(img), img_nr, img_nc), k * 8, 0);

      b = kron (pow2 (7:-1:0)', ones (1, img_nc));

      for i = 1:k
        tmp(i,:) = sum (bwimg(8*(i-1)+1:8*i,:) .* b);
      endfor

      fid = fopen (filename, "w");
      fprintf (fid, "P4\n%s\n%d %d\n", tagline, img_nr, img_nc);
      fwrite (fid, tmp, "char");
      fprintf (fid, "\n");
      fclose (fid);

    elseif (grey)

      fid = fopen (filename, "w");
      fprintf (fid, "P5\n%s\n%d %d\n255\n", tagline, img_nr, img_nc);
      fwrite (fid, map(img), "uchar");
      fprintf (fid, "\n");
      fclose (fid);

    else

      img_idx = ((1:3:3*img_sz)+2)';
      map_idx = ((2*map_nr+1):map_sz)';

      tmap = map(map_idx);
      tmp(img_idx--) = tmap(img);

      map_idx = map_idx - map_nr;
      tmap = map(map_idx);
      tmp(img_idx--) = tmap(img);

      map_idx = map_idx - map_nr;
      tmap = map(map_idx);
      tmp(img_idx--) = tmap(img);

      fid = fopen (filename, "w");
      fprintf (fid, "P6\n%s\n%d %d\n255\n", tagline, img_nr, img_nc);
      fwrite (fid, tmp, "uchar");
      fprintf (fid, "\n");
      fclose (fid);

    endif

  elseif (strcmp (img_form, "ps") == 1)

    if (! grey)
      error ("must have a greyscale color map for conversion to PostScript");
    endif

    bps = 8;
    dpi = 300;
    pagewid = 612;
    pagehgt = 762;
    MARGIN = 0.95;
    devpix = dpi / 72.0 + 0.5;
    pixfac = 72.0 / dpi * devpix;

    ## Compute padding to round cols * bps up to the nearest multiple of 8
    ## (nr and nc are switched because we transposed the image above).

    padright = (((img_nr * bps + 7) / 8) * 8 - img_nr * bps) / bps;

    scols = img_nr * pixfac;
    srows = img_nc * pixfac;
    scale = 1;

    if (scols > pagewid * MARGIN || srows > pagehgt * MARGIN)
      if (scols > pagewid * MARGIN)
        scale = scale * (pagewid / scols * MARGIN);
        scols = scale * img_nr * pixfac;
        srows = scale * img_nc * pixfac;
      endif
      if (srows > pagehgt * MARGIN)
        scale = scale * (pagehgt / srows * MARGIN);
        scols = scale * img_nr * pixfac;
        srows = scale * img_nc * pixfac;
      endif
      warning ("image too large for page, rescaling to %g", scale);
    endif

    llx = (pagewid - scols) / 2;
    lly = (pagehgt - srows) / 2;
    urx = llx + fix (scols + 0.5);
    ury = lly + fix (srows + 0.5);

    fid = fopen (filename, "w");

    fprintf (fid, "%%!PS-Adobe-2.0 EPSF-2.0\n");
    fprintf (fid, "%%%%Creator: Octave %s (saveimage.m)\n", OCTAVE_VERSION);
    fprintf (fid, "%%%%Title: %s\n", filename);
    fprintf (fid, "%%%%Pages: 1\n");
    fprintf (fid, "%%%%BoundingBox: %d %d %d %d\n",
             fix (llx), fix (lly), fix (urx), fix (ury));
    fprintf (fid, "%%%%EndComments\n");
    fprintf (fid, "/readstring {\n");
    fprintf (fid, "  currentfile exch readhexstring pop\n");
    fprintf (fid, "} bind def\n");
    fprintf (fid, "/picstr %d string def\n",
             fix ((img_nr + padright) * bps / 8));
    fprintf (fid, "%%%%EndProlog\n");
    fprintf (fid, "%%%%Page: 1 1\n");
    fprintf (fid, "gsave\n");
    fprintf (fid, "%g %g translate\n", llx, lly);
    fprintf (fid, "%g %g scale\n", scols, srows);
    fprintf (fid, "%d %d %d\n", img_nr, img_nc, bps);
    fprintf (fid, "[ %d 0 0 -%d 0 %d ]\n", img_nr, img_nc, img_nc);
    fprintf (fid, "{ picstr readstring }\n");
    fprintf (fid, "image\n");

    img = map(img);

    fmt = "%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x\n";
    fprintf (fid, fmt, img);

    if (rem (img_sz, 30) != 0)
      fprintf (fid, "\n");
    endif

    fprintf (fid, "grestore\n");
    fprintf (fid, "showpage\n");
    fprintf (fid, "%%%%Trailer\n");
    fclose (fid);

  else
    error ("saveimage: what happened to the image type?");
  endif

endfunction
