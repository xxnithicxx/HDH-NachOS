#include "syscall.h"

int main()
{
    SpaceId use, voinuoc;
    char word;
    int volume, voi1, voi2;
    int flag;

    voi1 = voi2 = 0;
    while (1)
    {
        Wait("synSinhVien_VoiNuoc");

        use = Open("use.txt", 0);
        if (use == -1)
        {
            PrintString("Can not open file use.txt\n");
            Signal("sinhvien");
            return;
        }

        while (1)
        {
            Wait("voinuoc");

            voinuoc = Open("voinuoc.txt", 1);
            if (voinuoc == -1)
            {
                PrintString("Can not open file voinuoc.txt\n");
                Signal("sinhvien");
                return;
            }

            flag = 0;
            while (1)
            {
                if (Read(&word, 1, voinuoc) == -2)
                {
                    Close(voinuoc);
                    break;
                }

                if (word != '*')
                {
                    volume = volume * 10 + Ctoi(word);
                }
                else
                {
                    flag = 1;
                    Close(voinuoc);
                    break;
                }
            }

            if (volume != 0)
            {
                if (voi1 <= voi2)
                {
                    voi1 += volume;
                    // Write(volume, 1, use);
                    Write("1", 1, use);
                }
                else
                {
                    voi2 += volume;
                    // Write(volume, 1, use);
                    Write("2", 1, use);
                }
            }

            if (flag == 1)
            {
                voi1 = voi2 = 0;
                Close(use);
                Signal("sinhvien");
                break;
            }
            
            Signal("sinhvien");
        }
    }
}