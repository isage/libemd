#include "scanemddef.h"

#include <stdlib.h>
#include <string.h>

static masterkey_table *masterkey_table_table;

// generic check funcs

int ck_symbolstring(Emd_token *token)
{
  TokenCode tc;

  tc = emd_token_get_type(token);
  return (tc + ~TC_ERROR_TOKEN < 2);
}

int ck_string(Emd_token *token)
{
  TokenCode tc;

  tc = emd_token_get_type(token);
  return (tc == TC_STRING);
}

int ck_symbolinteger(Emd_token *token)
{
  TokenCode tc;

  tc = emd_token_get_type(token);
  return (tc == TC_INTEGER || tc == TC_SYMBOL);
}

int ck_symbol(Emd_token *token)
{
  TokenCode tc;

  tc = emd_token_get_type(token);
  return (tc == TC_SYMBOL);
}

int ck_integer(Emd_token *token)
{
  TokenCode tc;

  tc = emd_token_get_type(token);
  return (tc == TC_INTEGER);
}

int ck_bindgw(Emd_token *token)
{
  char *str;

  if (emd_token_get_type(token) == TC_SYMBOL)
  {
    str = emd_token_get_string(token);
    if (strcmp(str, "global") != 0 && strcmp(str, "weak") != 0)
    {
      return 0;
    }
    return 1;
  }
  return 0;
}

// error funcs

void unexpected_token(Emd_token *tk, Emd_token **tokenlist)
{
  int col, line;
  const char *src;
  const char *string;

  src    = emd_token_get_srcfile(tk, &line, &col);
  string = emd_token_get_string(tk);
  fprintf(stderr, "%s: line:%d col=%d, unexpected data \'%s\'\n", src, line, col, string);
  dump_emd_token_list(stderr, tokenlist);
}

void needvalue_token(Emd_token *tk, Emd_token **tokenlist)
{
  int line, col;
  const char *src    = emd_token_get_srcfile(tk, &line, &col);
  const char *string = emd_token_get_string(tk);

  fprintf(stderr, "%s: line:%d col=%d, \'%s\' has no value\n", src, line, col, string);
  dump_emd_token_list(stderr, tokenlist);
}

void unknown_format_mainkey(Emd_token *tk, Emd_token **tokenlist)
{
  int line, col;
  const char *src = emd_token_get_srcfile(tk, &line, &col);

  fprintf(stderr, "%s: line:%d col=%d, unknown format data\n", src, line, col);
  dump_emd_token_list(stderr, tokenlist);
}

void unknown_format_subkey(Emd_token *tk, Emd_token **tokenlist)
{
  int line, col;
  const char *src = emd_token_get_srcfile(tk, &line, &col);

  fprintf(stderr, "%s: line:%d col=%d, unknown format data\n", src, line, col);
  dump_emd_token_list(stderr, tokenlist);
}

void unknown_format_optionkey(Emd_token *tk, Emd_token **tokenlist)
{
  int line, col;
  const char *src    = emd_token_get_srcfile(tk, &line, &col);
  const char *string = emd_token_get_string(tk);

  fprintf(stderr, "%s: line:%d col=%d, unknown format data %s\n", src, line, col, string);
  dump_emd_token_list(stderr, tokenlist);
}

void unknown_keys_combo(Emd_token *tk, Emd_token **tokenlist)
{
  int line, col;
  const char *src = emd_token_get_srcfile(tk, &line, &col);
  fprintf(stderr, "%s: line:%d col=%d, unknown keyword combination\n", src, line, col);
  dump_emd_token_list(stderr, tokenlist);
}

// preliminary checks

int check_key_value_pair(Emd_token **tokenlist)
{
  Emd_token *token            = *tokenlist;
  Emd_token **tokenlist_local = tokenlist;
  TokenCode tc;

  while (token != NULL)
  {
    Emd_token *next = tokenlist_local[1];
    tc              = emd_token_get_type(token);
    if (tc != TC_KEYWORD)
    {
      unexpected_token(token, tokenlist);
      return 1;
    }
    if ((next == NULL)
        || (((tc = emd_token_get_type(next), tc != TC_INTEGER && (tc = emd_token_get_type(next), tc != TC_STRING))
             && (tc = emd_token_get_type(next), tc != TC_SYMBOL))))
    {

      needvalue_token(token, tokenlist);
      return 1;
    }
    tokenlist_local = tokenlist_local + 2;
    token           = *tokenlist_local;
  }
  return 0;
}

masterkey_table *lookup_masterkey(Emd_token *token)
{
  KeywordCode key     = masterkey_table_table->master;
  masterkey_table *mp = masterkey_table_table;

  while (key != KC_NULL)
  {
    if (key == emd_token_get_keytype(token))
      return mp;
    mp++;
    key = mp->master;
  }
  return NULL;
}

emd_parse_table *lookup_subkey(Emd_token *token, masterkey_table *mt)
{
  KeywordCode key = mt->master;
  int i;
  for (i = 0; i < 16; ++i)
  {
    if (mt->sub[i] == emd_token_get_keytype(token))
      return &mt->table_top[i];
  }
  return NULL;
}

int lookup_optionkey(Emd_token *token, emd_parse_table *table)
{
  KeywordCode key;
  int i;
  key = emd_token_get_keytype(token);
  for (i = 0; i < 8; i++)
  {
    if (key == table->option[i])
    {
      return i;
      break;
    }
  }
  return -1;
}

int lookup_parse_table(emd_parse_args *parg)
{
  int i;
  masterkey_table *mp;
  emd_parse_table *table;
  Emd_token *masterkey, *subkey, **tkp;
  Emd_token **tokenlist;

  tokenlist = parg->inlist;
  masterkey = tokenlist[0];
  tkp       = tokenlist;
  while (masterkey != NULL)
  {
    mp = lookup_masterkey(masterkey);
    if (mp != NULL)
    {
      parg->main_value[0] = tkp[1];
      parg->main_key[0]   = tkp[0];
      if (mp->sub[0] == KC_NULL)
      {
        table      = mp->table_top;
        int result = table->check_keys[0](parg->main_value[0]);
        if (result == 0)
        {
          unexpected_token(parg->main_value[0], tokenlist);
          return 1;
        }
      }
      else
      {
        if (tkp[2] == NULL)
        {
          unknown_keys_combo(masterkey, tokenlist);
          return 1;
        }
        subkey = tkp[2];

        table = lookup_subkey(subkey, mp);
        if (table)
        {
          parg->main_value[0] = tkp[1];
          parg->main_key[0]   = tkp[0];

          parg->main_value[1] = tkp[3];
          parg->main_key[1]   = tkp[2];
          int result          = table->check_keys[0](parg->main_value[0]);
          if (result == 0)
          {
            unexpected_token(parg->main_value[0], tokenlist);
            return 1;
          }
          result = table->check_keys[1](parg->main_value[1]);
          if (result == 0)
          {
            unexpected_token(parg->main_value[1], tokenlist);
            return 1;
          }
        }
        else
        {
          unknown_format_subkey(subkey, tokenlist);
          return 1;
        }
      }
      // check optkeys
      {
        if (tkp[4])
        {
          int idx = lookup_optionkey(tkp[4], table);
          if (idx >= 0)
          {
            parg->opt_key[idx]   = tkp[4];
            parg->opt_value[idx] = tkp[5];
            int result           = table->check_optkeys[idx](parg->opt_value[idx]);
            if (result == 0)
            {
              unexpected_token(parg->opt_value[idx], tokenlist);
              return 1;
            }
          }
        }
      }
      parg->table = table;
      break;
    }
    else
    {
      unknown_format_mainkey(masterkey, tokenlist);
      return 0;
    }
    tkp       = tkp + 2;
    masterkey = tkp[0];
  }

  if (parg->table != NULL)
  {
    int res = parg->table->parse_func(parg);
    if (res == 1)
      return 0;
  }

  return 1;
}

// callback func
int parse_emd(Emd_token **tokenlist, void *opt)
{
  int *errcnt = (int *)opt;
  emd_parse_args parg;

  memset(&parg, 0, sizeof(emd_parse_args));

  if (check_key_value_pair(tokenlist) != 0)
  {
    *errcnt++;
    return 1;
  }

  parg.inlist = tokenlist;

  if (lookup_parse_table(&parg) != 0)
  {
    *errcnt++;
    return 1;
  }
  return 0;
}

void build_masterkey_table(emd_parse_table *parse_table, size_t table_size)
{
  int maxentry;
  masterkey_table *mp;
  emd_parse_table *ep;

  mp                    = (masterkey_table *)calloc(table_size, sizeof(masterkey_table));
  masterkey_table_table = mp;

  ep = parse_table;

  while (ep->parse_func != NULL)
  {
    mp->table_top = ep;
    mp->master    = ep->keys[0];

    maxentry          = 0;
    mp->sub[maxentry] = ep->keys[1];
    maxentry++;
    ep++;
    while (ep->keys[0] == mp->master)
    {
      mp->sub[maxentry] = ep->keys[1];
      maxentry++;
      ep++;
    }
    mp++;
  }
  mp->master = KC_NULL;
  return;
}

void free_masterkey_table(void)
{
  free(masterkey_table_table);
}

int scan_emd_entries(Emd_token_buffer *emdbuf, emd_parse_table *parse_table, size_t table_size)
{
  int errcnt = 0;

  build_masterkey_table(parse_table, table_size);
  traverse_emd_tokens(emdbuf, parse_emd, &errcnt);
  free_masterkey_table();
  return errcnt;
}
