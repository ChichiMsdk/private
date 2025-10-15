#ifndef CONSOLE_C
#define CONSOLE_C

typedef struct Console_Bridge
{
    HANDLE stdin_w;
    HANDLE stdout_r;
    PROCESS_INFORMATION pi;
} Console_Bridge;

static Console_Bridge gBridge = {0};

EXPORT bool
bridge_start(void)
{
	SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
	HANDLE stdin_r  = NULL, stdin_w  = NULL;
	HANDLE stdout_r = NULL, stdout_w = NULL;

	if (!CreatePipe(&stdout_r, &stdout_w, &sa, 0)) {report_error_box("CreatePipe"); return false;}
	if (!SetHandleInformation(stdout_r, HANDLE_FLAG_INHERIT, 0)) {report_error_box("SetHandleInformation"); return false;}

	if (!CreatePipe(&stdin_r, &stdin_w, &sa, 0)) {report_error_box("CreatePipe"); return false;}
	if (!SetHandleInformation(stdin_w, HANDLE_FLAG_INHERIT, 0)) {report_error_box("SetHandleInformation"); return false;}

	PROCESS_INFORMATION pi = {};
	STARTUPINFOA si = {
		.cb = sizeof(si),
		.dwFlags = STARTF_USESTDHANDLES,
		.hStdInput  = stdin_r,
		.hStdOutput = stdout_w,
		.hStdError  = stdout_w,
	};

	char* cmd = "C:\\Windows\\System32\\cmd.exe";
	DWORD flags = CREATE_NEW_CONSOLE | CREATE_BREAKAWAY_FROM_JOB;
	if (!CreateProcess(cmd, "/k rout", NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi))
	{ report_error_box("CreateProcess"); return false; }

	CloseHandle(stdout_w);
	CloseHandle(stdout_r);

	gBridge.stdout_r = stdout_r;
	gBridge.stdin_w  = stdin_w;
	gBridge.pi = pi;
	return true;
}

EXPORT bool
bridge_write(char *text)
{
	DWORD written;
	uint64_t len = strlen(text);
	if (!WriteFile(gBridge.stdin_w, text, (DWORD)len, &written, NULL)) return false;

	WriteFile(gBridge.stdin_w, "\r\n", 2, &written, NULL);
	return true;
}

EXPORT int
bridge_read(char *buf, DWORD buf_size)
{
	DWORD read = 0;
	if (!ReadFile(gBridge.stdout_r, buf, buf_size - 1, &read, NULL)) return 0;
	buf[read] = '\0';
	return (int)read;
}

EXPORT void
bridge_stop(void)
{
	if (gBridge.pi.hProcess)
	{
		bridge_write("exit");
		WaitForSingleObject(gBridge.pi.hProcess, 3000);
		CloseHandle(gBridge.pi.hThread);
		CloseHandle(gBridge.pi.hProcess);
	}
	if (gBridge.stdin_w)  CloseHandle(gBridge.stdin_w);
	if (gBridge.stdout_r) CloseHandle(gBridge.stdout_r);
}
#endif // CONSOLE_C
