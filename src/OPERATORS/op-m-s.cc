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
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#if defined (__GNUG__) && defined (USE_PRAGMA_INTERFACE_IMPLEMENTATION)
#pragma implementation
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gripes.h"
#include "oct-obj.h"
#include "ov.h"
#include "ov-re-mat.h"
#include "ov-scalar.h"
#include "ov-typeinfo.h"
#include "ops.h"
#include "xdiv.h"
#include "xpow.h"

// matrix by scalar ops.

DEFNDBINOP_OP (add, matrix, scalar, array, scalar, +)
DEFNDBINOP_OP (sub, matrix, scalar, array, scalar, -)
DEFNDBINOP_OP (mul, matrix, scalar, array, scalar, *)

DEFBINOP (div, matrix, scalar)
{
  CAST_BINOP_ARGS (const octave_matrix&, const octave_scalar&);

  double d = v2.double_value ();

  if (d == 0.0)
    gripe_divide_by_zero ();

  return octave_value (v1.array_value () / d);
}

DEFBINOP_FN (pow, matrix, scalar, xpow)

DEFBINOP (ldiv, matrix, scalar)
{
  CAST_BINOP_ARGS (const octave_matrix&, const octave_scalar&);

  Matrix m1 = v1.matrix_value ();
  Matrix m2 = v2.matrix_value ();

  return octave_value (xleftdiv (m1, m2));
}

DEFNDBINOP_FN (lt, matrix, scalar, array, scalar, mx_el_lt)
DEFNDBINOP_FN (le, matrix, scalar, array, scalar, mx_el_le)
DEFNDBINOP_FN (eq, matrix, scalar, array, scalar, mx_el_eq)
DEFNDBINOP_FN (ge, matrix, scalar, array, scalar, mx_el_ge)
DEFNDBINOP_FN (gt, matrix, scalar, array, scalar, mx_el_gt)
DEFNDBINOP_FN (ne, matrix, scalar, array, scalar, mx_el_ne)

DEFNDBINOP_OP (el_mul, matrix, scalar, array, scalar, *)

DEFBINOP (el_div, matrix, scalar)
{
  CAST_BINOP_ARGS (const octave_matrix&, const octave_scalar&);

  double d = v2.double_value ();

  if (d == 0.0)
    gripe_divide_by_zero ();

  return octave_value (v1.array_value () / d);
}

DEFNDBINOP_FN (el_pow, matrix, scalar, array, scalar, elem_xpow)

DEFBINOP (el_ldiv, matrix, scalar)
{
  CAST_BINOP_ARGS (const octave_matrix&, const octave_scalar&);

  return x_el_div (v2.double_value (), v1.array_value ());
}

DEFNDBINOP_FN (el_and, matrix, scalar, array, scalar, mx_el_and)
DEFNDBINOP_FN (el_or, matrix, scalar, array, scalar, mx_el_or)

DEFNDCATOP_FN (m_s, matrix, scalar, array, array, concat)

DEFNDASSIGNOP_FN (assign, matrix, scalar, array, assign)

void
install_m_s_ops (void)
{
  INSTALL_BINOP (op_add, octave_matrix, octave_scalar, add);
  INSTALL_BINOP (op_sub, octave_matrix, octave_scalar, sub);
  INSTALL_BINOP (op_mul, octave_matrix, octave_scalar, mul);
  INSTALL_BINOP (op_div, octave_matrix, octave_scalar, div);
  INSTALL_BINOP (op_pow, octave_matrix, octave_scalar, pow);
  INSTALL_BINOP (op_ldiv, octave_matrix, octave_scalar, ldiv);

  //  INSTALL_BINOP (op_lt, octave_matrix, octave_scalar, lt);

  octave_value_typeinfo::register_binary_op
    (octave_value::op_lt, octave_matrix::static_type_id (),
     octave_scalar::static_type_id (), oct_binop_lt);

  INSTALL_BINOP (op_le, octave_matrix, octave_scalar, le);
  INSTALL_BINOP (op_eq, octave_matrix, octave_scalar, eq);
  INSTALL_BINOP (op_ge, octave_matrix, octave_scalar, ge);
  INSTALL_BINOP (op_gt, octave_matrix, octave_scalar, gt);
  INSTALL_BINOP (op_ne, octave_matrix, octave_scalar, ne);
  INSTALL_BINOP (op_el_mul, octave_matrix, octave_scalar, el_mul);
  INSTALL_BINOP (op_el_div, octave_matrix, octave_scalar, el_div);
  INSTALL_BINOP (op_el_pow, octave_matrix, octave_scalar, el_pow);
  INSTALL_BINOP (op_el_ldiv, octave_matrix, octave_scalar, el_ldiv);
  INSTALL_BINOP (op_el_and, octave_matrix, octave_scalar, el_and);
  INSTALL_BINOP (op_el_or, octave_matrix, octave_scalar, el_or);

  INSTALL_CATOP (octave_matrix, octave_scalar, m_s);

  INSTALL_ASSIGNOP (op_asn_eq, octave_matrix, octave_scalar, assign);
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
