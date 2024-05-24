#ifndef __FREADWRITE_SAFE__
#define __FREADWRITE_SAFE__

#include <stdio.h>
#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif

void set_chunk_max_size(void);
size_t fread_file(void *ptr, size_t size, size_t nitems, FILE *stream, int *errflag, int *eofflag);
char *fread_all_data(char *infile, size_t *filesize, int *errflag, int *eofflag);

#if defined(__cplusplus)
}
#endif

#endif