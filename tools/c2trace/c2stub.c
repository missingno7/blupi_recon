/* Pass-2 stand-in for cl.exe /B2: copy the IL files named by -il to %C2STUB_OUT%, record the arguments,
   then run the real c2.exe with the same arguments so the compile completes normally. */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	char *il = NULL, *out = getenv("C2STUB_OUT"), *real = getenv("C2STUB_REAL");
	char pattern[MAX_PATH], src[MAX_PATH], dst[MAX_PATH], cmd[8192], *tail, *slash;
	WIN32_FIND_DATA fd;
	HANDLE h;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD code = 1;
	FILE *f;
	int i;

	for (i = 1; i < argc - 1; i++)
		if (strcmp(argv[i], "-il") == 0) il = argv[i + 1];
	if (out == NULL) {
		static char dir[MAX_PATH];
		GetModuleFileName(NULL, dir, MAX_PATH);
		strcpy(strrchr(dir, '\\') + 1, "il");
		out = dir;
	}
	if (real == NULL) real = "C:\\tools\\msvc5_sp3\\VC\\BIN\\c2.exe";
	if (il == NULL && getenv("MSC_CMD_FLAGS") != NULL) {
		static char flags[8192];
		char *p;
		strcpy(flags, getenv("MSC_CMD_FLAGS"));
		p = strstr(flags, "-il ");
		if (p != NULL) {
			il = p + 4;
			p = strchr(il, ' ');
			if (p != NULL) *p = '\0';
		}
		sprintf(dst, "%s\\flags.txt", out);
		f = fopen(dst, "w");
		if (f) { fprintf(f, "%s\n", getenv("MSC_CMD_FLAGS")); fclose(f); }
	}
	if (il == NULL) return 2;
	sprintf(pattern, "%s*", il);
	strcpy(src, il);
	slash = strrchr(src, '\\');
	h = FindFirstFile(pattern, &fd);
	if (h != INVALID_HANDLE_VALUE) {
		do {
			strcpy(slash + 1, fd.cFileName);
			sprintf(dst, "%s\\%s", out, fd.cFileName);
			CopyFile(src, dst, FALSE);
		} while (FindNextFile(h, &fd));
		FindClose(h);
	}
	tail = GetCommandLine();
	if (*tail == '"') { tail = strchr(tail + 1, '"') + 1; } else { while (*tail && *tail != ' ') tail++; }
	sprintf(dst, "%s\\args.txt", out);
	f = fopen(dst, "w");
	if (f) { fprintf(f, "%s\n", tail); fclose(f); }
	sprintf(cmd, "\"%s\"%s", real, tail);
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) return 3;
	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &code);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return (int)code;
}
