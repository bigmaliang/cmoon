
/* Useful defines to use with sparse */

#ifndef _SPARSE_H
#define _SPARSE_H

#ifdef __CHECKER__
# define __acquires(x) __attribute__((exact_context(x,0,1)))
# define __releases(x) __attribute__((exact_context(x,1,0)))
# define __with_lock_acquired(x) __attribute__((exact_context(x,1,1)))
# define __acquire(x) __context__(x,1,0)
# define __release(x) __context__(x,-1,1)
#else
# define __acquires(x)
# define __releases(x)
# define __with_lock_acquired(x)
# define __acquire(x) (void)0
# define __release(x) (void)0
#endif

#endif
