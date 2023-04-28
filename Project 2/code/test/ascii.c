#include "syscall.h"


int main()
{
	char c = 0;
	//PrintString ("ASCII: \n");
	int i = 0;
	for (i; i < 256; i++)
	{
		PrintInt(i);
		PrintString (". ");
		PrintChar(c);
		c +=1;
		PrintString("\n");
	}
	return 0;
}
