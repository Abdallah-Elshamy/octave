#! /bin/sh
#
# Copyright (C) 2016-2020 The Octave Project Developers
#
# See the file COPYRIGHT.md in the top-level directory of this distribution
# or <https://octave.org/COPYRIGHT.html/>.
#
#
# This file is part of Octave.
#
# Octave is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Octave is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Octave; see the file COPYING.  If not, see
# <https://www.gnu.org/licenses/>.

# Generate a header file that provides the public symbols from Octave's
# autoconf-generated config.h file.  See the notes at the top of the
# generated octave-config.h file for more details.

SED=${SED:-sed}

if [ $# -ne 1 ]; then
  echo "usage: mk-octave-config-h.sh CONFIG-FILE" 1>&2
  exit 1
fi

config_h_file=$1

cat << EOF
/* DO NOT EDIT!  Generated by mk-octave-config-h.sh.  */
/*

Copyright (C) 2016-2020 The Octave Project Developers

See the file COPYRIGHT.md in the top-level directory of this distribution
or <https://octave.org/COPYRIGHT.html/>.


This file is part of Octave.

Octave is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<https://www.gnu.org/licenses/>.

*/

/*

All Octave source files should begin with

  #if defined (HAVE_CONFIG_H)
  #  include "config.h"
  #endif

All public Octave header files should have the form

  #if ! defined (INCLUSION_GUARD_SYMBOL)
  #define INCLUSION_GUARD_SYMBOL 1

  #include "octave-config.h"

  // Contents of header file.

  #endif

In Octave source files, INCLUSION_GUARD_SYMBOL should have the form

  octave_NAME_h

with NAME formed from the header file name with '-' replaced by '_'.

It is safe to include octave-config.h unconditionally since it will
expand to an empty file if it is included after Octave's
autoconf-generated config.h file.

Users of Octave's libraries should not need to include octave-config.h
since all of Octave's header files already include it.

*/

#if ! defined (octave_octave_config_h)
#define octave_octave_config_h 1

#if ! defined (OCTAVE_AUTOCONFIG_H_INCLUDED)

#  if defined (__cplusplus)
#    include <cinttypes>
#  else
#    include <inttypes.h>
#  endif

#  if defined (__GNUC__)
#    if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#      define OCTAVE_DEPRECATED(ver, msg) __attribute__ ((__deprecated__ ("[" #ver "]: " msg)))
#    else
#      define OCTAVE_DEPRECATED(ver, msg) __attribute__ ((__deprecated__))
#    endif
#    define OCTAVE_NORETURN __attribute__ ((__noreturn__))
#    define OCTAVE_UNUSED __attribute__ ((__unused__))

#    define HAVE_OCTAVE_DEPRECATED_ATTR 1
#    define HAVE_OCTAVE_NORETURN_ATTR 1
#    define HAVE_OCTAVE_UNUSED_ATTR 1
#  else
#    define OCTAVE_DEPRECATED(ver, msg)
#    define OCTAVE_NORETURN
#    define OCTAVE_UNUSED

/* #    undef HAVE_OCTAVE_DEPRECATED_ATTR */
/* #    undef HAVE_OCTAVE_NORETURN_ATTR */
/* #    undef HAVE_OCTAVE_UNUSED_ATTR */
#  endif

#  if defined (__MINGW32__)
    /* MinGW requires special handling due to different format specifiers
     * on different platforms.  The macro __MINGW_PRINTF_FORMAT maps to
     * either gnu_printf or ms_printf depending on where we are compiling
     * to avoid warnings on format specifiers that are legal.
     * See: https://bugzilla.mozilla.org/show_bug.cgi?id=1331349  */
#    if defined (__cplusplus)
#      include <cstdio>
#    else
#      include <stdio.h>
#    endif

#    define OCTAVE_FORMAT_PRINTF(stringIndex, firstToCheck) \
       __attribute__ ((format (__MINGW_PRINTF_FORMAT, stringIndex, firstToCheck)))

#    define HAVE_OCTAVE_FORMAT_PRINTF_ATTR 1
#  elif defined (__GNUC__)
     /* The following attributes are used with gcc and clang compilers.  */
#    define OCTAVE_FORMAT_PRINTF(index, first) \
       __attribute__ ((__format__(printf, index, first)))

#    define HAVE_OCTAVE_FORMAT_PRINTF_ATTR 1
#  else
#    define OCTAVE_FORMAT_PRINTF(index, first)

/* #    undef HAVE_OCTAVE_FORMAT_PRINTF_ATTR */
#  endif

#  if ! defined (OCTAVE_FALLTHROUGH)
#    if defined (__cplusplus) && __cplusplus > 201402L
#      define OCTAVE_FALLTHROUGH [[fallthrough]]
#    elif defined (__GNUC__) && __GNUC__ < 7
#      define OCTAVE_FALLTHROUGH ((void) 0)
#    else
#      define OCTAVE_FALLTHROUGH __attribute__ ((__fallthrough__))
#    endif
#  endif

#  define OCTAVE_USE_DEPRECATED_FUNCTIONS 1

#  if defined (__cplusplus)
template <typename T>
static inline void
octave_unused_parameter (const T&)
{ }
#  else
#    define octave_unused_parameter(param) (void) param;
#  endif

#  if defined (_MSC_VER)
#    define OCTAVE_EXPORT __declspec(dllexport)
#    define OCTAVE_IMPORT __declspec(dllimport)
#  else
     /* All other compilers, at least for now. */
#    define OCTAVE_EXPORT
#    define OCTAVE_IMPORT
#  endif

#  define OCTAVE_API OCTAVE_IMPORT
#  define OCTINTERP_API OCTAVE_IMPORT
EOF

octave_idx_type="`$SED -n 's/#define OCTAVE_IDX_TYPE \([_a-zA-Z][_a-zA-Z0-9]*\)/\1/p' $config_h_file`"

if test -z "$octave_idx_type"; then
  echo "mk-octave-config-h.sh: failed to find OCTAVE_IDX_TYPE in $config_h_file" 1>&2
  exit 1
fi

octave_f77_int_type="`$SED -n 's/#define OCTAVE_F77_INT_TYPE \([_a-zA-Z][_a-zA-Z0-9]*\)/\1/p' $config_h_file`"

if test -z "$octave_f77_int_type"; then
  echo "mk-octave-config-h.sh: failed to find OCTAVE_F77_INT_TYPE in $config_h_file" 1>&2
  exit 1
fi

cat << EOF

typedef $octave_idx_type octave_idx_type;
typedef $octave_f77_int_type octave_f77_int_type;

#  define OCTAVE_HAVE_F77_INT_TYPE 1


#  if defined (__cplusplus) && ! defined (OCTAVE_THREAD_LOCAL)
#    define OCTAVE_THREAD_LOCAL
#  endif

EOF

if grep "#define OCTAVE_ENABLE_FLOAT_TRUNCATE 1" $config_h_file > /dev/null; then
  echo "#  define OCTAVE_FLOAT_TRUNCATE volatile"
else
  echo "#  define OCTAVE_FLOAT_TRUNCATE"
fi

echo ""

$SED -n 's/#\(\(undef\|define\) OCTAVE_ENABLE_64.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_ENABLE_BOUNDS_CHECK.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_ENABLE_OPENMP.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_HAVE_LONG_LONG_INT.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_HAVE_UNSIGNED_LONG_LONG_INT.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_HAVE_OVERLOAD_CHAR_INT8_TYPES.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_SIZEOF_F77_INT_TYPE.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) OCTAVE_SIZEOF_IDX_TYPE.*$\)/#  \1/p' $config_h_file

cat << EOF

#  if defined (OCTAVE_ENABLE_64)
#    define OCTAVE_IDX_TYPE_FORMAT PRId64
#  else
#    define OCTAVE_IDX_TYPE_FORMAT PRId32
#  endif

EOF

$SED -n 's/#\(\(undef\|define\) gid_t.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) uid_t.*$\)/#  \1/p' $config_h_file
$SED -n 's/#\(\(undef\|define\) nlink_t.*$\)/#  \1/p' $config_h_file

if grep "#undef HAVE_DEV_T" $config_h_file > /dev/null; then
  cat << EOF
typedef short dev_t;
EOF
else
  cat << EOF
/* typedef short dev_t; */
EOF
fi

if grep "#undef HAVE_INO_T" $config_h_file > /dev/null; then
    cat << EOF
typedef unsigned long ino_t;
EOF
else
    cat << EOF
/* typedef unsigned long ino_t; */
EOF
fi

echo ""

have_roundl=no
if grep "#define HAVE_ROUNDL" $config_h_file > /dev/null; then
  have_roundl=yes
fi

sizeof_long_double="`$SED -n 's/#define SIZEOF_LONG_DOUBLE \([0-9]*\)/\1/p' $config_h_file`"

if test -z "$sizeof_long_double"; then
  echo "mk-octave-config-h.sh: failed to find SIZEOF_LONG_DOUBLE in $config_h_file" 1>&2
  exit 1
fi

if test $sizeof_long_double -ge 10 && test $have_roundl = yes; then
  echo "#  define OCTAVE_INT_USE_LONG_DOUBLE 1"
  if test $sizeof_long_double -lt 16; then
    cat << EOF
#  if (defined (__i386__) || defined (__x86_64__)) && defined (__GNUC__)
#    define OCTAVE_ENSURE_LONG_DOUBLE_OPERATIONS_ARE_NOT_TRUNCATED 1
#  endif
EOF
  else
    cat << EOF
/* #  undef OCTAVE_ENSURE_LONG_DOUBLE_OPERATIONS_ARE_NOT_TRUNCATED */
EOF
  fi
else
  cat << EOF
/* #  undef OCTAVE_INT_USE_LONG_DOUBLE */
/* #  undef OCTAVE_ENSURE_LONG_DOUBLE_OPERATIONS_ARE_NOT_TRUNCATED */
EOF
fi

echo ""

$SED -n 's/#\(\(undef\|define\) F77_USES_.*$\)/#  \1/p' $config_h_file

echo ""

$SED -n 's/#\(\(undef\|define\) F77_FUNC.*$\)/#  \1/p' $config_h_file

cat << EOF

#endif

#endif
EOF
