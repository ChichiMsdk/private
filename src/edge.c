#ifndef EDGE_C
#define EDGE_C

#include <windows.h>
#include "WebView2.h" // from WebView2 SDK
/* #include "WebView2EnvironmentOptions.h" */

using namespace Microsoft::WRL;

// Simple global to hold result for the sample
global char* g_cookie_result;
global atomic bool g_done = false;

// Forward: show a small window and embed WebView2, navigate and let user login.
static HRESULT webview_open(HWND hwndParent, wchar_t* url)
{
	HRESULT hr = S_OK;
	ComPtr<ICoreWebView2Environment> env;
	// Create environment
	hr =
		CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
				Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
					[&env, &url, hwndParent](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
					if (FAILED(result) || !environment) return result;
					env = environment;
					// create controller
					environment->CreateCoreWebView2Controller(parent,
							Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
								[&env, &url, parent](HRESULT result2, ICoreWebView2Controller* controller) -> HRESULT {
								if (FAILED(result2) || !controller) return result2;
								ComPtr<ICoreWebView2Controller> webviewController(controller);
								ComPtr<ICoreWebView2> webview;
								webviewController->get_CoreWebView2(&webview);

								// Resize to parent client
								RECT bounds;
								GetClientRect(parent, &bounds);
								webviewController->put_Bounds(bounds);

								// Listen for navigation completed optionally
								// Provide a small "Done" UI: we'll rely on user to press CTRL+D to indicate done (simpler)
								// Alternatively, you can add a button in the surrounding Win32 UI and call back.

								// Navigate to URL
								webview->Navigate(url.c_str());

								// Register execute script completed handler to grab cookies when user signals done.
								// We'll also register a keyboard accelerator to detect Ctrl+D: host window will handle it.

								// Save the webview pointer somewhere accessible (store in window prop)
								SetProp(parent, L"WEBVIEW", webview.Get());

								// keep controller alive by storing as window prop
								SetProp(parent, L"WEBVIEWCONTROLLER", webviewController.Get());

								return S_OK;
								}).Get());
					return S_OK;
					}
	return hr;
}

// Win32 window proc for the helper window.
// When user presses Ctrl+D the code will call ExecuteScript("document.cookie") and store result.
LRESULT CALLBACK
HelperWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CREATE) return 0;
	if (msg == WM_KEYDOWN)
	{
		// CTRL+D indicates "I'm done; get cookies"
		if ((GetKeyState(VK_CONTROL) & 0x8000) && (wParam == 'D'))
		{
			ICoreWebView2* webview = (ICoreWebView2*)GetProp(hwnd, L"WEBVIEW");
			if (webview) {
				webview->ExecuteScript(L"document.cookie", 
						Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
							[](HRESULT err, PCWSTR resultJson) -> HRESULT {
							// resultJson is a JSON string with the cookie string, e.g. "\"name=value; other=...\""
							if (SUCCEEDED(err) && resultJson) {
							// strip quotes from JSON string (it arrives quoted)
							std::wstring w(resultJson);
							// Remove leading/trailing quotes if present
							if (w.size() >= 2 && w.front() == L'\"' && w.back() == L'\"') {
							w = w.substr(1, w.size()-2);
							}
							// Replace escaped sequences (a minimal unescape: JSON escapes for \" and \\)
							std::wstring cleaned;
							for (size_t i=0;i<w.size();++i){
							if (w[i] == L'\\' && i+1 < w.size()) {
							wchar_t next = w[i+1];
							if (next == L'\\' || next == L'\"') { cleaned.push_back(next); ++i; continue; }
							// simple handling - copy next anyway
							cleaned.push_back(next); ++i; continue;
							}
							cleaned.push_back(w[i]);
							}
							// convert to UTF-8
							int needed = WideCharToMultiByte(CP_UTF8, 0, cleaned.c_str(), (int)cleaned.size(), NULL, 0, NULL, NULL);
							if (needed > 0) {
								std::string s; s.resize(needed);
								WideCharToMultiByte(CP_UTF8, 0, cleaned.c_str(), (int)cleaned.size(), &s[0], needed, NULL, NULL);
								g_cookie_result = s;
							}
							}
							g_done = true;
							return S_OK;
							}).Get());
			}
		}
	}
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

// Expose a very small C API for your C program
extern "C" __declspec(dllexport)
int GetCookiesViaWebView2(const wchar_t* startUrl, char* outBuffer, int outBufferSize, int timeoutMs)
{
    // Initialize COM on this thread
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return -1;

    // Create a simple Win32 window to host WebView2
    HINSTANCE hInst = GetModuleHandle(NULL);
    const wchar_t* cls = L"WebViewCookieHelperWindowClass";
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = HelperWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = cls;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, cls, L"Login (press Ctrl+D when done)", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768, NULL, NULL, hInst, NULL);

    if (!hwnd) {
        CoUninitialize();
        return -2;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Create WebView2 environment and controller inside that window
    std::wstring url(startUrl);
    CreateWebViewAndGetCookies(hwnd, url);

    // Message loop: wait until g_done becomes true or timeout
    DWORD start = GetTickCount();
    MSG msg;
    while (!g_done) {
        while (PeekMessage(&msg, NULL, 0,0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
        if (timeoutMs > 0 && (int)(GetTickCount() - start) > timeoutMs) break;
    }

    // close window
    DestroyWindow(hwnd);

    // copy cookie into outBuffer
    int ret = 0;
    if (!g_cookie_result.empty()) 
		{
        int copyLen = (int)g_cookie_result.size();
        if (copyLen >= outBufferSize) copyLen = outBufferSize - 1;
        memcpy(outBuffer, g_cookie_result.c_str(), copyLen);
        outBuffer[copyLen] = 0;
        ret = copyLen;
    } else {
        if (!g_done) ret = -3; // timeout
        else ret = 0; // no cookie found
    }

    CoUninitialize();
    return ret;
}
#endif // EDGE_C
