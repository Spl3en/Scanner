#include "Scanner.h"

#define __DEBUG_OBJECT__ "Scanner"
#include "dbg/dbg.h"

DWORD
mem_scanner (
	unsigned char *desc,
	DWORD start, DWORD size,
	unsigned char *pattern,
	unsigned char *search_mask,
	unsigned char *res_mask
) {
	DWORD result = mem_search (start, size, pattern, search_mask);

	if (!result) {
		fail ("\"%s\" : Nothing found", desc);
		return 0;
	}

	dbg ("\"%s\" : Found in .text at 0x%08X", desc, result);

	BbQueue *res = scan_search ((void *) result, res_mask);

	foreach_bbqueue_item (res, Buffer *b)
	{
		int len = 0;
		DWORD ptr;
		memcpy(&ptr, b->data, sizeof(DWORD));

		while (*desc != '/' && *desc != '\0')
		{
			desc++;
			len++;
		}

		result = ptr;

		dbg ("\t%.*s -> 0x%.8x", len, desc-len, ptr);
		desc++;
	}

	return result;
}

BbQueue *memscan_search_cond (MemProc *mp, unsigned char *desc, unsigned char *pattern, unsigned char *search_mask, unsigned char *res_mask, bool (*cond)(MemProc *mp, BbQueue *q))
{
	memproc_search (mp, pattern, search_mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(mp);
	BbQueue *mbres = NULL;
	char *str = desc;
	DWORD ptr;
	int len;

	if (bb_queue_get_length(results) > 1)
	{
		dbg ("%s : (%d) occurences found : ", desc, bb_queue_get_length(results));
	}

	if (bb_queue_get_length(results) == 0)
	{
		fail ("\"%s\" : Nothing found", desc);
		bb_queue_free(results);
		return NULL;
	}

	do {
		if (bb_queue_get_length(results) == 0)
		{
			bb_queue_free(results);
			dbg ("Pattern found, but condition does not match.");
			return NULL;
		}

		MemBlock *block = bb_queue_get_first(results);

		str = block->data;
		ptr = block->addr;

		if (res_mask == NULL)
			res_mask = search_mask;

		mbres = scan_search(str, res_mask);

		dbg ("\"%s\" : found at 0x%.8x :", desc, ptr);
		str = desc;

		memblock_free(block);

	} while (cond != NULL && !cond(mp, mbres));

	foreach_bbqueue_item (mbres, Buffer *b)
	{
		len = 0;
		memcpy(&ptr, b->data, sizeof(DWORD));

		while (*desc != '/' && *desc != '\0')
		{
			desc++;
			len++;
		}

		char buffer[4] = {[0 ... 3] = 0};
		read_from_memory(mp->proc, buffer, ptr, 4);

		int ibuffer = 0;
		float fbuffer = 0;
		memcpy(&ibuffer, buffer, sizeof(buffer));
		memcpy(&fbuffer, buffer, sizeof(buffer));
		dbg ("\t%.*s -> 0x%.8x (%.2d - 0x%.8x - %.2f)", len, desc-len, ptr, ibuffer, ibuffer, fbuffer);
		desc++;
	}

	return mbres;
}

BbQueue *memscan_search_string (
	MemProc *mp,
	char *description,
	char *string
) {
	return memscan_search_buffer (mp, description, string, strlen (string));
}

DWORD memscan_string (
	char *description,
	DWORD start, DWORD end,
	char *string
) {
	return memscan_buffer (description, start, end, string, strlen (string));
}

DWORD memscan_buffer (
	char *description,
	DWORD start, DWORD end,
	char *buffer,
	int bufferSize
) {
	char *search_mask = malloc (bufferSize);
	memset (search_mask, 'x', bufferSize);

	DWORD res = mem_search (start, end, buffer, search_mask);
	free (search_mask);
	dbg ("%s : occurrence found : 0x%08X", description, res);

	return res;
}

DWORD memscan_buffer_mask (
	char *description,
	DWORD start, DWORD end,
	char *buffer,
	int bufferSize,
	char *search_mask
) {
	DWORD res = mem_search (start, end, buffer, search_mask);
	free (search_mask);
	dbg ("%s : occurrence found : 0x%08X", description, res);

	return res;
}

BbQueue *memscan_search_buffer (
	MemProc *mp,
	char *description,
	char *buffer,
	int bufferSize
) {
	char *search_mask = malloc (bufferSize);
	memset (search_mask, 'x', bufferSize);

	memproc_search (mp, buffer, search_mask, NULL, SEARCH_TYPE_BYTES);
	free (search_mask);
	BbQueue *res = memproc_get_res(mp);
	dbg ("%s : %d occurences found.", description, bb_queue_get_length(res));

	return res;
}

BbQueue *memscan_search_all (
	MemProc *mp,
	char *description,
	unsigned char *pattern,
	unsigned char *search_mask
) {
	memproc_search (mp, pattern, search_mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *res = memproc_get_res(mp);
	dbg ("%s : %d occurences found.", description, bb_queue_get_length(res));

	return res;
}

BbQueue *memscan_search (MemProc *mp, unsigned char *desc, unsigned char *pattern, unsigned char *search_mask, unsigned char *res_mask)
{
	return memscan_search_cond(mp, desc, pattern, search_mask, res_mask, NULL);
}

BbQueue *scan_search (unsigned char *pattern, unsigned char *mask)
{
	int len = strlen(mask);
	BbQueue *res = bb_queue_new();
	Ztring *z = ztring_new();

	for (int i = 0; i < len; i++)
	{
		if (mask[i] == '?')
			ztring_concat_letter(z, pattern[i]);

		else if (ztring_get_len(z) != 0)
		{
			bb_queue_add(res, z);
			z = ztring_new();
		}
	}

	if (ztring_get_len(z) != 0)
		bb_queue_add(res, z);

	BbQueue *res_buffer = bb_queue_new();

	while (bb_queue_get_length(res))
	{
		z = bb_queue_get_first(res);

		Buffer *b = buffer_new_from_ptr_noalloc(ztring_get_text(z), ztring_get_len(z));
		bb_queue_add (res_buffer, b);

		ztring_free(z);
	}

	bb_queue_free(res);

	return res_buffer;
}
