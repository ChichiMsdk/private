#ifndef TOZ_C
#define TOZ_C

#include <windows.h>
/*
 * #include <wrl.h>
 * #include <wrl/client.h>
 */
#include "webview2.h"

#define SAFE_RELEASE(x) if (x) { IUnknown_Release(x); (x) = NULL; }

HRESULT STDMETHODCALLTYPE CreateControllerCompleted(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* This, HRESULT result, ICoreWebView2Controller* controller);
HRESULT STDMETHODCALLTYPE CreateEnvironmentCompleted(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* This, HRESULT result, ICoreWebView2Environment* environment);

wchar_t g_url = NULL;
ICoreWebView2Environment* g_env = NULL;

typedef struct ControllerHandler
{
	ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl* lpVtbl;
	LONG refCount;
} ControllerHandler;

HRESULT STDMETHODCALLTYPE
ControllerHandler_QueryInterface(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* This, REFIID riid, void** ppv)
{
	if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler))
	{
		*ppv = This;
		IUnknown_AddRef((IUnknown*)This);
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE
ControllerHandler_AddRef(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* This)
{
	ControllerHandler* self = (ControllerHandler*)This;
	return InterlockedIncrement(&self->refCount);
}

ULONG STDMETHODCALLTYPE
ControllerHandler_Release(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* This)
{
	ControllerHandler* self = (ControllerHandler*)This;
	ULONG count = InterlockedDecrement(&self->refCount);
	if (count == 0) free(self);
	return count;
}

HRESULT STDMETHODCALLTYPE
ControllerHandler_Invoke(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* This, HRESULT result, ICoreWebView2Controller* controller)
{
	if (FAILED(result) || !controller)
		return result;

	ICoreWebView2* webview = NULL;

	ICoreWebView2Controller_get_CoreWebView2(controller, &webview);

	// Resize WebView to fit parent window
	RECT bounds;
	GetClientRect(g_hwnd, &bounds);
	ICoreWebView2Controller_put_Bounds(controller, bounds);

	// Navigate to URL
	ICoreWebView2_Navigate(webview, g_url);

	// Store pointers in window properties for later use
	SetProp(g_hwnd, L"WEBVIEW", webview);
	SetProp(g_hwnd, L"WEBVIEWCONTROLLER", controller);

	SAFE_RELEASE(webview);

	return S_OK;
}

ICoreWebView2CreateCoreWebView2ControllerCompletedHandlerVtbl g_ControllerHandler_Vtbl = {
	ControllerHandler_QueryInterface,
	ControllerHandler_AddRef,
	ControllerHandler_Release,
	ControllerHandler_Invoke
};

ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* CreateControllerHandler(void)
{
	ControllerHandler* handler;
	heap_alloc_dz(sizeof(ControllerHandler), handler);
	handler->lpVtbl = &g_ControllerHandler_Vtbl;
	handler->refCount = 1;
	return (ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*)handler;
}

typedef struct EnvHandler
{
	ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl* lpVtbl;
	LONG refCount;
} EnvHandler;

HRESULT STDMETHODCALLTYPE
EnvHandler_QueryInterface(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* This, REFIID riid, void** ppv)
{
	if (IsEqualIID(riid, &IID_IUnknown) ||
			IsEqualIID(riid, &IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler))
	{
		*ppv = This;
		IUnknown_AddRef((IUnknown*)This);
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE
EnvHandler_AddRef(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* This)
{
	EnvHandler* self = (EnvHandler*)This;
	return InterlockedIncrement(&self->refCount);
}

ULONG STDMETHODCALLTYPE
EnvHandler_Release(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* This)
{
	EnvHandler* self = (EnvHandler*)This;
	ULONG count = InterlockedDecrement(&self->refCount);
	if (count == 0) free(self);
	return count;
}

HRESULT STDMETHODCALLTYPE
EnvHandler_Invoke(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* This, HRESULT result, ICoreWebView2Environment* environment)
{
	if (FAILED(result) || !environment) return result;

	g_env = environment;
	ICoreWebView2Environment_AddRef(environment);

	ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* controllerHandler = CreateControllerHandler();
	return ICoreWebView2Environment_CreateCoreWebView2Controller(environment, g_hwnd, controllerHandler);
}

ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandlerVtbl g_EnvHandler_Vtbl =
{
	EnvHandler_QueryInterface,
	EnvHandler_AddRef,
	EnvHandler_Release,
	EnvHandler_Invoke
};

ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* CreateEnvHandler(void)
{
	EnvHandler* handler;
	heap_alloc_dz(sizeof(EnvHandler), handler);
	handler->lpVtbl = &g_EnvHandler_Vtbl;
	handler->refCount = 1;
	return (ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*)handler;
}

typedef HRESULT (*lpfn_wv)
    (PCWSTR, PCWSTR, ICoreWebView2EnvironmentOptions*, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);

typedef struct thread_params
{
	HWND		window;
	wchar_t* url;
	wchar_t* path;
	HMODULE lib;
} thread_params;

void
InitWebView2(void* param)
{
	thread_params *info = (thread_params*) param;
	g_hwnd = info->window;
	g_url = info->url;

	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{ report_error_box("CoInitializeEx"); exit(1); }

	ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* envHandler = CreateEnvHandler();

	lpfn_wv createcorewebview2 = (lpfn_wv)GetProcAddress(info->lib, "CreateCoreWebView2EnvironmentWithOptions");
	if (!createcorewebview2) {report_error_box("GetProcAddress"); exit(1);}

	if (FAILED(createcorewebview2(NULL, info->path, NULL, envHandler)))
	{ report_error_box("CreateCoreWebView2EnvironmentWithOptions"); exit(1); }
	/* CreateCoreWebView2EnvironmentWithOptions(NULL, NULL, NULL, envHandler); */
}
#endif // TOZ_C
