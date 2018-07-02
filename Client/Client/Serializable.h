#pragma once

class Serializable {
public:
	virtual size_t serializeSize() const = 0;
	virtual void serialize(char* dataOut) const = 0;
	virtual void deserialize(const char* dataIn) = 0;
};
