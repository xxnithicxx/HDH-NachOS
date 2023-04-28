#include "syscall.h"

int main()
{
    SpaceId bottle, voinuoc;
    char word;
    int length, fileCursor, haveBottle;
    int i;

    Signal("synSinhVien_VoiNuoc");
    while(1)
    {
        // Wait for the main process to write bottle volume to file
        Wait("sinhvien");

        if (CreateFile("use.txt") == -1)
        {
            PrintString("CreateFile use.txt failed\n");
            Signal("main");
            return;
        }

        if (CreateFile("voinuoc.txt") == -1)
        {
            PrintString("CreateFile voinuoc.txt failed\n");
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

        // Read voinuoc to write and read
        voinuoc = Open("voinuoc.txt", 0);
        if (voinuoc == -1)
        {
            PrintString("Can not open file voinuoc.txt\n");
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
                Write("*", 1, voinuoc);
                haveBottle = 1;
            }

            if (haveBottle == 1)
            {
                Close(voinuoc);
                Signal("voinuoc");
                
                // Wait for the main process to write bottle volume to file
                Wait("sinhvien");

                // Delete and create new "voinuoc.txt" 
                if (CreateFile("voinuoc.txt") == -1)
                {
                    PrintString("CreateFile voinuoc.txt failed\n");
                    Signal("main");
                    return;
                }

                // Open file voinuoc.txt to read and write
                voinuoc = Open("voinuoc.txt", 0);
            }

            fileCursor++;
        }

        Close(bottle);
        Signal("main");
    }
}