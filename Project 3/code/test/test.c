#include "syscall.h"


int main()
{
	// int i;
	// i = ReadInt();
	// PrintInt(i);

	// char c;
	// c = ReadChar();
	// PrintChar(c);

	// char s[100];
	// ReadString(s, 100);
	// PrintString(s);

	SpaceId si_input;
	char c_readFile;
	
	Create("input.txt");
	si_input = Open("input.txt", 0);
	Write("Hello world", 11, si_input);
	Read(&c_readFile, 1, si_input);
	return 0;

	return 0;
}

