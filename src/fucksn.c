#define COBJMACROS
#define SUB_WINDOWS

#include "cm_macro_defs.c"
#include "cm_entry.h"
#include "cm_io.c"
#include "cm_memory.c"

#define NOT_SHELL_EXECUTE
#include "cm_proc_threads.c"

HWND  g_hwnd = NULL;
/* #include "edge.c" */
#include "toz.c"

char	*g_buffer;
char	*g_cookie;
bool	g_once = false;
bool	g_done = false;

HRESULT
script_webview(HRESULT err, wchar_t* result_json)
{
	// NOTE: result_json is a JSON string with the cookie string, e.g. "\"name=value; other=...\""
	if (FAILED(err) || !result_json) return S_FALSE;

	// NOTE: Probably stupid
	uint64_t wide_len = wstrlen(result_json) * sizeof(wchar_t);
	char* json;
	heap_alloc_dz(wide_len, json);

	// FIXME: Takes only ASCII chars here 
	uint64_t json_len = wchar_to_char(result_json, json, wide_len);

	// NOTE: Remove leading/trailing quotes if present
	char* new_cleaned = json;
	if (json_len >= 2 && json[0] == "\"" && json[json_len] == "\"")
	{
		json_len -= 2;
		new_cleaned = json + 1;
	}
	char* cleaned;
	heap_alloc_dz(sizeof(char) * json_len + 1, cleaned);
	// NOTE: Replace escaped sequences (a minimal unescape: JSON escapes for \" and \\ \n \r \t \v)
	uint64_t j = 0;
	for (uint64_t i = 0; i < json_len; i++)
	{
		if (new_cleaned[i] == '\\' && i + 1 < json_len)
		{
			switch (new_cleaned[i + 1])
			{
				case '\\': cleaned[j++] = '\\'; i++; break;
				case '"':  cleaned[j++] = '"';  i++; break;
				case 'a':  cleaned[j++] = '\a'; i++; break;
				case 'r':  cleaned[j++] = '\r'; i++; break;
				case 't':  cleaned[j++] = '\t'; i++; break;
				case 'n':  cleaned[j++] = '\n'; i++; break;
				case 'v':  cleaned[j++] = '\v'; i++; break;
				default: message_box("Error while unescaping json, exitting"); message_box(&new_cleaned[i]); exit(1);
			}
			continue;
		}
		cleaned[j++] = new_cleaned[i];
	}
	heap_free_dz(json);
	g_cookie = cleaned;
	g_done = true;
	return S_OK;
}

void
on_key_down(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if ((GetKeyState(VK_CONTROL) & 0x8000) && (wParam == 'D'))
	{
		ICoreWebView2* webview = (ICoreWebView2*)GetProp(hwnd, L"WEBVIEW");
		if (!webview)
    {
      message_box("No webview!");
      exit(1);
      return ;
    }
		ICoreWebView2_ExecuteScript(webview, L"document.cookie", script_webview);
	}
}

LRESULT CALLBACK
WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_SIZE:		return 0;
		case WM_KEYDOWN: on_key_down(hwnd, msg, wParam, lParam); return 0;
    case WM_QUIT: exit(1);
    case WM_CLOSE: PostQuitMessage(0); break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

DWORD
prout(void* param)
{
	thread_params *info;
	heap_alloc_dz(sizeof(thread_params), info);
	info->lib = LoadLibrary("C:\\Users\\mouschi\\Downloads\\webview2\\runtimes\\win-x64\\native\\WebView2Loader.dll");
	if (!info->lib) {report_error_box("LoadLibrary"); exit(1) ;}

	heap_alloc_dz(sizeof(char) * 256, g_buffer);
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		report_error_box("CoInitializeEx");
		exit(1);
	}
	WNDCLASS wc = {
		.lpfnWndProc = WndProc,
		.hInstance = g_hInstance,
		.lpszClassName = "FuckSN_Class",
	};
	RegisterClass(&wc);
	g_hwnd = CreateWindowEx(0, wc.lpszClassName, "fucksn", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 900, 600, NULL, NULL, g_hInstance, NULL);
	ShowWindow(g_hwnd, SW_SHOWNORMAL);
	UpdateWindow(g_hwnd);

	info->path = L"C:\\Users\\mouschi\\Downloads\\fuckservicenow";
	info->url = L"https://google.com/";
	info->window = g_hwnd;

	Thread thread;
	InitWebView2(info);
	/* if (!thread_launch(info, InitWebView2, &thread)) return 2; */

	MSG msg;
	uint32_t timeout = 30000; // NOTE: 30s
	uint32_t start = GetTickCount();
	while (!g_done)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
      if (msg.message == WM_QUIT || msg.message == WM_CLOSE) break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		/* Sleep(10); */
		/* if ((GetTickCount() - start) > timeout) break; */
	}

	if (!g_done) message_box("Timed out!");
	else message_box(g_cookie);
	DestroyWindow(g_hwnd);
	CoUninitialize();
	heap_free_dz(g_buffer);
	heap_free_dz(info);
	return 0;
}

__declspec(dllexport) void
start(void)
{
	prout(NULL);
}

ENTRY
{
#ifdef CM_DLL
  /*
	 * g_hInstance = hinstDLL;
	 * if (g_once == false)
	 * {
	 * 	Thread thread;
	 * 	if (!thread_launch(NULL, prout, &thread)) return FALSE;
	 * 	g_once = true;
	 * }
   */
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH: break;
		case DLL_THREAD_ATTACH:	 break;
		case DLL_THREAD_DETACH:	 break;
		case DLL_PROCESS_DETACH: if (lpvReserved) { break; } break;
		default: break;
	}
	return TRUE;
#else
  prout(NULL);
	RETURN_FROM_MAIN(0);
#endif
}
