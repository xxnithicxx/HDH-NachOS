#include "syscall.h"

int main()
{
    SpaceId bottle, tap;
    char word;
    int length, fileCursor, haveBottle;

    Signal("sinhvien_voinuoc");
    while(1)
    {
        // Wait for the main process to write bottle volume to file
        Wait("sinhvien");

        // Open file bottle.txt to read only
        bottle = Open("bottle.txt", 1);
        if (bottle == -1)
        {
            PrintString("Can not open file bottle.txt\n");
            Signal("scheduler");
            return;
        }

        // Clear use.txt for the next use
        if (CreateFile("use.txt") == -1)
        {
            PrintString("CreateFile use.txt failed\n");
            return;
        }

        // Get length of file bottle.txt
        Seek(0, bottle);
        length = Seek(-1, bottle);
        Seek(0, bottle);
        fileCursor = 0;

		// Read tap to write and read
        tap = Open("tap.txt", 0);
        if (tap == -1)
        {
            PrintString("Can not open file tap.txt in SV\n");
			Close(bottle);
            Signal("scheduler");
            return;
        }

        while(fileCursor < length)
        {
            haveBottle = 0;
            Read(&word, 1, bottle);

            // Check if there is a bottle
            if (word != ' ')
            {
                Write(&word, 1, tap);
            }
            else
            {
                haveBottle = 1;
            }

            if (fileCursor == length - 1)
            {
                Write("\n", 1, tap);
                haveBottle = 1;
            }

            if (haveBottle == 1)
            {
                Close(tap);
                Signal("voinuoc");
                
                // Wait for the main process to write bottle volume to file
                Wait("sinhvien");

                // Delete and create new "tap.txt" 
                if (CreateFile("tap.txt") == -1)
                {
                    PrintString("CreateFile tap.txt failed\n");
					Close(bottle);
                    Signal("scheduler");
                    return;
                }

                // Open file tap.txt to read and write
                tap = Open("tap.txt", 0);
                if (tap == -1)
                {
                    PrintString("Can not open file tap.txt in SV\n");
					Close(bottle);
                    Signal("scheduler");
                    return;
                }
            }

            fileCursor++;
        }

        Close(bottle);
        Signal("scheduler");
    }
}