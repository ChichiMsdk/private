#define COBJMACROS
#define SUB_WINDOWS

#include "cm_macro_defs.c"
#include "cm_entry.h"
#include "cm_io.c"
#include "cm_memory.c"

#define NOT_SHELL_EXECUTE
#include "cm_proc_threads.c"
#include "WebView2.h"
#include "winhttp.c"

#include <wincred.h>

#include "webview.c"

void
on_key_down(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if ((GetKeyState(VK_CONTROL) & 0x8000) && (wparam == 'D'))
	{
		ICoreWebView2* webview = (ICoreWebView2*)GetProp(hwnd, L"WEBVIEW");
		if (!webview) { report_error_box("GetProp"); exit(1); }
		ICoreWebView2_ExecuteScript(webview, L"document.cookie", script_webview);
	}
}

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		case WM_KEYDOWN: on_key_down(hwnd, umsg, wparam, lparam); return 0;
		case WM_SIZE:
			{
				if (g_webview.controller)
				{
					RECT bounds;
					GetClientRect(hwnd, &bounds);
				ICoreWebView2Controller_put_Bounds(g_webview.controller, bounds);
				};
				break;
			}
		case WM_QUIT: ExitProcess(1);
		default: return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	return 0;
}

LRESULT CALLBACK
blabla(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		default: return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	return 0;
}

typedef HRESULT (*LPFN_WV) (PCWSTR, PCWSTR, ICoreWebView2EnvironmentOptions*, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);

DWORD
real_start(void* param)
{
  char *webview_path = "..\\webview2\\runtimes\\win-x64\\native\\WebView2Loader.dll";
	HMODULE wv_lib = LoadLibrary(webview_path);
	if (!wv_lib) { report_error_box("LoadLibrary"); return 1;}

  if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{ report_error_box("CoInitializeEx"); return 1;}

	WNDCLASS wndClass = {
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = WindowProc,
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.lpszClassName = APPLICATION_NAME,
	};

	g_hwnd = CreateWindowEx(0, (LPCWSTR)( MAKEINTATOM( RegisterClass(&wndClass))), APPLICATION_NAME, WS_OVERLAPPEDWINDOW, 100, 100, 800, 800, NULL, NULL, NULL, NULL);
	if (!g_hwnd) {report_error_box("CreateWindowEx"); return 1;}

	HWND main_window = CreateWindowEx(0, (LPCWSTR)( MAKEINTATOM( RegisterClass(&wndClass))), APPLICATION_NAME, WS_OVERLAPPEDWINDOW, 100, 100, 800, 800, NULL, NULL, NULL, NULL);
	if (!g_hwnd) {report_error_box("CreateWindowEx"); return 1;}

	ShowWindow(g_hwnd, SW_SHOWNORMAL);

	heap_alloc_dz(sizeof(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler), g_webview.environment_handler);
	heap_alloc_dz(sizeof(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl), g_webview.environment_handler->lpVtbl);

	g_webview.environment_handler->lpVtbl->AddRef = handler_add_ref;
	g_webview.environment_handler->lpVtbl->Invoke = handler_invoke;
	g_webview.environment_handler->lpVtbl->Release = handler_release;
	g_webview.environment_handler->lpVtbl->QueryInterface = handler_query_interface;

	UpdateWindow(g_hwnd);

	LPFN_WV cwv2_create_with_options = (LPFN_WV) GetProcAddress(wv_lib, "CreateCoreWebView2EnvironmentWithOptions");
	if (!cwv2_create_with_options) {report_error_box("GetProcAddress"); return 1;}

  wchar_t *trash_path = L"C:\\Users\\mouschi\\Downloads\\fuckservicenow";
  trash_path = L"C:\\devel\\private";
	cwv2_create_with_options(NULL, trash_path, NULL, g_webview.environment_handler);

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

typedef struct Proxy_Credentials
{
	char *server;
	char *username; uint32_t user_size;
	char *password; uint32_t pass_size;
	WINHTTP_AUTOPROXY_OPTIONS proxy_options;
	WINHTTP_PROXY_INFO proxy_info;

} Proxy_Credentials;

bool
gui_credentials_prompt(Proxy_Credentials *creds)
{
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG proxy_config = {};
	if (!WinHttpGetIEProxyConfigForCurrentUser(&proxy_config))
	{report_error_box("WinHttpGetIEProxyConfigForCurrentUser"); return false;}


	if (proxy_config.lpszAutoConfigUrl)
	{
		creds->proxy_options.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
		creds->proxy_options.lpszAutoConfigUrl = proxy_config.lpszAutoConfigUrl;
	}
	else if (proxy_config.fAutoDetect)
	{
		creds->proxy_options.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
		creds->proxy_options.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
	}
	else printb("No AutoProxy configuration found!");
	creds->proxy_options.fAutoLogonIfChallenged = TRUE;
	DWORD err;
	CREDUI_INFO ci = {
		.cbSize = sizeof(ci),
		.pszMessageText = "Enter proxy credentials",
		.pszCaptionText = "Proxy Authentication Required",
	};
	DWORD flags = CREDUI_FLAGS_DO_NOT_PERSIST | CREDUI_FLAGS_GENERIC_CREDENTIALS;
	BOOL save = FALSE;
	if (CredUIPromptForCredentials(&ci, NULL, NULL, 0, creds->username, creds->user_size, creds->password, creds->pass_size, &save, flags) != NO_ERROR)
	{ report_error_box("CredUIPromptForCredentials"); return false;}
  /*
	 * if (!CredUIConfirmCredentials(creds->server, save))
	 * { report_error_box("CredUIConfirmCredentials"); return false;}
   */

	return true;
}

inline Proxy_Credentials credentials_alloc(user_size, pass_size)
{
	Proxy_Credentials creds = {.user_size = (user_size), .pass_size = (pass_size)};
	heap_alloc_dz(sizeof(char) * creds.user_size, creds.username);
	heap_alloc_dz(sizeof(char) * creds.pass_size, creds.password);
	return creds;
}
#include "console.c"
#include "servicenow.c"
void
curl(void)
{
	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi = { 0 };

	wchar_t *agent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:143.0) Gecko/20100101 Firefox/143.0";
	DWORD proxy_opt    = WINHTTP_ACCESS_TYPE_NO_PROXY;
	// proxy_opt = WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;
	DWORD proxy_name   = WINHTTP_NO_PROXY_NAME;
	DWORD proxy_bypass = WINHTTP_NO_PROXY_BYPASS;
	HINTERNET session = http_open(agent, proxy_opt, proxy_name, proxy_bypass, 0);

	wchar_t	*host = L"digituat.service-now.com";
	wchar_t	*path = L"/api/now/graphql";

	HINTERNET cnct = http_connect(session, host, INTERNET_DEFAULT_HTTPS_PORT);
	HINTERNET rqst = http_open_request(cnct, L"POST", path, NULL, NULL, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
  /*
   * DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
   * http_set_option(rqst, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
   */
  DWORD decompression = WINHTTP_DECOMPRESSION_FLAG_GZIP | WINHTTP_DECOMPRESSION_FLAG_DEFLATE;
  WinHttpSetOption(rqst, WINHTTP_OPTION_DECOMPRESSION, &decompression, sizeof(decompression));
  char     *autocomplete = sn_data_autocomplete;
  http_add_request_headers(rqst, raw, (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);
  DWORD redirect = 0;
  DWORD size = sizeof(redirect);
  WinHttpSetOption(rqst, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, WINHTTP_NO_CLIENT_CERT_CONTEXT, 0);
  WinHttpQueryOption(rqst, WINHTTP_OPTION_REDIRECT_POLICY, &redirect, &size);

	bool done = false;
	DWORD last_status = 0;
	do {
    http_send_request(rqst, WINHTTP_NO_ADDITIONAL_HEADERS, 0, autocomplete, strlen(autocomplete), strlen(autocomplete), 0);
		http_receive_response(rqst);

		void* read_buffer;
		DWORD read = 0;

		DWORD nb_available = 0;
		uint64_t buffer_size = 0;
		if (WinHttpQueryDataAvailable(rqst, &buffer_size))
		{
			heap_alloc_dz(buffer_size, read_buffer);
			http_read_data(rqst, read_buffer, buffer_size, &read);
      printb("%s", read_buffer);
      heap_free_dz(read_buffer)
		}

		DWORD status = 0;
		DWORD size = sizeof(status);
		WinHttpQueryHeaders(rqst, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &status, &size, NULL);

    switch(status)
    {
      case 407:
        printb("Status 407 returned, needs auth!"); exit(1);
        /*
         * WinHttpSetCredentials(rqst, WINHTTP_AUTH_TARGET_PROXY, WINHTTP_AUTH_SCHEME_BASIC, creds.username, creds.password,NULL);
         * WinHttpSendRequest(rqst, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
         * WinHttpReceiveResponse(rqst, NULL );
         * if (last_status == 407) done = true;
         */
      case 200: done = true; break;
      default: done = true; break;
    }
		last_status = status;
	} while(!done);

  /*
   * heap_free_dz(headers);
   * heap_free_dz(autocomplete);
   */
	WinHttpCloseHandle(rqst);
	WinHttpCloseHandle(cnct);
	WinHttpCloseHandle(session);
}

__declspec(dllexport) int start(void)
{
  // real_start(NULL);
	curl();
  return 0;
}

/* BOOL DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved ) */
ENTRY
{
  start();
	RETURN_FROM_MAIN(1);
}
