#include "emdlib/emd_token_write.h"

int main(int argc, char *argv[])
{
  Emd_token_list* list = start_emd_token_list("test.emd"); // sets filename for write_emd_list

  add_emd_def(list);
  add_newline(list);
  add_library_attr(list, "TestLib", "auto_export");
  add_library_version(list, "TestLib", 1);
  add_library_stubfile(list, "TestLib", "TestLib_stub");
  add_library_nidsuffix(list, "TestLib", "TEST");
  add_library_nid(list, "TestLib", 0xDEADBEEF);
  add_library_function(list, "TestLib", "test_function", 0);
  add_library_function(list, "TestLib", "test_function2", 0x81680085);

  write_emd_list(list);

  // OR
  
  FILE* fp = fopen("test2.emd", "wb");
  write_emd_list_to_fp(fp, list);
  fclose(fp);

  free_emd_list(list);
  return 0;
}