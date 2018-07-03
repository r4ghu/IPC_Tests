# IPC_Tests
This is the implementation of Inter-Process Communications using named pipes. 

The Client creates a Person object and serializes it and passes it to the server. The server deserializes the object and makes various changes to it as per the client's requests. And finally the client will receive the updated object from the server and displays it on the terminal.

NOTE: This branch executes synchronous read/write. For synchronous ops check feature/async.

## Requirements

- Windows 10
- Visual Studio 2017 (for seeing the code)
- Some basic understanding of C++.

## Usage

To see the demo,
- **Step 1**: Go to `Bin/` directory and open `Server.exe` (or) Open the solution file `Server/Server.sln` and run from Visual Studio 2017. Make sure the server has setup the namedPipe successfully.
- **Step 2**: Go to `Bin/` directory and open `Client.exe` (or) Open the solution file `Client/Client.sln` and run from Visual Studio 2017. Keep pressing `Enter` to execute various Tasks.

## References

This repo wouldn't have been completed without the extensive documentation on Named Pipes from [Link](https://docs.microsoft.com/en-us/windows/desktop/ipc/pipes) and the Serializer has been designed with inspiration from [Link](https://stackoverflow.com/questions/26336414/serializing-object-to-byte-array-in-c/26337239#26337239)

## Author

Sri Raghu Malireddi / [@r4ghu](https://sriraghu.com)