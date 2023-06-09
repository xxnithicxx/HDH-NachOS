#include "stable.h"

// Constructor
STable::STable()
{
	this->bm = new BitMap(MAX_SEMAPHORE);

	for (int i = 0; i < MAX_SEMAPHORE; i++)
	{
		this->semTab[i] = NULL;
	}
}

// Destructor
STable::~STable()
{
	if (this->bm)
	{
		delete this->bm;
		this->bm = NULL;
	}
	for (int i = 0; i < MAX_SEMAPHORE; i++)
	{
		if (this->semTab[i])
		{
			delete this->semTab[i];
			this->semTab[i] = NULL;
		}
	}
}

int STable::Create(char *name, int init)
{
	// Check if semaphore name is already existed
	if (this->IsExist(name))
	{
		printf("This semaphore is already existed");
		return -1;
	}

	int id = this->FindFreeSlot();

	// If the semaphore table is full return -1
	if (id < 0)
	{
		return -1;
	}

	this->semTab[id] = new MyLock(name, init);
	return id;
}

int STable::Wait(char *name)
{
	// Check if semaphore name is already existed
	if (!this->IsExist(name))
	{
		printf("This semaphore is not existed");
		return -1;
	}

	MyLock *sem = this->GetSemaphore(name);

	sem->wait();
	return 0;
}

int STable::Signal(char *name)
{
	// Check if semaphore name is already existed
	if (!this->IsExist(name))
	{
		printf("This semaphore is not existed");
		return -1;
	}

	MyLock *sem = this->GetSemaphore(name);

	sem->signal();
	return 0;
}

int STable::FindFreeSlot()
{
	return this->bm->Find();
}

bool STable::IsExist(char *name)
{
	for (int i = 0; i < MAX_SEMAPHORE; i++)
	{
		if (bm->Test(i))
		{
			if (strcmp(name, semTab[i]->GetName()) == 0)
			{
				return true;
			}
		}
	}
	return false;
}

MyLock *STable::GetSemaphore(char *name)
{
	for (int i = 0; i < MAX_SEMAPHORE; i++)
	{
		if (bm->Test(i))
		{
			if (strcmp(name, semTab[i]->GetName()) == 0)
			{
				return semTab[i];
			}
		}
	}
	return NULL;
}