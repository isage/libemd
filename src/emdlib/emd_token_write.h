#ifndef __EMD_TOKEN_WRITE_H__
#define __EMD_TOKEN_WRITE_H__
#include "emd_token.h"
#include "emd_token_read.h"
#include <stdio.h>

typedef struct Emd_token_list {
    uint32_t count;
    struct Emd_token* tokenlist;
} Emd_token_list;

#if defined(__cplusplus)
extern "C" {
#endif

Emd_token_list* start_emd_token_list();
void free_emd_list(Emd_token_list* list);

void add_keyword_emd_token(Emd_token_list* list, KeywordCode value);
void add_int_emd_token(Emd_token_list* list, uint64_t value);
void add_string_emd_token(Emd_token_list* list, const char* value);
void add_symbol_emd_token(Emd_token_list* list, const char* value);

void add_keyword_emd_token(Emd_token_list* list, KeywordCode value);
void add_int_emd_token(Emd_token_list* list, uint64_t value);
void add_symbol_emd_token(Emd_token_list* list, const char* value);
void add_string_emd_token(Emd_token_list* list, const char* value);
void add_custom_emd_token(Emd_token_list* list, TokenCode tc);

void add_newline(Emd_token_list* list);
void add_emd_def(Emd_token_list* list);
void add_library_attr(Emd_token_list* list, const char* libname, const char* attr);
void add_library_version(Emd_token_list* list, const char* libname, uint64_t ver);
void add_library_stubfile(Emd_token_list* list, const char* libname, const char* stubfile);
void add_library_nidsuffix(Emd_token_list* list, const char* libname, const char* suffix);
void add_library_nid(Emd_token_list* list, const char* libname, uint64_t nid);
void add_library_function(Emd_token_list* list, const char* libname, const char* funcname, uint64_t nid);


void write_emd_list(Emd_token_list* list);
void write_emd_list_to_fp(FILE* fp, Emd_token_list* list);

#if defined(__cplusplus)
}
#endif

#endif
