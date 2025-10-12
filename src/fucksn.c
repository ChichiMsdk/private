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

typedef struct WebView2_Global
{
  bool                                                        created;
  uint32_t                                                    ref_count;
  ICoreWebView2*                                              window;
  ICoreWebView2Controller*                                    controller;
  ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*  controller_handler;
  ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* environment_handler;
} WebView2_Global;

WebView2_Global g_webview = {0};

HWND g_hwnd = NULL;
uint32_t handler_add_ref(IUnknown* this) { return ++g_webview.ref_count; }
uint32_t handler_release(IUnknown* this)
{
	--g_webview.ref_count;
	if (g_webview.ref_count) return g_webview.ref_count;
	if (g_webview.controller_handler)
	{
		heap_free_dz(g_webview.controller_handler->lpVtbl);
		heap_free_dz(g_webview.controller_handler);
	}
	if (g_webview.environment_handler)
	{
		heap_free_dz(g_webview.environment_handler->lpVtbl);
		heap_free_dz(g_webview.environment_handler);
	}
	return g_webview.ref_count;
}

HRESULT handler_query_interface(IUnknown* this, IID* riid, void** ppvObject)
{ *ppvObject = this; handler_add_ref(this); return S_OK; }

HRESULT
handler_invoke(IUnknown* this, HRESULT errorCode, void* arg)
{
	if (!g_webview.created)
	{
		g_webview.created = true;
		heap_alloc_dz(sizeof(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler), g_webview.controller_handler);
		heap_alloc_dz(sizeof(ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl), g_webview.controller_handler->lpVtbl);

		g_webview.controller_handler->lpVtbl->AddRef         = handler_add_ref;
		g_webview.controller_handler->lpVtbl->Invoke         = handler_invoke;
		g_webview.controller_handler->lpVtbl->Release        = handler_release;
		g_webview.controller_handler->lpVtbl->QueryInterface = handler_query_interface;

		ICoreWebView2Environment* env = arg;
		env->lpVtbl->CreateCoreWebView2Controller(env, g_hwnd, g_webview.controller_handler);
	}
	else
	{
		ICoreWebView2Controller* controller = arg;
		if (controller)
		{
			g_webview.controller = controller;
			g_webview.controller->lpVtbl->get_CoreWebView2(g_webview.controller, &g_webview.window);
			g_webview.controller->lpVtbl->AddRef(g_webview.controller);
		}
		ICoreWebView2Settings* settings;
		g_webview.window->lpVtbl->get_Settings(g_webview.window, &settings);
		settings->lpVtbl->put_IsScriptEnabled(settings, TRUE);
		settings->lpVtbl->put_AreDefaultScriptDialogsEnabled(settings, TRUE);
		settings->lpVtbl->put_IsWebMessageEnabled(settings, TRUE);
		settings->lpVtbl->put_AreDevToolsEnabled(settings, FALSE);
		settings->lpVtbl->put_AreDefaultContextMenusEnabled(settings, TRUE);
		settings->lpVtbl->put_IsStatusBarEnabled(settings, TRUE);

		RECT bounds;
		GetClientRect(g_hwnd, &bounds);
		g_webview.controller->lpVtbl->put_Bounds(g_webview.controller, bounds);
		wchar_t* url = L"https://digituat.service-now.com/now/sow/home";
		g_webview.window->lpVtbl->Navigate(g_webview.window, url);
		SetProp(g_hwnd, L"WEBVIEW", g_webview.window);
		SetProp(g_hwnd, L"WEBVIEWCONTROLLER", g_webview.controller);
	}
	return S_OK;
}

bool	g_once = false;
bool	g_done = false;
char	*g_cookie;

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
					g_webview.controller->lpVtbl->put_Bounds(g_webview.controller, bounds);
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

	g_hwnd = CreateWindowEx(0, (LPCWSTR)( MAKEINTATOM( RegisterClass(&wndClass))), APPLICATION_NAME, WS_OVERLAPPEDWINDOW, 100, 100, 800, 800, NULL, NULL, NULL, NULL);
	if (!g_hwnd) {report_error_box("CreateWindowEx"); exit(1);}

	HWND main_window = CreateWindowEx(0, (LPCWSTR)( MAKEINTATOM( RegisterClass(&wndClass))), APPLICATION_NAME, WS_OVERLAPPEDWINDOW, 100, 100, 800, 800, NULL, NULL, NULL, NULL);
	if (!g_hwnd) {report_error_box("CreateWindowEx"); exit(1);}

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
#include "error.c"
void
curl(char* url)
{
	// Now make a simple HTTPS GET to protected resource with WinHTTP
	wchar_t *agent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:143.0) Gecko/20100101 Firefox/143.0";
	DWORD proxy_opt    = WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;
	DWORD proxy_name   = WINHTTP_NO_PROXY_NAME;
	DWORD proxy_bypass = WINHTTP_NO_PROXY_BYPASS;
	HINTERNET session = WinHttpOpen(agent, proxy_opt, proxy_name, proxy_bypass, 0);
	if (!session) { report_error_box("WinHttpOpen"); exit(1); }

	wchar_t	*host = L"digituat.service-now.com";
	wchar_t	*path = L"/api/now/graphql";
	HINTERNET cnct = WinHttpConnect(session, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (!cnct) { report_error_box("WinHttpConnect"); WinHttpCloseHandle(session); exit(1); }

  wchar_t	*referrer = L"https://digituat.service-now.com/now/sow/record/interaction/b8c490552b243210729af47ebe91bfb6/params/extra-params/subTabIndex%2F0/selected-tab-index/0/selected-tab/id%3Dcl1kajg2y015e3f71kyb7f5qr/sub/record/incident/a11554552b243210729af47ebe91bf92";
  wchar_t *types[] = {L"*/*"};
	HINTERNET rqst = WinHttpOpenRequest(cnct, L"POST", path, referrer, NULL, types, WINHTTP_FLAG_SECURE);
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

  wchar_t *cookie =
    L"Content-Type application/json";
    /*
     * L"Accept-Language: en-US,en;q=0.5\r\n"
     * L"Accept-Encoding: gzip, deflate, br, zstd\r\n"
     * L"now-ux-interaction: xhz3q4s7cnwz-20654\r\n"
     * L"x-request-cancelable: cmgnzx9n3000h2a95umjt29ne-form_section_8e04439d77013010dc2c885d9f5a9908_reference_caller_id\r\n"
     * L"x-transaction-source: Interface=Web,Interface-Type=Configurable Workspace,Interface-Name=Service Operations Workspace,Interface-SysID=aa881cad73c4301045216238edf6a716,Interface-URI=/now/sow/record/interaction/b8c490552b243210729af47ebe91bfb6/params/extra-params/subTabIndex%2F0/selected-tab-index/0/selected-tab/id%3Dcl1kajg2y015e3f71kyb7f5qr/sub/record/incident/a11554552b243210729af47ebe91bf92\r\n"
     * L"x-usertoken: 70d966e12ba0b210729af47ebe91bfe0576a2f7d4f959a3ddb779cfc13aa5ddf2b51f03c\r\n"
     * L"Origin: https://digituat.service-now.com\r\n"
     * L"DNT: 1\r\n"
     * L"Sec-GPC: 1\r\n"
     * L"Sec-Fetch-Dest: empty\r\n"
     * L"Sec-Fetch-Mode: cors\r\n"
     * L"Sec-Fetch-Site: same-origin\r\n"
     * L"Connection: keep-alive\r\n"
     * L"Cookie: glide_user_route=glide.663f86fe884862f3a4671e8cd50bd134; glide_sso_id=ea698ec21b748c10e53dc9506e4bcb8e; BIGipServerpool_digituat=9c7f0f1612bfab18f52b5351949b02c0; JSESSIONID=AC700AB4ABA9A43A3F4ED2B8284201C0; glide_node_id_for_js=6020ee0e7cc8e143e7414a5588ada3a1f62e8d5af2c2f34687b86b5e67b0c1e0; glide_language=en; glide_user_activity=U0N2M18xOmN3eE9LQ2xLKzhSSUdXSk9lQm1kTHRycDljbW4za00rWnloYThlZmFrSTg9OjlaTFlkWVZkRkFsSllHWGZXbFRaK3FsS2tjR0J4aDQvWjlIYlp3eExUR3M9; __CJ_g_startTime=%221760291107512%22; glide_session_store=30D966E12BA0B210729AF47EBE91BFE0\r\n" L"Priority: u=4";
     */
  wchar_t *data = L"{\"operationName\":\"snRecordReferenceConnected\",\"query\":\"query snRecordReferenceConnected($table:String!$field:String!$sys_id:String$encodedRecord:String$serializedChanges:String$chars:String!$ignoreRefQual:Glide_Boolean$paginationLimit:Int$paginationOffset:Int$sortBy:String$referenceKey:String$overrideReferenceTable:String$query:String$orderByDisplayColumn:Glide_Boolean){GlideLayout_Query{referenceDataRetriever(tableName:$table fieldName:$field encodedRecord:$encodedRecord serializedChanges:$serializedChanges pagination:{limit:$paginationLimit offset:$paginationOffset}ignoreTotalCount:true sysId:$sys_id chars:$chars sysparm_ignore_ref_qual:$ignoreRefQual sortBy:$sortBy sysparm_ref_override:$overrideReferenceTable query:$query orderByDisplayColumn:$orderByDisplayColumn referenceKey:$referenceKey){totalCount recentCount matchesCount referenceRecentDataList{sysId referenceKeyValue referenceData{key value}}referenceDataList{sysId referenceKeyValue referenceData{key value}}}}}\",\"variables\":{\"table\":\"incident\",\"field\":\"caller_id\",\"sys_id\":\"a11554552b243210729af47ebe91bf92\",\"encodedRecord\":\"\",\"serializedChanges\":\"{\\\"caller_id\\\":\\\"\\\",\\\"location\\\":\\\"\\\"}\",\"chars\":\"C\",\"ignoreRefQual\":false,\"paginationLimit\":25,\"sortBy\":\"last_name\",\"referenceKey\":null,\"orderByDisplayColumn\":true},\"nowUxInteraction\":\"xhz3q4s7cnwz-20654\",\"nowUiInteraction\":\"xhz3q4s7cnwz-38975\",\"cacheable\":false,\"__unstableDisableBatching\":true,\"extensions\":{},\"queryContext\":null}";

  if (!WinHttpAddRequestHeaders(rqst, cookie, wstrlen(cookie), WINHTTP_ADDREQ_FLAG_ADD))
  {
    DWORD prout = GetLastError();
    printb("%s", winhttp_getstr(prout));
    SetLastError(prout);
    { report_error_box("AddRequestHeaders"); exit(1);}
  }
  /*
	 * if (!WinHttpSendRequest(rqst, WINHTTP_NO_ADDITIONAL_HEADERS, 0, data, wstrlen(data), wstrlen(data), 0))
	 * // if (!WinHttpSendRequest(rqst, cookie, -1L, data, wstrlen(data), wstrlen(data), 0))
   * {
   *   DWORD prout = GetLastError();
   *   printb("%s", winhttp_getstr(prout));
   *   SetLastError(prout);
   *   { report_error_box("SendRequest"); exit(1);}
   * }
   */

	if (!WinHttpReceiveResponse(rqst, NULL))
	{ report_error_box("ReceiveResponse"); exit(1);}

  void* read_buffer;
  uint64_t buffer_size = 10'000;
  heap_alloc_dz(buffer_size, read_buffer);
  DWORD read = 0;
  if (!WinHttpReadData(rqst, read_buffer, buffer_size, &read))
	{ report_error_box("ReadData"); exit(1);}
  printb("Read: %d bytes", read);
  printb("%s", read_buffer);

	DWORD status = 0;
	DWORD size = sizeof(status);
	WinHttpQueryHeaders(rqst, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &status, &size, NULL);
	printb("HTTP status: %u", (unsigned)status);

	WinHttpCloseHandle(rqst);
	WinHttpCloseHandle(cnct);
	WinHttpCloseHandle(session);
}

__declspec(dllexport) int start(void)
{
  // real_start(NULL);
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
