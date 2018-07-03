/**
IPC_Tests v0.1
Person.h
Purpose: Defines a person with different variables &
		 (De)Serialize the information through namedPipe.

@author Sri Malireddi
@version 0.1 02/07/2018
*/
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
	/**
	Default constructor

	@usage Person person;
	*/
	Person() {
		name = (char*)"";
		age = 0;
		weight = 0;
		ID = 0;
	}

	/**
	Constructor with person related information.

	@param nameIn Input name for the person.
	@param ageIn Input age for the person.
	@param weightIn Input weight for the person.

	@usage Person person(nameIn, ageIn, weightIn);
	*/
	Person(char* nameIn, int ageIn, float weightIn) {
		name = nameIn;
		age = ageIn;
		weight = weightIn;
		ID = 0;
	}

	/**
	Method that returns the size of the serialized message buffer 
	for person object

	@usage size_t sSize = person.serializeSize();
	*/
	virtual size_t serializeSize() const {
		return SerializablePOD<char*>::serializeSize(name) +
			SerializablePOD<int>::serializeSize(age) +
			SerializablePOD<float>::serializeSize(weight) +
			SerializablePOD<int>::serializeSize(ID);
	}

	/**
	Method to serialize message buffer before passing through
	named pipe.

	@usage person.serialize(message);
	*/
	virtual void serialize(char* dataOut) const {
		dataOut = SerializablePOD<char*>::serialize(dataOut, name);
		dataOut = SerializablePOD<int>::serialize(dataOut, age);
		dataOut = SerializablePOD<float>::serialize(dataOut, weight);
		dataOut = SerializablePOD<int>::serialize(dataOut, ID);
	}

	/**
	Method to deserialize message buffer after receiving through
	named pipe.

	@usage person.deserialize(message);
	*/
	virtual void deserialize(const char* dataIn) {
		dataIn = SerializablePOD<char*>::deserialize(dataIn, name);
		dataIn = SerializablePOD<int>::deserialize(dataIn, age);
		dataIn = SerializablePOD<float>::deserialize(dataIn, weight);
		dataIn = SerializablePOD<int>::deserialize(dataIn, ID);
	}

	// Get methods
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

	// Set methods
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