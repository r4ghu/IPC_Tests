// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "Person.h"

#define BUFSIZE 1024

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

	// TEST 1: Send an empy message to the server
	printf("Task 1: Create an empty message: {\"\",0,0}\n");
	Person person;
	char message[BUFSIZE];
	person.serialize(message);

	{
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
		printf("Task Sucesss. Press ENTER to start next task\n\n");
		_getch();
	}

	

	// Create the new person
	printf("Task 2 Creating a new user with credentials: {Jack, 25, 80}\n");

	char name[10] = "Jack";
	person.setName(name);
	person.setAge(25);
	person.setWeight(80);
	person.setID(CREATE_NEW_PERSON);
	char messageTask2[BUFSIZE];
	person.serialize(messageTask2);
	
	{
		lpvMessage = (LPTSTR)messageTask2;

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
		printf("Task Sucesss. Press ENTER to start next task\n\n");
		_getch();
	}

	// Task 3: Change Age
	printf("Task 3: Change Jack's age from 30 to 42.\n");
	Person personTask3;
	personTask3.setAge(42);
	personTask3.setID(CHANGE_PERSON_AGE);
	person = personTask3;
	char messageTask3[BUFSIZE];
	person.serialize(messageTask3);

	{
		lpvMessage = (LPTSTR)messageTask3;

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
		printf("Task Sucesss. Press ENTER to start next task\n\n");
		_getch();
	}

	// Task 4: Change Age
	printf("Task 4: Change Jack's weight from 80 to 72.\n");
	Person personTask4;
	personTask4.setWeight(72);
	personTask4.setID(CHANGE_PERSON_WEIGHT);
	person = personTask4;
	char messageTask4[BUFSIZE];
	person.serialize(messageTask4);

	{
		lpvMessage = (LPTSTR)messageTask4;

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
		printf("Task Sucesss. Press ENTER to start next task\n\n");
		_getch();
	}

	// Task 5: Change Name
	printf("Task 5: Change Jack's name from Jack to Jack Star.\n");
	Person personTask5;
	char JackNewName[] = "Jacky";
	personTask5.setName(JackNewName);
	personTask5.setID(CHANGE_PERSON_NAME);
	person = personTask5;
	char messageTask5[BUFSIZE];
	person.serialize(messageTask5);

	{
		lpvMessage = (LPTSTR)messageTask5;

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
		printf("Task Sucesss. Press ENTER to start next task\n\n");
		_getch();
	}

	// Task 6: Get Person Info
	printf("Task 6: Get Jack's current info from server.\n");
	Person personTask6;
	personTask6.setID(GET_PERSON_INFO);
	person = personTask6;
	char messageTask6[BUFSIZE];
	person.serialize(messageTask6);

	{
		lpvMessage = (LPTSTR)messageTask6;

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
		HANDLE hHeap = GetProcessHeap();
		TCHAR* personMessage = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));
		do {
			// Read from the pipe
			fSuccess = ReadFile(
				hPipe,					// pipe handle
				personMessage,					// buffer to receive reply
				BUFSIZE * sizeof(TCHAR),// size of buffer
				&cbRead,				// number of bytes to read
				NULL);					// not overlapped

			if (!fSuccess && GetLastError() != ERROR_MORE_DATA) {
				break;
			}
			person.deserialize((char*)personMessage);
			printf("Client Request Message:\n\tName: %s\n\tAge: %d\n\tWeight: %f\n", person.getName(), person.getAge(), person.getWeight());

		} while (!fSuccess); // repeat loop if ERROR_MORE_DATA

		if (!fSuccess) {
			_tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
			return -1;
		}
		printf("Task Sucesss. Press ENTER to start next task\n\n");
		_getch();
	}

	printf("\n<End of message, press ENTER to terminate connection and exit>");
	_getch();

	CloseHandle(hPipe);

	return 0;
}