#pragma once

#include "Serializable.h"
#include "SerializablePOD.h"

#define CREATE_NEW_PERSON -20
#define CHANGE_PERSON_NAME -19
#define CHANGE_PERSON_AGE -18
#define CHANGE_PERSON_WEIGHT -17
#define GET_PERSON_INFO -16

class Person : public Serializable {
public:
	// Constructors
	Person() {
		name = (char*)"";
		age = 0;
		weight = 0;
		ID = 0;
	}

	Person(char* nameIn, int ageIn, float weightIn) {
		name = nameIn;
		age = ageIn;
		weight = weightIn;
		ID = 0;
	}

	virtual size_t serializeSize() const {
		return SerializablePOD<char*>::serializeSize(name) +
			SerializablePOD<int>::serializeSize(age) +
			SerializablePOD<float>::serializeSize(weight) +
			SerializablePOD<int>::serializeSize(ID);
	}

	virtual void serialize(char* dataOut) const {
		dataOut = SerializablePOD<char*>::serialize(dataOut, name);
		dataOut = SerializablePOD<int>::serialize(dataOut, age);
		dataOut = SerializablePOD<float>::serialize(dataOut, weight);
		dataOut = SerializablePOD<int>::serialize(dataOut, ID);
	}

	virtual void deserialize(const char* dataIn) {
		dataIn = SerializablePOD<char*>::deserialize(dataIn, name);
		dataIn = SerializablePOD<int>::deserialize(dataIn, age);
		dataIn = SerializablePOD<float>::deserialize(dataIn, weight);
		dataIn = SerializablePOD<int>::deserialize(dataIn, ID);
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

	float getID() {
		return ID;
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

	void setID(int newID) {
		ID = newID;
	}

private:
	char* name;
	int age;
	float weight;
	int ID;
};