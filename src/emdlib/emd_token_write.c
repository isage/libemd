#include "emd_token_write.h"
#include <stdlib.h>
#include <string.h>

Emd_token_list* start_emd_token_list(const char* filename)
{
    Emd_token_list* list= (Emd_token_list*)calloc(1, sizeof(Emd_token_list));
    list->count = 1;

    Emd_token* token = (Emd_token*)calloc(1, sizeof(Emd_token));
    token->ctoken = (Cpp_token*)calloc(1, sizeof(Cpp_token));
    token->ctoken->tcode = TC_FILEINFO;
    token->ctoken->v.string = strdup(filename);
    list->tokenlist = token;
    return list;
}

void add_keyword_emd_token(Emd_token_list* list, KeywordCode value)
{
    list->tokenlist = (Emd_token*)realloc(list->tokenlist, (list->count+1) *sizeof(Emd_token));
    list->count++;
    Emd_token* token = list->tokenlist + (list->count - 1);
    token->ctoken = (Cpp_token*)calloc(1, sizeof(Cpp_token));
    token->ctoken->tcode = TC_KEYWORD;
    token->ctoken->v.value = value;
}

void add_int_emd_token(Emd_token_list* list, uint64_t value)
{
    list->tokenlist = (Emd_token*)realloc(list->tokenlist, (list->count+1) *sizeof(Emd_token));
    list->count++;
    Emd_token* token = list->tokenlist + (list->count - 1);
    token->ctoken = (Cpp_token*)calloc(1, sizeof(Cpp_token));
    token->ctoken->tcode = TC_INTEGER;
    token->ctoken->v.value = value;
}

void add_symbol_emd_token(Emd_token_list* list, const char* value)
{
    list->tokenlist = (Emd_token*)realloc(list->tokenlist, (list->count+1) *sizeof(Emd_token));
    list->count++;
    Emd_token* token = list->tokenlist + (list->count - 1);
    token->ctoken = (Cpp_token*)calloc(1, sizeof(Cpp_token));
    token->ctoken->tcode = TC_SYMBOL;
    token->ctoken->v.string = strdup(value);
}

void add_string_emd_token(Emd_token_list* list, const char* value)
{
    list->tokenlist = (Emd_token*)realloc(list->tokenlist, (list->count+1) *sizeof(Emd_token));
    list->count++;
    Emd_token* token = list->tokenlist + (list->count - 1);
    token->ctoken = (Cpp_token*)calloc(1, sizeof(Cpp_token));
    token->ctoken->tcode = TC_STRING;
    token->ctoken->v.string = strdup(value);
}

void add_custom_emd_token(Emd_token_list* list, TokenCode tc)
{
    list->tokenlist = (Emd_token*)realloc(list->tokenlist, (list->count+1) *sizeof(Emd_token));
    list->count++;
    Emd_token* token = list->tokenlist + (list->count - 1);
    token->ctoken = (Cpp_token*)calloc(1, sizeof(Cpp_token));
    token->ctoken->tcode = tc;
}

void add_newline(Emd_token_list* list)
{
    add_custom_emd_token(list, TC_EOL);
}

void add_emd_def(Emd_token_list* list)
{
  add_keyword_emd_token(list, KC_emd);
  add_custom_emd_token(list, TC_WSP);
  add_int_emd_token(list, 1);
  add_custom_emd_token(list, TC_EOL);
}

void add_library_attr(Emd_token_list* list, const char* libname, const char* attr)
{
  add_keyword_emd_token(list, KC_Library);
  add_custom_emd_token(list, TC_WSP);
  add_symbol_emd_token(list, libname);

  add_custom_emd_token(list, TC_WSP);

  add_keyword_emd_token(list, KC_attr);
  add_custom_emd_token(list, TC_WSP);
  add_symbol_emd_token(list, attr);
  add_custom_emd_token(list, TC_EOL);

}

void add_library_version(Emd_token_list* list, const char* libname, uint64_t ver)
{
  add_keyword_emd_token(list, KC_Library);
  add_custom_emd_token(list, TC_WSP);
  add_symbol_emd_token(list, libname);

  add_custom_emd_token(list, TC_WSP);

  add_keyword_emd_token(list, KC_version);
  add_custom_emd_token(list, TC_WSP);
  add_int_emd_token(list, ver);
  add_custom_emd_token(list, TC_EOL);

}

void add_library_stubfile(Emd_token_list* list, const char* libname, const char* stubfile)
{
  add_keyword_emd_token(list, KC_Library);
  add_custom_emd_token(list, TC_WSP);
  add_symbol_emd_token(list, libname);

  add_custom_emd_token(list, TC_WSP);

  add_keyword_emd_token(list, KC_stubfile);
  add_custom_emd_token(list, TC_WSP);
  add_string_emd_token(list, stubfile);
  add_custom_emd_token(list, TC_EOL);
}

void add_library_nidsuffix(Emd_token_list* list, const char* libname, const char* suffix)
{
  add_keyword_emd_token(list, KC_Library);
  add_custom_emd_token(list, TC_WSP);
  add_symbol_emd_token(list, libname);

  add_custom_emd_token(list, TC_WSP);

  add_keyword_emd_token(list, KC_nidsuffix);
  add_custom_emd_token(list, TC_WSP);
  add_string_emd_token(list, suffix);
  add_custom_emd_token(list, TC_EOL);
}

void add_library_nid(Emd_token_list* list, const char* libname, uint64_t nid)
{
  add_keyword_emd_token(list, KC_Library);
  add_custom_emd_token(list, TC_WSP);
  add_symbol_emd_token(list, libname);

  add_custom_emd_token(list, TC_WSP);

  add_keyword_emd_token(list, KC_libnamenid);
  add_custom_emd_token(list, TC_WSP);
  add_int_emd_token(list, nid);
  add_custom_emd_token(list, TC_EOL);
}

void add_library_function(Emd_token_list* list, const char* libname, const char* funcname, uint64_t nid)
{
  add_keyword_emd_token(list, KC_Library);
  add_custom_emd_token(list, TC_WSP);
  add_symbol_emd_token(list, libname);

  add_custom_emd_token(list, TC_WSP);

  add_keyword_emd_token(list, KC_function);
  add_custom_emd_token(list, TC_WSP);
  add_symbol_emd_token(list, funcname);

  if (nid != 0)
  {
      add_custom_emd_token(list, TC_WSP);
      add_keyword_emd_token(list, KC_nidvalue);
      add_custom_emd_token(list, TC_WSP);
      add_int_emd_token(list, nid);
  }

  add_custom_emd_token(list, TC_EOL);
}

void write_emd_list_to_fp(FILE *fp, Emd_token_list* list)
{
    Emd_token* token = list->tokenlist;
    while (token != NULL && token->ctoken != NULL)
    {
        switch(token->ctoken->tcode)
        {
            case TC_EOL:
                fprintf(fp, "\n");
                break;
            case TC_WSP:
                fprintf(fp, " ");
                break;
            case TC_KEYWORD:
                fprintf(fp, "%s:", emd_token_code_namestr(token->ctoken->v.value));
                break;
            case TC_SYMBOL:
                fprintf(fp, "%s", token->ctoken->v.string);
                break;
            case TC_STRING:
                fprintf(fp, "\"%s\"", token->ctoken->v.string);
                break;
            case TC_INTEGER:
                Emd_token* prev = token - 2;
                if (prev->ctoken->tcode == TC_KEYWORD && ((prev->ctoken->v.value == KC_nidvalue) || (prev->ctoken->v.value == KC_libnamenid)))
                    fprintf(fp, "0x%08X", token->ctoken->v.value);
                else
                    fprintf(fp, "%d", token->ctoken->v.value);
                break;
        }
        token++;
    }
}

void write_emd_list(Emd_token_list* list)
{
    Emd_token* token = list->tokenlist;
    if (!token || !token->ctoken || token->ctoken->tcode != TC_FILEINFO)
        return;

    FILE* fp = fopen(token->ctoken->v.string, "wb");
    write_emd_list_to_fp(fp, list);
    fclose(fp);
}

void free_emd_list(Emd_token_list* list)
{
    Emd_token* token = list->tokenlist;
    while (token != NULL && token->ctoken != NULL)
    {
        free(token->ctoken);
        token++;
    }
    free(list->tokenlist);
    free(list);
}

