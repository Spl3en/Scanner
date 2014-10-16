#include "Scanner.h"

BbQueue *memscan_search_cond (MemProc *mp, unsigned char *desc, unsigned char *pattern, unsigned char *search_mask, unsigned char *res_mask, bool (*cond)(MemProc *mp, BbQueue *q))
{
	memproc_search(mp, pattern, search_mask, NULL, SEARCH_TYPE_BYTES);
	BbQueue *results = memproc_get_res(mp);
	BbQueue *mbres = NULL;
	char *str;
	DWORD ptr;
	int len;

	if (bb_queue_get_length(results) > 1)
	{
		debug("%s : (%d) occurences found : ", desc, bb_queue_get_length(results));
	}

	if (bb_queue_get_length(results) == 0)
	{
		important("\"%s\" : Nothing found", desc);
		bb_queue_free(results);
		return NULL;
	}

	do {
		if (bb_queue_get_length(results) == 0)
		{
			bb_queue_free(results);
			debug("Pattern found, but condition does not match.");
			return NULL;
		}

		MemBlock *block = bb_queue_get_first(results);

		str = block->data;
		ptr = block->addr;

		if (res_mask == NULL)
			res_mask = search_mask;

		mbres = scan_search(str, res_mask);

		debug("\"%s\" : found at 0x%.8x :", desc, ptr);
		str = desc;

		memblock_free(block);

	} while (cond != NULL && !cond(mp, mbres));

	foreach_bbqueue_item (mbres, Buffer *b)
	{
		len = 0;
		memcpy(&ptr, b->data, sizeof(DWORD));

		while (*str != '/' && *str != '\0')
		{
			str++;
			len++;
		}

		char buffer[4] = {[0 ... 3] = 0};
		read_from_memory(mp->proc, buffer, ptr, 4);

		int ibuffer = 0;
		float fbuffer = 0;
		memcpy(&ibuffer, buffer, sizeof(buffer));
		memcpy(&fbuffer, buffer, sizeof(buffer));
		debug("\t%.*s -> 0x%.8x (%.2d - 0x%.8x - %.2f)", len, str-len, ptr, ibuffer, ibuffer, fbuffer);
		str++;
	}

	return mbres;
}

BbQueue *memscan_search_string (
	MemProc *mp,
	char *description,
	char *string
) {
	char *search_mask = strdup(string);
	memset(search_mask, 'x', strlen(search_mask));

	memproc_search (mp, string, search_mask, NULL, SEARCH_TYPE_BYTES);
	return memproc_get_res(mp);
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
