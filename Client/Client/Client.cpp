// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>

using namespace std;
int main() {

	cout << "NAMED PIPE CLIENT" << endl;

	// LOCAL VARIABLE DEFINITIONS
	HANDLE hCreateFile;

	// Readfile Local Variable Def
	BOOL bReadfile;
	DWORD dwNoBytesRead;
	char szReadFileBuffer[1023];
	DWORD dwReadFileBufferSize = sizeof(szReadFileBuffer);

	// WriteFile Local Variable Def
	BOOL bWriteFile;
	DWORD dwNoBytesWrite;
	char szWriteFileBuffer[1023] = "Hello from NamedPipe Client!";
	DWORD dwWriteFileBufferSize = sizeof(szWriteFileBuffer);

	// CreateFile for Pipe
	hCreateFile = CreateFile(L"\\\\.\\pipe\\MYNAMEDPIPE1",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE == hCreateFile) {
		cout << "File Creation Failed" << endl;
	}
	cout << "File Creation Success" << endl;

	// Read File
	bReadfile = ReadFile(hCreateFile, szReadFileBuffer, dwReadFileBufferSize, &dwNoBytesRead, NULL);
	if (bReadfile == FALSE) {
		cout << "ReadFile Failed: " << GetLastError() << endl;
	}
	cout << "ReadFile Success" << endl;
	cout << "DATA READING FROM SERVER -> " << szReadFileBuffer << endl;

	// WriteFile
	bWriteFile = WriteFile(hCreateFile, szWriteFileBuffer, dwWriteFileBufferSize, &dwNoBytesWrite, NULL);
	if (bWriteFile == FALSE) {
		cout << "WriteFile Failed ->" << GetLastError() << endl;
	}
	cout << "WriteFile Success" << endl;

	CloseHandle(hCreateFile);

	return 0;
}