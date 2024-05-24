#include "freadwrite_safe.h"
#include <stdint.h>

static int envset         = 0;
static int verbose        = 0;
static int chunk_max_size = 0;

void set_chunk_max_size(void)
{
  char *env_value;
  envset = 1;

  env_value = getenv("fread_verbose");
  if (env_value)
  {
    verbose = atoi(env_value);
  }
  env_value = getenv("fread_chunk");
  if (env_value)
  {
    chunk_max_size = atoi(env_value) << 0x14;
  }
  return;
}

size_t fread_file(void *ptr, size_t size, size_t nitems, FILE *stream, int *errflag, int *eofflag)
{
  void *_DstBuf;

  size_t read_bytes;
  uint32_t left;
  uint32_t count;
  uint32_t read;
  int tries;

  if (envset == 0)
  {
    set_chunk_max_size();
  }
  if ((chunk_max_size != 0) && (chunk_max_size < size))
  {
    fprintf(stderr, "Panic !! internal error in fread_file(). \n");
    exit(1);
  }

  count = 0;
  tries = 0;
  read  = 0;

  if (nitems > 0)
  {
    while (1)
    {
      left  = nitems - read;
      count = left;
      if ((chunk_max_size != 0) && (chunk_max_size < left * size))
      {
        count = chunk_max_size / size;
      }
      _DstBuf = (void *)(ptr + (size * read));

      read_bytes = fread(_DstBuf, size, count, (FILE *)stream);
      if (verbose != 0)
      {
        printf(" fread(%p, %ld, %ld, fp) retry %d --> %ld byte\n", _DstBuf, size, count, tries, read_bytes);
      }
      if (read_bytes < left)
      {
        count = read + read_bytes;
        if (feof(stream))
        {
          *eofflag = 1;
          break;
        }
        if (ferror(stream))
        {
          *errflag = 1;
          break;
        }
        tries = tries + (uint32_t)(read_bytes == 0);
        if (10000 < tries)
        {
          *errflag = 1;
          break;
        }
      }
      count = read + read_bytes;
      if (nitems <= count)
        break;
      read = read + read_bytes;
    }
  }

  if (verbose != 0)
  {
    printf("  fread_file result --> %ld\n", count);
  }
  return count;
}

char *fread_all_data(char *infile, size_t *filesize, int *errflag, int *eofflag)
{
  FILE *fp;
  size_t nitems;

  char *result;
  size_t count;

  *eofflag = 0;
  *errflag = 0;
  fp       = fopen(infile, "rb");
  if (!fp)
  {
    *errflag = 1;
    result   = "\"%s\" can\'t open\n";
  }
  else
  {
    fseek(fp, 0, 2);
    nitems = ftell(fp);
    fseek(fp, 0, 0);
    result = (char *)calloc(1, nitems + 4);
    if (!result)
    {
      perror(infile);
      fclose(fp);
      *filesize = 0;
      return NULL;
    }
    count = fread_file(result, 1, nitems, (FILE *)fp, errflag, eofflag);
    fclose(fp);
    if (nitems == count)
    {
      *filesize = nitems;
      return result;
    }
    *filesize = 0;
    free(result);
    result = "\"%s\" can\'t read\n";
  }
  fprintf(stderr, result, infile);
  return NULL;
}
