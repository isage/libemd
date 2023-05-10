#include "vita-import.h"
#include "yamlemitter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <yaml.h>

typedef struct
{
  int num;
  struct
  {
    const char *name;
    const char *postfix;
  } names[1024];
} libs_t;

void usage();

char *hextostr(int x)
{
  static char buf[20];
  sprintf(buf, "0x%08x", x);
  return buf;
}

int pack_export_symbols(yaml_emitter_t *emitter, yaml_event_t *event, vita_imports_stub_t **symbols, size_t symbol_n)
{

  if (!yamlemitter_mapping_start(emitter, event))
    return 0;

  for (int i = 0; i < symbol_n; ++i)
  {
    if (!yamlemitter_key_value(emitter, event, symbols[i]->name, hextostr(symbols[i]->NID)))
      return 0;
  }

  yaml_mapping_end_event_initialize(event);
  if (!yaml_emitter_emit(emitter, event))
    return 0;

  return -1;
}

int main(int argc, const char **argv)
{
  if (argc < 3)
  {
    usage();
    return EXIT_FAILURE;
  }

  vita_imports_t *imports = vita_imports_load(argv[1], 1);

  if (imports == NULL)
  {
    goto exit_failure;
  }

  yaml_emitter_t emitter;
  yaml_event_t event;

  /* Create the Emitter object. */
  yaml_emitter_initialize(&emitter);

  FILE *fp = fopen(argv[2], "w");

  yaml_emitter_set_output_file(&emitter, fp);

  if (!yamlemitter_stream_start(&emitter, &event))
    goto error;

  if (!yamlemitter_document_start(&emitter, &event))
    goto error;

  if (!yamlemitter_mapping_start(&emitter, &event))
    goto error;

  for (int m = 0; m < imports->n_modules; ++m)
  {
    vita_imports_module_t *mod = imports->modules[m];
    for (int i = 0; i < mod->n_libs; ++i)
    {
      vita_imports_lib_t *lib = mod->libs[i];

      if (!yamlemitter_key(&emitter, &event, lib->name))
      {
        goto error;
      }

      if (!yamlemitter_mapping_start(&emitter, &event))
      {
        goto error;
      }

      if (!yamlemitter_key_value(&emitter, &event, "nid", hextostr(lib->NID)))
      {
        goto error;
      }

      if (lib->n_functions)
      {

        if (!yamlemitter_key(&emitter, &event, "functions"))
          goto error;

        if (!pack_export_symbols(&emitter, &event, lib->functions, lib->n_functions))
          goto error;
      }

      if (lib->n_variables)
      {

        if (!yamlemitter_key(&emitter, &event, "variables"))
          goto error;

        if (!pack_export_symbols(&emitter, &event, lib->variables, lib->n_variables))
          goto error;
      }

      yaml_mapping_end_event_initialize(&event);
      if (!yaml_emitter_emit(&emitter, &event))
        goto error;
    }
  }

  if (!yamlemitter_mapping_end(&emitter, &event))
    goto error;

  if (!yamlemitter_document_end(&emitter, &event))
    goto error;

  if (!yamlemitter_stream_end(&emitter, &event))
    goto error;

  fclose(fp);
  vita_imports_free(imports);

  return EXIT_SUCCESS;

error:
  fclose(fp);
  yaml_emitter_delete(&emitter);

  vita_imports_free(imports);
exit_failure:
  return EXIT_FAILURE;
}

void usage()
{
  fprintf(stdout, "vita-emd-to-yaml by cat\n"
                  "usage: vita-emd-to-yaml lib.emd lib.yml\n");
}
