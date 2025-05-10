/*
	midi2gmn Library
	Copyright (C) 1992 - 2012 Juergen F. Kilian


	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#if !defined( __portable_h__)
#define __portable_h__

/*------------------------------------------------------------------
|	filename : portable.CPP
|	author     : Juergen Kilian
|	date	    : 1998
|	definitions of portable functions
------------------------------------------------------------------*/


/* From values.h */
#if !defined(__FLAT__)
#define MAXINT      0x7FFF
#else    /* defined __FLAT__ */
#define MAXINT      0x7FFFFFFF
#endif

#if (!defined(MAXLONG) /* && !defined(_MSC_VER) */)
	
#define MAXLONG      0x7fffffff  
#endif


/*
#if !defined(MAXLONG)
	#define MAXLONG      0x7FFFFFFF
#endif
*/


#if (!defined(__GNUC__) && !defined(_MSC_VER) && !defined(__BORLANDC__)  )
char *itoa(int value, char *string, int radix);
#endif


char * myStrupr( char *buffer );	// all chars to upper


/* MAX String lengths */
#define MAXPATH   256
#define MAXDIR    66
#define MAXFILE   9
#define MAXEXT    5


#ifndef max_
#define max_(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min_
#define min_(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif
