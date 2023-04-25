#ifndef __SCANEMDDEF_H__
#define __SCANEMDDEF_H__

#include "emd_token_read.h"

typedef struct emd_parse_args
{
  struct emd_parse_table *table;
  struct Emd_token **inlist;
  struct Emd_token *main_key[2];
  struct Emd_token *main_value[2];
  struct Emd_token *opt_key[8];
  struct Emd_token *opt_value[8];
} emd_parse_args;

typedef struct emd_parse_table
{
  int (*parse_func)(struct emd_parse_args *);
  enum KeywordCode keys[2];
  enum KeywordCode option[8];
  int (*check_keys[2])(struct Emd_token *);
  int (*check_optkeys[8])(struct Emd_token *);
} emd_parse_table;

typedef struct masterkey_table
{
  enum KeywordCode master;
  enum KeywordCode sub[16];
  struct emd_parse_table *table_top;
} masterkey_table;

int ck_symbolstring(Emd_token *token);
int ck_string(Emd_token *token);
int ck_symbolinteger(Emd_token *token);
int ck_symbol(Emd_token *token);
int ck_integer(Emd_token *token);
int ck_bindgw(Emd_token *token);

void build_masterkey_table(emd_parse_table *parse_table, size_t table_size);
void free_masterkey_table(void);
int scan_emd_entries(Emd_token_buffer *emdbuf, emd_parse_table *parse_table, size_t table_size);

#endif