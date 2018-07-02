// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "Person.h"

#define BUFSIZE 512

int _tmain(int argc, char* argv[]) {

	HANDLE hPipe;
	LPTSTR lpvMessage;
	char chBuf[BUFSIZE];
	BOOL fSuccess = FALSE;
	DWORD cbRead, cbToWrite, cbWritten, dwMode;
	LPTSTR lpszPipename = (LPTSTR)TEXT("\\\\.\\pipe\\IMGPROCPIPE");

	// Check if custom message needs to be sent
	if (argc > 1)
		lpvMessage = (LPTSTR)argv[1];

	// Try to open a named pipe, wait for it if necessary
	while (1) {
		hPipe = CreateFile(
			lpszPipename,	// pipe name
			GENERIC_READ |	// read and write access
			GENERIC_WRITE,
			0,				// no sharing
			NULL,			// default security attributes
			OPEN_EXISTING,	// opens exisiting pipe
			0,				// default attributes
			NULL);			// no template file

							// Break if the pipe handle is valid
		if (hPipe != INVALID_HANDLE_VALUE) {
			printf("Connection to Server Established.\n");
			break;
		}

		// Exit if an error other than ERROR_PIPE_BUSY occurs.
		if (GetLastError() != ERROR_PIPE_BUSY) {
			_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
			return -1;
		}

		// All pipe instances are busy, so wait for 5 seconds
		if (!WaitNamedPipe(lpszPipename, 5000)) {
			printf("Could not open pipe: 5 second wait timed out.");
			return -1;
		}
	}

	// The pipe connected, change to message-read mode.
	dwMode = PIPE_READMODE_MESSAGE;
	fSuccess = SetNamedPipeHandleState(
		hPipe,		// pipe handle
		&dwMode,	// new pipe mode
		NULL,		// don't set maximum bytes
		NULL);		// don't set maximum time

	if (!fSuccess) {
		_tprintf(TEXT("SetNamedPipeHandleState falied. GLE=%d\n"), GetLastError());
		return -1;
	}

	// Create the message
	printf("Creating a new user with credentials: {Jack, 25, 80}\n");

	char name[10] = "Jack";
	Person person(name, 25, 80);
	char message[BUFSIZE];
	person.serialize(message);
	

	lpvMessage = (LPTSTR)message;
	
	// Send a message to the pipe server
	cbToWrite = person.serializeSize() * sizeof(size_t);

	fSuccess = WriteFile(
		hPipe,			// pipe handle
		lpvMessage,		// message
		cbToWrite,		// message length
		&cbWritten,		// bytes written
		NULL);			// not overlapped

	if (!fSuccess)
	{
		_tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
		return -1;
	}


	printf("\nMessage sent to server, receiving reply as follows:\n");

	do {
		// Read from the pipe
		fSuccess = ReadFile(
			hPipe,					// pipe handle
			chBuf,					// buffer to receive reply
			BUFSIZE * sizeof(TCHAR),// size of buffer
			&cbRead,				// number of bytes to read
			NULL);					// not overlapped

		if (!fSuccess && GetLastError() != ERROR_MORE_DATA) {
			break;
		}

		_tprintf(TEXT("\"%s\"\n"), chBuf);
	} while (!fSuccess); // repeat loop if ERROR_MORE_DATA

	if (!fSuccess) {
		_tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
		return -1;
	}

	printf("\n<End of message, press ENTER to terminate connection and exit>");
	_getch();

	CloseHandle(hPipe);
	_getch();

	return 0;
}