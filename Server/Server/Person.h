#pragma once

#include "Serializable.h"
#include "SerializablePOD.h"

class Person : public Serializable {
public:
	// Constructors
	Person() {
		name = (char*)"";
		age = 0;
		weight = 0;
	}

	Person(char* nameIn, int ageIn, float weightIn) {
		name = nameIn;
		age = ageIn;
		weight = weightIn;
	}

	virtual size_t serializeSize() const {
		return SerializablePOD<char*>::serializeSize(name) +
			SerializablePOD<int>::serializeSize(age) +
			SerializablePOD<float>::serializeSize(weight);
	}

	virtual void serialize(char* dataOut) const {
		dataOut = SerializablePOD<char*>::serialize(dataOut, name);
		dataOut = SerializablePOD<int>::serialize(dataOut, age);
		dataOut = SerializablePOD<float>::serialize(dataOut, weight);
	}

	virtual void deserialize(const char* dataIn) {
		dataIn = SerializablePOD<char*>::deserialize(dataIn, name);
		dataIn = SerializablePOD<int>::deserialize(dataIn, age);
		dataIn = SerializablePOD<float>::deserialize(dataIn, weight);
	}

	char* getName() {
		return name;
	}

	int getAge() {
		return age;
	}

	float getWeight() {
		return weight;
	}

	void setName(char* newName) {
		name = newName;
	}

	void setAge(int newAge) {
		age = newAge;
	}

	void setWeight(float newWeight) {
		weight = newWeight;
	}

private:
	char* name;
	int age;
	float weight;
};