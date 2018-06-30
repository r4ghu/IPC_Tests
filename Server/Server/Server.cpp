// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
using namespace std;
int main() {
	cout << "NAMED PIPE SERVER" << endl;
	// NAMED Pipe Local variable
	HANDLE hCreateNamedPipe;
	char szInputBuffer[1024];
	char szOutputBuffer[1024];
	DWORD dwszInputBuffer = sizeof(szInputBuffer);
	DWORD dwszOutputBuffer = sizeof(szOutputBuffer);

	// ConnectNamedPipe Local Variable
	BOOL bConnectNamedPipe;

	// WriteFile Local variable
	BOOL bWriteFile;
	char szWriteFileBuffer[1024] = "Hello from NamedPipe Server!";
	DWORD dwWriteBufferSize = sizeof(szWriteFileBuffer);
	DWORD dwNoByteWrite;

	// FlushBuffer
	BOOL bFlushFileBuffer;

	//ReadFile Local Variable
	BOOL bReadFile;
	char szReadFileBuffer[1024];
	DWORD dwReadBufferSize = sizeof(szWriteFileBuffer);
	DWORD dwNoBytesRead;

	// CreateNamedPipe - Step 1
	hCreateNamedPipe = CreateNamedPipe(
						L"\\\\.\\pipe\\MYNAMEDPIPE1",
						PIPE_ACCESS_DUPLEX,
						PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
						PIPE_UNLIMITED_INSTANCES,
						dwszOutputBuffer,
						dwszInputBuffer,
						0,
						NULL);
	if (hCreateNamedPipe == INVALID_HANDLE_VALUE) {
		cout << "Pipe creation failed" << GetLastError() << endl;
	}
	cout << "NamedPipe connection success" << endl;

	// Connect Named Pipe : Step - 2
	bConnectNamedPipe = ConnectNamedPipe(hCreateNamedPipe, NULL);
	if (bConnectNamedPipe == FALSE) {
		cout << "Connection Failed & ERROR No - " << GetLastError() << endl;
	}
	cout << "Connection Success" << endl;

	// WriteFile Operation: STEP-3
	bWriteFile = WriteFile(hCreateNamedPipe, szWriteFileBuffer, dwWriteBufferSize, &dwNoByteWrite, NULL);
	if (bWriteFile == FALSE) {
		cout << "WriteFile Failed = " << GetLastError() << endl;
	}
	cout << "WriteFile Success" << endl;

	// Flush the file buffer: STEP -4
	bFlushFileBuffer = FlushFileBuffers(hCreateNamedPipe);
	if (bFlushFileBuffer == FALSE) {
		cout << "FlushBuffer failed = " << GetLastError() << endl;
	}
	cout << "Flush Buffer success" << endl;


	// ReadFile: Step-5
	bReadFile = ReadFile(hCreateNamedPipe, szReadFileBuffer, dwReadBufferSize, &dwNoBytesRead, NULL);
	if (bReadFile == FALSE) {
		cout << "ReadFile Failed = " << GetLastError() << endl;
	}
	cout << "ReadFile Success" << endl;

	cout << "Data Reading from the client -> " << szReadFileBuffer << endl;

	// Disconnect Named Pipe - Step6
	DisconnectNamedPipe(hCreateNamedPipe);
	// Close handle - Step 7
	CloseHandle(hCreateNamedPipe);
	return 0;

}