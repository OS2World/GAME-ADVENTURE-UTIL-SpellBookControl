/* general.c: some useful(?) functions

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

#include <stdlib.h>
#ifdef __MSDOS__
#include <alloc.h>
#else
/*#include <malloc.h> */
#endif
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include "general.hpp"

void fatal(char *fmt, ...) {
	va_list ap;

	va_start(ap,fmt);
	fprintf(stderr,"\nFatal Error: ");
	vfprintf(stderr,fmt,ap);
	fprintf(stderr,"\n");
	va_end(ap);
	exit(1);
}

void error(char *fmt, ...) {
	va_list ap;

	va_start(ap,fmt);
	fprintf(stderr,"\nError: ");
	vfprintf(stderr,fmt,ap);
	fprintf(stderr,"\n");
	va_end(ap);
}

void warning(char *fmt, ...) {
	va_list ap;

	va_start(ap,fmt);
	fprintf(stderr,"\nWarning: ");
	vfprintf(stderr,fmt,ap);
	fprintf(stderr,"\n");
	va_end(ap);
}

void *chkcalloc(size_t nitems, size_t size) {
	void *ret;

	if( (ret = calloc(nitems,size)) == NULL)
		fatal("Ran out of memory");
	return (ret);
}

char *upstr(char *str) {
	register char *s = str;

	if (str)
		while( *s = toupper(*s) )
			s++;

	return str;
}

char *lowstr(char *str) {
	register char *s = str;

	if (str)
		while( *s = tolower(*s) )
			s++;
		
	return str;
}

