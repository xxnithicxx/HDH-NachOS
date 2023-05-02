#include "syscall.h"

int main()
{
	char buffer[255];
	int size;

	size = Itoa(-123, buffer);

	PrintString(buffer);

	return 0;
}

