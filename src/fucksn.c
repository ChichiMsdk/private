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
	HRESULT hr;

  hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(hr)) { report_error_box("CoInitializeEx"); return 1;}

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

	DWORD protocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
	if (!WinHttpSetOption(session, WINHTTP_OPTION_SECURE_PROTOCOLS, &protocols, sizeof(protocols)))
	{report_error_box("WinHttpSetOption"); exit(1);}

	wchar_t	*host = L"digituat.service-now.com";
	wchar_t	*path = L"/api/now/graphql";
	HINTERNET cnct = WinHttpConnect(session, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (!cnct) { report_error_box("WinHttpConnect"); WinHttpCloseHandle(session); exit(1); }

	HINTERNET rqst = WinHttpOpenRequest(cnct, L"POST", path, NULL, NULL, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if (!rqst) { report_error_box("OpenRequest"); WinHttpCloseHandle(cnct); WinHttpCloseHandle(session); exit(1); }


  wchar_t *headers = L"Accept: */*\r\n"
		L"Accept-Encoding: gzip, deflate, br, zstd\r\n"
		L"Accept-Language: en-US,en;q=0.9\r\n"
		L"Connection: keep-alive\r\n"
		L"Content-Length: 2264\r\n"
		L"Cookie: __CJ_g_startTime=%221760363678225%22; glide_sso_id=ea698ec21b748c10e53dc9506e4bcb8e; BIGipServerpool_digituat=9c7f0f1612bfab18f52b5351949b02c0; glide_user_route=glide.663f86fe884862f3a4671e8cd50bd134; glide_node_id_for_js=6020ee0e7cc8e143e7414a5588ada3a1f62e8d5af2c2f34687b86b5e67b0c1e0; JSESSIONID=8F908261244335277291841928F2B868; glide_language=en; glide_user_activity=U0N2M18xOlpMVGxZM2RYeHJjbDBNNnVhaEEyUXJoSWJrUWViK3owNTRseTVEUlh5Tm89Ono5dzB2Vm5qbXdRS2VqV044ZTdkUFlDRnpqaWs5cHF4TWxXN05ST1hEdms9; glide_session_store=CDC347312B603610729AF47EBE91BFCE\r\n"
		L"Host: digituat.service-now.com\r\n"
		/* L"Origin: https://digituat.service-now.com\r\n" */
		L"Sec-Fetch-Dest: empty\r\n"
		L"Sec-Fetch-Mode: cors\r\n"
		L"Sec-Fetch-Site: same-origin\r\n"
		L"content-type: application/json\r\n"
		L"now-ux-interaction: 4vgu7hxugco6-651\r\n"
		L"sec-ch-ua: \"Google Chrome\";v=\"141\", \"Not?A_Brand\";v=\"8\", \"Chromium\";v=\"141\"\r\n"
		L"sec-ch-ua-mobile: ?0\r\n"
		L"sec-ch-ua-platform: \"Windows\"\r\n"
		L"x-request-cancelable: cmgp7w29s0001359e0wljr5bs-form_section_27d659f353010110b569ddeeff7b12ee_reference_opened_for\r\n"
		L"x-transaction-source: Interface=Web,Interface-Type=Configurable Workspace,Interface-Name=Service Operations Workspace,Interface-SysID=aa881cad73c4301045216238edf6a716\r\n"
		L"x-usertoken: 01c347312b603610729af47ebe91bfcf82fd4e6a7b38a23f734eee5a5ab05f926c7d5996\r\n";
	wchar_t *data = L"{\"operationName\":\"snRecordReferenceConnected\",\"query\":\"query snRecordReferenceConnected($table:String!$field:String!$sys_id:String$encodedRecord:String$serializedChanges:String$chars:String!$ignoreRefQual:Glide_Boolean$paginationLimit:Int$paginationOffset:Int$sortBy:String$referenceKey:String$overrideReferenceTable:String$query:String$orderByDisplayColumn:Glide_Boolean){GlideLayout_Query{referenceDataRetriever(tableName:$table fieldName:$field encodedRecord:$encodedRecord serializedChanges:$serializedChanges pagination:{limit:$paginationLimit offset:$paginationOffset}ignoreTotalCount:true sysId:$sys_id chars:$chars sysparm_ignore_ref_qual:$ignoreRefQual sortBy:$sortBy sysparm_ref_override:$overrideReferenceTable query:$query orderByDisplayColumn:$orderByDisplayColumn referenceKey:$referenceKey){totalCount recentCount matchesCount referenceRecentDataList{sysId referenceKeyValue referenceData{key value}}referenceDataList{sysId referenceKeyValue referenceData{key value}}}}}\",\"variables\":{\"table\":\"interaction\",\"field\":\"opened_for\",\"sys_id\":\"-1\",\"encodedRecord\":\"77ee77ef77eSZjc4M2E5ZGUyYmYxNTIxMGZhMmJmY2U4NWU5MWJmNDjvt6zvt5Qx77es77etaTY1WndsMHZROUszT1F3V1djUGVodz09WUF4Ml9ZYTV0TVNRNkdGNkJseEp1ZkxIQVcyR3RkejhZVFRocFlRU2dmdlZEM0VLdTVraXZtd0l6UXMtU2t0RDNwYmVLRFViN2Q5STJXZmFSM1F4NWhYV0RUd04xZjBEOUEwTDhWTkhScE55amx5SGJ3ekN4RFZNSXBOMWxrZjFhS0YwS0xyRDFZMGFiYVVaaGYzZ0tOOUVEOFRQZldfTG54SUMtTDcyS3JGZHJRT2dpSFlHVnNSU083OFFGUjhseW1nOXJuOW5nRFZZbmt5YXh6a0hpeENFOEJlWlJwVE5oWW1sNG1JbEZUdGpBN2R4d2ZRdUlya3hxNGdmVVFGazY4ODVnSV9MMTVqYVRocDRtX3RSejR2SFJ5Wm0zRF90a1BRSmpYbEIzSG1uc3h5MmZvd25OZFRPZWozTTU0ZW04LUVFbjhXN1UzRm1sM0stVzFSaDZKVVVDMFY5OF9Dckl2T0RFeVN5b2gzT01jRnBiNUZrQ0pueTE4NElPdFI1aE5WbnFJbkxhbElvbG43ZTdSQm5xOHhhYVdJOE81RzlNd0owdW9LVENyZUdlWVg1eV9mZ0RaWktETVl6UUIwaXhNM2txeDlLSEpRYXh2TVUwMm5wQXl3cVpHTnV0VHR5VmJZNjJ5aU1ISHM977eu77ev\",\"serializedChanges\":\"{\\\"assigned_to\\\":\\\"016ebac82b676210729af47ebe91bf2a\\\",\\\"assignment_group\\\":\\\"3efc509e87dd5910dd76873e8bbb354b\\\"}\",\"chars\":\"c\",\"ignoreRefQual\":false,\"paginationLimit\":25,\"sortBy\":\"name\",\"referenceKey\":null,\"orderByDisplayColumn\":true},\"nowUxInteraction\":\"4vgu7hxugco6-651\",\"nowUiInteraction\":\"4vgu7hxugco6-13900\",\"cacheable\":false,\"__unstableDisableBatching\":true,\"extensions\":{},\"queryContext\":null}";

  if (!WinHttpAddRequestHeaders(rqst, headers, (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD))
  {
    DWORD prout = GetLastError();
    printb("%s", winhttp_getstr(prout));
    SetLastError(prout);
    { report_error_box("AddRequestHeaders"); exit(1);}
  }
	if (!WinHttpSendRequest(rqst, WINHTTP_NO_ADDITIONAL_HEADERS, 0, data, wstrlen(data), wstrlen(data), 0))
	// if (!WinHttpSendRequest(rqst, cookie, -1L, data, wstrlen(data), wstrlen(data), 0))
  {
    DWORD prout = GetLastError();
    printb("%d %s", prout, winhttp_getstr(prout));
    SetLastError(prout);
    { report_error_box("SendRequest"); exit(1);}
  }

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
