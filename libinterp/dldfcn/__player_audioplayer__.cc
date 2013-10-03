/*

Copyright (C) 2013 Vytautas Jančauskas

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

#include "oct.h"
#include "ov-struct.h"

#ifdef HAVE_PORTAUDIO
#include "player_class.cc"
#include "recorder_class.cc"
#endif

DEFUN_DLD (__player_audioplayer__, args, ,
"__player_audioplayer__")
{
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  audioplayer* retval = new audioplayer ();
  bool is_function = args(0).is_string () || args(0).is_function_handle () || args(0).is_inline_function ();
  if (is_function)
    retval->set_y (args(0).function_value ());
  else
    retval->set_y (args(0));
  retval->set_fs (args(1).int_value ());
  switch (nargin)
    {
      case 3:
        retval->set_nbits (args(2).int_value ());
        break;
      case 4:
        retval->set_nbits (args(2).int_value ());
        retval->set_id (args(3).int_value ());
        break;
    }
  if (is_function)
    retval->init_fn ();
  else
    retval->init ();
  return octave_value (retval);
#else
  octave_value retval;
  error ("portaudio not found on your system and thus audio functionality is not present");
  return retval;
#endif
}

DEFUN_DLD (__player_get_channels__, args, ,
"__player_get_channels__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      retval = octave_value (player->get_channels ());
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_get_fs__, args, ,
"__player_get_fs__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      retval = octave_value (player->get_fs ());
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_get_id__, args, ,
"__player_get_id__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      retval = octave_value (player->get_id ());
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_get_nbits__, args, ,
"__player_get_nbits__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      retval = octave_value (player->get_nbits ());
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_get_sample_number__, args, ,
"__player_get_sample_number__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      retval = octave_value (player->get_sample_number ());
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_get_tag__, args, ,
"__player_get_tag__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      retval = octave_value (player->get_tag ());
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_get_total_samples__, args, ,
"__player_get_total_samples__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      retval = octave_value (player->get_total_samples ());
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_get_userdata__, args, ,
"__player_get_userdata__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      retval = player->get_userdata ();
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_isplaying__, args, ,
"__player_isplaying__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      if (player->isplaying ())
        return octave_value (1);
      else
        return octave_value (0);
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_pause__, args, ,
"__player_pause__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      player->pause ();
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_playblocking__, args, ,
"__player_playblocking__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      player->playblocking ();
    }
  else
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      if (args(1).is_matrix_type ())
        {
          unsigned int start, end;
          RowVector range = args(1).row_vector_value ();
          start = range.elem (0) - 1;
          end = range.elem (1) - 1;
          if (start < 0 or start > player->get_total_samples () or
              start > end or end < 0 or end > player->get_total_samples ())
            error ("audioplayer: invalid range specified for playback");
          player->set_sample_number (start);
          player->set_end_sample (end);
        }
      else
        {
          unsigned int start;
          start = args(1).int_value () - 1;
          if (start < 0 or start > player->get_total_samples ())
            error ("audioplayer: invalid range specified for playback");
          player->set_sample_number (start);
        }
      player->playblocking ();
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_play__, args, ,
"__player_play__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      player->play ();
    }
  else
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      if (args(1).is_matrix_type ())
        {
          unsigned int start, end;
          RowVector range = args(1).row_vector_value ();
          start = range.elem (0) - 1;
          end = range.elem (1) - 1;
          if (start < 0 or start > player->get_total_samples () or
              start > end or end < 0 or end > player->get_total_samples ())
            error ("audioplayer: invalid range specified for playback");
          player->set_sample_number (start);
          player->set_end_sample (end);
        }
      else
        {
          unsigned int start;
          start = args(1).int_value () - 1;
          if (start < 0 or start > player->get_total_samples ())
            error ("audioplayer: invalid range specified for playback");
          player->set_sample_number (start);
        }
      player->play ();
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_resume__, args, ,
"__player_resume__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      player->resume ();
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_set_fs__, args, ,
"__player_set_fs__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 2)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      player->set_fs (args(1).int_value ());
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_set_tag__, args, ,
"__player_set_tag__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 2)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      player->set_tag (args(1).char_matrix_value ());
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_set_userdata__, args, ,
"__player_set_userdata__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 2)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      player->set_userdata (args(1));
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}

DEFUN_DLD (__player_stop__, args, ,
"__player_stop__")
{
  octave_value retval;
#ifdef HAVE_PORTAUDIO
  int nargin = args.length ();
  if (nargin == 1)
    {
      const octave_base_value& rep = args(0).get_rep ();
      audioplayer *player = &((audioplayer &)rep);
      player->stop ();
    }
#else
  error ("portaudio not found on your system and thus audio functionality is not present");
#endif
  return retval;
}
