#define COBJMACROS
#define SUB_WINDOWS

#include "cm_macro_defs.c"
#include "cm_entry.h"
#include "cm_io.c"
#include "cm_memory.c"

#define NOT_SHELL_EXECUTE
#include "cm_proc_threads.c"
#include "WebView2.h"
#include <winhttp.h>

#define APPLICATION_NAME TEXT("WebView2")

HWND hWnd = NULL;
BOOL bEnvCreated = FALSE;
ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* envHandler;
ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* completedHandler;
ICoreWebView2Controller* webviewController = NULL;
ICoreWebView2* webviewWindow = NULL;

ULONG HandlerRefCount = 0;
ULONG HandlerAddRef(IUnknown* This) { return ++HandlerRefCount; }
ULONG HandlerRelease(IUnknown* This)
{
	--HandlerRefCount;
	if (HandlerRefCount) return HandlerRefCount;
	if (completedHandler)
	{
		heap_free_dz(completedHandler->lpVtbl);
		heap_free_dz(completedHandler);
	}
	if (envHandler)
	{
		heap_free_dz(envHandler->lpVtbl);
		heap_free_dz(envHandler);
	}
	return HandlerRefCount;
}

HRESULT HandlerQueryInterface(IUnknown* This, IID* riid, void** ppvObject)
{ *ppvObject = This; HandlerAddRef(This); return S_OK; }

HRESULT
HandlerInvoke(IUnknown* This, HRESULT errorCode, void* arg)
{
	if (!bEnvCreated)
	{
		bEnvCreated = TRUE;
		heap_alloc_dz(sizeof(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler), completedHandler);
		heap_alloc_dz(sizeof(ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl), completedHandler->lpVtbl);

		completedHandler->lpVtbl->AddRef         = HandlerAddRef;
		completedHandler->lpVtbl->Invoke         = HandlerInvoke;
		completedHandler->lpVtbl->Release        = HandlerRelease;
		completedHandler->lpVtbl->QueryInterface = HandlerQueryInterface;

		ICoreWebView2Environment* env = arg;
		env->lpVtbl->CreateCoreWebView2Controller(env, hWnd, completedHandler);
	}
	else
	{
		ICoreWebView2Controller* controller = arg;
		if (controller)
		{
			webviewController = controller;
			webviewController->lpVtbl->get_CoreWebView2(webviewController, &webviewWindow);
			webviewController->lpVtbl->AddRef(webviewController);
		}
		ICoreWebView2Settings* Settings;
		webviewWindow->lpVtbl->get_Settings(webviewWindow, &Settings);
		Settings->lpVtbl->put_IsScriptEnabled(Settings, TRUE);
		Settings->lpVtbl->put_AreDefaultScriptDialogsEnabled(Settings, TRUE);
		Settings->lpVtbl->put_IsWebMessageEnabled(Settings, TRUE);
		Settings->lpVtbl->put_AreDevToolsEnabled(Settings, FALSE);
		Settings->lpVtbl->put_AreDefaultContextMenusEnabled(Settings, TRUE);
		Settings->lpVtbl->put_IsStatusBarEnabled(Settings, TRUE);

		RECT bounds;
		GetClientRect(hWnd, &bounds);
		webviewController->lpVtbl->put_Bounds(webviewController, bounds);
		wchar_t* url = L"https://digit.service-now.com/now/sow/home";
		webviewWindow->lpVtbl->Navigate(webviewWindow, url);
		SetProp(hWnd, L"WEBVIEW", webviewWindow);
		SetProp(hWnd, L"WEBVIEWCONTROLLER", webviewController);
	}
	return S_OK;
}

bool	g_once = false;
bool	g_done = false;
char	*g_cookie;

#define my_function(x, y) ({ int __err = 0; do { __err = function(x, y); switch(__err) { case ERROR: fprintf(stderr, "Error!\n"); break; }} while(0); __err; })

HRESULT
script_webview(HRESULT err, wchar_t* result_json)
{
	message_box("Retrieving cookie..");
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
		if (!webview) { report_error_box("GetProp"); exit(1); }
		ICoreWebView2_ExecuteScript(webview, L"document.cookie", script_webview);
	}
}

LRESULT CALLBACK
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_KEYDOWN: on_key_down(hWnd, uMsg, wParam, lParam); return 0;
		case WM_SIZE:
			{
				if (webviewController)
				{
					RECT bounds;
					GetClientRect(hWnd, &bounds);
					webviewController->lpVtbl->put_Bounds(webviewController, bounds);
				};
				break;
			}
		default: return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK
blabla(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		default: return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

typedef HRESULT (*LPFN_WV) (PCWSTR, PCWSTR, ICoreWebView2EnvironmentOptions*, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);

DWORD real_start(void* param)
{
  char *webview_path = "C:\\Users\\mouschi\\Downloads\\webview2\\runtimes\\win-x64\\native\\WebView2Loader.dll";
  webview_path = "C:\\devel\\webview\\runtimes\\win-x64\\native\\WebView2Loader.dll";
	HMODULE wv_lib = LoadLibrary(webview_path);
	if (!wv_lib) { report_error_box("LoadLibrary"); return 1;}
	HRESULT hr;

  hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(hr)) { report_error_box("CoInitializeEx"); exit(1);}

	WNDCLASS wndClass = {
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = WindowProc,
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.lpszClassName = APPLICATION_NAME,
	};

	hWnd = CreateWindowEx(0, (LPCWSTR)( MAKEINTATOM( RegisterClass(&wndClass))), APPLICATION_NAME, WS_OVERLAPPEDWINDOW, 100, 100, 800, 800, NULL, NULL, NULL, NULL);
	if (!hWnd) {report_error_box("CreateWindowEx"); exit(1);}

	HWND main_window = CreateWindowEx(0, (LPCWSTR)( MAKEINTATOM( RegisterClass(&wndClass))), APPLICATION_NAME, WS_OVERLAPPEDWINDOW, 100, 100, 800, 800, NULL, NULL, NULL, NULL);
	if (!hWnd) {report_error_box("CreateWindowEx"); exit(1);}

	ShowWindow(hWnd, SW_SHOWNORMAL);

	heap_alloc_dz(sizeof(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler), envHandler);
	heap_alloc_dz(sizeof(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl), envHandler->lpVtbl);

	envHandler->lpVtbl->AddRef = HandlerAddRef;
	envHandler->lpVtbl->Invoke = HandlerInvoke;
	envHandler->lpVtbl->Release = HandlerRelease;
	envHandler->lpVtbl->QueryInterface = HandlerQueryInterface;

	UpdateWindow(hWnd);

	LPFN_WV cwv2_create_with_options = (LPFN_WV) GetProcAddress(wv_lib, "CreateCoreWebView2EnvironmentWithOptions");
	if (!cwv2_create_with_options) {report_error_box("GetProcAddress"); return 1;}

  wchar_t *trash_path = L"C:\\Users\\mouschi\\Downloads\\fuckservicenow";
  trash_path = L"C:\\devel\\private";
	cwv2_create_with_options(NULL, trash_path, NULL, envHandler);

	MSG msg;
	BOOL bRet;
	while ((bRet = GetMessage( &msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1) break;
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
  return 1;
}

void
curl(char* url)
{
	// Now make a simple HTTPS GET to protected resource with WinHTTP
	wchar_t *agent = L"MyUserAgent/1.0";
	DWORD proxy_opt    = WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;
	DWORD proxy_name   = WINHTTP_NO_PROXY_NAME;
	DWORD proxy_bypass = WINHTTP_NO_PROXY_BYPASS;
	HINTERNET session = WinHttpOpen(agent, proxy_opt, proxy_name, proxy_bypass, 0);
	if (!session) { report_error_box("WinHttpOpen"); exit(1); }

	wchar_t	*host = L"example.com";
	wchar_t	*path = L"/protected/resource";

	HINTERNET cnct = WinHttpConnect(session, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (!cnct) { report_error_box("WinHttpConnect"); WinHttpCloseHandle(session); exit(1); }

	HINTERNET rqst = WinHttpOpenRequest(cnct, L"GET", path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if (!rqst) { report_error_box("OpenRequest"); WinHttpCloseHandle(cnct); WinHttpCloseHandle(session); exit(1); }

  /*
	 * // Prepare cookie header
	 * char *header;
	 * heap_alloc_dz(sizeof(char) * 8192, header);
	 * wsprintf(header, "Cookie: %s", cookies);
   * 
	 * // Convert header to wide char because WinHttpAddRequestHeaders expects wide char
	 * wchar_t *wideHeader;
	 * heap_alloc_dz(sizeof(wchar_t) * 8192, wideHeader);
	 * MultiByteToWideChar(CP_UTF8, 0, header, -1, wideHeader, _countof(wideHeader));
   * 
	 * if (!WinHttpAddRequestHeaders(rqst, wideHeader, -1L, WINHTTP_ADDREQ_FLAG_ADD))
	 * { report_error_box("AddRequestHeaders"); exit(1);}
   */

	if (!WinHttpSendRequest(rqst, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
	{ report_error_box("SendRequest"); exit(1);}

	if (!WinHttpReceiveResponse(rqst, NULL))
	{ report_error_box("ReceiveResponse"); exit(1);}

	// Read response (simple)
	DWORD status = 0;
	DWORD size = sizeof(status);
	WinHttpQueryHeaders(rqst, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &status, &size, NULL);
	char *buffer;
	heap_alloc_dz(sizeof(char) * 1024, buffer);
	wsprintf(buffer, "HTTP status: %u", (unsigned)status);
	message_box(buffer);

	// cleanup
	WinHttpCloseHandle(rqst);
	WinHttpCloseHandle(cnct);
	WinHttpCloseHandle(session);
}

__declspec(dllexport) int start(void)
{
  /* real_start(NULL); */
	curl(NULL);
  return 0;
}

/* BOOL DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved ) */
ENTRY
// int main(void)
{
	/* MessageBox(NULL, "Loaded dll!", "Info", MB_OK); */
	return TRUE;
}
