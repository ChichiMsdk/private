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
  /* trash_path = L"C:\\devel\\private"; */
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
void
curl(void)
{
	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi = { 0 };

  /*
	 * Proxy_Credentials creds = credentials_alloc(256, 256);
	 * if (!gui_credentials_prompt(&creds)) { exit(1); }
   */

	// Now make a simple HTTPS GET to protected resource with WinHTTP
	wchar_t *agent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:143.0) Gecko/20100101 Firefox/143.0";
	DWORD proxy_opt    = WINHTTP_ACCESS_TYPE_NO_PROXY;
	proxy_opt = WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;
	DWORD proxy_name   = WINHTTP_NO_PROXY_NAME;
	DWORD proxy_bypass = WINHTTP_NO_PROXY_BYPASS;
	HINTERNET session = http_open(agent, proxy_opt, proxy_name, proxy_bypass, 0);

	wchar_t	*host = L"digit.service-now.com";
	wchar_t *url  = L"https://digit.service-now.com/api/now/graphql";
	/* url  = L"https://youtube.com"; */
	/* host = L"youtube.com"; */
	wchar_t	*path = L"/api/now/graphql";

	/* http_get_proxy_for_url(session, url, &creds.proxy_options, &creds.proxy_info); */

	HINTERNET cnct = http_connect(session, host, INTERNET_DEFAULT_HTTPS_PORT);
	HINTERNET rqst = http_open_request(cnct, L"POST", path, NULL, NULL, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

  /*
	 * http_set_option(rqst, WINHTTP_OPTION_PROXY, &creds.proxy_info, sizeof(creds.proxy_info));
	 * http_set_option(rqst, WINHTTP_OPTION_PROXY_USERNAME, creds.username, creds.user_size * 2);
	 * http_set_option(rqst, WINHTTP_OPTION_PROXY_PASSWORD, creds.password, creds.pass_size * 2);
   */


	char *data = "{\"operationName\":\"snRecordReferenceConnected\",\"query\":\"query snRecordReferenceConnected($table:String!$field:String!$sys_id:String$encodedRecord:String$serializedChanges:String$chars:String!$ignoreRefQual:Glide_Boolean$paginationLimit:Int$paginationOffset:Int$sortBy:String$referenceKey:String$overrideReferenceTable:String$query:String$orderByDisplayColumn:Glide_Boolean){GlideLayout_Query{referenceDataRetriever(tableName:$table fieldName:$field encodedRecord:$encodedRecord serializedChanges:$serializedChanges pagination:{limit:$paginationLimit offset:$paginationOffset}ignoreTotalCount:true sysId:$sys_id chars:$chars sysparm_ignore_ref_qual:$ignoreRefQual sortBy:$sortBy sysparm_ref_override:$overrideReferenceTable query:$query orderByDisplayColumn:$orderByDisplayColumn referenceKey:$referenceKey){totalCount recentCount matchesCount referenceRecentDataList{sysId referenceKeyValue referenceData{key value}}referenceDataList{sysId referenceKeyValue referenceData{key value}}}}}\",\"variables\":{\"table\":\"interaction\",\"field\":\"opened_for\",\"sys_id\":\"-1\",\"encodedRecord\":\"77ee77ef77eSN2MyZDQ2ZjYzYjU2MTIxMDZlZDkxZDI0YzNlNDVhODPvt6zvt5Qx77es77etLUpxejZSVjk4ci1MN0hBeW1HVGdEZz09OE52WlV0ZU9RWFFRTnYxNXN0NkhyczAzOHNzaGZGejR1a0JkRklMbUNPbENoQlBsSTYxRTZ5akNTOXVMMVoyblZNeXFCem1Bc043ZmYzTko5bHpEZWhtZXU3Mm8wVDdkVE1SNXo2aEg4cUMzdWJGaVNMUjFvZll0OW5TZkRCS0ViMVgySUN1SUlIRWJjclRzT2R1Y3lBXzRoemRjQTdOcExNWEhLZEFtZjFTdnNUVjI2NXY0bUdNQ3JsRTZwU2t5VEdQLWZCZm4zWGVMZExhTGw4c1JPVG1ZLXhOdS1HM1BOMWJuWl9aSFFVLXpEV2lCbUdDNGk4cUVDeFJYMGxQOFlXTkE5cGltMS1uUnlQek5DSl9XSjNkZUhkYjJ1WlZOc1kwZlN4RzhVRlRCcWV1OEtzUUZKOUxsNWl5cmNyeTBPcnFCTFJLV1ZyWlZXWG9hVTdRLXlpRDJ1MmM5QlZMUGQxem9lSTVOaDlzc2lIV0V2bmNtOUFkY0NJX2JRRnZEemxIWFEteXpnaTQxeEs1QWZRRkFDaEdzM2dlQVVPdUY1Z0lhT1RkYlczUVhPbE5veXdRY0EzWjNvUjVIdkluUDJHNWFWb19VQlhMRERnZ29OZE8wV3Z2MHZ5T25FdzFndl94Wkk1clV6WDQ977eu77ev\",\"serializedChanges\":\"{\\\"assigned_to\\\":\\\"52b10f803b632a1097cb6547f4e45ac8\\\",\\\"assignment_group\\\":\\\"3efc509e87dd5910dd76873e8bbb354b\\\"}\",\"chars\":\"c\",\"ignoreRefQual\":false,\"paginationLimit\":25,\"sortBy\":\"name\",\"referenceKey\":null,\"orderByDisplayColumn\":true},\"nowUxInteraction\":\"ez7ff0jmd2xu-881\",\"nowUiInteraction\":\"ez7ff0jmd2xu-11535\",\"cacheable\":false,\"__unstableDisableBatching\":true,\"extensions\":{},\"queryContext\":null}";
	wchar_t* headers = 
		L"Accept: */*\r\n"
		L"Accept-Encoding: gzip, deflate, br, zstd\r\n"
		L"Accept-Language: en-US,en;q=0.9,en-IE;q=0.8\r\n"
		L"Connection: keep-alive\r\n"
		L"Content-Length: 2264\r\n"
		L"Cookie: glide_sso_id=ea698ec21b748c10e53dc9506e4bcb8e; BIGipServerpool_digit=f29e46f58849f1a6c9eb36cf6a4bec0f; glide_user_route=glide.e78bd78402e8f271710f3ec687fa309d; glide_node_id_for_js=be0926185098e520392c47319999e3d13d4354965a39939a0a139832609c8f63; glide_language=en; JSESSIONID=6ACD31CF5AFD37812399234AF10EC025; glide_user_activity=U0N2M18xOnhXdG9hbTVMd0RCb2FPL3VHOS9TeTdJZFZEVGxQRHRCc3Zjc0RkVmxaSWM9OmpFZ0p0UDFJZFV6ei93RGlhZmFDdHE4dHRkOGhTcFkySVY3Ri9PR1dWajg9; glide_session_store=28D0E82A2B2076503463F7E0F291BF2B\r\n"
		L"Host: digit.service-now.com\r\n"
		L"Origin: https://digit.service-now.com\r\n"
		/* L"Referer: https://digit.service-now.com/now/sow/record/interaction/-1_uid_3\r\n" */
		L"Sec-Fetch-Dest: empty\r\n"
		L"Sec-Fetch-Mode: cors\r\n"
		L"Sec-Fetch-Site: same-origin\r\n"
		L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/141.0.0.0 Safari/537.36 Edg/141.0.0.0\r\n"
		L"content-type: application/json\r\n"
		L"now-ux-interaction: ez7ff0jmd2xu-881 \r\n"
		L"sec-ch-ua: \"Microsoft Edge\";v=\"141\", \"Not?A_Brand\";v=\"8\", \"Chromium\";v=\"141\"\r\n"
		L"sec-ch-ua-mobile: ?0\r\n"
		L"sec-ch-ua-platform: \"Windows\"\r\n"
		L"x-request-cancelable: cmgrl5t3000033j9fe63fqi10-form_section_27d659f353010110b569ddeeff7b12ee_reference_opened_for\r\n"
		L"x-transaction-source: Interface=Web,Interface-Type=Configurable Workspace,Interface-Name=Service Operations Workspace,Interface-SysID=aa881cad73c4301045216238edf6a716\r\n"
	  L"x-usertoken: 68d0e82a2b2076503463f7e0f291bf2bdbb0f2cf25dc570807ebad13acf5531899c103d0";

  http_add_request_headers(rqst, headers, (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);

	bool done = false;
	DWORD last_status = 0;
	uint32_t data_len = strlen(data);
	do {
		http_send_request(rqst, WINHTTP_NO_ADDITIONAL_HEADERS, 0, data, data_len, data_len, 0);
		http_receive_response(rqst);

		void* read_buffer;
		DWORD read = 0;

		DWORD nb_available = 0;
		uint64_t buffer_size = 0;
		if (WinHttpQueryDataAvailable(rqst, &buffer_size))
		{
			heap_alloc_dz(buffer_size, read_buffer);
			if (!WinHttpReadData(rqst, read_buffer, buffer_size, &read)) { report_error_box("ReadData"); exit(1);}
			char* path = "C:\\Users\\mouschi\\Downloads\\fuckservicenow\\log.txt";
			cmFile file;
			if (file_open(path, GENERIC_READ | GENERIC_WRITE, 0, &file) != CM_OK) {report_error_box("file_open"); exit(1);}
			write_file(file.h_file, read_buffer, read);
			file_close(&file);
		}

		DWORD status = 0;
		DWORD size = sizeof(status);
		WinHttpQueryHeaders(rqst, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &status, &size, NULL);
		/* printb("HTTP status: %u, Read: %d bytes |%s|", (unsigned) status, read, (char*)read_buffer); */

		if (status == 407)
		{
			printb("Status 407 returned, needs auth!"); exit(1);
      /*
			 * WinHttpSetCredentials(rqst, WINHTTP_AUTH_TARGET_PROXY, WINHTTP_AUTH_SCHEME_BASIC, creds.username, creds.password,NULL);
			 * WinHttpSendRequest(rqst, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
			 * WinHttpReceiveResponse(rqst, NULL );
			 * if (last_status == 407) done = true;
       */
		}
		else break;
		last_status = status;
	} while(!done);

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
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hinstDLL);
			break;
		case DLL_PROCESS_DETACH:
			FreeConsole();
			break;
	}
	return TRUE;
}
