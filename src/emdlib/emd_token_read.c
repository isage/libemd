#include "emd_token_read.h"

#include <stdlib.h>
#include <string.h>

const char *emd_keyword_table[42] = {"<kc_null> ",
                                     "Library",
                                     "Module",
                                     "OpenLink_space",
                                     "StubFile",
                                     "attr",
                                     "bind",
                                     "class",
                                     "define",
                                     "demangle",
                                     "emd",
                                     "exporter_long_data",
                                     "filter",
                                     "flag",
                                     "function",
                                     "importer_long_data",
                                     "label",
                                     "libnamenid",
                                     "localname",
                                     "major_version",
                                     "minor_version",
                                     "module_attr",
                                     "module_function",
                                     "module_reserve_size",
                                     "module_variable",
                                     "moduleinfo_output_format",
                                     "name",
                                     "namespace",
                                     "nidname",
                                     "nidsuffix",
                                     "nidvalue",
                                     "openlevel",
                                     "prototype",
                                     "sceModuleInfo_type",
                                     "section",
                                     "size",
                                     "stubfile",
                                     "type",
                                     "variable",
                                     "version",
                                     "visibility",
                                     0};

void dump_emd_token(FILE *fp, Emd_token *etoken)
{
  Cpp_token *ctoken;
  const char *tokenstr;

  ctoken   = etoken->ctoken;
  tokenstr = cpp_token_code_namestr(ctoken->tcode);

  fprintf(fp, "%s: %3d:%2d %15s ", etoken->srcfile, ctoken->linenum, ctoken->column, tokenstr);
  if (ctoken->tcode < TC_INTEGER)
  {
    fputc(10, fp);
  }
  else if (ctoken->tcode < TC_KEYWORD)
  {
    fprintf(fp, " %I64d\n", ctoken->v.value);
  }
  else if (ctoken->tcode == TC_KEYWORD)
  {
    fprintf(fp, " %s:\n", emd_keyword_table[ctoken->v.value]);
  }
  else if (ctoken->tcode == TC_STRING)
  {
    fprintf(fp, " \"%s\"\n", ctoken->v.string);
  }
  else
  {
    fprintf(fp, " %s\n", ctoken->v.string);
  }
  return;
}

void dump_emd_token_buffer(FILE *fp, Emd_token_buffer *buf)
{
  TokenCode tc;
  Emd_token *etoken;

  tc     = buf->tknlist->ctoken->tcode;
  etoken = buf->tknlist;
  while (tc != TC_NULL)
  {
    dump_emd_token(fp, etoken);
    tc     = (etoken[1].ctoken)->tcode;
    etoken = etoken + 1;
  }
  return;
}

void dump_emd_token_list(FILE *fp, Emd_token **tokenlist)
{
  Emd_token *etoken;

  etoken = *tokenlist;
  while (etoken != NULL)
  {
    tokenlist = tokenlist + 1;
    dump_emd_token(fp, etoken);
    etoken = *tokenlist;
  }
  return;
}

uint64_t emd_token_get_integer(Emd_token *token)
{
  if (token->ctoken->tcode != TC_INTEGER)
  {
    return 0;
  }
  return token->ctoken->v.value;
}

int emd_token_get_keytype(Emd_token *token)
{
  if (token->ctoken->tcode != TC_KEYWORD)
  {
    return -1;
  }
  return token->ctoken->v.value;
}

char *emd_token_get_srcfile(Emd_token *token, int *line, int *col)
{
  if (line)
  {
    *line = token->ctoken->linenum;
  }
  if (col)
  {
    *col = token->ctoken->column;
  }
  return token->srcfile;
}

char *emd_token_get_string(Emd_token *token)
{
  Cpp_token *ctoken;
  TokenCode tc;
  char *result;
  static char tmp[60];

  ctoken = token->ctoken;
  result = " ";
  tc     = ctoken->tcode;
  if ((tc != TC_WSP) && (result = "\\n", tc != TC_EOL))
  {
    if (tc == TC_INTEGER)
    {
      sprintf(tmp, "%I64d", ctoken->v.value);
      return tmp;
    }
    if (tc == TC_KEYWORD)
    {
      sprintf(tmp, "%s:", emd_keyword_table[ctoken->v.value]);
      return tmp;
    }
    result = ctoken->v.string;
  }
  return result;
}

TokenCode emd_token_get_type(Emd_token *token)
{
  return token->ctoken->tcode;
}

const char *emd_token_code_namestr(TokenCode token)
{
  return emd_keyword_table[token];
}

void free_emd_token_buffer(Emd_token_buffer *buf)
{
  if (buf != NULL)
  {
    free_cpp_token_buffer(buf->cbuf);
    free(buf->tknlist);
    free(buf);
    return;
  }
  return;
}

Emd_token_buffer *build_token_list(Emd_token_buffer *buf)
{
  TokenCode tc;
  Cpp_token *ctoken;
  Emd_token *etoken;
  Cpp_token_buffer *ct_buf;
  short column;
  int count;
  int level;
  char *filename;

  ctoken       = buf->cbuf->tbuf;
  etoken       = (Emd_token *)calloc(buf->cbuf->ntoken, sizeof(Emd_token));
  ct_buf       = buf->cbuf;
  buf->tknlist = etoken;

  if (0 < ct_buf->ntoken)
  {
    ctoken   = ctoken + 1;
    count    = 0;
    level    = 0;
    filename = NULL;
    do
    {
      while (1)
      {
        tc = ctoken[-1].tcode;
        if (tc != TC_FILEINFO)
          break;
        filename = ctoken[-1].v.string;
        count    = count + 1;
        ctoken   = ctoken + 1;
        if (ct_buf->ntoken <= count)
        {
          return buf;
        }
      }
      if (tc == TC_ERROR_TOKEN)
      {
        column = ctoken[-1].column;
        fprintf(stderr, "%s:line=%d col=%d %s\n", filename, ctoken[-1].linenum, column, ctoken[-1].v.string);
        free_emd_token_buffer(buf);
        return NULL;
      }
      if ((level == 0) && (tc == TC_SYMBOL))
      {
        if (ctoken->tcode == TC_COLON)
        {
          char *str = ctoken[-1].v.string;
          if (emd_keyword_table[0] != NULL)
          {
            int idx             = 0;
            const char *keyword = emd_keyword_table[0];
            int res             = 0;
            while (res = strcmp(keyword, str), res != 0)
            {
              keyword = emd_keyword_table[idx + 1];
              if (keyword == NULL)
              {
                column = ctoken[-1].column;
                fprintf(stderr, "%s:line=%d col=%d unknown keyword \'%s:\'\n", filename, ctoken[-1].linenum, column,
                        str);
                free_emd_token_buffer(buf);
                return NULL;
              }
              idx = idx + 1;
            }
            if (idx != 0)
            {
              ctoken[-1].v.value = idx;
              ctoken[-1].tcode   = TC_KEYWORD;
              etoken->ctoken     = ctoken + -1;
              etoken->srcfile    = filename;
              etoken             = etoken + 1;
              ct_buf             = buf->cbuf;
              ctoken->tcode      = TC_WSP;
              count              = count + 1;
              ctoken             = ctoken + 1;
              continue;
            }
          }
          column = ctoken[-1].column;
          fprintf(stderr, "%s:line=%d col=%d unknown keyword \'%s:\'\n", filename, ctoken[-1].linenum, column, str);
          free_emd_token_buffer(buf);
          return NULL;
        }
        etoken->srcfile = filename;
        etoken->ctoken  = ctoken + -1;
        etoken          = etoken + 1;
        if (tc == TC_BRACKETLEFT)
        {
          level = level + 1;
        }
        else if (tc == TC_BRACKETRIGHT)
        {
          if (level == 0)
          {
            fprintf(stderr, "%s:line=%d col=%d unexpected \']\'\n", filename, ctoken[-1].linenum,
                    (int)ctoken[-1].column);
            free_emd_token_buffer(buf);
            return NULL;
          }
          level = level + -1;
        }
      }
      else if ((tc != TC_WSP) || (0 < level))
      {
        etoken->srcfile = filename;
        etoken->ctoken  = ctoken + -1;
        etoken          = etoken + 1;
        if (tc == TC_BRACKETLEFT)
        {
          level = level + 1;
        }
        else if (tc == TC_BRACKETRIGHT)
        {
          if (level == 0)
          {
            fprintf(stderr, "%s:line=%d col=%d unexpected \']\'\n", filename, ctoken[-1].linenum,
                    (int)ctoken[-1].column);
            free_emd_token_buffer(buf);
            return NULL;
          }
          level = level + -1;
        }
      }

      count  = count + 1;
      ctoken = ctoken + 1;
    } while (count <= ct_buf->ntoken);
  }
  return buf;
}

Emd_token_buffer *read_emd_token_from_buffer(char *strbuf, size_t strsize)
{
  Emd_token_buffer *et_buf;
  Cpp_token_buffer *ct_buf;

  et_buf       = (Emd_token_buffer *)calloc(1, 8);
  ct_buf       = read_cpp_token_from_buffer(strbuf, strsize);
  et_buf->cbuf = ct_buf;
  if (ct_buf != NULL)
  {
    return build_token_list(et_buf);
  }
  free(et_buf);
  return NULL;
}

Emd_token_buffer *read_emd_token_from_file(const char *filename)
{
  Emd_token_buffer *et_buf;
  Cpp_token_buffer *ct_buf;

  et_buf       = (Emd_token_buffer *)calloc(1, sizeof(Emd_token_buffer));
  ct_buf       = read_cpp_token_from_file(filename);
  et_buf->cbuf = ct_buf;
  if (ct_buf != NULL)
  {
    return build_token_list(et_buf);
  }
  free(et_buf);
  return NULL;
}

int traverse_emd_tokens(Emd_token_buffer *buf, traverse_emd_func func, void *opt)
{
  Cpp_token *ctoken;
  Emd_token *etoken;
  Emd_token **etoken_list;
  int etoken_list_size;
  TokenCode tc;
  int level;
  int start[32];

  int cnt1;
  int cnt2;

  cnt2 = 1;

  etoken_list_size = 32;
  etoken_list      = (Emd_token **)calloc(etoken_list_size, sizeof(Emd_token));
  cnt1             = 0;

  start[0] = 0;
  level    = 0;
  etoken   = buf->tknlist;

  do
  {
    ctoken = etoken->ctoken;
    tc     = ctoken->tcode;

    switch (tc)
    {
      case TC_NULL:
      case TC_EOL:
      case TC_SEMICOLON:
        if ((tc == TC_NULL) && (level != 0))
        {
          fprintf(stderr, "%s:line=%d col=%d missing \'}\'\n", etoken->srcfile, ctoken->linenum, ctoken->column);
          opt = opt + 1;
          free(etoken_list);
          return 0;
        }
        cnt2 = start[level];
        if (cnt2 < cnt1)
        {
          cnt1 = (*func)(etoken_list, opt);
          if (cnt1 == 1)
          {
            opt = opt + 1;
            free(etoken_list);
            return 0;
          }
          tc = etoken->ctoken->tcode;
        }
        if (tc == TC_NULL)
        {
          free(etoken_list);
          return 1;
        }
        etoken = etoken + 1;
        cnt1   = cnt2;
        break;
      case TC_WSP:
        break;
      case TC_BRACELEFT:
        if (29 < level)
        {
          fprintf(stderr, "%s:line=%d col=%d too many \'{\'\n", etoken->srcfile, ctoken->linenum, ctoken->column);
          opt = opt + 1;
          free(etoken_list);
          return 0;
        }
        level        = level + 1;
        start[level] = cnt1;
        do
        {
          etoken = etoken + 1;
          tc     = etoken->ctoken->tcode;
          if ((tc == TC_NULL) || (tc != TC_KEYWORD))
            break;
        } while (etoken->ctoken->v.value - 1 < 2);
        break;
      case TC_BRACERIGHT:
        if (level == 0)
        {
          fprintf(stderr, "%s:line=%d col=%d unexpected \'}\'\n", etoken->srcfile, ctoken->linenum, ctoken->column);
          opt = opt + 1;
          free(etoken_list);
          return 0;
        }
        if ((start[level] < cnt1) && (cnt2 = (*func)(etoken_list, opt), cnt2 == 1))
        {
          opt = opt + 1;
          free(etoken_list);
          return 0;
        }
        level  = level - 1;
        cnt2   = start[level];
        etoken = etoken + 1;
        cnt1   = cnt2;
        break;
      default:
        etoken_list[cnt1] = etoken;
        etoken_list[cnt2] = NULL;
        etoken            = etoken + 1;
        cnt1              = cnt2;
        break;
    }

    cnt2 = cnt1 + 1;
    if (etoken_list_size <= cnt2)
    {
      etoken_list      = (Emd_token **)realloc(etoken_list, etoken_list_size * 2 * sizeof(Emd_token));
      etoken_list_size = etoken_list_size * 2;
    }
  } while (1);
}
