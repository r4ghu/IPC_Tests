/**
IPC_Tests v0.1
Serializable.h
Purpose: Helper class with virtual functions to
(De)Serialize the information through namedPipe.

@author Sri Malireddi
@version 0.1 02/07/2018
*/
#pragma once

class Serializable {
public:
	virtual size_t serializeSize() const = 0;
	virtual void serialize(char* dataOut) const = 0;
	virtual void deserialize(const char* dataIn) = 0;
};
