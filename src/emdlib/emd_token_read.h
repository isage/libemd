#ifndef __EMD_TOKEN_READ_H__
#define __EMD_TOKEN_READ_H__
#include "cpp_token_read.h"

typedef enum KeywordCode
{
  KC_NULL                     = 0,
  KC_Library                  = 1,
  KC_Module                   = 2,
  KC_OpenLink_space           = 3,
  KC_StubFile                 = 4,
  KC_attr                     = 5,
  KC_bind                     = 6,
  KC_class                    = 7,
  KC_define                   = 8,
  KC_demangle                 = 9,
  KC_emd                      = 10,
  KC_exporter_long_data       = 11,
  KC_filter                   = 12,
  KC_flag                     = 13,
  KC_function                 = 14,
  KC_importer_long_data       = 15,
  KC_label                    = 16,
  KC_libnamenid               = 17,
  KC_localname                = 18,
  KC_major_version            = 19,
  KC_minor_version            = 20,
  KC_module_attr              = 21,
  KC_module_function          = 22,
  KC_module_reserve_size      = 23,
  KC_module_variable          = 24,
  KC_moduleinfo_output_format = 25,
  KC_name                     = 26,
  KC_namespace                = 27,
  KC_nidname                  = 28,
  KC_nidsuffix                = 29,
  KC_nidvalue                 = 30,
  KC_openlevel                = 31,
  KC_prototype                = 32,
  KC_sceModuleInfo_type       = 33,
  KC_section                  = 34,
  KC_size                     = 35,
  KC_stubfile                 = 36,
  KC_type                     = 37,
  KC_variable                 = 38,
  KC_version                  = 39,
  KC_visibility               = 40,
  KC_MAX_NUMBER               = 41
} KeywordCode;

typedef struct Emd_token Emd_token, *PEmd_token;

struct Emd_token
{
  struct Cpp_token *ctoken;
  char *srcfile;
};

typedef struct Emd_token_buffer Emd_token_buffer, *PEmd_token_buffer;

struct Emd_token_buffer
{
  struct Cpp_token_buffer *cbuf;
  struct Emd_token *tknlist;
};

typedef int (*traverse_emd_func)(Emd_token **, void *);

void dump_emd_token(FILE *fp, Emd_token *etoken);
void dump_emd_token_buffer(FILE *fp, Emd_token_buffer *buf);
void dump_emd_token_list(FILE *fp, Emd_token **tokenlist);

uint64_t emd_token_get_integer(Emd_token *token);
int emd_token_get_keytype(Emd_token *token);
char *emd_token_get_srcfile(Emd_token *token, int *line, int *col);
char *emd_token_get_string(Emd_token *token);
TokenCode emd_token_get_type(Emd_token *token);

void free_emd_token_buffer(Emd_token_buffer *buf);
Emd_token_buffer *read_emd_token_from_buffer(char *strbuf, size_t strsize);
Emd_token_buffer *read_emd_token_from_file(const char *filename);
int traverse_emd_tokens(Emd_token_buffer *buf, traverse_emd_func func, void *opt);

#endif
