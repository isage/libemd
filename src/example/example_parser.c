#include "emdlib/scanemddef.h"

#include <stdlib.h>
#include <string.h>

// parse funcs

int emd_def(emd_parse_args *args)
{
  uint64_t val = emd_token_get_integer(args->main_value[0]);
  if (val != 1)
  {
    fprintf(stderr, "%s: unsupport emd version %d.\n > ", "", (uint32_t)val);
    dump_emd_token_list(stderr, args->inlist);
  }
  fprintf(stderr, "emd version %d\n", (uint32_t)val);
  return (val == 1);
}

int modinfo_format(emd_parse_args *args)
{
  // STUB
  return 1;
}

int libattr_def(emd_parse_args *args)
{
  char *libname;
  Emd_token *token;
  TokenCode tc;
  uint32_t attr;

  libname = emd_token_get_string(args->main_value[0]);
  token   = args->main_value[1];
  tc      = emd_token_get_type(token);

  if (tc == TC_INTEGER)
  {
    uint64_t val = emd_token_get_integer(token);
    attr         = val;
    if ((attr & 0xff000000) != 0)
    {
      char *str = emd_token_get_string(args->main_value[1]);
      fprintf(stderr, "Library \"%s\" invalid attribute \'%s\'.\n > ", libname, str);
      dump_emd_token_list(stderr, args->inlist);
      return 0;
    }
  }
  else
  {
    char *str_attr = emd_token_get_string(token);
    attr           = 0;
    if (strcmp(str_attr, "none") == 0)
    {
      attr = 0;
    }
    else if (strcmp(str_attr, "auto_export") == 0)
    {
      attr = 0x1;
    }
    else if (strcmp(str_attr, "weak_export") == 0)
    {
      attr = 0x2;
    }
    else if (strcmp(str_attr, "nolink_export") == 0)
    {
      attr = 0x4;
    }
    else if (strcmp(str_attr, "loose_import") == 0)
    {
      attr = 0x8;
    }
    else if (strcmp(str_attr, "plugin_link") == 0)
    {
      attr = 0x2; // should be 0x2000?
    }
    else if (strcmp(str_attr, "syscall_export") == 0)
    {
      attr = 0x4000;
    }
    else
    {
      char *str = emd_token_get_string(args->main_value[1]);
      fprintf(stderr, "Library \"%s\" invalid attribute \'%s\'.\n > ", libname, str);
      dump_emd_token_list(stderr, args->inlist);
      return 0;
    }
  }
  return 1;
}

int libver_def(emd_parse_args *args)
{
  char *libname;
  uint64_t ver;

  libname = emd_token_get_string(args->main_value[0]);
  ver     = emd_token_get_integer(args->main_value[1]);

  fprintf(stderr, "lib name %s\n", libname);
  fprintf(stderr, "lib version %d\n", (uint32_t)ver);

  return 1;
}

int libstub_def(emd_parse_args *args)
{
  char *libname;
  char *stubname;

  libname  = emd_token_get_string(args->main_value[0]);
  stubname = emd_token_get_string(args->main_value[1]);

  fprintf(stderr, "lib name %s\n", libname);
  fprintf(stderr, "lib stub %s\n", stubname);

  return 1;
}

int libstuball_def(emd_parse_args *args)
{
  char *libname;
  char *stubtmpl;

  libname  = emd_token_get_string(args->main_value[0]);
  stubtmpl = emd_token_get_string(args->main_value[1]);
  return 1;
}

int libsuf_def(emd_parse_args *args)
{
  char *libname;
  char *nidsuffix_src;

  libname       = emd_token_get_string(args->main_value[0]);
  nidsuffix_src = emd_token_get_string(args->main_value[1]);
  return 1;
}

int libnid_def(emd_parse_args *args)
{
  char *libname;
  uint64_t libnid;

  libname = emd_token_get_string(args->main_value[0]);
  libnid  = emd_token_get_integer(args->main_value[1]);
  fprintf(stderr, "lib name %s\n", libname);
  fprintf(stderr, "lib nid 0x%08x\n", libnid);
  return 1;
}

int libfunc_def(emd_parse_args *args)
{
  char *libname;
  char *funcname;

  libname  = emd_token_get_string(args->main_value[0]);
  funcname = emd_token_get_string(args->main_value[1]);
  fprintf(stderr, "lib name %s\n", libname);
  fprintf(stderr, "func name %s\n", funcname);
  if (args->opt_value[4])
    fprintf(stderr, "func nid 0x%08x\n", emd_token_get_integer(args->opt_value[4]));

  return 1;
}

int libvar_def(emd_parse_args *args)
{
  char *libname;
  char *varname;

  libname = emd_token_get_string(args->main_value[0]);
  varname = emd_token_get_string(args->main_value[1]);
  fprintf(stderr, "lib name %s\n", libname);
  fprintf(stderr, "var name %s\n", varname);
  if (args->opt_value[4])
    fprintf(stderr, "var nid 0x%08x\n", emd_token_get_integer(args->opt_value[4]));

  return 1;
}

int implong_data_def(emd_parse_args *args)
{
  char *libname;
  TokenCode tc;
  char *symvalue;
  uint64_t value;

  libname = emd_token_get_string(args->main_value[0]);
  tc      = emd_token_get_type(args->main_value[1]);
  if (tc == TC_INTEGER)
  {
    symvalue = NULL;
    value    = emd_token_get_integer(args->main_value[1]);
  }
  else
  {
    value    = 0;
    symvalue = emd_token_get_string(args->main_value[1]);
  }

  return 1;
}

int long_data_def(emd_parse_args *args)
{
  TokenCode tc;
  char *symvalue;
  uint64_t value;

  if (emd_token_get_type(args->main_value[0]) == TC_INTEGER)
  {
    value    = emd_token_get_integer(args->main_value[0]);
    symvalue = NULL;
  }
  else
  {
    symvalue = emd_token_get_string(args->main_value[0]);
    value    = 0;
  }
  return 1;
}

int stubfile_def(emd_parse_args *args)
{
  char *str;

  str = emd_token_get_string(args->main_value[0]);
  if (args->opt_value[0] == NULL)
  {
  }
  else
  {
    str = emd_token_get_string(args->opt_value[0]);
  }
  return 1;
}

int module_def(emd_parse_args *args)
{
  char *modname;
  uint8_t minor;
  uint8_t major;
  uint64_t val;

  major = 0;
  if (args->opt_value[0] != NULL)
  {
    val   = emd_token_get_integer(args->opt_value[0]);
    major = val;
  }
  minor = 0;
  if (args->opt_value[1] != NULL)
  {
    val   = emd_token_get_integer(args->opt_value[1]);
    minor = val;
  }
  modname = emd_token_get_string(args->main_value[0]);

  return 1;
}

int modattr_def(emd_parse_args *args)
{
  // STUB
  return 1;
}

int modfunc_def(emd_parse_args *args)
{
  char *exportname;
  char *localname;

  exportname = emd_token_get_string(args->main_value[0]);
  localname  = emd_token_get_string(args->main_value[1]);

  // STUB
  return 1;
}

int modvar_def(emd_parse_args *args)
{
  char *exportname;
  char *localname;

  exportname = emd_token_get_string(args->main_value[0]);
  localname  = emd_token_get_string(args->main_value[1]);

  // STUB

  return 1;
}

int modsize_def(emd_parse_args *args)
{
  char *segname;
  int res;
  char *in_ECX;
  char *libname;
  uint64_t val;

  segname = emd_token_get_string(args->main_value[0]);
  val     = emd_token_get_integer(args->main_value[1]);

  fprintf(stderr, "\'module_reserve_size:\' %d\n", val);
  return 1;
}

int moditype_def(emd_parse_args *args)
{
  uint64_t val;
  val = emd_token_get_integer(args->main_value[0]);
  return 1;
}

// parse table with token->subtokens, checks and parse funcs

#define PARSE_TABLE_SIZE 20

emd_parse_table emd_parse_table_table[PARSE_TABLE_SIZE]
    = {{&emd_def,
        {KC_emd, KC_NULL},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_integer, NULL},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&modinfo_format,
        {KC_moduleinfo_output_format, KC_NULL},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, NULL},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&libattr_def,
        {KC_Library, KC_attr},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_symbolinteger},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&libver_def,
        {KC_Library, KC_version},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_integer},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&libstub_def,
        {KC_Library, KC_stubfile},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_string},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&libstuball_def,
        {KC_Library, KC_StubFile},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_string},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&libsuf_def,
        {KC_Library, KC_nidsuffix},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_string},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&libnid_def,
        {KC_Library, KC_libnamenid},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_integer},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&libfunc_def,
        {KC_Library, KC_function},
        {KC_localname, KC_nidname, KC_openlevel, KC_prototype, KC_nidvalue, KC_bind, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_symbol},
        {&ck_symbolstring, &ck_symbolstring, &ck_integer, &ck_string, &ck_integer, &ck_bindgw, NULL, NULL}},
       {&libvar_def,
        {KC_Library, KC_variable},
        {KC_localname, KC_nidname, KC_openlevel, KC_prototype, KC_nidvalue, KC_bind, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_symbol},
        {&ck_symbolstring, &ck_symbolstring, &ck_integer, &ck_string, &ck_integer, &ck_bindgw, NULL, NULL}},
       {&implong_data_def,
        {KC_Library, KC_importer_long_data},
        {KC_label, KC_section, KC_flag, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_symbolinteger},
        {&ck_symbol, &ck_string, &ck_string, NULL, NULL, NULL, NULL, NULL}},
       {&stubfile_def,
        {KC_StubFile, KC_NULL},
        {KC_Library, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_string, NULL},
        {&ck_symbol, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&module_def,
        {KC_Module, KC_NULL},
        {KC_major_version, KC_minor_version, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, NULL},
        {&ck_integer, &ck_integer, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&modattr_def,
        {KC_module_attr, KC_NULL},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbolinteger, NULL},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&modfunc_def,
        {KC_module_function, KC_localname},
        {KC_bind, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_symbol},
        {&ck_bindgw, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&modvar_def,
        {KC_module_variable, KC_localname},
        {KC_bind, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_symbol},
        {&ck_bindgw, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&modsize_def,
        {KC_module_reserve_size, KC_size},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbol, &ck_integer},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&moditype_def,
        {KC_sceModuleInfo_type, KC_NULL},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_integer, NULL},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}},
       {&long_data_def,
        {KC_exporter_long_data, KC_NULL},
        {KC_label, KC_section, KC_flag, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {&ck_symbolinteger, NULL},
        {&ck_symbol, &ck_string, &ck_string, NULL, NULL, NULL, NULL, NULL}},
       {NULL,
        {KC_NULL, KC_NULL},
        {KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL, KC_NULL},
        {NULL, NULL},
        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}}};

// load and parse
void sample_parse(const char *filename)
{
  Emd_token_buffer *buf = read_emd_token_from_file(filename);
  int res               = scan_emd_entries(buf, emd_parse_table_table, PARSE_TABLE_SIZE);
  free_emd_token_buffer(buf);
}
