#ifndef STABLE_H
#define STABLE_H

#include "synch.h"
#include "bitmap.h"

#define MAX_SEMAPHORE 10

// This class is used to manage semaphore by name, it is a wrapper class of Semaphore class provided by Nachos
class MyLock
{
private:
	char name[50];	
	Semaphore* sem;		
public:
	// If type = 0, this is synchronization semaphore
	// If type = 1, this is mutual exclusion semaphore
	MyLock(char* name, int type)
	{
		strcpy(this->name, name);
		sem = new Semaphore(this->name, type);
	}

	~MyLock()
	{
		if(sem)
			delete sem;
	}

	void wait()
	{
		sem->P();	// Down(sem)
	}

	void signal()
	{
		sem->V();	// Up(sem)
	}
	
	char* GetName()
	{
		return this->name;
	}
};

class STable
{
private:
	BitMap* bm;						// BitMap for managing semaphore
	MyLock* semTab[MAX_SEMAPHORE];	// Actual semaphore table	
public:
	// Source: https://github.com/nguyenthanhchungfit/Nachos-Programing-HCMUS
	STable();		

	~STable();	

	// Create a semaphore with name and initial value, if semaphore is already existed, return -1
	int Create(char *name, int init);

	// Wait(Signal) on a semaphore, if semaphore is not existed, return -1
	int Wait(char *name);

	int Signal(char *name);
	
	// Find a free slot in semaphore table
	int FindFreeSlot();

	// Check if semaphore is existed
	bool IsExist(char* name);

	// Get semaphore by name
	MyLock* GetSemaphore(char* name);
};
#endif 