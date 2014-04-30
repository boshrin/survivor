/*
 * Softpixel MemCheckDeluxe v1.3.0
 *  2003 Christopher Wright <cwright@softpixel.com>
 */

/* Copyright (C) 2002, 2003 Softpixel (http://softpixel.com/)
 * This file is covered by a BSD-Style License, available in
 * the LICENSE file in the root directory of this package, as well as at
 * http://prj.softpixel.com/licenses/#bsd
 */


#ifndef MCD_H
#define MCD_H

#define MCD_VERSION	0x010300

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
	#define __FUNCTION__ __FILE__
#endif



#ifdef malloc
#undef malloc
	#ifndef WIN32
	#warning ------ Redefining malloc() ------
	#endif
#endif
#ifdef calloc
#undef calloc
	#ifndef WIN32
	#warning ------ Redefining calloc() ------
	#endif
#endif

#ifdef realloc
#undef realloc
	#ifndef WIN32
	#warning ------ Redefining realloc() ------
	#endif
#endif

#ifdef free
#undef free
	#ifndef WIN32
	#warning ------ Redefining free() ------
	#endif
#endif

#ifdef strdup
#undef strdup
	#ifndef WIN32
	#warning ------ Redefining strdup() ------
	#endif
#endif

#ifdef strndup
#undef strndup
	#ifndef WIN32
	#warning ------ Redefining strndup() ------
	#endif
#endif

#ifdef asprintf
#undef asprintf
	#ifndef WIN32
	#warning ------ Redefining asprintf() ------
	#endif
#endif

#ifdef vasprintf
#undef vasprintf
	#ifndef WIN32
	#warning ------ Redefining vasprintf() ------
	#endif
#endif

#ifdef scanf
#undef scanf
	#ifndef WIN32
	#warning ------ Redefining scanf() ------
	#endif
#endif

#ifdef fscanf
#undef fscanf
	#ifndef WIN32
	#warning ------ Redefining fscanf() ------
	#endif
#endif

#ifdef sscanf
#undef sscanf
	#ifndef WIN32
	#warning ------ Redefining sscanf() ------
	#endif
#endif

#ifdef getcwd
#undef getcwd
	#ifndef WIN32
	#warning ------ Redefining getcwd() ------
	#endif
#endif


#define strdup(p)		MCD_Strdup (p,  __FILE__,__FUNCTION__,__LINE__)
/* no strndup in WIN32 */
#ifndef WIN32
	#define strndup(p,n)		MCD_Strndup(p,n,__FILE__,__FUNCTION__,__LINE__)
#endif

#define malloc(z)		MCD_Malloc (z,  __FILE__,__FUNCTION__,__LINE__)
#define calloc(n,s)		MCD_Calloc (s*n,__FILE__,__FUNCTION__,__LINE__)
#define realloc(p,s)		MCD_Realloc(p,s,__FILE__,__FUNCTION__,__LINE__)
#define free(p)			MCD_Free   (p,  __FILE__,__FUNCTION__,__LINE__)

#ifdef _GNU_SOURCE
#define asprintf(p,f,args...)	MCD_Asprintf (p,f,   __FILE__,__FUNCTION__,__LINE__, ## args)
#define vasprintf(p,f,ap)	MCD_Vasprintf(p,f,ap,__FILE__,__FUNCTION__,__LINE__)
#endif

#ifndef WIN32
/* windows doesn't like variable arguments in #define's */
#define scanf(f,args...)	MCD_Scanf (f,  __FILE__,__FUNCTION__,__LINE__, ## args)
#define fscanf(s,f,args...)	MCD_Fscanf(s,f,__FILE__,__FUNCTION__,__LINE__, ## args)
#define sscanf(s,f,args...)	MCD_Sscanf(s,f,__FILE__,__FUNCTION__,__LINE__, ## args) 
#define getcwd(p,s)		MCD_Getcwd(p,s,__FILE__,__FUNCTION__,__LINE__)
#endif  /* WIN32 */

void *MCD_Malloc (int, const char*, const char*, int);
void *MCD_Calloc (int, const char*, const char*, int);
void *MCD_Realloc(void *, int, const char*, const char*, int);
void  MCD_Free	 (void*, const char*, const char*, int);
char* MCD_Getcwd (char*, int, const char*, const char*, int);

#ifdef __GNUC__
void *MCD_Strdup (const char *, const char*, const char*, int);
void *MCD_Strndup(const char*, int, const char*, const char*, int);
#else
void *MCD_Strdup (char *, const char*, const char*, int);
void *MCD_Strndup(char*, int, const char*, const char*, int);
#endif

#ifdef _GNU_SOURCE
#include <stdarg.h>
int   MCD_Asprintf (char **, const char *, const char*, const char*, int,...);
int   MCD_Vasprintf(char **, const char *,va_list, const char*, const char*, int);
#endif

#ifndef WIN32
int   MCD_Scanf (const char *, const char*, const char*, int,...);
int   MCD_Fscanf(FILE *, const char*, const char*, const char*, int,...);
int   MCD_Sscanf(const char*, const char*, const char*, const char*, int,...);
#endif

#ifdef __cplusplus
}	/* extern "C" */

#ifndef WIN32
	#warning MCD C++ Extentions Enabled
#endif

#ifdef new
#undef new
#endif

#ifdef delete
#undef delete
#endif

inline void* operator new	(unsigned int size, const char *file,
 const char*fun,int line)
{
	void*ret=MCD_Malloc(size,file,fun,line);
	if(!ret)
		throw 1;
	return ret;
}

inline void* operator new[]	(unsigned int size, const char*file, 
 const char*fun,int line)
{
	void*ret=MCD_Malloc(size,file,fun,line);
	if(!ret)
		throw 1;
	return ret;
}

inline void  operator delete	(void * buf)
{
	MCD_Free(buf,0,0,0);
}

inline void  operator delete[]	(void * buf)
{
	MCD_Free(buf,0,0,0);
}

#ifdef WIN32
	#define new	new(__FILE__,__FILE__,__LINE__)
#else
	#define new	new(__FILE__,__FUNCTION__,__LINE__)
#endif


#endif	/* __cplusplus */

#endif /* MCD_H */
