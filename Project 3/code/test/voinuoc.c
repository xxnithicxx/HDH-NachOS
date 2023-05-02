#include "syscall.h"

int main()
{
	SpaceId use, tap;
	char temp[255];
	char word;
	int volume, tap1, tap2, size;
	int flag;

	tap1 = tap2 = 0;
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

			tap = Open("tap.txt", 1);
			if (tap == -1)
			{
				PrintString("Can not open file tap.txt in VN\n");
				Signal("sinhvien");
				return;
			}

			flag = 0;
			volume = 0;
			while (1)
			{
				if (Read(&word, 1, tap) == -2)
				{
					Close(tap);
					break;
				}

				if (word != '\n')
				{
					if (word >= '0' && word <= '9')
					{
						volume = volume * 10 + Ctoi(word);
					}
				}
				else
				{
					flag = 1;
					Close(tap);
					break;
				}
			}

			if (volume != 0)
			{
				if (tap1 <= tap2)
				{
					tap1 += volume;

					size = Itoa(volume, temp);

					Write(temp, size, use);
					Write("1 ", 2, use);
				}
				else
				{
					tap2 += volume;	

					size = Itoa(volume, temp);
					
					Write(temp, size, use);
					Write("2 ", 2, use);
				}
			}

			if (flag == 1)
			{
				tap1 = tap2 = 0;
				Close(use);
				Signal("sinhvien");
				break;
			}

			Signal("sinhvien");
		}
	}
}