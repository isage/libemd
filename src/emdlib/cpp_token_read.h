#ifndef __CPP_TOKEN_READ_H__
#define __CPP_TOKEN_READ_H__
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "cpp_token.h"


char *add_str_to_cpp_token_buffer(Cpp_token_buffer *buf, const char *str, int strsize);
const char *cpp_token_code_namestr(TokenCode code);
void dump_cpp_token_buffer(FILE *fp, Cpp_token_buffer *buf);
void free_cpp_token_buffer(Cpp_token_buffer *buf);
void get_cpp_token(char *src, size_t *length, Cpp_token *token);
void read_cpp_token_sub(Cpp_token_buffer *buf, char *src);

Cpp_token_buffer *read_cpp_token_from_file(const char *filename);
Cpp_token_buffer *read_cpp_token_from_buffer(char *strbuf, size_t strsize);

#endif