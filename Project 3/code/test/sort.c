#include "syscall.h"

int main()
{
	int n, i, i1, i2, j, t;

	int a[100];
	PrintString("Input n = ");
	n = ReadInt();

	PrintString("Input the array: \n");
	for (i = 0; i < n; i++)
	{
		PrintString("a[");
		PrintInt(i);
		PrintString("] = ");
		a[i] = ReadInt();
	}

	// BubbleSort
	for (i1 = 0; i1 < n - 1; i1++)
		for (j = 0; j < n - 1 - i1; j++)
			if (a[j + 1] > a[j])
			{
				t = a[j + 1];
				a[j + 1] = a[j];
				a[j] = t;
			}

	// Output
	PrintString("Output: \n");

	for (i2 = 0; i2 < n; i2++)
	{
		PrintInt(a[i2]);
		PrintChar(' ');
	}

	// delete a;
	return 0;
}
