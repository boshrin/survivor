/* Kosada MemCheckDeluxe v1.3.0
 *  2003 Christopher Wright <cwright@kosada.com>
 *
 * Copyright (C) 2002, 2003 Kosada (http://www.kosada.com/)
 * This file is covered by a BSD-Style License, available in
 * the LICENSE file in the root directory of this package, as well as at
 * http://prj.olandis.com/licenses/#bsd
 */
 
 /* Basic structure:
  *  The allocation information is kept in a hashtable,
  *   which uses the address as the hash seed.
  *  To check for double-frees, we have another hashtable that marks
  *   pointers that have been freed, but not reallocated.
  *  The hashtables still use linked-lists for collisions, but it's still
  *   considerably better than the old version (all in one big list)
  */

#ifndef WIN32
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#include <signal.h>
#endif

#define MCD_MAGIC	0x5c022d98
#define MCD_RTL		MCD_Settings.RealTimeLog
#define MCD_STAT	MCD_Settings.StatLog

#ifdef WIN32
typedef LONGLONG mcd64;
#else
typedef long long mcd64;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* nodes serve 2 purposes:
  1) the outstanding allocations list
  2) the free'd pointers list (for double-free catching)
*/
typedef struct MCD_Node
{
	void		*data;
	struct MCD_Node	*next;
}MCD_NODE;

/* The Chunk structures are for allocation purposes */
typedef struct
{
	unsigned int	magic;		/* help make sure the chunk is really a chunk */
	int		size;		/* size of allocation in bytes */
	int		line;		/* line it was alocated on */
	mcd64		id;		/* 64bit allocation ID */
	const char		*function;	/* creating function(file in win32) */
	const char		*file;		/* file function is in */
	void		*ptr;		/* pointer to allocation */
}MCD_CHUNK;

#define PTR_ALLOCATED	0	/* once it hass been allocated */
#define PTR_FREED	1	/* once the allocated one's been freed */

typedef struct
{
	void	*ptr;
	int	state;
}MCD_PTR_STATE;
 
typedef struct
{
	unsigned int size;
	MCD_NODE **Lists;
}MCD_HASH_TABLE;

/* our various flag values */
#define MCD_INITIALIZED	0x01
#define MCD_DOUBLE_FREE	0x02	/* check for double-frees? */
#define MCD_OVERFLOW	0x04	/* check for under/overflows? */
#define MCD_VERBOSE	0x08	/* output all kinds of stuff */
#define MCD_PASSTHRU	0x80	/* skip everything, just call malloc/free */

typedef struct
{
	FILE		*StatLog, *RealTimeLog;
	unsigned int	HashTableSize;
	unsigned int	OverflowPad;
	unsigned int	verbosity;
	unsigned char	Flags;
	unsigned int	sigTime; /* how long between retriggers before we raise real one */
	unsigned int	mcdSig;	/* which signal should we use? */
	void		(*oldsig)(int);
	/* struct sigaction *sa; */
} MCD_SETTINGS;

typedef struct
{
	unsigned int	biggest,	/* biggest allocation */
			smallest;	/* smallest allocation */
	unsigned int	totHigh,	/* total number amount of */
			totLow;		/*  allocated memory (64bits) */
	unsigned int	allocations,	/* allocation count */
			frees;		/* free count */
	unsigned int	maxAllocations;	/* peak outstanding allocations */
	unsigned int	maxMemusage;	/* highest memory usage */
	unsigned int	unfreed;	/* presently unfreed memory */
	unsigned int	doublefree;	/* doublefree counter */
	unsigned int	overflow;	/* buffer overflow counter */
	unsigned int	invalid;	/* free() with invalid pointer */
	time_t		lastInt;	/* last interrupt time */
}MCD_STATS;

/* our working variables... */

MCD_SETTINGS	MCD_Settings=	{0,0,512,64,0,MCD_DOUBLE_FREE|MCD_OVERFLOW,1,SIGINT,NULL};
MCD_STATS	MCD_Stats=	{0,0,0,0,0,0,0,0,0,0,0,0,0};
MCD_HASH_TABLE	MCD_Allocations={0,0},
		MCD_Frees=	{0,0};
mcd64		MCD_ID=		0;
volatile char	MCD_lock=	0;

/* called when we hash a pointer to get it's place in the table */
unsigned int MCD_Hash_Address(void *p)
{
#if 0
	/* This hash function was recommended to me in an email.
	   however, in practice it seemed to only use 1/8 of the table entries */
	/* this is only for 2^n sized hash tables though probably... */
	return ((unsigned int)p)>>2;	/*skip bottom 2 bits because they're
					  probably aligned to 4-byte anyway */
#else
	/* This function is a slight variation on the above that uses the higher order
	   bits to modify the lower order ones (which are still present after the modulo).
	   this seems to use a lot more of the table, but still allows some long
	   chains from time to time... */
	return (((unsigned int)p)>>2)^(((unsigned int)p)>>9);
#endif
}

void __MCD_ShowStatus(int d)
{
	MCD_NODE *n;
	MCD_CHUNK*c;
	unsigned int i,j=0;
	time_t now;

	if(d==SIGINT)
	{
		now=time(NULL);
		if(now-MCD_Stats.lastInt<MCD_Settings.sigTime && MCD_Settings.sigTime)
		{
			fprintf(MCD_STAT,"\n###\n### 2 Signals in %i %s, invoking real sig handler.\n###\n",MCD_Settings.sigTime,MCD_Settings.sigTime>2?"seconds":"second");
			fflush(MCD_STAT);
			fflush(MCD_RTL);
			if(MCD_Settings.oldsig)
				MCD_Settings.oldsig(d);
			else
				exit(0);
		}
		MCD_Stats.lastInt=now;
	}
	while(MCD_lock);
	MCD_lock=1;
		
	fprintf(MCD_STAT,"\n### Memory Allocation Status ###\n");
	fprintf(MCD_STAT,"### Unfreed Pointers:\n");
	for(i=0;i<MCD_Allocations.size;++i)
	{
		n=MCD_Allocations.Lists[i];
		while(n)
		{
			c=(MCD_CHUNK*)n->data;
			fprintf(MCD_STAT,"###   * 0x%08x (%i bytes) allocated in %s:%s() line %i\n",
					(int)c->ptr,
					(int)c->size,
					c->file,
					c->function,
					c->line);
			if(c->ptr)
			  fprintf(MCD_STAT,"==>  [%s]\r\n", (char *)c->ptr);
			j++;
			n=(MCD_NODE*)n->next;
		}
	}
	if(j==0)
		fprintf(MCD_STAT,"###   * No unfreed memory (no leaks)\n");
	else
		fprintf(MCD_STAT,"###   * %i %s\n",j,((j==1)?"leak":"leaks"));
	fprintf(MCD_STAT,"### Allocation Statistics:\n");
	fprintf(MCD_STAT,"###   * Biggest  Allocation:    %12i bytes\n",MCD_Stats.biggest);
	fprintf(MCD_STAT,"###   * Smallest Allocation:    %12i bytes\n",MCD_Stats.smallest);
	fprintf(MCD_STAT,"###   * Total memory allocated: %12i bytes\n",MCD_Stats.totLow);
	fprintf(MCD_STAT,"###   * Allocations (total):    %12i\n",MCD_Stats.allocations);
	fprintf(MCD_STAT,"###   * Frees (total):          %12i\n",MCD_Stats.frees);
	fprintf(MCD_STAT,"###   * Peak # of allocations:  %12i\n",MCD_Stats.maxAllocations);
	fprintf(MCD_STAT,"###   * Peak Memory Usage:      %12i bytes\n",MCD_Stats.maxMemusage);
	fprintf(MCD_STAT,"###   * Unfreed memory:         %12i bytes\n",MCD_Stats.unfreed);
	fprintf(MCD_STAT,"###   * Double Frees:           %12i\n",MCD_Stats.doublefree);
	fprintf(MCD_STAT,"###   * Overflows:              %12i\n",MCD_Stats.overflow);
	fprintf(MCD_STAT,"###   * Invalid frees:          %12i\n",MCD_Stats.invalid);
	signal(MCD_Settings.mcdSig, __MCD_ShowStatus);
	MCD_lock=0;
}

/* This is called at program termination.  It shows all the various stats. */
void __MCD_AtExit()
{
	unsigned int i;
	MCD_NODE *n,*nn;
	MCD_CHUNK *c;

	__MCD_ShowStatus(0);
	/* should clean up internal structure, to be leak free itself */
	
	for(i=0;i<MCD_Allocations.size;++i)
	{
		n=MCD_Allocations.Lists[i];
		while(n)
		{
			c=(MCD_CHUNK*)n->data;
			free(c->ptr);
			free(c);
			nn=(MCD_NODE*)n->next;
			free(n);
			n=nn;
		}
	}

	for(i=0;i<MCD_Frees.size;++i)
	{
		n=MCD_Frees.Lists[i];
		while(n)
		{
			free(n->data);
			nn=(MCD_NODE*)n->next;
			free(n);
			n=nn;
		}
	}
	
	free(MCD_Allocations.Lists);
	free(MCD_Frees.Lists);
}

/* signal descriptions, somewhat portable hopefully(incomplete) */
char *__MCD_Strsignal(int i)
{
#ifdef _GNU_SOURCE
	return strsignal(i);
#else
	switch(i)
	{
		case SIGHUP: return "SIGHUP";
		case SIGINT: return "SIGINT";
		case SIGTERM: return "SIGTERM";
		case SIGUSR1: return "SIGUSR1";
		case SIGUSR2: return "SIGUSR2";
		default: return "Signal";
	}
#endif
}
		
/* This does the initialization, using environment variables */
static void MCD_Init()
{
	unsigned int i;
	
	if(MCD_Settings.Flags&1)
		return;
	MCD_Settings.Flags|=MCD_INITIALIZED;
	
	if(getenv("MCD_RTLOG"))
		MCD_Settings.RealTimeLog=fopen(getenv("MCD_RTLOG"),"wo");
	if(!MCD_Settings.RealTimeLog)
		MCD_Settings.RealTimeLog=stderr;
	
	if(getenv("MCD_STATLOG"))
		MCD_Settings.StatLog=fopen(getenv("MCD_RTLOG"),"wo");
	if(!MCD_Settings.StatLog)
		MCD_Settings.StatLog=stderr;
	
	fprintf(MCD_RTL,"### MCD_Initialized\n");
			
	if(getenv("MCD_NO_DOUBLEFREE"))
		MCD_Settings.Flags&=~MCD_DOUBLE_FREE;
	else
		MCD_Settings.Flags|=MCD_DOUBLE_FREE;
	
	if(getenv("MCD_NO_OVERFLOW"))
		MCD_Settings.Flags&=~MCD_OVERFLOW;
	else
		MCD_Settings.Flags|=MCD_OVERFLOW;
	
	if(getenv("MCD_OVERFLOW"))
		MCD_Settings.OverflowPad=atoi(getenv("MCD_OVERFLOW"));
	
	if(getenv("MCD_SIGNAL"))
		MCD_Settings.mcdSig=atoi(getenv("MCD_SIGNAL"));
	
	if(MCD_Settings.mcdSig==0)
		MCD_Settings.mcdSig=SIGINT;
	
	if(getenv("MCD_VERBOSE"))
	{		
		MCD_Settings.Flags|=MCD_VERBOSE;
		MCD_Settings.verbosity=atoi(getenv("MCD_VERBOSE"));
	}
	else
	{
		MCD_Settings.verbosity=0;
		MCD_Settings.Flags&=~MCD_VERBOSE;
	}
	
	if(getenv("MCD_HASH_SIZE"))
		MCD_Settings.HashTableSize=atoi(getenv("MCD_HASH_SIZE"));
	else
		MCD_Settings.HashTableSize=32;

	if(getenv("MCD_SIGNAL_TIME"))
		MCD_Settings.sigTime=atoi(getenv("MCD_SIGNAL_TIME"));
	else
		MCD_Settings.sigTime=1;
	
	if(!MCD_Settings.Flags&MCD_OVERFLOW)
		MCD_Settings.OverflowPad=0;
	
	if(MCD_Settings.HashTableSize<4)
		MCD_Settings.HashTableSize=4;
	
	MCD_Allocations.size=MCD_Settings.HashTableSize;
	MCD_Frees.size=MCD_Settings.HashTableSize;
	MCD_Allocations.Lists=(MCD_NODE**)malloc(sizeof(void *)*MCD_Settings.HashTableSize);
	MCD_Frees.Lists=(MCD_NODE**)malloc(sizeof(void *)*MCD_Settings.HashTableSize);
	
	for(i=0;i<MCD_Settings.HashTableSize;++i)
	{
		MCD_Allocations.Lists[i]=0;
		MCD_Frees.Lists[i]=0;
	}
	
	if(MCD_Settings.verbosity>4)
		MCD_Settings.verbosity=5;
	
	fprintf(MCD_RTL,"### MCD Setup :: Flags:   %10x\n"
			"###   * Overflow Pad:     %10i bytes\n"
			"###   * Hash Table Size:  %10i entries\n"
			"###   * Verbosity:        %10i\n"
			"###   * Status on signal  %10i (%s)\n"
			"###      * Signal interval%10i %s\n"
			"###   * Logs:\n"
			"###      * RealTime:      0x%08x (%s)\n"
			"###      * Status:        0x%08x (%s)\n",
		MCD_Settings.Flags,
		MCD_Settings.OverflowPad,
		MCD_Settings.HashTableSize,
		MCD_Settings.verbosity,
		MCD_Settings.mcdSig,
		__MCD_Strsignal(MCD_Settings.mcdSig),
		MCD_Settings.sigTime,
		MCD_Settings.sigTime==1?"second":"seconds",
		(int)MCD_RTL,getenv("MCD_RTLOG")?getenv("MCD_RTLOG"):"stderr",
		(int)MCD_STAT,getenv("MCD_STATLOG")?getenv("MCD_STATLOG"):"stderr");
	atexit(__MCD_AtExit);
	MCD_Settings.oldsig=signal(MCD_Settings.mcdSig, __MCD_ShowStatus);
	if(MCD_Settings.oldsig==SIG_ERR)
		MCD_Settings.oldsig=NULL;
}

/* prints the whole hash table, including all pointers currently recognized. */
void __MCD_Display_Hash(MCD_HASH_TABLE *hash)
{
	unsigned int i;
	MCD_NODE *n;
	MCD_CHUNK *c;
	
	if(MCD_Settings.verbosity<5)
		return;
	while(MCD_lock);
	MCD_lock=1;
	fprintf(MCD_RTL,"##### Size: %i\n",hash->size);
	for(i=0;i<hash->size;++i)
	{
		n=hash->Lists[i];
		fprintf(MCD_RTL,"##### List %i\n",i);
		while(n)
		{
			c=(MCD_CHUNK*)n->data;
			fprintf(stderr,"[0x%08x] -> ",(int)c->ptr);
			n=(MCD_NODE*)n->next;
			if(!n)
				fprintf(stderr,"\n");
		}
	}
	MCD_lock=0;
	fprintf(stderr,"\n");
}

/* this looks up the allocation in the hash table */
MCD_CHUNK* MCD_Hash_Lookup_Alloc(MCD_HASH_TABLE *hash, void *key)
{
	unsigned int offset;
	MCD_NODE *p;
	MCD_CHUNK *c;

	if(MCD_Settings.verbosity>2)
		fprintf(MCD_RTL,"### Looking up allocation 0x%08x\n",(int)key);
	while(MCD_lock);
	MCD_lock=1;
	offset=MCD_Hash_Address(key);
	offset%=hash->size;
	p=hash->Lists[offset];
	while(p)
	{
		c=(MCD_CHUNK*)p->data;
		if(c->ptr==key)
		{
			if(MCD_Settings.verbosity>2)
				fprintf(MCD_RTL,"### Found\n");
			MCD_lock=0;
			return c;
		}
		p=(MCD_NODE*)p->next;
	}
	MCD_lock=0;
	if(MCD_Settings.verbosity>2)
		fprintf(MCD_RTL,"### Not Found\n");
	return (MCD_CHUNK*)0;
}

/* and similarly, this looks up the freeness of a pointer
   returns 1 if the pointer has been freed (or never allocates), 0 otherwise */
MCD_PTR_STATE* MCD_Hash_Lookup_Freed(void *key)
{
	unsigned int offset;
	MCD_NODE *p;
	MCD_PTR_STATE *ps;

	if(MCD_Settings.verbosity>2)
		fprintf(MCD_RTL,"### Looking up Freeness of 0x%08x\n",(int)key);

	while(MCD_lock);
	MCD_lock=1;
	offset=MCD_Hash_Address(key);
	offset%=MCD_Frees.size;
	p=MCD_Frees.Lists[offset];
	while(p)
	{
		ps=(MCD_PTR_STATE*)p->data;
		if(ps->ptr==key)
		{
			if(MCD_Settings.verbosity>2)
				fprintf(stderr,"### Found\n");
			MCD_lock=0;
			return ps;
		}
		p=(MCD_NODE*)p->next;
	}
	MCD_lock=0;
	if(MCD_Settings.verbosity>2)
		fprintf(MCD_RTL,"### Not Found\n");

	return (MCD_PTR_STATE*)0;
}

void	MCD_Add_To_Hash(MCD_HASH_TABLE *dest,void*key, void*data)
{
	unsigned int offset;
	MCD_NODE *p,*n=(MCD_NODE*)malloc(sizeof(MCD_NODE));
	n->next=(MCD_NODE*)0;
	n->data=data;
	
	if(MCD_Settings.verbosity>2)
		fprintf(MCD_RTL,"### Adding 0x%08x to table\n",(int)key);
	
	while(MCD_lock);
	MCD_lock=1;
				
	offset=MCD_Hash_Address(key);
	offset%=dest->size;
	p=dest->Lists[offset];
	while(p)
	{
		if(p->next)
			p=(MCD_NODE*)p->next;
		else
		{
			p->next=(MCD_NODE*)n;
			MCD_lock=0;
			return;
		}
	}
	dest->Lists[offset]=n;	/* list was empty */
	MCD_lock=0;
}

/* deletes a chunk from the allocations list
   we never delete stuff from the free'd table, because we might double free at end */
void	MCD_Del_From_Hash(MCD_HASH_TABLE*hash,void*key)
{
	unsigned int offset;
	MCD_NODE*n,*prev;
	MCD_CHUNK*c;
	
	if(MCD_Settings.verbosity>2)
		fprintf(MCD_RTL,"### Deleting 0x%08x from table\n",(int)key);

	while(MCD_lock);
	MCD_lock=1;
	
	offset=MCD_Hash_Address(key);
	offset%=hash->size;
	prev=(MCD_NODE*)0;
	n=hash->Lists[offset];
	while(n)
	{
		c=(MCD_CHUNK*)n->data;
		/* printf("c:0x%x 0x%x\n",c,c?c->ptr:-1); */
		if(c->ptr==key)
		{
			if(prev)
			{
				prev->next=(MCD_NODE*)n->next;
				free(c);
				free(n);
			}
			else	/* no prev means first in list */
			{
				hash->Lists[offset]=(MCD_NODE*)n->next;
				free(c);
				free(n);
			}
			MCD_lock=0;
			return;			
		}
		prev=n;
		n=(MCD_NODE*)n->next;
	}
	MCD_lock=0;
}

/*  This does most of our grunt work.  it actually allocates the memory, and
	performs any necessary frobbing for under/overflow.
*/
void*	__MCD_Allocate(unsigned int size,int line, const char* function, const char* file)
{
	void *ptr;
	char *p;
	unsigned int i;
	MCD_CHUNK *chunk;
	MCD_PTR_STATE *ps;
	
	if(MCD_Settings.verbosity>1)
		fprintf(MCD_RTL,"## Allocating %i bytes for %s:%s, line %i\n",
				size,file,function,line);

	MCD_Stats.allocations++;
	
	if(MCD_Stats.allocations-MCD_Stats.frees>MCD_Stats.maxAllocations)
		MCD_Stats.maxAllocations=MCD_Stats.allocations-MCD_Stats.frees;
	
	MCD_Stats.totLow+=size;
	MCD_Stats.unfreed+=size;
	
	if(MCD_Stats.unfreed>MCD_Stats.maxMemusage)
		MCD_Stats.maxMemusage=MCD_Stats.unfreed;
	
	if(size>MCD_Stats.biggest)
		MCD_Stats.biggest=size;
	if(size<MCD_Stats.smallest || MCD_Stats.smallest==0)
		MCD_Stats.smallest=size;
	
	ptr=malloc(size+MCD_Settings.OverflowPad);
	chunk=(MCD_CHUNK*)malloc(sizeof(MCD_CHUNK));
	chunk->ptr=ptr;
	chunk->size=size;
	chunk->magic=MCD_MAGIC;
	chunk->line=line;
	chunk->function=function;
	chunk->file=file;
	chunk->id=MCD_ID++;
	
	p=(char*)ptr;
	for(i=0;i<MCD_Settings.OverflowPad;++i)
	{
		p[size+i]=0xff-(i&0xff);	/* fffefd...  */
	}
	
	/* add the new pointer to the Allocations table */
	MCD_Add_To_Hash(&MCD_Allocations,ptr,chunk);
	
	/* update for double free, if requested... */
	if(MCD_Settings.Flags&MCD_DOUBLE_FREE)
	{
		/* clear pointer in Frees list (reused pointer) */
		if((ps=MCD_Hash_Lookup_Freed(ptr))!=0)
		{
			ps->state=PTR_ALLOCATED;
		}
		else	/* not already in list, so we have to add it... */
		{
			ps=(MCD_PTR_STATE*)malloc(sizeof(MCD_PTR_STATE));
			ps->ptr=ptr;
			ps->state=PTR_ALLOCATED;
			MCD_Add_To_Hash(&MCD_Frees, ptr, ps);
		}
	}

	if(MCD_Settings.verbosity>1)
		fprintf(MCD_RTL,"## New Pointer is 0x%08x\n",(int)ptr);

	__MCD_Display_Hash(&MCD_Allocations);
						
	return ptr;
}

/* our various error codes (for __MCD_Free essentially) */
#define MCD_ERROR_NONE		0	/* all's well */
#define MCD_ERROR_INVALID	1	/* invalid pointer */
#define MCD_ERROR_OVERFLOW	2	/* pointer was overflowed */
#define MCD_ERROR_DOUBLE	3	/* pointer was already freed */
#define MCD_ERROR_CORRUPT	4	/* our structure is broken... */

/*  This does the clean up work, including checking for overflowed writes.
	returns 0 for ok, else an MCD_ERROR_* code.
*/
int __MCD_Free(void *ptr)
{
	/* to free:
		1) look up pointer to see if it's already been freed
		  if so, write a message and bail
		2) check for overflows
		  if so, write a message and continue
		3) free the chunk
		if at any time chunk->magic doesnt equal MCD_MAGIC, die.
	*/
	unsigned int	i;
	unsigned char		*p,ret=MCD_ERROR_NONE;
	MCD_PTR_STATE	*ps;
	MCD_CHUNK	*chunk;

	if(MCD_Settings.verbosity>1)
		fprintf(MCD_RTL,"## Freeing 0x%08x\n",(int)ptr);

	if(MCD_Settings.Flags&MCD_DOUBLE_FREE)
	{
		/*fprintf(stderr,"#### Checking for doublefree... "); */
		ps=MCD_Hash_Lookup_Freed(ptr);
		/*fprintf(stderr,"Found 0x%x [%i]\n",ps,ps?ps->state:-1); */
		/* ok, the pointer isn't guaranteed valid yet, so ps might be bad. */
		if(ps)
		{
			if(ps->state==PTR_FREED)
				return MCD_ERROR_DOUBLE;
			ps->state=PTR_FREED;
		}
		else
			return MCD_ERROR_INVALID;	/* save the alloc lookup */
	}
	chunk=MCD_Hash_Lookup_Alloc(&MCD_Allocations, ptr);
	if(!chunk)
		return MCD_ERROR_INVALID;
	if(chunk->magic!=MCD_MAGIC)
		return MCD_ERROR_CORRUPT;
	MCD_Stats.unfreed-=chunk->size;
	if(MCD_Settings.Flags&MCD_OVERFLOW)
	{
		p=(unsigned char*)ptr;
		for(i=0;i<MCD_Settings.OverflowPad;++i)
		{
			if(p[chunk->size+i]!=0xff-(i&0xff))
			{
				/*fprintf(stderr,"%i disagrees [%x] [%x]\n",i,
						p[chunk->size+i],
						0xff-(i&0xff));*/
				ret=MCD_ERROR_OVERFLOW;
			}
		}
	}
	
	MCD_Del_From_Hash(&MCD_Allocations, ptr);	/* this cleans up chunk for us */
	free(ptr);
	MCD_Stats.frees++;

	__MCD_Display_Hash(&MCD_Allocations);
			
	return ret;
}

/* and finally, our 'public' interface stuff */

#include <pthread.h>  /* Added for survivor debugging, pthread_self() */

void MCD_Free(void *ptr,const char*file, const char*function, int line)
{
	int ret;

	/* this shoulnd't normally happen anyway... */
	if((MCD_Settings.Flags&MCD_INITIALIZED)==0)
		MCD_Init();

	if(MCD_Settings.Flags&MCD_VERBOSE)
		fprintf(MCD_RTL,"### %s::%s Line %i: -- Freeing pointer 0x%08x\n",file,function,line,(int)ptr);
		
	ret=__MCD_Free(ptr);
	
	/* fprintf(stderr,"#### ret=%i, ptr=0x%x\n",ret,(int)ptr); */
	if(ret!=MCD_ERROR_NONE)
	{
		switch(ret)
		{
			case MCD_ERROR_DOUBLE:
				fprintf(MCD_RTL,"### (%d) %s::%s Line: %i -- Pointer 0x%08x has already been freed!\n",pthread_self(),file,function,line,(int)ptr);
				MCD_Stats.doublefree++;
				break;
			case MCD_ERROR_INVALID:
				fprintf(MCD_RTL,"### (%d) %s::%s Line: %i -- Pointer 0x%08x has not been allocated.\n",pthread_self(),file,function,line,(int)ptr);
				MCD_Stats.invalid++;
				break;
			case MCD_ERROR_CORRUPT:
				fprintf(MCD_RTL,"### (%d) %s::%s Line: %i -- **Internal State Appears to Be Corrupt!**\n",pthread_self(),file,function,line);
				break;
			case MCD_ERROR_OVERFLOW:
				fprintf(MCD_RTL,"### (%d) %s::%s Line: %i -- Pointer 0x%08x was overflowed.\n",pthread_self(),file,function,line,(int)ptr);
				MCD_Stats.overflow++;
				break;
			default:
				fprintf(MCD_RTL,"### (%d) %s::%s Line: %i -- How did this happen?  E-Mail cwright@kosada.com\n",pthread_self(),file,function,line);
		}
	}
}

void *MCD_Malloc(unsigned int size, const char *file,  const char*function, 
int line)
{	
	if((MCD_Settings.Flags&MCD_INITIALIZED)==0)
		MCD_Init();
	
	return(__MCD_Allocate(size,line,function, file));
}


void *MCD_Calloc (int size , const char*file, const char*function, int 
line)
{
	char *ptr;
	if((MCD_Settings.Flags&MCD_INITIALIZED)==0)
		MCD_Init();
	ptr=(char*)__MCD_Allocate(size,line,function, file);
	
	memset(ptr,0,size);
	/*for(i=0;i<size;++i)
		ptr[i]=0;*/
	return (void*)ptr;
}

void *MCD_Realloc(void *old, int size, const char*file, const char*function, int line)
{
	void		*p;
	MCD_CHUNK	*chunk;
	MCD_PTR_STATE	*ps;
	
	if((MCD_Settings.Flags&MCD_INITIALIZED)==0)
		MCD_Init();
	
	if(size==0)	/* behave like free */
	{
		MCD_Free(old, file, function, line);
		return NULL;
	}
	
	chunk=MCD_Hash_Lookup_Alloc(&MCD_Allocations, old);
	if(!chunk)
	{
		/* pointer isn't legal ... */
		
		return NULL;
	}
			
	p=realloc(old, size);
	chunk->size=size;
	chunk->ptr=p;
	
	if(MCD_Settings.Flags & MCD_DOUBLE_FREE)
	{
		/* clear allocation status on old pointer,
		   set allocated status on new pointer (p)
		*/
		ps=MCD_Hash_Lookup_Freed(old);
		if(!ps)
		{
			/* allocation chunk, but no pointer status?  corruption... */
			return p;
		}
		ps->state=PTR_FREED;
		ps=(MCD_PTR_STATE*)malloc(sizeof(MCD_PTR_STATE));
		ps->ptr=p;
		ps->state=PTR_ALLOCATED;
		MCD_Add_To_Hash(&MCD_Frees,p,ps);
	}
	
	return p;
}

char* MCD_Getcwd (char*p, int size, const char*file, const char*function, int line)
{
	int olderrno,wsize=256;
	char *tmp=NULL;
	
	if((MCD_Settings.Flags&MCD_INITIALIZED)==0)
		MCD_Init();
	
	/* according to man page:
	 * if p==NULL, size bytes are allocated
	 * if p==NULL && size==0, strlen(wd) bytes are allocated */
	
	if(p!=NULL)
		return (char*)getcwd(p,size);
	if(!size)
	{
		while(tmp==NULL)
		{
			tmp=(char*)malloc(wsize);
			olderrno=errno;
			if((void*)getcwd(tmp,0)==NULL)
			{
				/* too small, so double size and retry */
				wsize*=2;
				free(tmp);
			}
			errno=olderrno;	/* restore previous stuff w/o getcwd damage */
		}
		p=(char*)__MCD_Allocate(strlen(tmp)+1,line,function, file);
		memcpy(p,tmp,strlen(tmp)+1);
		free(tmp);
	}
	else
	{
		p=(char*)__MCD_Allocate(size,line,function, file);
		getcwd(p,size);
	}
	return p;
}

#ifdef __GNUC__
void *MCD_Strdup (const char *s, const char*file, const char*function, int line)
#else
void *MCD_Strdup (char *s, const char*file, const char*function, int line)
#endif	/* __GNUC__ */
{
	char *p=(char*)__MCD_Allocate(strlen(s),line,function,file);
	memcpy(p,s,strlen(s));
	return p;
}

#ifndef WIN32
#ifdef __GNUC__
void *MCD_Strndup(const char*s, int n, const char*file, const char*function, int line)
#else
void *MCD_Strndup(char*s, int n, const char*file, const char*function, int line)
#endif	/* __GNUC__ */
{
	char *p;
	int len;
	len=strlen(s);
	
	if((MCD_Settings.Flags&MCD_INITIALIZED)==0)
		MCD_Init();
	
	if(len>n)	/* too big, so truncate */
	{
		p=(char*)__MCD_Allocate(n,line,function,file);
		memcpy(p,s,n);
		p[n-1]=0;
	}
	else
	{
		p=(char*)__MCD_Allocate(len+1,line,function,file);
		memcpy(p,s,len+1);
	}
	return p;
}
#endif	/* WIN32 */

/* asprintf/vasprintf support based on a patch from Stephen Lee <slee@tuxsoft.com> */
int   MCD_Asprintf (char **ptr, const char *fmt, const char*file, const char*function, int line,...)
{
	int retval;
	va_list argptr;
	char *finalptr;

	if((MCD_Settings.Flags&MCD_INITIALIZED)==0)
		MCD_Init();
        
	va_start(argptr,line);
	if((retval=vasprintf(ptr, fmt, argptr))<0)
	{
		return retval;
	}
	va_end(argptr);

	finalptr=(char*)__MCD_Allocate(strlen(*ptr)+1,line,function,file);
	memcpy(finalptr,*ptr,strlen(*ptr)+1);
	free(*ptr);
	*ptr=finalptr;

	return retval;
}

int   MCD_Vasprintf(char **ptr, const char *fmt,va_list argptr, const char*file, const char*function, int line)
{
	int retval;
	char *finalptr;
	
	if((retval=vasprintf(ptr,fmt,argptr))<0)
	{
		return retval;
	}
	finalptr=(char*)__MCD_Allocate(strlen(*ptr)+1,line,function,file);
	memcpy(finalptr,*ptr,strlen(*ptr)+1);
	free(*ptr);
	*ptr=finalptr;
	
	return retval;
}


#ifndef WIN32
/* scanf family support provided by Stephen Lee too */

void scan_args(const char*fmt,va_list argptr,const char*file,const char*function,int line)
{
	char **ptr;
	char *finalptr;
        void *dummy;    /* clear up the unused warning */

	for(;*fmt;fmt++)
	{
		if(*fmt!='%')
			continue;
		switch(*(++fmt))
		{
			case 'a': /* malloc'd string */
				ptr=(char **)va_arg(argptr,char *);
				finalptr=(char*)__MCD_Allocate(strlen(*ptr)+1,line,function,file);
				memcpy(finalptr,*ptr,strlen(*ptr+1));
				free(*ptr);
				*ptr=finalptr;
				break;
			case '%':
				break;
			default: /* next arg */
				dummy=va_arg(argptr,void *);
				break;
		}
	}
	return;

}

int   MCD_Scanf (const char *fmt, const char*file, const char*function, int line, ...)
{
	int retval;
	va_list argptr;
	
	va_start(argptr,line);
	if((retval=vscanf(fmt,argptr))<1) {
		/* no args, so let's return */
		va_end(argptr);
		return retval;
	}
	va_end(argptr);
	va_start(argptr,line);
	scan_args(fmt,argptr,file,function,line);
	va_end(argptr);
	return retval;
}



int   MCD_Fscanf(FILE *stream,   const char*fmt, const char*file, const char*function, int line, ...)
{
	int retval;
	va_list argptr;

	va_start(argptr,line);
	if((retval=vfscanf(stream,fmt,argptr))<1) {
		/* no args, so let's return */
		va_end(argptr);
		return retval;
	}
	va_end(argptr);
	va_start(argptr,line);
	scan_args(fmt,argptr,file,function,line);
	va_end(argptr);
	return retval;
}

int   MCD_Sscanf(const char*str, const char*fmt, const char*file, const char*function, int line, ...)
{
	int retval;
	va_list argptr;

	va_start(argptr,line);
	if((retval=vsscanf(str,fmt,argptr))<1) {
		/* no args, so let's return */
		va_end(argptr);
		return retval;
	}
	va_end(argptr);
	va_start(argptr,line);
	scan_args(fmt,argptr,file,function,line);
	va_end(argptr);
	return retval;
}

#endif /* WIN32 */

#ifdef __cplusplus
}
#endif
