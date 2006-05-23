/*

Copyright (C) 1996, 1997 John W. Eaton

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
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#include "quit.h"

#define OCTAVE_CONCAT_FN2(T1, T2) \
  DEFNDCATOP_FN2 (T1 ## _ ## T2 ## _s_s, T1 ## _scalar, T2 ## _scalar, , T1 ## NDArray, T1 ## _array, T2 ## _array, concat) \
  DEFNDCATOP_FN2 (T1 ## _ ## T2 ## _s_m, T1 ## _scalar, T2 ## _matrix, , T1 ## NDArray, T1 ## _array, T2 ## _array, concat) \
  DEFNDCATOP_FN2 (T1 ## _ ## T2 ## _m_s, T1 ## _matrix, T2 ## _scalar, , T1 ## NDArray, T1 ## _array, T2 ## _array, concat) \
  DEFNDCATOP_FN2 (T1 ## _ ## T2 ## _m_m, T1 ## _matrix, T2 ## _matrix, , T1 ## NDArray, T1 ## _array, T2 ## _array, concat)

#define OCTAVE_INSTALL_CONCAT_FN2(T1, T2) \
  INSTALL_CATOP (octave_ ## T1 ## _scalar, octave_ ## T2 ## _scalar, T1 ## _ ## T2 ## _s_s) \
  INSTALL_CATOP (octave_ ## T1 ## _scalar, octave_ ## T2 ## _matrix, T1 ## _ ## T2 ## _s_m) \
  INSTALL_CATOP (octave_ ## T1 ## _matrix, octave_ ## T2 ## _scalar, T1 ## _ ## T2 ## _m_s) \
  INSTALL_CATOP (octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, T1 ## _ ## T2 ## _m_m)

#define OCTAVE_DOUBLE_INT_CONCAT_FN(TYPE) \
  DEFNDCATOP_FN2 (double ## _ ## TYPE ## _s_s, scalar, TYPE ## _scalar, TYPE ## NDArray, , array, TYPE ## _array, concat) \
  DEFNDCATOP_FN2 (double ## _ ## TYPE ## _s_m, scalar, TYPE ## _matrix, TYPE ## NDArray, , array, TYPE ## _array, concat) \
  DEFNDCATOP_FN2 (double ## _ ## TYPE ## _m_s, matrix, TYPE ## _scalar, TYPE ## NDArray, , array, TYPE ## _array, concat) \
  DEFNDCATOP_FN2 (double ## _ ## TYPE ## _m_m, matrix, TYPE ## _matrix, TYPE ## NDArray, , array, TYPE ## _array, concat)

#define OCTAVE_INSTALL_DOUBLE_INT_CONCAT_FN(TYPE) \
  INSTALL_CATOP (octave_scalar, octave_ ## TYPE ## _scalar, double ## _ ## TYPE ## _s_s) \
  INSTALL_CATOP (octave_scalar, octave_ ## TYPE ## _matrix, double ## _ ## TYPE ## _s_m) \
  INSTALL_CATOP (octave_matrix, octave_ ## TYPE ## _scalar, double ## _ ## TYPE ## _m_s) \
  INSTALL_CATOP (octave_matrix, octave_ ## TYPE ## _matrix, double ## _ ## TYPE ## _m_m)

#define OCTAVE_INT_DOUBLE_CONCAT_FN(TYPE) \
  DEFNDCATOP_FN2 (TYPE ## _ ## double ## _s_s, TYPE ## _scalar, scalar, , TYPE ## NDArray, TYPE ## _array, array, concat) \
  DEFNDCATOP_FN2 (TYPE ## _ ## double ## _s_m, TYPE ## _scalar, matrix, , TYPE ## NDArray, TYPE ## _array, array, concat) \
  DEFNDCATOP_FN2 (TYPE ## _ ## double ## _m_s, TYPE ## _matrix, scalar, , TYPE ## NDArray, TYPE ## _array, array, concat) \
  DEFNDCATOP_FN2 (TYPE ## _ ## double ## _m_m, TYPE ## _matrix, matrix, , TYPE ## NDArray, TYPE ## _array, array, concat)

#define OCTAVE_INSTALL_INT_DOUBLE_CONCAT_FN(TYPE) \
  INSTALL_CATOP (octave_ ## TYPE ## _scalar, octave_scalar, TYPE ## _ ## double ## _s_s) \
  INSTALL_CATOP (octave_ ## TYPE ## _scalar, octave_matrix, TYPE ## _ ## double ## _s_m) \
  INSTALL_CATOP (octave_ ## TYPE ## _matrix, octave_scalar, TYPE ## _ ## double ## _m_s) \
  INSTALL_CATOP (octave_ ## TYPE ## _matrix, octave_matrix, TYPE ## _ ## double ## _m_m)

// For compatibility, concatenation with a character always returns a
// character.

#define OCTAVE_CHAR_INT_CONCAT_FN(TYPE) \
  DEFNDCHARCATOP_FN (char ## _ ## TYPE ## _m_s, char_matrix, TYPE ## _scalar, concat) \
  DEFNDCHARCATOP_FN (char ## _ ## TYPE ## _m_m, char_matrix, TYPE ## _matrix, concat)

#define OCTAVE_INSTALL_CHAR_INT_CONCAT_FN(TYPE) \
  INSTALL_CATOP (octave_char_matrix_str, octave_ ## TYPE ## _scalar, char ## _ ## TYPE ## _m_s) \
  INSTALL_CATOP (octave_char_matrix_str, octave_ ## TYPE ## _matrix, char ## _ ## TYPE ## _m_m) \
  INSTALL_CATOP (octave_char_matrix_sq_str, octave_ ## TYPE ## _scalar, char ## _ ## TYPE ## _m_s) \
  INSTALL_CATOP (octave_char_matrix_sq_str, octave_ ## TYPE ## _matrix, char ## _ ## TYPE ## _m_m)

#define OCTAVE_INT_CHAR_CONCAT_FN(TYPE) \
  DEFNDCHARCATOP_FN (TYPE ## _ ## char ## _s_m, TYPE ## _scalar, char_matrix, concat) \
  DEFNDCHARCATOP_FN (TYPE ## _ ## char ## _m_m, TYPE ## _matrix, char_matrix, concat)

#define OCTAVE_INSTALL_INT_CHAR_CONCAT_FN(TYPE) \
  INSTALL_CATOP (octave_ ## TYPE ## _scalar, octave_char_matrix_str, TYPE ## _ ## char ## _s_m) \
  INSTALL_CATOP (octave_ ## TYPE ## _matrix, octave_char_matrix_str, TYPE ## _ ## char ## _m_m) \
  INSTALL_CATOP (octave_ ## TYPE ## _scalar, octave_char_matrix_sq_str, TYPE ## _ ## char ## _s_m) \
  INSTALL_CATOP (octave_ ## TYPE ## _matrix, octave_char_matrix_sq_str, TYPE ## _ ## char ## _m_m)

#define OCTAVE_CONCAT_FN(TYPE) \
  DEFNDCATOP_FN (TYPE ## _s_s, TYPE ## _scalar, TYPE ## _scalar, TYPE ## _array, TYPE ## _array, concat) \
  DEFNDCATOP_FN (TYPE ## _s_m, TYPE ## _scalar, TYPE ## _matrix, TYPE ## _array, TYPE ## _array, concat) \
  DEFNDCATOP_FN (TYPE ## _m_s, TYPE ## _matrix, TYPE ## _scalar, TYPE ## _array, TYPE ## _array, concat) \
  DEFNDCATOP_FN (TYPE ## _m_m, TYPE ## _matrix, TYPE ## _matrix, TYPE ## _array, TYPE ## _array, concat)

#define OCTAVE_INSTALL_CONCAT_FN(TYPE) \
  INSTALL_CATOP (octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _scalar, TYPE ## _s_s) \
  INSTALL_CATOP (octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _matrix, TYPE ## _s_m) \
  INSTALL_CATOP (octave_ ## TYPE ## _matrix, octave_ ## TYPE ## _scalar, TYPE ## _m_s) \
  INSTALL_CATOP (octave_ ## TYPE ## _matrix, octave_ ## TYPE ## _matrix, TYPE ## _m_m)

#define OCTAVE_S_INT_UNOPS(TYPE) \
  /* scalar unary ops. */  \
 \
  DEFUNOP_OP (s_not, TYPE ## _scalar, !) \
  DEFUNOP_OP (s_uplus, TYPE ## _scalar, /* no-op */) \
  DEFUNOP_OP (s_uminus, TYPE ## _scalar, -) \
  DEFUNOP_OP (s_transpose, TYPE ## _scalar, /* no-op */) \
  DEFUNOP_OP (s_hermitian, TYPE ## _scalar, /* no-op */) \
 \
  DEFNCUNOP_METHOD (s_incr, TYPE ## _scalar, increment) \
  DEFNCUNOP_METHOD (s_decr, TYPE ## _scalar, decrement)

#define OCTAVE_SS_INT_ARITH_OPS(PFX, T1, T2) \
  /* scalar by scalar ops. */ \
 \
  DEFBINOP_OP (PFX ## _add, T1 ## scalar, T2 ## scalar, +) \
  DEFBINOP_OP (PFX ## _sub, T1 ## scalar, T2 ## scalar, -) \
  DEFBINOP_OP (PFX ## _mul, T1 ## scalar, T2 ## scalar, *) \
 \
  DEFBINOP (PFX ## _div, T1 ## scalar, T2 ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## T1 ## scalar&, const octave_ ## T2 ## scalar&); \
 \
    if (! v2.T2 ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v1.T1 ## scalar_value () / v2.T2 ## scalar_value ()); \
  } \
 \
  DEFBINOP_FN (PFX ## _pow, T1 ## scalar, T2 ## scalar, xpow) \
 \
  DEFBINOP (PFX ## _ldiv, T1 ## scalar, T2 ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## T1 ## scalar&, const octave_ ## T2 ## scalar&); \
 \
    if (! v1.T1 ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v2.T2 ## scalar_value () / v1.T1 ## scalar_value ()); \
  } \
 \
  DEFBINOP_OP (PFX ## _el_mul, T1 ## scalar, T2 ## scalar, *) \
 \
  DEFBINOP (PFX ## _el_div, T1 ## scalar, T2 ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## T1 ## scalar&, const octave_ ## T2 ## scalar&); \
 \
    if (! v2.T2 ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v1.T1 ## scalar_value () / v2.T2 ## scalar_value ()); \
  } \
 \
  DEFBINOP_FN (PFX ## _el_pow, T1 ## scalar, T2 ## scalar, xpow) \
 \
  DEFBINOP (PFX ## _el_ldiv, T1 ## scalar, T2 ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## T1 ## scalar&, const octave_ ## T2 ## scalar&); \
 \
    if (! v1.T1 ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v2.T2 ## scalar_value () / v1.T1 ## scalar_value ()); \
  } \

#define OCTAVE_SS_INT_BOOL_OPS(PFX, T1, T2, Z1, Z2) \
  DEFBINOP (PFX ## _el_and, T2, T2) \
  { \
    CAST_BINOP_ARGS (const octave_ ## T1 ## scalar&, const octave_ ## T2 ## scalar&); \
 \
    return v1.T1 ## scalar_value () != Z1 && v2.T2 ## scalar_value () != Z2; \
  } \
 \
  DEFBINOP (PFX ## _el_or, T1, T2) \
  { \
    CAST_BINOP_ARGS (const octave_ ## T1 ## scalar&, const octave_ ## T2 ## scalar&); \
 \
    return v1.T1 ## scalar_value () != Z1 || v2.T2 ## scalar_value () != Z2; \
  }

#define OCTAVE_SS_INT_CMP_OPS(PFX, T1, T2) \
  DEFBINOP_OP (PFX ## _lt, T1 ## scalar, T2 ## scalar, <) \
  DEFBINOP_OP (PFX ## _le, T1 ## scalar, T2 ## scalar, <=) \
  DEFBINOP_OP (PFX ## _eq, T1 ## scalar, T2 ## scalar, ==) \
  DEFBINOP_OP (PFX ## _ge, T1 ## scalar, T2 ## scalar, >=) \
  DEFBINOP_OP (PFX ## _gt, T1 ## scalar, T2 ## scalar, >) \
  DEFBINOP_OP (PFX ## _ne, T1 ## scalar, T2 ## scalar, !=)

#define OCTAVE_SS_POW_OPS(T1, T2) \
  octave_value \
  xpow (const octave_ ## T1& a, const octave_ ## T2& b) \
  { \
    return pow (a, b); \
  } \
 \
  octave_value \
  xpow (const octave_ ## T1& a, double b) \
  { \
    return pow (a, b); \
  } \
 \
  octave_value \
  xpow (double a, const octave_ ## T1& b) \
  { \
    return pow (a, b); \
  }

#define OCTAVE_SS_INT_OPS(TYPE) \
  OCTAVE_S_INT_UNOPS (TYPE) \
  OCTAVE_SS_POW_OPS (TYPE, TYPE) \
  OCTAVE_SS_INT_ARITH_OPS (ss, TYPE ## _, TYPE ## _) \
  OCTAVE_SS_INT_ARITH_OPS (sx, TYPE ## _, ) \
  OCTAVE_SS_INT_ARITH_OPS (xs, , TYPE ## _) \
  OCTAVE_SS_INT_CMP_OPS (ss, TYPE ## _, TYPE ## _) \
  OCTAVE_SS_INT_CMP_OPS (sx, TYPE ## _, ) \
  OCTAVE_SS_INT_CMP_OPS (xs, , TYPE ## _) \
  OCTAVE_SS_INT_BOOL_OPS (ss, TYPE ## _, TYPE ## _, octave_ ## TYPE (0), octave_ ## TYPE (0)) \
  OCTAVE_SS_INT_BOOL_OPS (sx, TYPE ## _, , octave_ ## TYPE (0), 0) \
  OCTAVE_SS_INT_BOOL_OPS (xs, , TYPE ## _, 0, octave_ ## TYPE (0))

#define OCTAVE_SM_INT_ARITH_OPS(PFX, TS, TM) \
  /* scalar by matrix ops. */ \
 \
  DEFNDBINOP_OP (PFX ## _add, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, +) \
  DEFNDBINOP_OP (PFX ## _sub, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, -) \
  DEFNDBINOP_OP (PFX ## _mul, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, *) \
 \
  /* DEFBINOP (PFX ## _div, TS ## scalar, TM ## matrix) */ \
  /* { */ \
  /* CAST_BINOP_ARGS (const octave_ ## TS ## scalar&, const octave_ ## TM ## matrix&); */ \
  /* */ \
  /* Matrix m1 = v1.TM ## matrix_value (); */ \
  /* Matrix m2 = v2.TM ## matrix_value (); */ \
  /* */ \
  /* return octave_value (xdiv (m1, m2)); */ \
  /* } */ \
 \
  /* DEFBINOP_FN (PFX ## _pow, TS ## scalar, TM ## matrix, xpow) */ \
 \
  DEFBINOP (PFX ## _ldiv, TS ## scalar, TM ## matrix) \
  { \
    CAST_BINOP_ARGS (const octave_ ## TS ## scalar&, const octave_ ## TM ## matrix&); \
 \
    if (! v1.TS ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v2.TS ## scalar_value () / v1.TS ## scalar_value ()); \
  } \
 \
  DEFNDBINOP_OP (PFX ## _el_mul, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, *) \
  /* DEFNDBINOP_FN (PFX ## _el_div, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, x_el_div) */ \
  DEFNDBINOP_FN (PFX ## _el_pow, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, elem_xpow) \
 \
  DEFBINOP (PFX ## _el_ldiv, TS ## scalar, TM ## matrix) \
  { \
    CAST_BINOP_ARGS (const octave_ ## TS ## scalar&, const octave_ ## TM ## matrix&); \
 \
    if (! v1.TS ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v2.TM ## array_value () / v1.TS ## scalar_value ()); \
  }

#define OCTAVE_SM_INT_CMP_OPS(PFX, TS, TM) \
  DEFNDBINOP_FN (PFX ## _lt, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_lt) \
  DEFNDBINOP_FN (PFX ## _le, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_le) \
  DEFNDBINOP_FN (PFX ## _eq, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_eq) \
  DEFNDBINOP_FN (PFX ## _ge, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_ge) \
  DEFNDBINOP_FN (PFX ## _gt, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_gt) \
  DEFNDBINOP_FN (PFX ## _ne, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_ne)

#define OCTAVE_SM_INT_BOOL_OPS(PFX, TS, TM) \
  DEFNDBINOP_FN (PFX ## _el_and, TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_and) \
  DEFNDBINOP_FN (PFX ## _el_or,  TS ## scalar, TM ## matrix, TS ## scalar, TM ## array, mx_el_or)

#define OCTAVE_SM_POW_OPS(T1, T2) \
  octave_value \
  elem_xpow (const octave_ ## T1& a, const T2 ## NDArray& b) \
  { \
    T2 ## NDArray result (b.dims ()); \
    for (int i = 0; i < b.length (); i++) \
      { \
	OCTAVE_QUIT; \
	result (i) = pow (a, b(i)); \
      } \
    return octave_value (result); \
  }

#define OCTAVE_SM_CONV(TS, TM) \
  DEFCONV (TS ## s_ ## TM ## m_conv, TM ## scalar, TM ## matrix) \
  { \
    CAST_CONV_ARG (const octave_ ## TS ## scalar&); \
 \
    return new octave_ ## TM ## matrix (v.TM ## array_value ()); \
  }

#define OCTAVE_SM_INT_OPS(TYPE) \
  OCTAVE_SM_POW_OPS (TYPE, TYPE) \
  OCTAVE_SM_INT_ARITH_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_SM_INT_ARITH_OPS (xm, , TYPE ## _) \
  OCTAVE_SM_INT_CMP_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_SM_INT_CMP_OPS (xm, , TYPE ## _) \
  OCTAVE_SM_INT_CMP_OPS (smx, TYPE ## _, ) \
  OCTAVE_SM_INT_BOOL_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_SM_INT_BOOL_OPS (xm, , TYPE ## _) \
  OCTAVE_SM_INT_BOOL_OPS (smx, TYPE ## _, ) \
  OCTAVE_SM_CONV (TYPE ## _, TYPE ## _) \
  OCTAVE_SM_CONV (TYPE ## _, complex_)

#define OCTAVE_MS_INT_ARITH_OPS(PFX, TM, TS) \
  /* matrix by scalar ops. */ \
 \
  DEFNDBINOP_OP (PFX ## _add, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, +) \
  DEFNDBINOP_OP (PFX ## _sub, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, -) \
  DEFNDBINOP_OP (PFX ## _mul, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, *) \
 \
  DEFBINOP (PFX ## _div, TM ## matrix, TS ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## TM ## matrix&, const octave_ ## TS ## scalar&); \
 \
    if (! v2.TS ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v1.TM ## array_value () / v2.TS ## scalar_value ()); \
  } \
 \
  /* DEFBINOP_FN (PFX ## _pow, TM ## matrix, TS ## scalar, xpow) */ \
 \
  /* DEFBINOP (PFX ## _ldiv, TM ## matrix, TS ## scalar) */ \
  /* { */ \
  /* CAST_BINOP_ARGS (const octave_ ## TM ## matrix&, const octave_ ## TS ## scalar&); */ \
  /* */ \
  /* Matrix m1 = v1.TM ## matrix_value (); */ \
  /* Matrix m2 = v2.TM ## matrix_value (); */ \
  /* */ \
  /* return octave_value (xleftdiv (m1, m2)); */ \
  /* } */ \
 \
  DEFNDBINOP_OP (PFX ## _el_mul, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, *) \
 \
  DEFBINOP (PFX ## _el_div, TM ## matrix, TS ## scalar) \
  { \
    CAST_BINOP_ARGS (const octave_ ## TM ## matrix&, const octave_ ## TS ## scalar&); \
 \
    if (! v2.TS ## scalar_value ()) \
      gripe_divide_by_zero (); \
 \
    return octave_value (v1.TM ## array_value () / v2.TS ## scalar_value ()); \
  } \
 \
  DEFNDBINOP_FN (PFX ## _el_pow, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, elem_xpow) \
 \
  /* DEFBINOP (el_ldiv, TM ## matrix, TS ## scalar) */ \
  /* { */ \
  /* CAST_BINOP_ARGS (const octave_ ## TM ## matrix&, const octave_ ## TS ## scalar&); */ \
  /* */ \
  /* return x_el_div (v2.TM ## _ ## TS ## scalar_value (), v1.TM ## array_value ()); */ \
  /* } */

#define OCTAVE_MS_INT_CMP_OPS(PFX, TM, TS) \
  DEFNDBINOP_FN (PFX ## _lt, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_lt) \
  DEFNDBINOP_FN (PFX ## _le, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_le) \
  DEFNDBINOP_FN (PFX ## _eq, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_eq) \
  DEFNDBINOP_FN (PFX ## _ge, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_ge) \
  DEFNDBINOP_FN (PFX ## _gt, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_gt) \
  DEFNDBINOP_FN (PFX ## _ne, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_ne)

#define OCTAVE_MS_INT_BOOL_OPS(PFX, TM, TS) \
  DEFNDBINOP_FN (PFX ## _el_and, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_and) \
  DEFNDBINOP_FN (PFX ## _el_or, TM ## matrix, TS ## scalar, TM ## array, TS ## scalar, mx_el_or)

#define OCTAVE_MS_INT_ASSIGN_OPS(PFX, TM, TS, TE) \
  DEFNDASSIGNOP_FN (PFX ## _assign, TM ## matrix, TS ## scalar, TE ## array, assign)

#define OCTAVE_MS_POW_OPS(T1, T2) \
octave_value elem_xpow (T1 ## NDArray a, octave_ ## T2  b) \
{ \
  T1 ## NDArray result (a.dims ()); \
  for (int i = 0; i < a.length (); i++) \
    { \
      OCTAVE_QUIT; \
      result (i) = pow (a(i), b);		\
    } \
  return octave_value (result); \
}

#define OCTAVE_MS_INT_OPS(TYPE) \
  OCTAVE_MS_POW_OPS (TYPE, TYPE) \
  OCTAVE_MS_INT_ARITH_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_MS_INT_ARITH_OPS (mx, TYPE ## _, ) \
  OCTAVE_MS_INT_CMP_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_MS_INT_CMP_OPS (mx, TYPE ## _, ) \
  OCTAVE_MS_INT_CMP_OPS (mxs, , TYPE ## _) \
  OCTAVE_MS_INT_BOOL_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_MS_INT_BOOL_OPS (mx, TYPE ## _, ) \
  OCTAVE_MS_INT_BOOL_OPS (mxs, , TYPE ## _) \
  OCTAVE_MS_INT_ASSIGN_OPS (ms, TYPE ## _, TYPE ## _, TYPE ## _) \
  OCTAVE_MS_INT_ASSIGN_OPS (mx, TYPE ## _, , )

#define OCTAVE_M_INT_UNOPS(TYPE) \
  /* matrix unary ops. */ \
 \
  DEFNDUNOP_OP (m_not, TYPE ## _matrix, TYPE ## _array, !) \
  DEFNDUNOP_OP (m_uplus, TYPE ## _matrix, TYPE ## _array, /* no-op */) \
  DEFNDUNOP_OP (m_uminus, TYPE ## _matrix, TYPE ## _array, -) \
 \
  DEFUNOP (m_transpose, TYPE ## _matrix) \
  { \
    CAST_UNOP_ARG (const octave_ ## TYPE ## _matrix&); \
 \
    if (v.ndims () > 2) \
      { \
	error ("transpose not defined for N-d objects"); \
	return octave_value (); \
      } \
    else \
      return octave_value (v.TYPE ## _array_value().transpose ()); \
  } \
 \
  DEFNCUNOP_METHOD (m_incr, TYPE ## _matrix, increment) \
  DEFNCUNOP_METHOD (m_decr, TYPE ## _matrix, decrement)

#define OCTAVE_MM_INT_ARITH_OPS(T1, T2) \
  /* matrix by matrix ops. */ \
 \
  DEFNDBINOP_OP (mm_add, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, +) \
  DEFNDBINOP_OP (mm_sub, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, -) \
 \
  /* DEFBINOP_OP (mm_mul, T1 ## _matrix, T2 ## _matrix, *) */ \
  /* DEFBINOP_FN (mm_div, T1 ## _matrix, T2 ## _matrix, xdiv) */ \
 \
  DEFBINOPX (mm_pow, T1 ## _matrix, T2 ## _matrix) \
  { \
    error ("can't do A ^ B for A and B both matrices"); \
    return octave_value (); \
  } \
 \
  /* DEFBINOP_FN (ldiv, T1 ## _matrix, T2 ## _matrix, xleftdiv) */ \
 \
  DEFNDBINOP_FN (mm_el_mul, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, product) \
 \
  DEFNDBINOP_FN (mm_el_div, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, quotient) \
 \
  DEFNDBINOP_FN (mm_el_pow, T1 ## _matrix, T2 ## _matrix, T1 ## _array, T2 ## _array, elem_xpow) \
 \
  /* DEFBINOP (mm_el_ldiv, T1 ## _matrix, T2 ## _matrix) */ \
  /* { */ \
  /* CAST_BINOP_ARGS (const octavematrix&, const octavematrix&); */ \
  /* */ \
  /* return octave_value (quotient (v2.array_value (), v1.array_value ())); */ \
  /* } */

#define OCTAVE_MM_INT_CMP_OPS(PFX, T1, T2) \
  DEFNDBINOP_FN (PFX ## _lt, T1 ## matrix, T2 ## matrix, T1 ## array, T2 ## array, mx_el_lt) \
  DEFNDBINOP_FN (PFX ## _le, T1 ## matrix, T2 ## matrix, T1 ## array, T2 ## array, mx_el_le) \
  DEFNDBINOP_FN (PFX ## _eq, T1 ## matrix, T2 ## matrix, T1 ## array, T2 ## array, mx_el_eq) \
  DEFNDBINOP_FN (PFX ## _ge, T1 ## matrix, T2 ## matrix, T1 ## array, T2 ## array, mx_el_ge) \
  DEFNDBINOP_FN (PFX ## _gt, T1 ## matrix, T2 ## matrix, T1 ## array, T2 ## array, mx_el_gt) \
  DEFNDBINOP_FN (PFX ## _ne, T1 ## matrix, T2 ## matrix, T1 ## array, T2 ## array, mx_el_ne)

#define OCTAVE_MM_INT_BOOL_OPS(PFX, T1, T2) \
  DEFNDBINOP_FN (PFX ## _el_and, T1 ## matrix, T2 ## matrix, T1 ## array, T2 ## array, mx_el_and) \
  DEFNDBINOP_FN (PFX ## _el_or,  T1 ## matrix, T2 ## matrix, T1 ## array, T2 ## array, mx_el_or)

#define OCTAVE_MM_INT_ASSIGN_OPS(PFX, TLHS, TRHS, TE) \
  DEFNDASSIGNOP_FN (PFX ## _assign, TLHS ## matrix, TRHS ## matrix, TE ## array, assign)

#define OCTAVE_MM_POW_OPS(T1, T2) \
  octave_value \
  elem_xpow (const T1 ## NDArray& a, const T2 ## NDArray& b) \
  { \
    dim_vector a_dims = a.dims (); \
    dim_vector b_dims = b.dims (); \
    if (a_dims != b_dims) \
      { \
	gripe_nonconformant ("operator .^", a_dims, b_dims); \
	return octave_value (); \
      } \
    T1 ## NDArray result (a_dims); \
    for (int i = 0; i < a.length (); i++) \
      { \
	OCTAVE_QUIT; \
	result (i) = pow (a(i), b(i)); \
      } \
    return octave_value (result); \
  }

#define OCTAVE_MM_CONV(T1, T2) \
  DEFCONV (T1 ## m_ ## T2 ## m_conv, T1 ## matrix, T2 ## matrix) \
  { \
    CAST_CONV_ARG (const octave_ ## T1 ## matrix&); \
 \
    return new octave_ ## T2 ## matrix (v.T2 ## array_value ()); \
  }

#define OCTAVE_MM_INT_OPS(TYPE) \
  OCTAVE_M_INT_UNOPS (TYPE) \
  OCTAVE_MM_POW_OPS (TYPE, TYPE) \
  OCTAVE_MM_INT_ARITH_OPS (TYPE, TYPE) \
  OCTAVE_MM_INT_CMP_OPS (mm, TYPE ## _, TYPE ## _) \
  OCTAVE_MM_INT_CMP_OPS (mmx, TYPE ## _, ) \
  OCTAVE_MM_INT_CMP_OPS (mxm, , TYPE ## _) \
  OCTAVE_MM_INT_BOOL_OPS (mm, TYPE ## _, TYPE ## _) \
  OCTAVE_MM_INT_BOOL_OPS (mmx, TYPE ## _, ) \
  OCTAVE_MM_INT_BOOL_OPS (mxm, , TYPE ## _) \
  OCTAVE_MM_INT_ASSIGN_OPS (mm, TYPE ## _, TYPE ## _, TYPE ## _) \
  OCTAVE_MM_INT_ASSIGN_OPS (mmx, TYPE ## _, , ) \
  OCTAVE_MM_CONV(TYPE ## _, complex_)

#define OCTAVE_RE_INT_ASSIGN_OPS(TYPE) \
  DEFNDASSIGNOP_FN (TYPE ## ms_assign, matrix, TYPE ## _scalar, array, assign) \
  DEFNDASSIGNOP_FN (TYPE ## mm_assign, matrix, TYPE ## _matrix, array, assign)

#define OCTAVE_CX_INT_ASSIGN_OPS(TYPE) \
  DEFNDASSIGNOP_FN (TYPE ## cms_assign, complex_matrix, TYPE ## _scalar, complex_array, assign) \
  DEFNDASSIGNOP_FN (TYPE ## cmm_assign, complex_matrix, TYPE ## _matrix, complex_array, assign)

#define OCTAVE_INT_OPS(TYPE) \
  OCTAVE_SS_INT_OPS (TYPE) \
  OCTAVE_SM_INT_OPS (TYPE) \
  OCTAVE_MS_INT_OPS (TYPE) \
  OCTAVE_MM_INT_OPS (TYPE) \
  OCTAVE_CONCAT_FN (TYPE) \
  OCTAVE_RE_INT_ASSIGN_OPS (TYPE) \
  OCTAVE_CX_INT_ASSIGN_OPS (TYPE)

#define OCTAVE_INSTALL_S_INT_UNOPS(TYPE) \
  INSTALL_UNOP (op_not, octave_ ## TYPE ## _scalar, s_not); \
  INSTALL_UNOP (op_uplus, octave_ ## TYPE ## _scalar, s_uplus); \
  INSTALL_UNOP (op_uminus, octave_ ## TYPE ## _scalar, s_uminus); \
  INSTALL_UNOP (op_transpose, octave_ ## TYPE ## _scalar, s_transpose); \
  INSTALL_UNOP (op_hermitian, octave_ ## TYPE ## _scalar, s_hermitian); \
 \
  INSTALL_NCUNOP (op_incr, octave_ ## TYPE ## _scalar, s_incr); \
  INSTALL_NCUNOP (op_decr, octave_ ## TYPE ## _scalar, s_decr);

#define OCTAVE_INSTALL_SS_INT_ARITH_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_add, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _add); \
  INSTALL_BINOP (op_sub, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _sub); \
  INSTALL_BINOP (op_mul, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _mul); \
  INSTALL_BINOP (op_div, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _div); \
  INSTALL_BINOP (op_pow, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _pow); \
  INSTALL_BINOP (op_ldiv, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _ldiv); \
  INSTALL_BINOP (op_el_mul, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_mul); \
  INSTALL_BINOP (op_el_div, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_div); \
  INSTALL_BINOP (op_el_pow, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_pow); \
  INSTALL_BINOP (op_el_ldiv, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_ldiv);

#define OCTAVE_INSTALL_SS_INT_CMP_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_lt, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _lt); \
  INSTALL_BINOP (op_le, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _le); \
  INSTALL_BINOP (op_eq, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _eq); \
  INSTALL_BINOP (op_ge, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _ge); \
  INSTALL_BINOP (op_gt, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _gt); \
  INSTALL_BINOP (op_ne, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _ne);

#define OCTAVE_INSTALL_SS_INT_BOOL_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_el_and, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_and); \
  INSTALL_BINOP (op_el_or, octave_ ## T1 ## scalar, octave_ ## T2 ## scalar, PFX ## _el_or);

#define OCTAVE_INSTALL_SS_INT_OPS(TYPE) \
  OCTAVE_INSTALL_S_INT_UNOPS (TYPE) \
  OCTAVE_INSTALL_SS_INT_ARITH_OPS (ss, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SS_INT_ARITH_OPS (sx, TYPE ## _, ) \
  OCTAVE_INSTALL_SS_INT_ARITH_OPS (xs, , TYPE ## _) \
  OCTAVE_INSTALL_SS_INT_CMP_OPS (ss, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SS_INT_CMP_OPS (sx, TYPE ## _, ) \
  OCTAVE_INSTALL_SS_INT_CMP_OPS (xs, , TYPE ## _) \
  OCTAVE_INSTALL_SS_INT_BOOL_OPS (ss, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SS_INT_BOOL_OPS (sx, TYPE ## _, ) \
  OCTAVE_INSTALL_SS_INT_BOOL_OPS (xs, , TYPE ## _) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _matrix) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_scalar, octave_ ## TYPE ## _matrix) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_complex_scalar, octave_complex_matrix)

#define OCTAVE_INSTALL_SM_INT_ARITH_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_add, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _add); \
  INSTALL_BINOP (op_sub, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _sub); \
  INSTALL_BINOP (op_mul, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _mul); \
  /* INSTALL_BINOP (op_div, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _div); */ \
  /* INSTALL_BINOP (op_pow, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _pow); */ \
  INSTALL_BINOP (op_ldiv, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _ldiv); \
  INSTALL_BINOP (op_el_mul, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_mul); \
  /* INSTALL_BINOP (op_el_div, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_div); */ \
  INSTALL_BINOP (op_el_pow, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_pow); \
  INSTALL_BINOP (op_el_ldiv, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_ldiv);

#define OCTAVE_INSTALL_SM_INT_CMP_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_lt, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _lt); \
  INSTALL_BINOP (op_le, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _le); \
  INSTALL_BINOP (op_eq, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _eq); \
  INSTALL_BINOP (op_ge, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _ge); \
  INSTALL_BINOP (op_gt, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _gt); \
  INSTALL_BINOP (op_ne, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _ne);

#define OCTAVE_INSTALL_SM_INT_BOOL_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_el_and, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_and); \
  INSTALL_BINOP (op_el_or, octave_ ## T1 ## scalar, octave_ ## T2 ## matrix, PFX ## _el_or);

#define OCTAVE_INSTALL_SM_INT_OPS(TYPE) \
  OCTAVE_INSTALL_SM_INT_ARITH_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SM_INT_ARITH_OPS (xm, , TYPE ## _) \
  OCTAVE_INSTALL_SM_INT_CMP_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SM_INT_CMP_OPS (xm, , TYPE ## _) \
  OCTAVE_INSTALL_SM_INT_CMP_OPS (smx, TYPE ## _, ) \
  OCTAVE_INSTALL_SM_INT_BOOL_OPS (sm, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_SM_INT_BOOL_OPS (xm, , TYPE ## _) \
  OCTAVE_INSTALL_SM_INT_BOOL_OPS (smx, TYPE ## _, ) \
  INSTALL_WIDENOP (octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _matrix, TYPE ## _s_ ## TYPE ## _m_conv) \
  INSTALL_WIDENOP (octave_ ## TYPE ## _scalar, octave_complex_matrix, TYPE ## _s_complex_m_conv) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_ ## TYPE ## _matrix, octave_ ## TYPE ## _matrix) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_matrix, octave_ ## TYPE ## _matrix) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _scalar, octave_complex_matrix, octave_complex_matrix)

#define OCTAVE_INSTALL_MS_INT_ARITH_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_add, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _add); \
  INSTALL_BINOP (op_sub, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _sub); \
  INSTALL_BINOP (op_mul, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _mul); \
  INSTALL_BINOP (op_div, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _div); \
  /* INSTALL_BINOP (op_pow, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _pow); */ \
  /* INSTALL_BINOP (op_ldiv, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _ldiv); */ \
 \
  INSTALL_BINOP (op_el_mul, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_mul); \
  INSTALL_BINOP (op_el_div, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_div); \
  INSTALL_BINOP (op_el_pow, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_pow); \
  /* INSTALL_BINOP (op_el_ldiv, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_ldiv); */

#define OCTAVE_INSTALL_MS_INT_CMP_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_lt, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _lt); \
  INSTALL_BINOP (op_le, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _le); \
  INSTALL_BINOP (op_eq, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _eq); \
  INSTALL_BINOP (op_ge, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _ge); \
  INSTALL_BINOP (op_gt, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _gt); \
  INSTALL_BINOP (op_ne, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _ne);

#define OCTAVE_INSTALL_MS_INT_BOOL_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_el_and, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_and); \
  INSTALL_BINOP (op_el_or, octave_ ## T1 ## matrix, octave_ ## T2 ## scalar, PFX ## _el_or);

#define OCTAVE_INSTALL_MS_INT_ASSIGN_OPS(PFX, TLHS, TRHS) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_ ## TLHS ## matrix, octave_ ## TRHS ## scalar, PFX ## _assign)

#define OCTAVE_INSTALL_MS_INT_OPS(TYPE) \
  OCTAVE_INSTALL_MS_INT_ARITH_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MS_INT_ARITH_OPS (mx, TYPE ## _, ) \
  OCTAVE_INSTALL_MS_INT_CMP_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MS_INT_CMP_OPS (mx, TYPE ## _, ) \
  OCTAVE_INSTALL_MS_INT_CMP_OPS (mxs, , TYPE ## _) \
  OCTAVE_INSTALL_MS_INT_BOOL_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MS_INT_BOOL_OPS (mx, TYPE ## _, ) \
  OCTAVE_INSTALL_MS_INT_BOOL_OPS (mxs, , TYPE ## _) \
  OCTAVE_INSTALL_MS_INT_ASSIGN_OPS (ms, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MS_INT_ASSIGN_OPS (mx, TYPE ## _, ) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _matrix, octave_complex_scalar, octave_complex_matrix)

#define OCTAVE_INSTALL_M_INT_UNOPS(TYPE) \
  INSTALL_UNOP (op_not, octave_ ## TYPE ## _matrix, m_not); \
  INSTALL_UNOP (op_uplus, octave_ ## TYPE ## _matrix, m_uplus); \
  INSTALL_UNOP (op_uminus, octave_ ## TYPE ## _matrix, m_uminus); \
  INSTALL_UNOP (op_transpose, octave_ ## TYPE ## _matrix, m_transpose); \
  INSTALL_UNOP (op_hermitian, octave_ ## TYPE ## _matrix, m_transpose); \
 \
  INSTALL_NCUNOP (op_incr, octave_ ## TYPE ## _matrix, m_incr); \
  INSTALL_NCUNOP (op_decr, octave_ ## TYPE ## _matrix, m_decr);

#define OCTAVE_INSTALL_MM_INT_ARITH_OPS(T1, T2) \
  INSTALL_BINOP (op_add, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_add); \
  INSTALL_BINOP (op_sub, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_sub); \
  /* INSTALL_BINOP (op_mul, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_mul); */ \
  /* INSTALL_BINOP (op_div, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_div); */ \
  INSTALL_BINOP (op_pow, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_pow); \
  /* INSTALL_BINOP (op_ldiv, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_ldiv); */ \
  INSTALL_BINOP (op_el_mul, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_el_mul); \
  INSTALL_BINOP (op_el_div, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_el_div); \
  INSTALL_BINOP (op_el_pow, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_el_pow); \
  /* INSTALL_BINOP (op_el_ldiv, octave_ ## T1 ## _matrix, octave_ ## T2 ## _matrix, mm_el_ldiv); */

#define OCTAVE_INSTALL_MM_INT_CMP_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_lt, octave_ ## T1 ## matrix, octave_ ## T2 ## matrix, PFX ## _lt); \
  INSTALL_BINOP (op_le, octave_ ## T1 ## matrix, octave_ ## T2 ## matrix, PFX ## _le); \
  INSTALL_BINOP (op_eq, octave_ ## T1 ## matrix, octave_ ## T2 ## matrix, PFX ## _eq); \
  INSTALL_BINOP (op_ge, octave_ ## T1 ## matrix, octave_ ## T2 ## matrix, PFX ## _ge); \
  INSTALL_BINOP (op_gt, octave_ ## T1 ## matrix, octave_ ## T2 ## matrix, PFX ## _gt); \
  INSTALL_BINOP (op_ne, octave_ ## T1 ## matrix, octave_ ## T2 ## matrix, PFX ## _ne);

#define OCTAVE_INSTALL_MM_INT_BOOL_OPS(PFX, T1, T2) \
  INSTALL_BINOP (op_el_and, octave_ ## T1 ## matrix, octave_ ## T2 ## matrix, PFX ## _el_and); \
  INSTALL_BINOP (op_el_or, octave_ ## T1 ## matrix, octave_ ## T2 ## matrix, PFX ## _el_or);

#define OCTAVE_INSTALL_MM_INT_ASSIGN_OPS(PFX, TLHS, TRHS) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_ ## TLHS ## matrix, octave_ ## TRHS ## matrix, PFX ## _assign)

#define OCTAVE_INSTALL_MM_INT_OPS(TYPE) \
  OCTAVE_INSTALL_M_INT_UNOPS (TYPE) \
  OCTAVE_INSTALL_MM_INT_ARITH_OPS (TYPE, TYPE) \
  OCTAVE_INSTALL_MM_INT_CMP_OPS (mm, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MM_INT_CMP_OPS (mmx, TYPE ## _, ) \
  OCTAVE_INSTALL_MM_INT_CMP_OPS (mxm, , TYPE ## _) \
  OCTAVE_INSTALL_MM_INT_BOOL_OPS (mm, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MM_INT_BOOL_OPS (mmx, TYPE ## _, ) \
  OCTAVE_INSTALL_MM_INT_BOOL_OPS (mxm, , TYPE ## _) \
  OCTAVE_INSTALL_MM_INT_ASSIGN_OPS (mm, TYPE ## _, TYPE ## _) \
  OCTAVE_INSTALL_MM_INT_ASSIGN_OPS (mmx, TYPE ## _, ) \
  INSTALL_WIDENOP (octave_ ## TYPE ## _matrix, octave_complex_matrix, TYPE ## _m_complex_m_conv) \
  INSTALL_ASSIGNCONV (octave_ ## TYPE ## _matrix, octave_complex_matrix, octave_complex_matrix)

#define OCTAVE_INSTALL_RE_INT_ASSIGN_OPS(TYPE) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_matrix, octave_ ## TYPE ## _scalar, TYPE ## ms_assign) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_matrix, octave_ ## TYPE ## _matrix, TYPE ## mm_assign) \
  INSTALL_ASSIGNCONV (octave_scalar, octave_ ## TYPE ## _scalar, octave_matrix) \
  INSTALL_ASSIGNCONV (octave_scalar, octave_ ## TYPE ## _matrix, octave_matrix)

#define OCTAVE_INSTALL_CX_INT_ASSIGN_OPS(TYPE) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_complex_matrix, octave_ ## TYPE ## _scalar, TYPE ## cms_assign) \
  INSTALL_ASSIGNOP (op_asn_eq, octave_complex_matrix, octave_ ## TYPE ## _matrix, TYPE ## cmm_assign) \
  INSTALL_ASSIGNCONV (octave_complex_scalar, octave_ ## TYPE ## _scalar, octave_complex_matrix) \
  INSTALL_ASSIGNCONV (octave_complex_scalar, octave_ ## TYPE ## _matrix, octave_complex_matrix)

#define OCTAVE_INSTALL_INT_OPS(TYPE) \
  OCTAVE_INSTALL_SS_INT_OPS (TYPE) \
  OCTAVE_INSTALL_SM_INT_OPS (TYPE) \
  OCTAVE_INSTALL_MS_INT_OPS (TYPE) \
  OCTAVE_INSTALL_MM_INT_OPS (TYPE) \
  OCTAVE_INSTALL_CONCAT_FN (TYPE) \
  OCTAVE_INSTALL_RE_INT_ASSIGN_OPS (TYPE) \
  OCTAVE_INSTALL_CX_INT_ASSIGN_OPS (TYPE)

#define OCTAVE_INSTALL_SM_INT_ASSIGNCONV(TLHS, TRHS) \
  INSTALL_ASSIGNCONV (octave_ ## TLHS ## _scalar, octave_ ## TRHS ## _scalar, octave_ ## TLHS ## _matrix) \
  INSTALL_ASSIGNCONV (octave_ ## TLHS ## _scalar, octave_ ## TRHS ## _matrix, octave_ ## TLHS ## _matrix)

#define OCTAVE_MIXED_INT_CMP_OPS(T1, T2) \
  OCTAVE_SS_INT_CMP_OPS (T1 ## _ ## T2 ## _ss, T1 ## _, T2 ## _) \
  OCTAVE_SM_INT_CMP_OPS (T1 ## _ ## T2 ## _sm, T1 ## _, T2 ## _) \
  OCTAVE_MS_INT_CMP_OPS (T1 ## _ ## T2 ## _ms, T1 ## _, T2 ## _) \
  OCTAVE_MM_INT_CMP_OPS (T1 ## _ ## T2 ## _mm, T1 ## _, T2 ## _)

#define OCTAVE_INSTALL_MIXED_INT_CMP_OPS(T1, T2) \
  OCTAVE_INSTALL_SS_INT_CMP_OPS (T1 ## _ ## T2 ## _ss, T1 ## _, T2 ## _) \
  OCTAVE_INSTALL_SM_INT_CMP_OPS (T1 ## _ ## T2 ## _sm, T1 ## _, T2 ## _) \
  OCTAVE_INSTALL_MS_INT_CMP_OPS (T1 ## _ ## T2 ## _ms, T1 ## _, T2 ## _) \
  OCTAVE_INSTALL_MM_INT_CMP_OPS (T1 ## _ ## T2 ## _mm, T1 ## _, T2 ## _)

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
