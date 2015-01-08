// --- Author	: Moreau Cyril - Spl3en
// --- File		: Scanner.h
// --- Date		: 2013-04-15-12.05.11
// --- Version	: 1.0

#ifndef Scanner_H_INCLUDED
#define Scanner_H_INCLUDED


// ---------- Includes ------------
#include <stdlib.h>
#include "../MemProc/MemProc.h"

// ---------- Defines -------------


// ------ Struct declaration -------



// --------- Constructors ---------


// ----------- Functions ------------
BbQueue *memscan_search_cond (
	MemProc *mp,
	unsigned char *desc,
	unsigned char *pattern,
	unsigned char *search_mask,
	unsigned char *res_mask,
	bool (*cond)(MemProc *mp, BbQueue *q)
);

BbQueue *memscan_search_string (
	MemProc *mp,
	char *description,
	char *string
);

BbQueue *memscan_search_buffer (
	MemProc *mp,
	char *description,
	char *buffer,
	int bufferSize
);

BbQueue *memscan_search_all (
	MemProc *mp,
	char *description,
	unsigned char *pattern,
	unsigned char *search_mask
);

BbQueue *memscan_search (
	MemProc *mp,
	unsigned char *desc,
	unsigned char *pattern,
	unsigned char *search_mask,
	unsigned char *res_mask
);

BbQueue *scan_search (
		unsigned char *pattern,
		unsigned char *mask
);

// Directly in the process
DWORD memscan_string (
	char *description,
	DWORD start, DWORD end,
	char *string
);

DWORD memscan_buffer (
	char *description,
	DWORD start, DWORD end,
	char *buffer,
	int bufferSize
);

DWORD memscan_buffer_mask (
	char *description,
	DWORD start, DWORD end,
	char *buffer,
	int bufferSize,
	char *search_mask
);

DWORD
mem_scanner (
	unsigned char *desc,
	DWORD start, DWORD size,
	unsigned char *pattern,
	unsigned char *search_mask,
	unsigned char *res_mask
);


// --------- Destructors ----------





#endif // Scanner_INCLUDED
