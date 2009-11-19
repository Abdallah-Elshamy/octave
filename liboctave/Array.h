// Template array classes
/*

Copyright (C) 2008, 2009 Jaroslav Hajek
Copyright (C) 1993, 1994, 1995, 1996, 1997, 2000, 2001, 2002, 2003,
              2004, 2005, 2006, 2007 John W. Eaton

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

#if !defined (octave_Array_h)
#define octave_Array_h 1

#include <cassert>
#include <cstddef>

#include <algorithm>
#include <iosfwd>

#include "dim-vector.h"
#include "idx-vector.h"
#include "lo-traits.h"
#include "lo-utils.h"
#include "oct-sort.h"
#include "quit.h"
#include "oct-mem.h"

// One dimensional array class.  Handles the reference counting for
// all the derived classes.

template <class T>
class
Array
{
protected:

  //--------------------------------------------------------------------
  // The real representation of all arrays.
  //--------------------------------------------------------------------

  class ArrayRep
  {
  public:

    T *data;
    octave_idx_type len;
    int count;

    ArrayRep (T *d, octave_idx_type l, bool copy = false) 
      : data (copy ? no_ctor_new<T> (l) : d), len (l), count (1) 
        { 
          if (copy)
            copy_or_memcpy (l, d, data);
        }

    ArrayRep (void) : data (0), len (0), count (1) { }

    explicit ArrayRep (octave_idx_type n) : data (no_ctor_new<T> (n)), len (n), count (1) { }

    explicit ArrayRep (octave_idx_type n, const T& val)
      : data (no_ctor_new<T> (n)), len (n), count (1)
      {
        fill_or_memset (n, val, data);
      }

    ArrayRep (const ArrayRep& a)
      : data (no_ctor_new<T> (a.len)), len (a.len), count (1)
      {
        copy_or_memcpy (a.len, a.data, data);
      }
 
    ~ArrayRep (void) { no_ctor_delete<T> (data); }

    octave_idx_type length (void) const { return len; }

  private:

    // No assignment!

    ArrayRep& operator = (const ArrayRep& a);
  };

  //--------------------------------------------------------------------

public:

  void make_unique (void)
    {
      if (rep->count > 1)
        {
          --rep->count;
          rep = new ArrayRep (slice_data, slice_len, true);
          slice_data = rep->data;
        }
    }

  typedef T element_type;

  typedef bool (*compare_fcn_type) (typename ref_param<T>::type,
				    typename ref_param<T>::type);

protected:

  typename Array<T>::ArrayRep *rep;

  dim_vector dimensions;

  // Rationale:
  // slice_data is a pointer to rep->data, denoting together with slice_len the
  // actual portion of the data referenced by this Array<T> object. This allows
  // to make shallow copies not only of a whole array, but also of contiguous
  // subranges. Every time rep is directly manipulated, slice_data and slice_len
  // need to be properly updated.

  T* slice_data;
  octave_idx_type slice_len;

  Array (T *d, octave_idx_type n)
    : rep (new typename Array<T>::ArrayRep (d, n)), dimensions (n) 
    { 
      slice_data = rep->data;
      slice_len = rep->len;
    }

  Array (T *d, const dim_vector& dv)
    : rep (new typename Array<T>::ArrayRep (d, dv.numel ())),
      dimensions (dv) 
    { 
      slice_data = rep->data;
      slice_len = rep->len;
    }

  // slice constructor
  Array (const Array<T>& a, const dim_vector& dv,
         octave_idx_type l, octave_idx_type u)
    : rep(a.rep), dimensions (dv)
    {
      rep->count++;
      slice_data = a.slice_data + l;
      slice_len = u - l;
    }

private:

  typename Array<T>::ArrayRep *nil_rep (void) const
    {
      static typename Array<T>::ArrayRep *nr
	= new typename Array<T>::ArrayRep ();

      return nr;
    }

  template <class U>
  T *
  coerce (const U *a, octave_idx_type len)
  {
    T *retval = no_ctor_new<T> (len);

    for (octave_idx_type i = 0; i < len; i++)
      retval[i] = T (a[i]);

    return retval;
  }

public:

  Array (void)
    : rep (nil_rep ()), dimensions () 
    { 
      rep->count++; 
      slice_data = rep->data;
      slice_len = rep->len;
    }

  explicit Array (octave_idx_type n)
    : rep (new typename Array<T>::ArrayRep (n)), dimensions (n) 
    { 
      slice_data = rep->data;
      slice_len = rep->len;
    }

  explicit Array (octave_idx_type n, const T& val)
    : rep (new typename Array<T>::ArrayRep (n)), dimensions (n)
    {
      slice_data = rep->data;
      slice_len = rep->len;
      fill (val);
    }

  // Type conversion case.
  template <class U>
  Array (const Array<U>& a)
    : rep (new typename Array<T>::ArrayRep (coerce (a.data (), a.length ()), a.length ())),
      dimensions (a.dims ())
    {
      slice_data = rep->data;
      slice_len = rep->len;
    }

  // No type conversion case.
  Array (const Array<T>& a)
    : rep (a.rep), dimensions (a.dimensions)
    {
      rep->count++;
      slice_data = a.slice_data;
      slice_len = a.slice_len;
    }

public:

  Array (const dim_vector& dv)
    : rep (new typename Array<T>::ArrayRep (dv.safe_numel ())),
      dimensions (dv) 
    { 
      slice_data = rep->data;
      slice_len = rep->len;
    }

  Array (const dim_vector& dv, const T& val)
    : rep (new typename Array<T>::ArrayRep (dv.safe_numel ())),
      dimensions (dv)
    {
      slice_data = rep->data;
      slice_len = rep->len;
      fill (val);
    }

  // Reshape constructor.
  Array (const Array<T>& a, const dim_vector& dv);

  ~Array (void)
    {
      if (--rep->count <= 0)
        delete rep;
    }

  Array<T>& operator = (const Array<T>& a)
    {
      if (this != &a)
        {
          if (--rep->count <= 0)
            delete rep;

          rep = a.rep;
          rep->count++;

          dimensions = a.dimensions;
          slice_data = a.slice_data;
          slice_len = a.slice_len;
        }

      return *this;
    }

  void fill (const T& val); 

  void clear (void);
  void clear (const dim_vector& dv);
  void clear (octave_idx_type n)
    { clear (dim_vector (n)); }

  void clear (octave_idx_type r, octave_idx_type c)
    { clear (dim_vector (r, c)); }

  octave_idx_type capacity (void) const { return slice_len; }
  octave_idx_type length (void) const { return capacity (); }
  octave_idx_type nelem (void) const { return capacity (); }
  octave_idx_type numel (void) const { return nelem (); }

  octave_idx_type dim1 (void) const { return dimensions(0); }
  octave_idx_type dim2 (void) const { return dimensions(1); }
  octave_idx_type dim3 (void) const { return dimensions(2); }

  octave_idx_type rows (void) const { return dim1 (); }
  octave_idx_type cols (void) const { return dim2 (); }
  octave_idx_type columns (void) const { return dim2 (); }
  octave_idx_type pages (void) const { return dim3 (); }

  size_t byte_size (void) const { return numel () * sizeof (T); }

  // Return a const-reference so that dims ()(i) works efficiently.
  const dim_vector& dims (void) const { return dimensions; }

  Array<T> squeeze (void) const;
  
  void chop_trailing_singletons (void) 
  { dimensions.chop_trailing_singletons (); }
  
  octave_idx_type compute_index (const Array<octave_idx_type>& ra_idx) const;

  T range_error (const char *fcn, octave_idx_type n) const;
  T& range_error (const char *fcn, octave_idx_type n);

  T range_error (const char *fcn, octave_idx_type i, octave_idx_type j) const;
  T& range_error (const char *fcn, octave_idx_type i, octave_idx_type j);

  T range_error (const char *fcn, octave_idx_type i, octave_idx_type j, octave_idx_type k) const;
  T& range_error (const char *fcn, octave_idx_type i, octave_idx_type j, octave_idx_type k);

  T range_error (const char *fcn, const Array<octave_idx_type>& ra_idx) const;
  T& range_error (const char *fcn, const Array<octave_idx_type>& ra_idx);

  // No checking, even for multiple references, ever.

  T& xelem (octave_idx_type n) { return slice_data [n]; }
  T xelem (octave_idx_type n) const { return slice_data [n]; }

  T& xelem (octave_idx_type i, octave_idx_type j) { return xelem (dim1()*j+i); }
  T xelem (octave_idx_type i, octave_idx_type j) const { return xelem (dim1()*j+i); }

  T& xelem (octave_idx_type i, octave_idx_type j, octave_idx_type k) { return xelem (i, dim2()*k+j); }
  T xelem (octave_idx_type i, octave_idx_type j, octave_idx_type k) const { return xelem (i, dim2()*k+j); }

  T& xelem (const Array<octave_idx_type>& ra_idx)
    { return xelem (compute_index (ra_idx)); }

  T xelem (const Array<octave_idx_type>& ra_idx) const
    { return xelem (compute_index (ra_idx)); }

  // FIXME -- would be nice to fix this so that we don't
  // unnecessarily force a copy, but that is not so easy, and I see no
  // clean way to do it.

  T& checkelem (octave_idx_type n)
    {
      if (n < 0 || n >= slice_len)
	return range_error ("T& Array<T>::checkelem", n);
      else
	{
	  make_unique ();
	  return xelem (n);
	}
    }

  T& checkelem (octave_idx_type i, octave_idx_type j)
    {
      if (i < 0 || j < 0 || i >= dim1 () || j >= dim2 ())
	return range_error ("T& Array<T>::checkelem", i, j);
      else
	return elem (dim1()*j+i);
    }

  T& checkelem (octave_idx_type i, octave_idx_type j, octave_idx_type k)
    {
      if (i < 0 || j < 0 || k < 0 || i >= dim1 () || j >= dim2 () || k >= dim3 ())
	return range_error ("T& Array<T>::checkelem", i, j, k);
      else
	return elem (i, dim2()*k+j);
    }

  T& checkelem (const Array<octave_idx_type>& ra_idx)
    {
      octave_idx_type i = compute_index (ra_idx);

      if (i < 0)
	return range_error ("T& Array<T>::checkelem", ra_idx);
      else
	return elem (i);
    }

  T& elem (octave_idx_type n)
    {
      make_unique ();
      return xelem (n);
    }

  T& elem (octave_idx_type i, octave_idx_type j) { return elem (dim1()*j+i); }

  T& elem (octave_idx_type i, octave_idx_type j, octave_idx_type k) { return elem (i, dim2()*k+j); }

  T& elem (const Array<octave_idx_type>& ra_idx)
    { return Array<T>::elem (compute_index (ra_idx)); }

#if defined (BOUNDS_CHECKING)
  T& operator () (octave_idx_type n) { return checkelem (n); }
  T& operator () (octave_idx_type i, octave_idx_type j) { return checkelem (i, j); }
  T& operator () (octave_idx_type i, octave_idx_type j, octave_idx_type k) { return checkelem (i, j, k); }
  T& operator () (const Array<octave_idx_type>& ra_idx) { return checkelem (ra_idx); }
#else
  T& operator () (octave_idx_type n) { return elem (n); }
  T& operator () (octave_idx_type i, octave_idx_type j) { return elem (i, j); }
  T& operator () (octave_idx_type i, octave_idx_type j, octave_idx_type k) { return elem (i, j, k); }
  T& operator () (const Array<octave_idx_type>& ra_idx) { return elem (ra_idx); }
#endif

  T checkelem (octave_idx_type n) const
    {
      if (n < 0 || n >= slice_len)
	return range_error ("T Array<T>::checkelem", n);
      else
	return xelem (n);
    }

  T checkelem (octave_idx_type i, octave_idx_type j) const
    {
      if (i < 0 || j < 0 || i >= dim1 () || j >= dim2 ())
	return range_error ("T Array<T>::checkelem", i, j);
      else
	return elem (dim1()*j+i);
    }

  T checkelem (octave_idx_type i, octave_idx_type j, octave_idx_type k) const
    {
      if (i < 0 || j < 0 || k < 0 || i >= dim1 () || j >= dim2 () || k >= dim3 ())
	return range_error ("T Array<T>::checkelem", i, j, k);
      else
	return Array<T>::elem (i, Array<T>::dim1()*k+j);
    }

  T checkelem (const Array<octave_idx_type>& ra_idx) const
    {
      octave_idx_type i = compute_index (ra_idx);

      if (i < 0)
	return range_error ("T Array<T>::checkelem", ra_idx);
      else
	return Array<T>::elem (i);
    }

  T elem (octave_idx_type n) const { return xelem (n); }

  T elem (octave_idx_type i, octave_idx_type j) const { return elem (dim1()*j+i); }

  T elem (octave_idx_type i, octave_idx_type j, octave_idx_type k) const { return elem (i, dim2()*k+j); }

  T elem (const Array<octave_idx_type>& ra_idx) const
    { return Array<T>::elem (compute_index (ra_idx)); }

#if defined (BOUNDS_CHECKING)
  T operator () (octave_idx_type n) const { return checkelem (n); }
  T operator () (octave_idx_type i, octave_idx_type j) const { return checkelem (i, j); }
  T operator () (octave_idx_type i, octave_idx_type j, octave_idx_type k) const { return checkelem (i, j, k); }
  T operator () (const Array<octave_idx_type>& ra_idx) const { return checkelem (ra_idx); }
#else
  T operator () (octave_idx_type n) const { return elem (n); }
  T operator () (octave_idx_type i, octave_idx_type j) const { return elem (i, j); }
  T operator () (octave_idx_type i, octave_idx_type j, octave_idx_type k) const { return elem (i, j, k); }
  T operator () (const Array<octave_idx_type>& ra_idx) const { return elem (ra_idx); }
#endif

  // Fast extractors. All of these produce shallow copies.
  // Warning: none of these do check bounds, unless BOUNDS_CHECKING is on!

  // Extract column: A(:,k+1).
  Array<T> column (octave_idx_type k) const;
  // Extract page: A(:,:,k+1).
  Array<T> page (octave_idx_type k) const;

  // Give this array as a column vector: A(:).
  Array<T> linearize (void) const;

  // Extract a slice from this array as a column vector: A(:)(lo+1:up).
  // Must be 0 <= lo && up <= numel. May be up < lo.
  Array<T> linear_slice (octave_idx_type lo, octave_idx_type up) const;

  Array<T> reshape (const dim_vector& new_dims) const
    { return Array<T> (*this, new_dims); }

  Array<T> permute (const Array<octave_idx_type>& vec, bool inv = false) const;
  Array<T> ipermute (const Array<octave_idx_type>& vec) const
    { return permute (vec, true); }

  bool is_square (void) const { return (dim1 () == dim2 ()); }

  bool is_empty (void) const { return numel () == 0; }

  bool is_vector (void) const { return dimensions.is_vector (); }

  Array<T> transpose (void) const;
  Array<T> hermitian (T (*fcn) (const T&) = 0) const;

  const T *data (void) const { return slice_data; }

  const T *fortran_vec (void) const { return data (); }

  T *fortran_vec (void);

  bool is_shared (void) { return rep->count > 1; }

  int ndims (void) const { return dimensions.length (); }

  void maybe_delete_dims (void);

  // Indexing without resizing.

  Array<T> index (const idx_vector& i) const;

  Array<T> index (const idx_vector& i, const idx_vector& j) const;

  Array<T> index (const Array<idx_vector>& ia) const;

  static T resize_fill_value (); 

  // Resizing (with fill).

  void resize_fill (octave_idx_type n, const T& rfv);

  void resize_fill (octave_idx_type nr, octave_idx_type nc, const T& rfv);

  void resize_fill (const dim_vector& dv, const T& rfv);

  // Resizing with default fill.
  // Rationale: 
  // These use the default fill value rather than leaving memory uninitialized.
  // Resizing without fill leaves the resulting array in a rather weird state,
  // where part of the data is initialized an part isn't.

  void resize (octave_idx_type n)
    { resize_fill (n, resize_fill_value ()); }

  // FIXME -- this method cannot be defined here because it would
  // clash with
  //
  //   void resize (octave_idx_type, const T&)
  //
  // (these become indistinguishable when T = octave_idx_type).
  // In the future, I think the resize (.., const T& rfv) overloads
  // should go away in favor of using resize_fill.

  // void resize (octave_idx_type nr, octave_idx_type nc)
  //  { resize_fill (nr, nc, resize_fill_value ()); }

  void resize (dim_vector dv)
    { resize_fill (dv, resize_fill_value ()); }

  // FIXME -- these are here for backward compatibility. They should
  // go away in favor of using resize_fill directly.
  void resize (octave_idx_type n, const T& rfv)
    { resize_fill (n, static_cast<T> (rfv)); }

  void resize (octave_idx_type nr, octave_idx_type nc, const T& rfv)
    { resize_fill (nr, nc, rfv); }

  void resize (dim_vector dv, const T& rfv)
    { resize_fill (dv, rfv); }

  // Indexing with possible resizing and fill
  // FIXME -- this is really a corner case, that should better be
  // handled directly in liboctinterp.

  Array<T> index (const idx_vector& i, bool resize_ok,
                  const T& rfv = resize_fill_value ()) const;

  Array<T> index (const idx_vector& i, const idx_vector& j, 
                  bool resize_ok, const T& rfv = resize_fill_value ()) const;

  Array<T> index (const Array<idx_vector>& ia,
                  bool resize_ok, const T& rfv = resize_fill_value ()) const;

  // Indexed assignment (always with resize & fill).

  void assign (const idx_vector& i, const Array<T>& rhs, 
               const T& rfv = resize_fill_value ());

  void assign (const idx_vector& i, const idx_vector& j, const Array<T>& rhs,
               const T& rfv = resize_fill_value ());

  void assign (const Array<idx_vector>& ia, const Array<T>& rhs,
               const T& rfv = resize_fill_value ());

  // Deleting elements.

  // A(I) = [] (with a single subscript)
  void delete_elements (const idx_vector& i);

  // A(:,...,I,...,:) = [] (>= 2 subscripts, one of them is non-colon)
  void delete_elements (int dim, const idx_vector& i);

  // Dispatcher to the above two.
  void delete_elements (const Array<idx_vector>& ia);

  // FIXME -- are these required? What exactly are they supposed to do?.

  Array<T>& insert (const Array<T>& a, octave_idx_type r, octave_idx_type c);
  Array<T>& insert2 (const Array<T>& a, octave_idx_type r, octave_idx_type c);
  Array<T>& insertN (const Array<T>& a, octave_idx_type r, octave_idx_type c);

  Array<T>& insert (const Array<T>& a, const Array<octave_idx_type>& idx);

  void maybe_economize (void)
    {
      if (rep->count == 1 && slice_len != rep->len)
        {
          ArrayRep *new_rep = new ArrayRep (slice_data, slice_len, true);
          delete rep;
          rep = new_rep;
          slice_data = rep->data;
        }
    }

  void print_info (std::ostream& os, const std::string& prefix) const;

  // Unsafe.  This function exists to support the MEX interface.
  // You should not use it anywhere else.
  void *mex_get_data (void) const { return const_cast<T *> (data ()); }

  Array<T> sort (int dim = 0, sortmode mode = ASCENDING) const;
  Array<T> sort (Array<octave_idx_type> &sidx, int dim = 0,
		 sortmode mode = ASCENDING) const;

  // Ordering is auto-detected or can be specified.
  sortmode is_sorted (sortmode mode = UNSORTED) const;

  // Sort by rows returns only indices.
  Array<octave_idx_type> sort_rows_idx (sortmode mode = ASCENDING) const;

  // Ordering is auto-detected or can be specified.
  sortmode is_sorted_rows (sortmode mode = UNSORTED) const;

  // Do a binary lookup in a sorted array.
  // Mode can be specified or is auto-detected by comparing 1st and last element.
  octave_idx_type lookup (const T& value, sortmode mode = UNSORTED) const;

  // Ditto, but for an array of values, specializing on long runs.
  // If linf is true, the leftmost interval is extended to infinity 
  // (indices will be >= 1).
  // If rinf is true, the rightmost interval is extended to infinity 
  // (indices will be <= length ()-1).
  Array<octave_idx_type> lookup (const Array<T>& values, sortmode mode = UNSORTED, 
                                 bool linf = false, bool rinf = false) const;

  // This looks up only exact matches, giving their indices. Non-exact matches get
  // the value -1.
  Array<octave_idx_type> lookupm (const Array<T>& values, sortmode mode = UNSORTED) const;

  // This looks up only exact matches, returning true/false if match.
  Array<bool> lookupb (const Array<T>& values, sortmode mode = UNSORTED) const;

  // Find indices of (at most n) nonzero elements. If n is specified, backward
  // specifies search from backward.
  Array<octave_idx_type> find (octave_idx_type n = -1, bool backward = false) const;

  // Returns the n-th element in increasing order, using the same ordering as
  // used for sort. n can either be a scalar index or a contiguous range.
  Array<T> nth_element (const idx_vector& n, int dim = 0) const;

  Array<T> diag (octave_idx_type k = 0) const;

  template <class U, class F>
  Array<U>
  map (F fcn) const
  {
    octave_idx_type len = length ();

    const T *m = data ();

    Array<U> result (dims ());
    U *p = result.fortran_vec ();

    octave_idx_type i;
    for (i = 0; i < len - 3; i += 4)
      {
	OCTAVE_QUIT;

        p[i] = fcn (m[i]);
        p[i+1] = fcn (m[i+1]);
        p[i+2] = fcn (m[i+2]);
        p[i+3] = fcn (m[i+3]);
      }

    OCTAVE_QUIT;

    for (; i < len; i++)
      p[i] = fcn (m[i]);

    return result;
  }

  // Overloads for function references.
  template <class U>
  Array<U>
  map (U (&fcn) (T)) const
  { return map<U, U (&) (T)> (fcn); }

  template <class U>
  Array<U>
  map (U (&fcn) (const T&)) const
  { return map<U, U (&) (const T&)> (fcn); }

  template <class U> friend class Array;

private:
  static void instantiation_guard ();
};

// This is a simple wrapper template that will subclass an Array<T> type or any
// later type derived from it and override the default non-const operator() to
// not check for the array's uniqueness. It is, however, the user's
// responsibility to ensure the array is actually unaliased whenever elements
// are accessed.

template<class ArrayClass>
class NoAlias : public ArrayClass
{
  typedef typename ArrayClass::element_type T;
public:
  NoAlias () : ArrayClass () { }

  // FIXME: this would be simpler once C++0x is available
  template <class X>
    explicit NoAlias (X x) : ArrayClass (x) { }

  template <class X, class Y>
    explicit NoAlias (X x, Y y) : ArrayClass (x, y) { }

  template <class X, class Y, class Z>
    explicit NoAlias (X x, Y y, Z z) : ArrayClass (x, y, z) { }

  T& operator () (octave_idx_type n)
    { return ArrayClass::xelem (n); }
  T& operator () (octave_idx_type i, octave_idx_type j)
    { return ArrayClass::xelem (i, j); }
  T& operator () (octave_idx_type i, octave_idx_type j, octave_idx_type k)
    { return ArrayClass::xelem (i, j, k); }
  T& operator () (const Array<octave_idx_type>& ra_idx)
    { return ArrayClass::xelem (ra_idx); }
};

template <class T>
std::ostream&
operator << (std::ostream& os, const Array<T>& a);

#endif

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
