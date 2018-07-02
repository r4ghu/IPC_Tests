#pragma once
#include <cstring>

template <typename T>
class SerializablePOD {
public:
	static size_t serializeSize(T str) {
		return sizeof(T);
	}

	static char* serialize(char* target, T value) {
		memcpy(target, &value, serializeSize(value));
		return target + serializeSize(value); // TODO: Might try void
	}

	static const char* deserialize(const char* source, T& target) {
		memcpy(&target, source, serializeSize(target));
		return source + serializeSize(target);
	}
};

template<>
size_t SerializablePOD<char*>::serializeSize(char* str) {
	if (str == NULL) return 0;
	return sizeof(size_t) + strlen(str) + 1; // add 1 to copy the \0 termination
}

template<>
char* SerializablePOD<char*>::serialize(char* target, char* value) {
	size_t length = strlen(value) + 1;
	memcpy(target, &length, sizeof(size_t));
	memcpy(target + sizeof(size_t), value, length);
	return target + length + sizeof(size_t);
}

template<>
const char* SerializablePOD<char*>::deserialize(const char* source, char*& target) {
	size_t length;
	memcpy(&length, source, sizeof(size_t));
	target = new char[length];
	memcpy(target, source + sizeof(size_t), length);
	return source + sizeof(size_t) + length;
}
