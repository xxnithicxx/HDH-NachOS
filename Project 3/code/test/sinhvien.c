#include "syscall.h"

int main()
{
    SpaceId bottle, voinuoc;
    char word;
    int length, fileCursor, haveBottle;

    Signal("synSinhVien_VoiNuoc");
    while(1)
    {
        // Wait for the main process to write bottle volume to file
        Wait("sinhvien");
		// PrintString("\nWait sinhvien done");

        if (CreateFile("use.txt") == -1)
        {
            PrintString("CreateFile use.txt failed\n");
            Signal("main");
            return;
        }

        if (CreateFile("tap.txt") == -1)
        {
            PrintString("CreateFile tap.txt failed\n");
            Signal("main");
            return;
        }

        // Open file bottle.txt to read only
        bottle = Open("bottle.txt", 1);
        if (bottle == -1)
        {
            PrintString("Can not open file bottle.txt\n");
            Signal("main");
            return;
        }

        length = Seek(-1, bottle);
        Seek(0, bottle);
        fileCursor = 0;

		// PrintInt(length);

		// Read voinuoc to write and read
        voinuoc = Open("tap.txt", 0);
        if (voinuoc == -1)
        {
            PrintString("Can not open file tap.txt in SV\n");
			Close(bottle);
            Signal("main");
            return;
        }

        while(fileCursor < length)
        {
            haveBottle = 0;
            Read(&word, 1, bottle);

            // Check if there is a bottle
            if (word != ' ')
            {
                Write(&word, 1, voinuoc);
            }
            else
            {
                haveBottle = 1;
            }

            if (fileCursor == length - 1)
            {
                Write("\n", 1, voinuoc);
                haveBottle = 1;
            }

            if (haveBottle == 1)
            {
                Close(voinuoc);
                Signal("voinuoc");
                
                // Wait for the main process to write bottle volume to file
                Wait("sinhvien");

                // Delete and create new "tap.txt" 
                if (CreateFile("tap.txt") == -1)
                {
                    PrintString("CreateFile tap.txt failed\n");
					Close(bottle);
                    Signal("main");
                    return;
                }

                // Open file tap.txt to read and write
                voinuoc = Open("tap.txt", 0);
                if (voinuoc == -1)
                {
                    PrintString("Can not open file tap.txt in SV\n");
					Close(bottle);
                    Signal("main");
                    return;
                }
            }

            fileCursor++;
        }

        Close(bottle);
        Signal("main");
    }
}