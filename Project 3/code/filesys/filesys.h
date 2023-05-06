// filesys.h
//	Data structures to represent the Nachos file system.
//
//	A file system is a set of files stored on disk, organized
//	into directories.  Operations on the file system have to
//	do with "naming" -- creating, opening, and deleting files,
//	given a textual file name.  Operations on an individual
//	"open" file (read, write, close) are to be found in the OpenFile
//	class (openfile.h).
//
//	We define two separate implementations of the file system.
//	The "STUB" version just re-defines the Nachos file system
//	operations as operations on the native UNIX file system on the machine
//	running the Nachos simulation.  This is provided in case the
//	multiprogramming and virtual memory assignments (which make use
//	of the file system) are done before the file system assignment.
//
//	The other version is a "real" file system, built on top of
//	a disk simulator.  The disk is simulated using the native UNIX
//	file system (in a file named "DISK").
//
//	In the "real" implementation, there are two key data structures used
//	in the file system.  There is a single "root" directory, listing
//	all of the files in the file system; unlike UNIX, the baseline
//	system does not provide a hierarchical directory structure.
//	In addition, there is a bitmap for allocating
//	disk sectors.  Both the root directory and the bitmap are themselves
//	stored as files in the Nachos file system -- this causes an interesting
//	bootstrap problem when the simulated disk is initialized.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef FS_H
#define FS_H

#include "copyright.h"
#include "openfile.h"

#define MAX_OPEN_FILE 15

#ifdef FILESYS_STUB // Temporarily implement file system calls as
// calls to UNIX, until the real file system
// implementation is available
class FileSystem
{
public:
	OpenFile **openList;	// List of open files (for easy access by id)

public:
	FileSystem(bool format) {
		openList = new OpenFile*[MAX_OPEN_FILE];
		for (int i = 0; i < MAX_OPEN_FILE; i++)
			openList[i] = NULL;

		openList[0] = this->Open("consolein");
		openList[1] = this->Open("consoleout");    
		this->Create("consolein", 0);
		this->Create("consoleout", 0);
	}

	~FileSystem() {
		for (int i = 0; i < MAX_OPEN_FILE; i++)
			if (openList[i] != NULL)
				delete openList[i];
		delete[] openList;
	}

	bool Create(char *name, int initialSize)
	{
		int fileDescriptor = OpenForWrite(name);

		if (fileDescriptor == -1)
			return FALSE;
		Close(fileDescriptor);
		return TRUE;
	}

//  This function is temporary to build the system
	OpenFile *Open(char *name)
	{
		int fileDescriptor = OpenForReadWrite(name, FALSE);

		if (fileDescriptor == -1)
			return NULL;

		OpenFile *of = new OpenFile(fileDescriptor);
		of->setName(name); 
		
		return of;
	}

	OpenFile *Open(char *name, int type)
	{
		int fileDescriptor = OpenForReadWrite(name, FALSE);

		if (fileDescriptor == -1)
			return NULL;
			
		OpenFile *of = new OpenFile(fileDescriptor, type);
		of->setName(name); 
		
		return of;
	}

	bool Remove(char *name) { return Unlink(name) == 0; }

	bool IsExist(char *name)
	{
		int fileDescriptor = OpenForReadWrite(name, FALSE);

		if (fileDescriptor == -1)
			return FALSE;
		Close(fileDescriptor);
		return TRUE;
	}

	int SlotLeft()
	{
		int slot = 0;
		for (int i = 0; i < MAX_OPEN_FILE; i++)
			if (openList[i] == NULL)
			{
				slot++;
			}
		return slot;
	}
};

#else // FILESYS
class FileSystem
{
public:
	FileSystem(bool format); // Initialize the file system.
							 // Must be called *after* "synchDisk"
							 // has been initialized.
							 // If "format", there is nothing on
							 // the disk, so initialize the directory
							 // and the bitmap of free blocks.

	bool Create(char *name, int initialSize);
	// Create a file (UNIX creat)

	OpenFile *Open(char *name); // Open a file (UNIX open)

	OpenFile *Open(char *name, int type); 	// Open a file with type (UNIX open)
											// type = 0: read and write
											// type = 1: read only

	bool Remove(char *name); // Delete a file (UNIX unlink)

	void List(); // List all the files in the file system

	void Print(); // List all the files and their contents

	bool IsExist(char *name); // Check if a file is exist

	int SlotLeft();				 // Return number of slot left in openList

	OpenFile **openList; // List of open files (for easy access by id)

private:
	OpenFile *freeMapFile;	 // Bit map of free disk blocks,
							 // represented as a file
	OpenFile *directoryFile; // "Root" directory -- list of
							 // file names, represented as a file
};

#endif // FILESYS

#endif // FS_H
