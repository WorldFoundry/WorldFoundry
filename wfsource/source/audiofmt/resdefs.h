
/*
 * FILE: stdefs.h
 *   BY: Christopher Lee Fraley
 * DESC: Defines standard stuff for inclusion in C programs.
 * DATE: 6-JUN-88
 * VERS: 1.0  (6-JUN-88, 2:00pm)
 */


#define TRUE  1
#define FALSE 0

#define PI (3.14159265358979232846)
#define PI2 (6.28318530717958465692)
#define D2R (0.01745329348)          /* (2*pi)/360 */
#define R2D (57.29577951)            /* 360/(2*pi) */

#define MAX(x,y) ((x)>(y) ?(x):(y))
#define MIN(x,y) ((x)<(y) ?(x):(y))
#define ABS(x)   ((x)<0   ?(-(x)):(x))
#define SGN(x)   ((x)<0   ?(-1):((x)==0?(0):(1)))

typedef char           BOOL;
typedef short          HcbWORD;
typedef unsigned short UHcbWORD;
typedef int            IcbWORD;
#ifndef	cbWORD
typedef int		cbWORD;
#endif
typedef unsigned int   UcbWORD;

