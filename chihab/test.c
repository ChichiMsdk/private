#define NO_CRT_LINKED
#define SUB_CONSOLE
#define WIN32_NO_BS

#include <cm_entry.h>
#include <cm_macro_defs.c>
#include <cm_string.c>
#include <cm_error_handling.c>
#include <cm_allocator.c>
#include <cm_memory.c>
#include <cm_io.c>

void
dump_file_content(cmFile file)
{
  if (write_file(STDERROR(), file.buffer.view, (uint32_t)file.buffer.size) < 0)
  {
    report_error("write_file");
  }
}

ENTRY
{
  R(r, ET_WIN_BOOL);
  AllocConsole();
  cmFile file = {0};
  char* path = "test.c";
  printf("%s\n", path);
  CM_CODE code = file_exist_open_map_ro("test.c", &file);
  if (code != CM_OK)
  {
    report_error("file_exist_open_map_ro", path);
    RETURN_FROM_MAIN(EXIT_FAILURE);
  }
  else
  {
    dump_file_content(file);
  }
  code = file_close(&file);
  if (code != CM_OK)
  {
    report_error("file_close", file.path);
  }
  console_pause();
  RETURN_FROM_MAIN(EXIT_SUCCESS);
}
