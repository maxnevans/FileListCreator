#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <shlwapi.h>

#define DEFAULT_OUTPUT_FILENAME	L"files_list.txt"

int wmain(int argc, wchar_t** argv)
{
	wchar_t initDir[MAX_PATH + 1];
	GetModuleFileName(NULL, initDir, MAX_PATH + 1);
	int s_len = wcslen(initDir);
	for (int i = s_len - 1; i > 0; i--) {
		if (initDir[i] == L'\\') {
			initDir[i] = 0;
			break;
		}
		else {
			initDir[i] = 0;
		}
	}

	const wchar_t *outputFileName = L"";
	const wchar_t *inputDirName = L"";

	if (argc == 1) {
		const wchar_t *testFileName = DEFAULT_OUTPUT_FILENAME;
		FILE *testFile;
		_wfopen_s(&testFile, testFileName, L"rb");
		if (testFile != NULL) {
			wprintf(L"File \"%s\" already exists! Specify the --rewrite flag to rewrite it.", testFileName);
			return 0;
		}

		outputFileName = DEFAULT_OUTPUT_FILENAME;
		inputDirName = initDir;
	}
	else if (argc == 2) {
		if (wcscmp(argv[1], L"--rewrite") == 0)
		{
			outputFileName = DEFAULT_OUTPUT_FILENAME;
			inputDirName = initDir;
		}
		else if (wcscmp(argv[1], L"--help") == 0)
		{
			wchar_t executablePath[MAX_PATH];
			if (GetModuleFileName(NULL, executablePath, MAX_PATH) == FALSE)
			{
				wprintf(L"Failed to GetModuleFileName of current executable\n");
				return -1;
			}
			wchar_t* executableFileName = PathFindFileName(executablePath);
			wprintf(L"Usage:\n"
				"\t%s                    -    Creates default output file \"%s\" if it does not exist, otherwise returns failure\n"
				"\t%s --rewrite          -    Rewrites default \"%s\" files list output file\n"
				"\t%s <output> <input>   -    Creates output file that contains all file names and edit dates form <input> dir\n", 
				executableFileName, DEFAULT_OUTPUT_FILENAME, 
				executableFileName, DEFAULT_OUTPUT_FILENAME, 
				executableFileName
			);
			return 0;
		}
		else
		{
			wprintf(L"Error! Unsupported argument!");
			return -1;
		}
	}
	else if (argc == 3) {
		outputFileName = argv[1];
		inputDirName = argv[2];
	}

	FILE* of;
	WIN32_FIND_DATA wfd;
	int sOrigLen = wcslen(inputDirName);
	wchar_t *idnFormated = (wchar_t*)calloc(sOrigLen + 3, sizeof(wchar_t));
	wcscpy_s(idnFormated, sOrigLen + 1, inputDirName);
	idnFormated[sOrigLen] = L'\\';
	idnFormated[sOrigLen + 1] = L'*';
	idnFormated[sOrigLen + 2] = 0;
	HANDLE hFiles = FindFirstFile(idnFormated, &wfd);

	if (hFiles == INVALID_HANDLE_VALUE) {
		wprintf(L"Invalid input path specified: %s!", inputDirName);
		return -1;
	}

	//Skipping .. dir
	FindNextFile(hFiles, &wfd);

	_wfopen_s(&of, outputFileName, L"wt");
	if (of == NULL) {
		wprintf(L"Could not create list file \"%s\"!", outputFileName);
		return -1;
	}
	
	wchar_t *bufferStr = (wchar_t*)calloc(256, sizeof(wchar_t));
	SYSTEMTIME st;
	fputws(inputDirName, of);
	while (FindNextFile(hFiles, &wfd)) {
		if (wcscmp(wfd.cFileName, PathFindFileNameW(outputFileName)) == 0) continue;
		fputwc(L'\n', of);
		if (FileTimeToSystemTime(&wfd.ftLastWriteTime, &st)) {
			swprintf_s(bufferStr, 256, L"[ %02.2d:%02.2d:%02.2d %02.2d/%02.2d/%04.4d ]    ",
				st.wHour,
				st.wMinute,
				st.wSecond,
				st.wDay,
				st.wMonth,
				st.wYear
			);
			fputws(bufferStr, of);
		}
		fputws(wfd.cFileName, of);
	}
	wprintf(L"List file %s has been created!\n", outputFileName);
	return 0;
}