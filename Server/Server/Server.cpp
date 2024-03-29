/**
IPC_Tests v0.1
Server.cpp

@author Sri Malireddi
@version 0.1 02/07/2018
*/
// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Person.h"

DWORD WINAPI InstanceThread(LPVOID);
VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD);
LPTSTR processRequestbyID(Person);

// Defining a global variable for Person (master user)
// for storing up-to-date information from Client.
// Remarks: Really bad implementation and done in a quick 
// hacky way.
// TODO: Possibly store it in some log file or std::vector<Person>
// or some FIFO type struct.
Person person;

int _tmain(VOID) {
	BOOL fConnected = FALSE;
	DWORD dwThreadId = 0;
	HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
	LPTSTR lpszPipename = (LPTSTR)TEXT("\\\\.\\pipe\\PERSONPIPE");

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
		
		// Special case: If Request ID is GET_PERSON_INFO
		// Doing it because there is some data corruption going
		// TODO: Need to identify the error in future.
		Person testpp; testpp.deserialize((char*)pchRequest);
		if (testpp.getID() == GET_PERSON_INFO) {
			char message[BUFSIZE];
			person.serialize(message);
			pchReply = (LPTSTR)message;
		}

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
	// TODO: HeapFree execution for pchReply.
	// Need to solve the data corruption before doing this.

	printf("InstanceThread exiting. \n");
	return 1;
}

VOID GetAnswerToRequest(LPTSTR pchRequest, LPTSTR pchReply,
	LPDWORD pchBytes) {
	// Simple function to print client requests to the console and 
	// populate a default reply buffer
	// We need to replace this code with actual client request processing code

	// Deserialize the request first
	Person currPerson;
	currPerson.deserialize((char*)pchRequest);

	printf("Client Request Message:\n\tName: %s\n\tAge: %d\n\tWeight: %f\n", currPerson.getName(), currPerson.getAge(), currPerson.getWeight());

	// After the message is deserialized, process request by ID
	LPTSTR retMsg = processRequestbyID(currPerson);

	// Check the outgoing message to make sure it's not too long for the buffer.
	if (FAILED(StringCchCopy(pchReply,
		BUFSIZE,
		retMsg))) {
		*pchBytes = 0;
		pchReply[0] = 0;
		printf("StringCchCopy failed, no outgoing message.\n");
		return;
	}

	

	if (currPerson.getID() == GET_PERSON_INFO) {
		*pchBytes = currPerson.serializeSize() * sizeof(size_t);
	}
	else {
		*pchBytes = (lstrlen(pchReply) + 1) * sizeof(TCHAR);
	}
}

LPTSTR processRequestbyID(Person currPerson) {
	int ID = currPerson.getID();
	LPTSTR retMsg;
	switch (ID) {
	case CREATE_NEW_PERSON:
	{
		person = currPerson;
		retMsg = (LPTSTR)TEXT("New Person created!!!");
		break;
	}
		
	case CHANGE_PERSON_AGE:
	{
		int oldAge = person.getAge();
		int newAge = currPerson.getAge();
		person.setAge(newAge);
		retMsg = (LPTSTR)TEXT("Person's age successfully changed");
		break;
	}

	case CHANGE_PERSON_WEIGHT:
	{
		int oldWeight = person.getWeight();
		int newWeight = currPerson.getWeight();
		person.setWeight(newWeight);
		retMsg = (LPTSTR)TEXT("Person's weight successfully changed");
		break;
	}

	case CHANGE_PERSON_NAME:
	{
		char* oldName = person.getName();
		char* newName = currPerson.getName();
		person.setName(newName);
		retMsg = (LPTSTR)TEXT("Person's name successfully changed");
		break;
	}
	
	case GET_PERSON_INFO:
	{
		printf("Current Person Details:\n");
		printf("Sending this to Client:\n\tName: %s\n\tAge: %d\n\tWeight: %f\n", person.getName(), person.getAge(), person.getWeight());
		char message[BUFSIZE];
		person.serialize(message);
		retMsg = (LPTSTR)message;
		break;
	}
		
	default:
		retMsg = (LPTSTR)TEXT("You asked me to do nothing...");
		break;
	}
	return retMsg;
}
