/* xalloc.h -- malloc with out-of-memory checking

   Copyright (C) 1990-2000, 2003-2004, 2006-2019 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef XALLOC_H_
# define XALLOC_H_

# include <stddef.h>

#include "intprops.h"


# ifdef __cplusplus
extern "C" {
# endif


# ifndef __attribute__
#  if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 8)
#   define __attribute__(arg)
#  endif
# endif

# ifndef ATTRIBUTE_NORETURN
#  define ATTRIBUTE_NORETURN __attribute__ ((__noreturn__))
# endif

# ifndef ATTRIBUTE_MALLOC
#  if __GNUC__ >= 3
#   define ATTRIBUTE_MALLOC __attribute__ ((__malloc__))
#  else
#   define ATTRIBUTE_MALLOC
#  endif
# endif

/* This function is always triggered when memory is exhausted.
   It must be defined by the application, either explicitly
   or by using gnulib's xalloc-die module.  This is the
   function to call when one wants the program to die because of a
   memory allocation failure.  */
extern void xalloc_die (void) ATTRIBUTE_NORETURN;

void *xmalloc (size_t s) ATTRIBUTE_MALLOC;
void *xzalloc (size_t s) ATTRIBUTE_MALLOC;
void *xcalloc (size_t n, size_t s) ATTRIBUTE_MALLOC;
void *xrealloc (void *p, size_t s);
void *x2realloc (void *p, size_t *pn);
void *xpalloc (void *pa, idx_t *nitems, idx_t nitems_incr_min,
               ptrdiff_t nitems_max, idx_t item_size);
void *xmemdup (void const *p, size_t s) ATTRIBUTE_MALLOC;
char *xstrdup (char const *str) ATTRIBUTE_MALLOC;

/* Return 1 if an array of N objects, each of size S, cannot exist due
   to size arithmetic overflow.  S must be positive and N must be
   nonnegative.  This is a macro, not an inline function, so that it
   works correctly even when SIZE_MAX < N.

   By gnulib convention, SIZE_MAX represents overflow in size
   calculations, so the conservative dividend to use here is
   SIZE_MAX - 1, since SIZE_MAX might represent an overflowed value.
   However, malloc (SIZE_MAX) fails on all known hosts where
   sizeof (ptrdiff_t) <= sizeof (size_t), so do not bother to test for
   exactly-SIZE_MAX allocations on such hosts; this avoids a test and
   branch when S is known to be 1.  */
# define xalloc_oversized(n, s) \
    ((size_t) (sizeof (ptrdiff_t) <= sizeof (size_t) ? -1 : -2) / (s) < (n))


/* In the following macros, T must be an elementary or structure/union or
   typedef'ed type, or a pointer to such a type.  To apply one of the
   following macros to a function pointer or array type, you need to typedef
   it first and use the typedef name.  */

/* Allocate an object of type T dynamically, with error checking.  */
/* extern t *XMALLOC (typename t); */
# define XMALLOC(t) ((t *) xmalloc (sizeof (t)))

/* Allocate memory for N elements of type T, with error checking.  */
/* extern t *XNMALLOC (size_t n, typename t); */
# define XNMALLOC(n, t) \
    ((t *) (sizeof (t) == 1 ? xmalloc (n) : xnmalloc (n, sizeof (t))))

/* Allocate an object of type T dynamically, with error checking,
   and zero it.  */
/* extern t *XZALLOC (typename t); */
# define XZALLOC(t) ((t *) xzalloc (sizeof (t)))

/* Allocate memory for N elements of type T, with error checking,
   and zero it.  */
/* extern t *XCALLOC (size_t n, typename t); */
# define XCALLOC(n, t) \
    ((t *) (sizeof (t) == 1 ? xzalloc (n) : xcalloc (n, sizeof (t))))

/*
 * Gawk uses this file only to keep dfa.c happy.
 * We're therefore safe in manually defining HAVE_INLINE to
 * make the !@#$%^&*() thing just work.
 */
#ifdef GAWK
#define HAVE_INLINE	1	/* so there. nyah, nyah, nyah. */
#endif

# if HAVE_INLINE
#  define static_inline static inline
# else
void *xnmalloc (size_t n, size_t s) ATTRIBUTE_MALLOC;
void *xnrealloc (void *p, size_t n, size_t s);
void *x2nrealloc (void *p, size_t *pn, size_t s);
char *xcharalloc (size_t n) ATTRIBUTE_MALLOC;
# endif

# ifdef static_inline

/* Allocate an array of N objects, each with S bytes of memory,
   dynamically, with error checking.  S must be nonzero.  */

static_inline void *xnmalloc (size_t n, size_t s) ATTRIBUTE_MALLOC;
static_inline void *
xnmalloc (size_t n, size_t s)
{
  if (xalloc_oversized (n, s))
    xalloc_die ();
  return xmalloc (n * s);
}

#ifdef GAWK
#include <errno.h>
extern void r_fatal(const char *msg, ...) ATTRIBUTE_NORETURN ;

void *
xmalloc(size_t bytes)
{
  void *p;
  if (bytes == 0)
    bytes = 1;	/* avoid dfa.c mishegos */
  if ((p = malloc(bytes)) == NULL)
    xalloc_die ();
  return p;
}

/* Allocate an array of N objects, each with S bytes of memory,
   dynamically, with error checking.  S must be nonzero.
   Clear the contents afterwards.  */

void *
xcalloc(size_t nmemb, size_t size)
{
  void *p;

  if (nmemb == 0 || size == 0)
    nmemb = size = 1; 	/* avoid dfa.c mishegos */
  if ((p = calloc(nmemb, size)) == NULL)
    xalloc_die ();
  return p;
}

/* Reallocate a pointer to a new size, with error checking. */

void *
xrealloc(void *p, size_t size)
{
   void *new_p = realloc(p, size);
   if (new_p ==  0)
     xalloc_die ();

   return new_p;
}

/* xalloc_die --- fatal error message when malloc fails, needed by dfa.c */

void
xalloc_die (void)
{
	r_fatal(_("xalloc: malloc failed: %s"), strerror(errno));
}

/* Clone an object P of size S, with error checking.  There's no need
   for xnmemdup (P, N, S), since xmemdup (P, N * S) works without any
   need for an arithmetic overflow check.  */

void *
xmemdup (void const *p, size_t s)
{
  return memcpy (xmalloc (s), p, s);
}

/* xstrdup --- strdup and die if fails */
char *xstrdup(const char *s)
{
	char *p;
	int l;

	if (s == NULL)
		r_fatal(_("xstrdup: null parameter"));

	l = strlen(s);
	p = xmemdup(s, l + 1);
	p[l] = '\0';

	return p;
}
#endif

/* Change the size of an allocated block of memory P to an array of N
   objects each of S bytes, with error checking.  S must be nonzero.  */

static_inline void *
xnrealloc (void *p, size_t n, size_t s)
{
  if (xalloc_oversized (n, s))
    xalloc_die ();
  return xrealloc (p, n * s);
}

/* If P is null, allocate a block of at least *PN such objects;
   otherwise, reallocate P so that it contains more than *PN objects
   each of S bytes.  S must be nonzero.  Set *PN to the new number of
   objects, and return the pointer to the new block.  *PN is never set
   to zero, and the returned pointer is never null.

   Repeated reallocations are guaranteed to make progress, either by
   allocating an initial block with a nonzero size, or by allocating a
   larger block.

   In the following implementation, nonzero sizes are increased by a
   factor of approximately 1.5 so that repeated reallocations have
   O(N) overall cost rather than O(N**2) cost, but the
   specification for this function does not guarantee that rate.

   Here is an example of use:

     int *p = NULL;
     size_t used = 0;
     size_t allocated = 0;

     void
     append_int (int value)
       {
         if (used == allocated)
           p = x2nrealloc (p, &allocated, sizeof *p);
         p[used++] = value;
       }

   This causes x2nrealloc to allocate a block of some nonzero size the
   first time it is called.

   To have finer-grained control over the initial size, set *PN to a
   nonzero value before calling this function with P == NULL.  For
   example:

     int *p = NULL;
     size_t used = 0;
     size_t allocated = 0;
     size_t allocated1 = 1000;

     void
     append_int (int value)
       {
         if (used == allocated)
           {
             p = x2nrealloc (p, &allocated1, sizeof *p);
             allocated = allocated1;
           }
         p[used++] = value;
       }

   */

static_inline void *
x2nrealloc (void *p, size_t *pn, size_t s)
{
  size_t n = *pn;

  if (! p)
    {
      if (! n)
        {
          /* The approximate size to use for initial small allocation
             requests, when the invoking code specifies an old size of
             zero.  64 bytes is the largest "small" request for the
             GNU C library malloc.  */
          enum { DEFAULT_MXFAST = 64 * sizeof (size_t) / 4 };

          n = DEFAULT_MXFAST / s;
          n += !n;
        }
    }
  else
    {
      /* Set N = ceil (1.5 * N) so that progress is made if N == 1.
         Check for overflow, so that N * S stays in size_t range.
         The check is slightly conservative, but an exact check isn't
         worth the trouble.  */
      if ((size_t) -1 / 3 * 2 / s <= n)
        xalloc_die ();
      n += n / 2 + 1;
    }

  *pn = n;
  return xrealloc (p, n * s);
}

/* Return a pointer to a new buffer of N bytes.  This is like xmalloc,
   except it returns char *.  */

static_inline char *xcharalloc (size_t n) ATTRIBUTE_MALLOC;
static_inline char *
xcharalloc (size_t n)
{
  return XNMALLOC (n, char);
}

/* Allocate S bytes of zeroed memory dynamically, with error checking.
   There's no need for xnzalloc (N, S), since it would be equivalent
   to xcalloc (N, S).  */

inline void *
xzalloc (size_t s)
{
  return xcalloc(1, s);
}

# endif

# ifdef __cplusplus
}

/* C++ does not allow conversions from void * to other pointer types
   without a cast.  Use templates to work around the problem when
   possible.  */

template <typename T> inline T *
xrealloc (T *p, size_t s)
{
  return (T *) xrealloc ((void *) p, s);
}

template <typename T> inline T *
xnrealloc (T *p, size_t n, size_t s)
{
  return (T *) xnrealloc ((void *) p, n, s);
}

template <typename T> inline T *
x2realloc (T *p, size_t *pn)
{
  return (T *) x2realloc ((void *) p, pn);
}

template <typename T> inline T *
x2nrealloc (T *p, size_t *pn, size_t s)
{
  return (T *) x2nrealloc ((void *) p, pn, s);
}

template <typename T> inline T *
xmemdup (T const *p, size_t s)
{
  return (T *) xmemdup ((void const *) p, s);
}

# endif

/* Grow PA, which points to an array of *NITEMS items, and return the
   location of the reallocated array, updating *NITEMS to reflect its
   new size.  The new array will contain at least NITEMS_INCR_MIN more
   items, but will not contain more than NITEMS_MAX items total.
   ITEM_SIZE is the size of each item, in bytes.

   ITEM_SIZE and NITEMS_INCR_MIN must be positive.  *NITEMS must be
   nonnegative.  If NITEMS_MAX is -1, it is treated as if it were
   infinity.

   If PA is null, then allocate a new array instead of reallocating
   the old one.

   Thus, to grow an array A without saving its old contents, do
   { free (A); A = xpalloc (NULL, &AITEMS, ...); }.  */

#define MIN(x, y) ((x) < (y) ? (x) : (y))

void *
xpalloc (void *pa, idx_t *nitems, idx_t nitems_incr_min,
         ptrdiff_t nitems_max, idx_t item_size)
{
  idx_t n0 = *nitems;

  /* The approximate size to use for initial small allocation
     requests.  This is the largest "small" request for the GNU C
     library malloc.  */
  enum { DEFAULT_MXFAST = 64 * sizeof (size_t) / 4 };

  /* If the array is tiny, grow it to about (but no greater than)
     DEFAULT_MXFAST bytes.  Otherwise, grow it by about 50%.
     Adjust the growth according to three constraints: NITEMS_INCR_MIN,
     NITEMS_MAX, and what the C language can represent safely.  */

  idx_t n, nbytes;
  if (INT_ADD_WRAPV (n0, n0 >> 1, &n))
    n = IDX_MAX;
  if (0 <= nitems_max && nitems_max < n)
    n = nitems_max;

  idx_t adjusted_nbytes
    = ((INT_MULTIPLY_WRAPV (n, item_size, &nbytes) || SIZE_MAX < nbytes)
       ? MIN (IDX_MAX, SIZE_MAX)
       : nbytes < DEFAULT_MXFAST ? DEFAULT_MXFAST : 0);
  if (adjusted_nbytes)
    {
      n = adjusted_nbytes / item_size;
      nbytes = adjusted_nbytes - adjusted_nbytes % item_size;
    }

  if (! pa)
    *nitems = 0;
  if (n - n0 < nitems_incr_min
      && (INT_ADD_WRAPV (n0, nitems_incr_min, &n)
          || (0 <= nitems_max && nitems_max < n)
          || INT_MULTIPLY_WRAPV (n, item_size, &nbytes)))
    xalloc_die ();
  pa = xrealloc (pa, nbytes);
  *nitems = n;
  return pa;
}

/* Clone an object P of size S, with error checking.  Append
   a terminating NUL byte.  */

char *
ximemdup0 (void const *p, idx_t s)
{
  char *result = malloc(s + 1);
  result[s] = 0;
  return memcpy (result, p, s);
}


#endif /* !XALLOC_H_ */
