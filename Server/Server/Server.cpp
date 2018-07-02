// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "Person.h"

#define BUFSIZE 512

DWORD WINAPI InstanceThread(LPVOID);
VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD);

int _tmain(VOID) {
	BOOL fConnected = FALSE;
	DWORD dwThreadId = 0;
	HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
	LPTSTR lpszPipename = (LPTSTR)TEXT("\\\\.\\pipe\\IMGPROCPIPE");

	// Create an instance of named pipe and wait for a client
	// When a client is connected, create a thread
	for (;;)
	{
		_tprintf(TEXT("\nPipe Server: Main thread waiting for client connection on %s\n"), lpszPipename);
		hPipe = CreateNamedPipe(
			lpszPipename,				// pipe name
			PIPE_ACCESS_DUPLEX,			// read/write access
			PIPE_TYPE_MESSAGE |			// message type pipe
			PIPE_READMODE_MESSAGE |		// message-read mode
			PIPE_WAIT,					// blocking mode
			PIPE_UNLIMITED_INSTANCES,	// maximum instances
			BUFSIZE,					// output buffer size
			BUFSIZE,					// input buffer size
			0,							// client time-out
			NULL);						// default security attribute

		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("CreateNamedPipe failed, GLE=%d. \n"), GetLastError());
			return -1;
		}

		// Wait for the client to connect 
		// If the pipe connects successfully, returns non-zero value,
		// else, returns zero with GetLastError = ERROR_PIPE_CONNECTED
		fConnected = ConnectNamedPipe(hPipe, NULL) ?
			TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (fConnected) {
			_tprintf(TEXT("Client connected, creating a processing thread.\n"));

			// Create a thread for this client
			hThread = CreateThread(
				NULL,					// no security attribute
				0,						// default stack size
				InstanceThread,			// thread proc
				(LPVOID)hPipe,			// thread parameter
				0,						// not suspended
				&dwThreadId);			// returns thread ID

			if (hThread == NULL) {
				_tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError());
				return -1;
			}
			else CloseHandle(hThread);
		}
		else
			// The client couldn't connect, so close the pipe.
			CloseHandle(hPipe);
	}

	return 0;
}

DWORD WINAPI InstanceThread(LPVOID lpvParam) {
	// Thread processing function to read from and reply to a client.
	HANDLE hHeap = GetProcessHeap();
	TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));
	TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));

	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = NULL;

	// Make sure there are no errors as the app runs even if the thread fails
	if (lpvParam == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("	InstanceThread got an unexpected NULL value in lpvParam.\n");
		printf("	InstanceThread exiting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return (DWORD)-1;
	}
	if (pchRequest == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("	InstanceThread got an unexpected NULL heap allocation.\n");
		printf("	InstanceThread exiting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		return (DWORD)-1;
	}
	if (pchReply == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("	InstanceThread got an unexpected NULL heap allocation.\n");
		printf("	InstanceThread exiting.\n");
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return (DWORD)-1;
	}

	// Print verbose messages. In production code, this should be for debugging only.
	printf("InstanceThread created, receiving and processing messages.\n");

	// The thread's parameter is a handle to a pipe object instance.
	hPipe = (HANDLE)lpvParam;

	// Loop until done reading
	while (1) {
		// Read client requests from the pipe.
		// LIMITATION: messages length should be < BUFSIZE
		fSuccess = ReadFile(
			hPipe,					// handle to pipe
			pchRequest,				// buffer to receive data
			BUFSIZE * sizeof(TCHAR),// size of buffer
			&cbBytesRead,			// number of bytes read
			NULL);					// not overlapped I/0

		if (!fSuccess || cbBytesRead == 0) {
			if (GetLastError() == ERROR_BROKEN_PIPE) {
				_tprintf(TEXT("InstanceThread: client disconnected. \n"), GetLastError());
			}
			else {
				_tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
			}
			break;
		}

		// Process the incoming message
		GetAnswerToRequest(pchRequest, pchReply, &cbReplyBytes);

		// Write the reply to the pipe
		fSuccess = WriteFile(
			hPipe,				// handle to pipe
			pchReply,			// buffer to write from
			cbReplyBytes,		// number of bytes to write
			&cbWritten,			// number of bytes written
			NULL);				// not overlapped I/O

		if (!fSuccess || cbReplyBytes != cbWritten) {
			_tprintf(TEXT("InstanceThread WriteFile failed, GLE=%d.\n"), GetLastError());
			break;
		}
	}

	// Flush the pipe to allow the client to read the pipe's contents
	// before disconnecting. Then disconnect the pipe, and close the 
	// handle to this pipe instance.
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);

	HeapFree(hHeap, 0, pchRequest);
	HeapFree(hHeap, 0, pchReply);

	printf("InstanceThread exiting. \n");
	return 1;
}

VOID GetAnswerToRequest(LPTSTR pchRequest, LPTSTR pchReply,
	LPDWORD pchBytes) {
	// Simple function to print client requests to the console and 
	// populate a default reply buffer
	// We need to replace this code with actual client request processing code

	// Deserialize the request first
	Person person;
	person.deserialize((char*)pchRequest);

	printf("Client Request Message:\n\tName: %s\n\tAge: %d\n\tWeight: %f\n", person.getName(), person.getAge(), person.getWeight());

	// Check the outgoing message to make sure it's not too long for the buffer.
	if (FAILED(StringCchCopy(pchReply,
		BUFSIZE,
		TEXT("default answer from server")))) {
		*pchBytes = 0;
		pchReply[0] = 0;
		printf("StringCchCopy failed, no outgoing message.\n");
		return;
	}
	*pchBytes = (lstrlen(pchReply) + 1) * sizeof(TCHAR);

}
