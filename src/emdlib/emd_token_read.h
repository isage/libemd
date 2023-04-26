#ifndef __EMD_TOKEN_READ_H__
#define __EMD_TOKEN_READ_H__
#include "emd_token.h"
#include <stdio.h>

typedef int (*traverse_emd_func)(Emd_token **, void *);

const char *emd_token_code_namestr(TokenCode token);
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
