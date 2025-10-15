#ifndef WEBVIEW_C 
#define WEBVIEW_C 

#define APPLICATION_NAME TEXT("WebView2")

typedef ICoreWebView2CreateCoreWebView2ControllerCompletedHandler ControllerCompletedHandler;
typedef ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler EnvironmentCompletedHandler;

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
handler_invoke(IUnknown* this, HRESULT error_code, void* arg)
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
		ICoreWebView2_get_Settings(g_webview.window, &settings);
		ICoreWebView2Settings_put_IsScriptEnabled(settings, TRUE);
		ICoreWebView2Settings_put_AreDefaultScriptDialogsEnabled(settings, TRUE);
		ICoreWebView2Settings_put_IsWebMessageEnabled(settings, TRUE);
		ICoreWebView2Settings_put_AreDevToolsEnabled(settings, FALSE);
		ICoreWebView2Settings_put_AreDefaultContextMenusEnabled(settings, TRUE);
		ICoreWebView2Settings_put_IsStatusBarEnabled(settings, TRUE);

		RECT bounds;
		GetClientRect(g_hwnd, &bounds);
		ICoreWebView2Controller_put_Bounds(g_webview.controller, bounds);
		wchar_t* url = L"https://digituat.service-now.com/now/sow/home";
		ICoreWebView2_Navigate(g_webview.window, url);
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

#endif // WEBVIEW_C 
