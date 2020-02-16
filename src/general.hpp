/* general.hpp:  Headers for general interest routines

    Copyright (C) 1993 John-Marc Chandonia

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _GENERAL_HPP
#define _GENERAL_HPP

/* error handlers */
void fatal(char *fmt, ...);
void error(char *fmt, ...);
void warning(char *fmt, ...);

/* memory allocation */
void *chkcalloc(size_t nitems, size_t size);

/* string functions for unix system */
char *upstr(char *str);
char *lowstr(char *str);

/* general stuff */
typedef enum boolean {false,true} boolean;
#define forever while (true)

#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef min
#define min(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef sqr
#define sqr(x) ((x)*(x))
#endif

#ifndef abs
#define abs(x) ((x)>0.0 ? (x) : 0.0-(x))
#endif

#endif

