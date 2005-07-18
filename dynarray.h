/* dynarray.h - dynamic array header 

  DESCRIPTION:

  The functions in this header can be used to create a system for
  handling dynamic arrays with just about any type as the base type.

  The following macros may be predefined:

  PROTOS_ONLY - Return prototypes only.
  Tctor - Use this function to construct a type T (Initialize pointers, etc.)
  Tdtor - Use this function to give an instance of type T its last rites
          (deallocate memory, etc.)
  Tassign - Use this function to assign an instance of a type T to another.
            (If pointers are involved, assigning may have some additional
                         steps beyond just using operator=.)
  
  The following macros must be predefined:

  T - the base type for the dynamic array
  TS - a "slug" prefix for all the functions in the header.

  Because this file may be needed more than once or #included more than once
  in the same file with different parameters, it does not have #include
  guards.

  COPYRIGHT NOTICE AND DISCLAIMER:

  Copyright (C) 2003 Gregory Pietsch

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>
#include <stdio.h>
#include "defines.h"

#define _VAL(y,x)       y ## x
#define _NM(y,x)        _VAL(y,x)

#ifdef PROTOS_ONLY

/* types */

/* _ARRAY_T: The array type.  */
typedef struct _NM (TS, _ARRAY_T)
{
  T *_Ptr;
  size_t _Len, _Res;
} _NM (TS, _ARRAY_T);


/* Function prototypes.  */
_NM (TS, _ARRAY_T) * _NM (TS, _create) (void);
     void
     _NM (TS, _destroy) (_NM (TS, _ARRAY_T) *);
     void
     _NM (TS, _ctor) (_NM (TS, _ARRAY_T) *);
     void
     _NM (TS, _ctor_with_size) (_NM (TS, _ARRAY_T) *, size_t, capacity);
     void
     _NM (TS, _copy_ctor) (_NM (TS, _ARRAY_T) *, _NM (TS, _ARRAY_T) *);
     void
     _NM (TS, _ctor_from_ptr) (_NM (TS, _ARRAY_T) *, T *, size_t);
     void
     _NM (TS, _dtor) (_NM (TS, _ARRAY_T) *);
_NM (TS, _ARRAY_T) * _NM (TS, _append) (_NM (TS, _ARRAY_T) *, T *, size_t,
                                        size_t);
_NM (TS, _ARRAY_T) * _NM (TS, _assign) (_NM (TS, _ARRAY_T) *, T *, size_t,
                                        size_t);
_NM (TS, _ARRAY_T) * _NM (TS, _insert) (_NM (TS, _ARRAY_T) *, size_t, T *,
                                        size_t, size_t);
_NM (TS, _ARRAY_T) * _NM (TS, _remove) (_NM (TS, _ARRAY_T) *, size_t, size_t);
_NM (TS, _ARRAY_T) * _NM (TS, _subarray) (_NM (TS, _ARRAY_T) *,
                                          _NM (TS, _ARRAY_T) *, size_t,
                                          size_t);
     void
     _NM (TS, _swap) (_NM (TS, _ARRAY_T) *, _NM (TS, _ARRAY_T) *);
     T *
     _NM (TS, _get_at) (_NM (TS, _ARRAY_T) *, size_t);
     void
     _NM (TS, _put_at) (_NM (TS, _ARRAY_T) *, size_t, T *);
     T *
     _NM (TS, _base) (_NM (TS, _ARRAY_T) *);
size_t
_NM (TS, _length) (_NM (TS, _ARRAY_T) *);
     void _NM (TS, _resize) (_NM (TS, _ARRAY_T) *, size_t, T *);
size_t
_NM (TS, _reserve) (_NM (TS, _ARRAY_T) *);
     void _NM (TS, _set_reserve) (_NM (TS, _ARRAY_T) *, size_t);

#else

#ifndef Tctor
#define Tctor(x)
#endif
#ifndef Tdtor
#define Tdtor(x)
#endif
#ifndef Tassign
#define Tassign(x,y)    (*(x) = *(y))
#endif

/* functions */

static void _NM (TS, _Xinv) (void)
{
  fputs ("Invalid dynamic array argument\n", stderr);
  abort ();
}

static void _NM (TS, _Xlen) (void)
{
  fputs ("Length error: dynamic array too long\n", stderr);
  abort ();
}

static void _NM (TS, _Xran) (void)
{
  fputs ("Out of range: invalid dynamic array position\n", stderr);
  abort ();
}

/* _Tidy: Tidy up an array either when it's initially constructed
   or right before it is freed.  */
static void _NM (TS, _Tidy) (_NM (TS, _ARRAY_T) * this, int _Constructed)
{
  size_t i;

  if (_Constructed && this->_Ptr != 0)
    {
      for (i = 0; i < this->_Len; i++)
        Tdtor (this->_Ptr + i);
      free (this->_Ptr);
    }
  this->_Len = 0;
  this->_Ptr = 0;
  this->_Res = 0;
}

/* _Grow: Reallocate an array to make it size N.  Initialize the unused
   elements in the array to _S.  */
static void _NM (TS, _Grow) (_NM (TS, _ARRAY_T) * this, size_t _N, T * _S,
                             int _Trim)
{
  size_t _Os = this->_Ptr == 0 ? 0 : this->_Res;
  size_t _I, _M, _R;
  T *_Np;

  if (_N == 0)
    {
      if (_Trim)
        _NM (TS, _Tidy) (this, 1);
    }
  else if (_N == _Os || _N < _Os && !_Trim);
  else
    {
      _M = this->_Ptr == 0 && _N < this->_Res ? this->_Res : _N;
      _Np = calloc (_M, sizeof (T));
      if (_Np == 0)
        Nomemory ();            /* no memory */
      for (_I = 0; _I < _M; _I++)
        Tctor (_Np + _I);
      _R = _M;
      _M = _N < this->_Len ? _N : this->_Len;
      for (_I = 0; _I < _M; ++_I)
        Tassign (_Np + _I, this->_Ptr + _I);
      if (_S != 0)
        for (; _I < this->_Res; ++_I)
          Tassign (_Np + _I, _S);
      _NM (TS, _Tidy) (this, 1);
      this->_Ptr = _Np;
      this->_Res = _R;
    }
  this->_Len = _N;
}

/* Exported functions */

/* _create: Create an array out of thin air.  */
_NM (TS, _ARRAY_T) * _NM (TS, _create) (void)
{
  _NM (TS, _ARRAY_T) * x = malloc (sizeof (_NM (TS, _ARRAY_T)));

  if (x == 0)
    Nomemory ();
  _NM (TS, _Tidy) (x, 0);
  return x;
}

/* _destroy: Destroy all elements in the array and free the array.  */
void _NM (TS, _destroy) (_NM (TS, _ARRAY_T) * x)
{
  _NM (TS, _Tidy) (x, 1);
  free (x);
}

/* _ctor: Construct a newly-allocated array.  */
void _NM (TS, _ctor) (_NM (TS, _ARRAY_T) * this)
{
  _NM (TS, _Tidy) (this, 0);
}

/* _ctor_with_size: Construct a newly-allocated array, given a size and
   whether the new elements are a default_size or should be in reserve.  */
void _NM (TS, _ctor_with_size) (_NM (TS, _ARRAY_T) * this, size_t n,
                                capacity c)
{
  _NM (TS, _Tidy) (this, 0);
  this->_Res = n;
  if (c == default_size)
    _NM (TS, _Grow) (this, n, 0, 0);
}

/* _copy_ctor: Construct a newly-allocated array based on the elements of an
   existing array.  */
void _NM (TS, _copy_ctor) (_NM (TS, _ARRAY_T) * this, _NM (TS, _ARRAY_T) * x)
{
  size_t i;

  _NM (TS, _Tidy) (this, 0);
  _NM (TS, _Grow) (this, _NM (TS, _length) (x), 0, 0);
  for (i = 0; i < this->_Len; i++)
    Tassign (this->_Ptr + i, x->_Ptr + i);
}

/* _ctor_from_ptr: Construct an array based on the elements of a normal C array
   (decayed to a pointer).  The n parameter is the length.  */
void _NM (TS, _ctor_from_ptr) (_NM (TS, _ARRAY_T) * this, T * s, size_t n)
{
  if (s == 0)
    _NM (TS, _Xinv) ();
  _NM (TS, _Tidy) (this, 0);
  _NM (TS, _assign) (this, s, n, 1);
}

/* _dtor: Give an array its last rites before free()ing it.  */
void _NM (TS, _dtor) (_NM (TS, _ARRAY_T) * this)
{
  _NM (TS, _Tidy) (this, 1);
}

/* _append: Append new elements to an existing array.  _D is 1 if the new
   elements are in an array, 0 if there is only one element that should be
   propagated through the new space.  */
_NM (TS, _ARRAY_T) * _NM (TS, _append) (_NM (TS, _ARRAY_T) * this, T * _S,
                                        size_t _N, size_t _D)
{
  size_t _I;

  if (NPOS - this->_Len <= _N)
    _NM (TS, _Xlen) ();
  _I = this->_Len;
  for (_NM (TS, _Grow) (this, _N += _I, 0, 0); _I < _N; ++_I, _S += _D)
    Tassign (this->_Ptr + _I, _S);
  return this;
}

/* _assign: Similar to _append, except that the array is assigned to
   instead of appended to.  */
_NM (TS, _ARRAY_T) * _NM (TS, _assign) (_NM (TS, _ARRAY_T) * this, T * _S,
                                        size_t _N, size_t _D)
{
  size_t _I;

  _NM (TS, _Grow) (this, _N, 0, 1);
  for (_I = 0; _I < _N; ++_I, _S += _D)
    Tassign (this->_Ptr + _I, _S);
  return this;
}

/* _insert: Insert _N elements from _S into this at position _P. _D is 1 if _S
   should be incremented after the new element is assigned to, 0 if _S should
   not be incremented.  */
_NM (TS, _ARRAY_T) * _NM (TS, _insert) (_NM (TS, _ARRAY_T) * this, size_t _P,
                                        T * _S, size_t _N, size_t _D)
{
  size_t _I;

  if (this->_Len < _P)
    _NM (TS, _Xran) ();
  if (NPOS - this->_Len <= _N)
    _NM (TS, _Xlen) ();
  if (0 < _N)
    {
      _I = this->_Len - _P;
      for (_NM (TS, _Grow) (this, _N + this->_Len, 0, 0); 0 < _I;)
        {
          --_I;
          Tassign (this->_Ptr + (_P + _N + _I), this->_Ptr + (_P + _I));
        }
      for (_I = 0; _I < _N; ++_I, _S += _D)
        Tassign (this->_Ptr + (_P + _I), _S);
    }
  return this;
}

/* _remove: Remove and destroy the _N elements starting at this[_P].  */
_NM (TS, _ARRAY_T) * _NM (TS, _remove) (_NM (TS, _ARRAY_T) * this, size_t _P,
                                        size_t _N)
{
  size_t _M, _I;

  if (this->_Len < _P)
    _NM (TS, _Xran) ();
  if (this->_Len - _P < _N)
    _N = this->_Len - _P;
  if (0 < _N)
    {
      _M = this->_Len - _P - _N;
      for (_I = 0; _I < _M; ++_I)
        Tassign (this->_Ptr + (_P + _I), this->_Ptr + (_P + _I + _N));
      _NM (TS, _Grow) (this, this->_Len - _N, 0, 0);
    }
  return this;
}

/* _subarray: Assign the _N elements starting at this[_P] to _X.  It's okay
   if this and _X are the same array.  */
_NM (TS, _ARRAY_T) * _NM (TS, _subarray) (_NM (TS, _ARRAY_T) * this,
                                          _NM (TS, _ARRAY_T) * _X, size_t _P,
                                          size_t _N)
{
  if (this->_Len < _P)
    _NM (TS, _Xran) ();
  if (this->_Len - _P < _N)
    _N = this->_Len - _P;
  return this == _X ? (_NM (TS, _remove) (this, _P + _N, NPOS),
                       _NM (TS, _remove) (this, 0, _P))
    : _NM (TS, _assign) (_X, this->_Ptr + _P, _N, 1);
}

/* _Swap: swap the contents of two arrays.  */
void _NM (TS, _swap) (_NM (TS, _ARRAY_T) * this, _NM (TS, _ARRAY_T) * _X)
{
  T *_Tp;
  size_t _T;

  _Tp = this->_Ptr;
  this->_Ptr = _X->_Ptr;
  _X->_Ptr = _Tp;
  _T = this->_Len;
  this->_Len = _X->_Len;
  _X->_Len = _T;
  _T = this->_Res;
  this->_Res = _X->_Res;
  _X->_Res = _T;
}

/* _get_at: Return the element at this[_I].  */
T *_NM (TS, _get_at) (_NM (TS, _ARRAY_T) * this, size_t _I)
{
  if (this->_Len <= _I)
    _NM (TS, _Xran) ();
  return this->_Ptr + _I;
}

/* _put_at: Assign the element _X to this[_i].  */
void _NM (TS, _put_at) (_NM (TS, _ARRAY_T) * this, size_t _I, T * _X)
{
  if (this->_Len < _I)
    _NM (TS, _Xran) ();
  else if (this->_Len == _I)
    _NM (TS, _append) (this, _X, 1, 1);
  else
    Tassign (this->_Ptr + _I, _X);
}

/* _base: Return the base pointer, or a null pointer if there aren't
   any elements in the array. */
T *_NM (TS, _base) (_NM (TS, _ARRAY_T) * this)
{
  return this->_Len != 0 ? this->_Ptr : 0;
}

/* _length: Return how many elements are there in the array */
size_t _NM (TS, _length) (_NM (TS, _ARRAY_T) * this)
{
  return this->_Len;
}

/* _resize: Resize an array to _N elements, using _X for new elements
   (if we're growing the array).  */
void _NM (TS, _resize) (_NM (TS, _ARRAY_T) * this, size_t _N, T * _X)
{
  _NM (TS, _Grow) (this, _N, _X, 1);
}

/* _reserve: Return how many elements there are in reserve.  */
size_t _NM (TS, _reserve) (_NM (TS, _ARRAY_T) * this)
{
  return this->_Res;
}

/* _set_reserve: Sets how many elements there are in reserve.  Only works if
   the array is empty.  */
void _NM (TS, _set_reserve) (_NM (TS, _ARRAY_T) * this, size_t _R)
{
  if (this->_Ptr == 0)
    this->_Res = _R;
}

#endif

#undef _NM
#undef _VAL

/* END OF FILE */
