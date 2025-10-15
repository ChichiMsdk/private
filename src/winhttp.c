#ifndef WINHTTP_C
#define WINHTTP_C

#include <winhttp.h>
#include "error.c"

inline HINTERNET
http_open_request_impl(HINTERNET hConnect, LPCWSTR pwszVerb, LPCWSTR pwszObjectName, LPCWSTR pwszVersion, LPCWSTR pwszReferrer, LPCWSTR FAR * ppwszAcceptTypes, DWORD dwFlags, char* file, char* line, char* function)
{
	HINTERNET request = WinHttpOpenRequest(hConnect, pwszVerb, pwszObjectName, pwszVersion, pwszReferrer, ppwszAcceptTypes, dwFlags);
	if (!request) {	DWORD s = GetLastError(); char* d = winhttp_getstr(s); SetLastError(s); debug_info_gather(g_debug_info, file, line, "WinHttpOpenRequest", function); show_error_msg_box(d, s); exit(1); }
	return request;
}

inline HINTERNET
http_connect_impl(HINTERNET hSession, LPCWSTR pswzServerName, INTERNET_PORT nServerPort, char* file, char* line, char* function)
{
	HINTERNET connect = WinHttpConnect(hSession, pswzServerName, nServerPort, 0);
	if (!connect) {	DWORD s = GetLastError(); char* d = winhttp_getstr(s); SetLastError(s); debug_info_gather(g_debug_info, file, line, "WinHttpConnect", function); show_error_msg_box(d, s); exit(1); }
	return connect;
}

inline void
http_get_proxy_for_url_impl(HINTERNET hSession, LPCWSTR lpcwszUrl, WINHTTP_AUTOPROXY_OPTIONS *pAutoProxyOptions, WINHTTP_PROXY_INFO *pProxyInfo, char* file, char* line, char* function)
{
	BOOL result = WinHttpGetProxyForUrl(hSession, lpcwszUrl, pAutoProxyOptions, pProxyInfo);
	if (!result) {	DWORD s = GetLastError(); char* d = winhttp_getstr(s); SetLastError(s); debug_info_gather(g_debug_info, file, line, "WinHttpGetProxyUrl", function); show_error_msg_box(d, s); exit(1); }
}


inline HINTERNET
http_open_impl(LPCWSTR pszAgentW, DWORD dwAccessType, LPCWSTR pszProxyW, LPCWSTR pszProxyBypassW, DWORD dwFlags, char* file, char* line, char* function)
{
	HINTERNET session = WinHttpOpen(pszAgentW, dwAccessType, pszProxyW, pszProxyBypassW, dwFlags);
	if (!session) {	DWORD s = GetLastError(); char* d = winhttp_getstr(s); SetLastError(s); debug_info_gather(g_debug_info, file, line, "WinHttpOpen", function); show_error_msg_box(d, s); exit(1); }
	return session;
}

inline void
http_add_request_headers_impl(HINTERNET hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwModifiers, char* file, char* line, char* function)
{
	BOOL result = WinHttpAddRequestHeaders(hRequest, lpszHeaders, dwHeadersLength, dwModifiers);
	if (!result) {	DWORD s = GetLastError(); char* d = winhttp_getstr(s); SetLastError(s); debug_info_gather(g_debug_info, file, line, "WinHttpAddRequestHeaders", function); show_error_msg_box(d, s); exit(1); }
}

inline void
http_set_option_impl(HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength, char* file, char* line, char* function)
{
	BOOL result = WinHttpSetOption(hInternet, dwOption, lpBuffer , dwBufferLength);
	if (!result) {	DWORD s = GetLastError(); char* d = winhttp_getstr(s); SetLastError(s); debug_info_gather(g_debug_info, file, line, "WinHttpSetOption", function); show_error_msg_box(d, s); exit(1); }
}

inline void
http_send_request_impl(HINTERNET hRequest, LPCWSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength, DWORD dwTotalLength, DWORD_PTR dwContext, char* file, char* line, char* function)
{
	BOOL result = WinHttpSendRequest(hRequest, lpszHeaders, dwHeadersLength , lpOptional, dwOptionalLength, dwTotalLength, dwContext);
	if (!result) {	DWORD s = GetLastError(); char* d = winhttp_getstr(s); printb("%d", s); SetLastError(s); debug_info_gather(g_debug_info, file, line, "WinHttpSendRequest", function); show_error_msg_box(d, s); exit(1); }
}

inline void
http_receive_response_impl(HINTERNET hRequest, char* file, char* line, char* function)
{
	BOOL result = WinHttpReceiveResponse(hRequest, NULL);
	if (!result) {	DWORD s = GetLastError(); char* d = winhttp_getstr(s); SetLastError(s); debug_info_gather(g_debug_info, file, line, "WinHttpReceiveResponse", function); show_error_msg_box(d, s); exit(1); }
}

inline void
http_read_data_impl(HINTERNET hRequest, LPVOID buffer, DWORD to_read, LPDWORD read, char* file, char* line, char* function)
{
	BOOL result = WinHttpReadData(hRequest, buffer, to_read, read);
	if (!result) {	DWORD s = GetLastError(); char* d = winhttp_getstr(s); SetLastError(s); debug_info_gather(g_debug_info, file, line, "WinHttpReadData", function); show_error_msg_box(d, s); exit(1); }
}

    
#define http_open(a, b, c, d, e)								http_open_impl((a), (b), (c), (d), (e), __FILE__, __LINE__, __FUNCTION__)
#define http_connect(a, b, c)										http_connect_impl((a), (b), (c), __FILE__, __LINE__, __FUNCTION__)
#define http_read_data(a, b, c, d)							http_read_data_impl((a), (b), (c), (d), __FILE__, __LINE__, __FUNCTION__)
#define http_set_option(a, b, c, d)							http_set_option_impl((a), (b), (c), (d), __FILE__, __LINE__, __FUNCTION__)
#define http_send_request(a, b, c, d, e, f, g)	http_send_request_impl((a), (b), (c), (d), (e), (f), (g), __FILE__, __LINE__, __FUNCTION__)
#define http_open_request(a, b, c, d, e, f, g)	http_open_request_impl((a), (b), (c), (d), (e), (f), (g), __FILE__, __LINE__, __FUNCTION__)
#define http_receive_response(a)								http_receive_response_impl((a), __FILE__, __LINE__, __FUNCTION__)
#define http_get_proxy_for_url(a, b, c, d)			http_get_proxy_for_url_impl((a), (b), (c), (d), __FILE__, __LINE__, __FUNCTION__)
#define http_add_request_headers(a, b, c, d)	  http_add_request_headers_impl((a), (b), (c), (d), __FILE__, __LINE__, __FUNCTION__)




#endif // WINHTTP_C
