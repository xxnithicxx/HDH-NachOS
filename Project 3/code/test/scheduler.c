#include "syscall.h"

#define MAX_FILE_LENGTH 32

/* void main()
{
	int pingPID, pongPID;
	PrintString("Ping-Pong test starting...\n\n");
	pingPID = Exec("./test/ping");
	pongPID = Exec("./test/pong");
	Join(pingPID);
	Join(pongPID);
} */

void main()
{
	SpaceId input, output, bottle, use;
	int numberOfTry;
	char word;

	// Create all semaphores to synchronize the processes
	if (CreateSemaphore("main", 1) == -1)
	{
		PrintString("CreateSemaphore input failed\n");
		return;
	}

	if (CreateSemaphore("sinhvien", 1) == -1)
	{
		PrintString("CreateSemaphore output failed\n");
		return;
	}

	if (CreateSemaphore("voinuoc", 1) == -1)
	{
		PrintString("CreateSemaphore bottle failed\n");
		return;
	}

	if (CreateSemaphore("synSinhVien_VoiNuoc", 1) == -1)
	{
		PrintString("CreateSemaphore use failed\n");
		return;
	}

	CreateFile("output.txt");
	// Open file output.txt to write and read
	output = Open("output.txt", 0);
	if (output == -1)
	{
		PrintString("Can not open file output.txt\n");
		return;
	}

	// Open file input.txt to read only
	input = Open("input.txt", 1);
	if (input == -1)
	{
		PrintString("Can not open file input.txt\n");
		return;
	}

	// Read number of try from input.txt
	numberOfTry = 0;
	while(1)
	{
		Read(&word, 1, input);
		if (word == '\n')
		{
			break;
		}

		numberOfTry = numberOfTry * 10 + Ctoi(word);
	}

	if (Exec("./test/sinhvien") == -1)
	{
		PrintString("Exec sinh vien failed\n");
		return;
	}

	if (Exec("./test/voinuoc") == -1)
	{
		PrintString("Exec voi nuoc failed\n");
		return;
	}

	// We loop until the number of try is 0
	while (numberOfTry > 0)
	{
		if (CreateFile("bottle.txt") == -1)
		{
			PrintString("Create bottle.txt failed\n");
			return;
		}

		// Open file bottle.txt to write and read
		bottle = Open("bottle.txt", 0);
		if (bottle == -1)
		{
			PrintString("Can not open file bottle.txt\n");
			return;
		}

		// Write the try list to bottle.txt
		while(1)
		{
			if (Read(&word, 1, input) < 1 || word == '\n')
			{
				break;
			}

			Write(&word, 1, bottle);
		}

		// Close file bottle.txt
		Close(bottle);

		// Call sinhvien process to take the bottle and run
		Signal("sinhvien");
		Wait("main");

		// When sinhvien process is done, we read the use from use.txt
		use = Open("use.txt", 1);
		if (use == -1)
		{
			PrintString("Can not open file use.txt\n");
			return;
		}

		// Write the result to output.txt
		while(1)
		{
			// Call sinhvien to write bottle volume to output.txt
			if (Read(&word, 1, use) < 1)
			{
				Write("\r\n", 1, output);
				Close(use);
				break;
			}

			// Write the result to output.txt next to the bottle volume
			Write(&word, 1, output);
			Write(" ", 1, output);
		}
	}

	// Close file
	Close(input);
	Close(output);
}