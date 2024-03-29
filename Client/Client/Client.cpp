/**
IPC_Tests v0.1
Client.cpp

@author Sri Malireddi
@version 0.1 02/07/2018
*/
// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Person.h"

int CompleteTask(HANDLE, Person);

int _tmain(int argc, char* argv[]) {

	HANDLE hPipe;
	LPTSTR lpvMessage;
	char chBuf[BUFSIZE];
	BOOL fSuccess = FALSE;
	DWORD cbRead, cbToWrite, cbWritten, dwMode;
	LPTSTR lpszPipename = (LPTSTR)TEXT("\\\\.\\pipe\\PERSONPIPE");

	
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

	// TASK 1: Send an empy message to the server
	printf("Task 1: Create an empty message: {\"\",0,0}\n");
	Person person;
	CompleteTask(hPipe, person);
	

	// TASK 2: Create the new person
	printf("Task 2 Creating a new user with credentials: {Jack, 25, 80}\n");

	char name[10] = "Jack";
	person.setName(name);
	person.setAge(25);
	person.setWeight(80);
	person.setID(CREATE_NEW_PERSON);
	CompleteTask(hPipe, person);
	
	

	// Task 3: Change Age
	printf("Task 3: Change Jack's age from 30 to 42.\n");
	Person personTask3;
	personTask3.setAge(42);
	personTask3.setID(CHANGE_PERSON_AGE);
	CompleteTask(hPipe, personTask3);

	// Task 4: Change Age
	printf("Task 4: Change Jack's weight from 80 to 72.\n");
	Person personTask4;
	personTask4.setWeight(72);
	personTask4.setID(CHANGE_PERSON_WEIGHT);
	CompleteTask(hPipe, personTask4);

	// Task 5: Change Name
	printf("Task 5: Change Jack's name from Jack to Jack Star.\n");
	Person personTask5;
	char JackNewName[] = "Jack Star";
	personTask5.setName(JackNewName);
	personTask5.setID(CHANGE_PERSON_NAME);
	CompleteTask(hPipe, personTask5);

	// Task 6: Get Person Info
	printf("Task 6: Get Jack's current info from server.\n");
	Person personTask6;
	personTask6.setID(GET_PERSON_INFO);
	CompleteTask(hPipe, personTask6);

	

	printf("\n<End of message, press ENTER to terminate connection and exit>");
	_getch();

	CloseHandle(hPipe);

	return 0;
}

int CompleteTask(HANDLE hPipe, Person person) {
	char message[BUFSIZE], chBuf[BUFSIZE];
	person.serialize(message);
	LPTSTR lpvMessage = (LPTSTR)message;

	// Send a message to the pipe server
	DWORD cbToWrite = person.serializeSize() * sizeof(size_t);
	DWORD cbWritten, cbRead;

	BOOL fSuccess = WriteFile(
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
			&cbRead,				// number of bytes read
			NULL);					// not overlapped

		if (!fSuccess && GetLastError() != ERROR_MORE_DATA) {
			break;
		}

		if(person.getID()!=GET_PERSON_INFO)
			_tprintf(TEXT("\"%s\"\n"), chBuf);
	} while (!fSuccess); // repeat loop if ERROR_MORE_DATA

	if (person.getID() == GET_PERSON_INFO) {
		person.deserialize((char*)chBuf);
		printf("Client Request Message:\n\tName: %s\n\tAge: %d\n\tWeight: %f\n", person.getName(), person.getAge(), person.getWeight());
	}

	if (!fSuccess) {
		_tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
		return -1;
	}

	printf("Task Success. Press ENTER to start next task\n\n");
	_getch();
	return 0;
}