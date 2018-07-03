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

typedef struct
{
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	TCHAR chRequest[BUFSIZE];
	DWORD cbRead;
	TCHAR chReply[BUFSIZE];
	DWORD cbToWrite;
} PIPEINST, *LPPIPEINST;

VOID DisconnectAndClose(LPPIPEINST);
BOOL CreateAndConnectInstance(LPOVERLAPPED);
BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);
VOID GetAnswerToRequest(LPPIPEINST);
VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD);
LPTSTR processRequestbyID(Person);

VOID WINAPI CompletedWriteRoutine(DWORD, DWORD, LPOVERLAPPED);
VOID WINAPI CompletedReadRoutine(DWORD, DWORD, LPOVERLAPPED);

HANDLE hPipe;

// Defining a global variable for Person (master user)
// for storing up-to-date information from Client.
// Remarks: Really bad implementation and done in a quick 
// hacky way.
// TODO: Possibly store it in some log file or std::vector<Person>
// or some FIFO type struct.
Person person;

int _tmain(VOID)
{
	HANDLE hConnectEvent;
	OVERLAPPED oConnect;
	LPPIPEINST lpPipeInst;
	DWORD dwWait, cbRet;
	BOOL fSuccess, fPendingIO;

	// Create one event object for the connect operation. 

	hConnectEvent = CreateEvent(
		NULL,    // default security attribute
		TRUE,    // manual reset event 
		TRUE,    // initial state = signaled 
		NULL);   // unnamed event object 

	if (hConnectEvent == NULL)
	{
		printf("CreateEvent failed with %d.\n", GetLastError());
		return 0;
	}

	oConnect.hEvent = hConnectEvent;

	// Call a subroutine to create one instance, and wait for 
	// the client to connect. 

	fPendingIO = CreateAndConnectInstance(&oConnect);

	while (1)
	{
		// Wait for a client to connect, or for a read or write 
		// operation to be completed, which causes a completion 
		// routine to be queued for execution. 

		dwWait = WaitForSingleObjectEx(
			hConnectEvent,  // event object to wait for 
			INFINITE,       // waits indefinitely 
			TRUE);          // alertable wait enabled 

		switch (dwWait)
		{
			// The wait conditions are satisfied by a completed connect 
			// operation. 
		case 0:
			// If an operation is pending, get the result of the 
			// connect operation. 

			if (fPendingIO)
			{
				fSuccess = GetOverlappedResult(
					hPipe,     // pipe handle 
					&oConnect, // OVERLAPPED structure 
					&cbRet,    // bytes transferred 
					FALSE);    // does not wait 
				if (!fSuccess)
				{
					printf("ConnectNamedPipe (%d)\n", GetLastError());
					return 0;
				}
			}

			// Allocate storage for this instance. 

			lpPipeInst = (LPPIPEINST)GlobalAlloc(
				GPTR, sizeof(PIPEINST));
			if (lpPipeInst == NULL)
			{
				printf("GlobalAlloc failed (%d)\n", GetLastError());
				return 0;
			}

			lpPipeInst->hPipeInst = hPipe;

			// Start the read operation for this client. 
			// Note that this same routine is later used as a 
			// completion routine after a write operation. 

			lpPipeInst->cbToWrite = 0;
			CompletedWriteRoutine(0, 0, (LPOVERLAPPED)lpPipeInst);

			// Create new pipe instance for the next client. 

			fPendingIO = CreateAndConnectInstance(
				&oConnect);
			break;

			// The wait is satisfied by a completed read or write 
			// operation. This allows the system to execute the 
			// completion routine. 

		case WAIT_IO_COMPLETION:
			break;

			// An error occurred in the wait function. 

		default:
		{
			printf("WaitForSingleObjectEx (%d)\n", GetLastError());
			return 0;
		}
		}
	}
	return 0;
}

// CompletedWriteRoutine(DWORD, DWORD, LPOVERLAPPED) 
// This routine is called as a completion routine after writing to 
// the pipe, or when a new client has connected to a pipe instance.
// It starts another read operation. 

VOID WINAPI CompletedWriteRoutine(DWORD dwErr, DWORD cbWritten,
	LPOVERLAPPED lpOverLap)
{
	LPPIPEINST lpPipeInst;
	BOOL fRead = FALSE;

	// lpOverlap points to storage for this instance. 

	lpPipeInst = (LPPIPEINST)lpOverLap;

	// The write operation has finished, so read the next request (if 
	// there is no error). 

	if ((dwErr == 0) && (cbWritten == lpPipeInst->cbToWrite))
		fRead = ReadFileEx(
			lpPipeInst->hPipeInst,
			lpPipeInst->chRequest,
			BUFSIZE * sizeof(TCHAR),
			(LPOVERLAPPED)lpPipeInst,
			(LPOVERLAPPED_COMPLETION_ROUTINE)CompletedReadRoutine);

	// Disconnect if an error occurred. 

	if (!fRead)
		DisconnectAndClose(lpPipeInst);
}

// CompletedReadRoutine(DWORD, DWORD, LPOVERLAPPED) 
// This routine is called as an I/O completion routine after reading 
// a request from the client. It gets data and writes it to the pipe. 

VOID WINAPI CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead,
	LPOVERLAPPED lpOverLap)
{
	LPPIPEINST lpPipeInst;
	BOOL fWrite = FALSE;

	// lpOverlap points to storage for this instance. 

	lpPipeInst = (LPPIPEINST)lpOverLap;

	// The read operation has finished, so write a response (if no 
	// error occurred). 

	if ((dwErr == 0) && (cbBytesRead != 0))
	{
		GetAnswerToRequest(lpPipeInst);

		// Special case: If Request ID is GET_PERSON_INFO
		// Doing it because there is some data corruption going
		// TODO: Need to identify the error in future.
		Person testpp; testpp.deserialize((char*)lpPipeInst->chRequest);
		if (testpp.getID() == GET_PERSON_INFO) {
			char message[BUFSIZE];
			person.serialize(message);
			fWrite = WriteFileEx(
				lpPipeInst->hPipeInst,
				(LPTSTR)message,
				lpPipeInst->cbToWrite,
				(LPOVERLAPPED)lpPipeInst,
				(LPOVERLAPPED_COMPLETION_ROUTINE)CompletedWriteRoutine);

		}
		else {
			fWrite = WriteFileEx(
				lpPipeInst->hPipeInst,
				lpPipeInst->chReply,
				lpPipeInst->cbToWrite,
				(LPOVERLAPPED)lpPipeInst,
				(LPOVERLAPPED_COMPLETION_ROUTINE)CompletedWriteRoutine);
		}
		
	}

	// Disconnect if an error occurred. 

	if (!fWrite)
		DisconnectAndClose(lpPipeInst);
}

// DisconnectAndClose(LPPIPEINST) 
// This routine is called when an error occurs or the client closes 
// its handle to the pipe. 

VOID DisconnectAndClose(LPPIPEINST lpPipeInst)
{
	// Disconnect the pipe instance. 

	if (!DisconnectNamedPipe(lpPipeInst->hPipeInst))
	{
		printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
	}

	// Close the handle to the pipe instance. 

	CloseHandle(lpPipeInst->hPipeInst);

	// Release the storage for the pipe instance. 

	if (lpPipeInst != NULL)
		GlobalFree(lpPipeInst);
}

// CreateAndConnectInstance(LPOVERLAPPED) 
// This function creates a pipe instance and connects to the client. 
// It returns TRUE if the connect operation is pending, and FALSE if 
// the connection has been completed. 

BOOL CreateAndConnectInstance(LPOVERLAPPED lpoOverlap)
{
	LPTSTR lpszPipename = (LPTSTR)TEXT("\\\\.\\pipe\\PERSONPIPE");

	hPipe = CreateNamedPipe(
		lpszPipename,             // pipe name 
		PIPE_ACCESS_DUPLEX |      // read/write access 
		FILE_FLAG_OVERLAPPED,     // overlapped mode 
		PIPE_TYPE_MESSAGE |       // message-type pipe 
		PIPE_READMODE_MESSAGE |   // message read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // unlimited instances 
		BUFSIZE * sizeof(TCHAR),    // output buffer size 
		BUFSIZE * sizeof(TCHAR),    // input buffer size 
		PIPE_TIMEOUT,             // client time-out 
		NULL);                    // default security attributes
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		printf("CreateNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}

	// Call a subroutine to connect to the new client. 

	return ConnectToNewClient(hPipe, lpoOverlap);
}

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
	BOOL fConnected, fPendingIO = FALSE;

	// Start an overlapped connection for this pipe instance. 
	fConnected = ConnectNamedPipe(hPipe, lpo);

	// Overlapped ConnectNamedPipe should return zero. 
	if (fConnected)
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}

	switch (GetLastError())
	{
		// The overlapped connection in progress. 
	case ERROR_IO_PENDING:
		fPendingIO = TRUE;
		break;

		// Client is already connected, so signal an event. 

	case ERROR_PIPE_CONNECTED:
		if (SetEvent(lpo->hEvent))
			break;

		// If an error occurs during the connect operation... 
	default:
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}
	}
	return fPendingIO;
}

VOID GetAnswerToRequest(LPPIPEINST pipe)
{
	// Deserialize the request first
	Person currPerson;
	currPerson.deserialize((char*)pipe->chRequest);
	printf("[%d] Client Request Message:\n\tName: %s\n\tAge: %d\n\tWeight: %f\n", pipe->hPipeInst, 
		currPerson.getName(), currPerson.getAge(), currPerson.getWeight());

	// After the message is deserialized, process request by ID
	LPTSTR retMsg = processRequestbyID(currPerson);

	// Check the outgoing message to make sure it's not too long for the buffer.
	if (FAILED(StringCchCopy(pipe->chReply,
		BUFSIZE,
		retMsg))) {
		pipe->cbToWrite = 0;
		pipe->chReply[0] = 0;
		printf("StringCchCopy failed, no outgoing message.\n");
		return;
	}
	
	if (currPerson.getID() == GET_PERSON_INFO) {
		pipe->cbToWrite = currPerson.serializeSize() * sizeof(size_t);
	}
	else {
		pipe->cbToWrite = (lstrlen(pipe->chReply) + 1) * sizeof(TCHAR);
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