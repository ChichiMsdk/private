#define SUB_WINDOWS
#define NO_CRT_LINKED

#include "cm_entry.h"
#include "cm_macro_defs.c"
#include "cm_error_handling.c"

typedef int (*start)(void);

ENTRY
{
  /*
   * HMODULE ya = LoadLibrary("C:\\users\\chiha\\Desktop\\webview\\build\\native\\x64\\WebView2Loader.dll");
   * if (!ya) { report_error_box("LoadLibrary"); }
   * else { printf("Success loadlibrary\n"); }
   */

  /*
   * HMODULE yo = LoadLibraryEx("C:\\Users\\chiha\\Desktop\\webview\\runtimes\\win-x64\\native\\WebView2Loader.dll", NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
   * if (!yo) { MessageBox(NULL, "LoadLibrary yo", "Alert", MB_OK); }
   */
  HMODULE ya = LoadLibrary("bin\\test.dll");
  if (!ya) { report_error_box("LoadLibrary"); }
  else { printf("Success loadlibrary\n"); }

  start pute = (start)GetProcAddress(ya, "start");
  pute();

  RETURN_FROM_MAIN(0);
}
