// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

#define MaxFileLength 32

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

/*
Input:
Output:
Purpose: - Increment PC register by 4 because this is MIPS architecture
- To prevent infinite loop when executing syscall
*/
void IncreasePC()
{
    /*
    These two function are in "machine/machine.h" under Machine class
    The PCReg is the program counter register
    PrevPCReg is the previous program counter register (used for debugging)
    NextPCReg is the next program counter register (used for branch delay)
    */
    int counter = machine->ReadRegister(PCReg);
    machine->WriteRegister(PrevPCReg, counter);
    counter = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, counter);
    machine->WriteRegister(NextPCReg, counter + 4);
}

/*
Input: - User space address (int)
- Limit of buffer (int)
Output:- Buffer (char*)
Purpose: Copy buffer from User memory space to System memory space
*/
char *User2System(int virtAddr, int limit)
{
    int i; // index
    int oneChar;
    char *kernelBuf = NULL;
    kernelBuf = new char[limit + 1]; // need for terminal string
    if (kernelBuf == NULL)
        return kernelBuf;
    memset(kernelBuf, 0, limit + 1);
    // printf("\n Filename u2s:");
    for (i = 0; i < limit; i++)
    {
        machine->ReadMem(virtAddr + i, 1, &oneChar);
        kernelBuf[i] = (char)oneChar;
        // printf("%c",kernelBuf[i]);
        if (oneChar == 0)
            break;
    }
    return kernelBuf;
}

/*
Input: - User space address (int)
- Limit of buffer (int)
- Buffer (char[])
Output:- Number of bytes copied (int)
Purpose: Copy buffer from System memory space to User memory space
*/
int System2User(int virtAddr, int len, char *buffer)
{
    if (len < 0)
        return -1;
    if (len == 0)
        return len;
    int i = 0;
    int oneChar = 0;
    do
    {
        oneChar = (int)buffer[i];
        machine->WriteMem(virtAddr + i, 1, oneChar);
        i++;
    } while (i < len && oneChar != 0);
    return i;
}

void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch (which)
    {
    case NoException:
        return;

    case PageFaultException:
        printf("\n No valid translation found");
        interrupt->Halt();
        break;
    case ReadOnlyException:
        printf("\n Write attempted to page marked \"read-only\"");
        interrupt->Halt();
        break;
    case BusErrorException:
        printf("\n Translation resulted in an invalid physical address");
        interrupt->Halt();
        break;
    case AddressErrorException:
        printf("\n Unaligned reference or one that was beyond the end of the address space");
        interrupt->Halt();
        break;
    case OverflowException:
        printf("\n Integer overflow in add or sub.");
        interrupt->Halt();
        break;
    case IllegalInstrException:
        printf("\n Unimplemented or reserved instr.");
        interrupt->Halt();
        break;
    case NumExceptionTypes:
        printf("\n Number of exception types");
        interrupt->Halt();
        break;

    case SyscallException:
        switch (type)
        {
        case SC_Halt:
            /*
            Input: No input
            Output: No output
            Purpose: Halt the machine
            */
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;

        case SC_Exit:
        {
            /*
            Input: Exit code (int)
            Output: No output
            Purpose: Exit the current user program
            */
            int exitCode = machine->ReadRegister(4);

            if (exitCode != 0)
            {
                printf("\nError: process exited abnormally with code %d", exitCode);
                IncreasePC();
                return;
            }

            // Assign exit code to the current thread
            pTab->ExitUpdate(exitCode);

            // Free the space of the current thread and finish it
            currentThread->FreeSpace();
            currentThread->Finish();
            IncreasePC();
            return;
        }

        case SC_Exec:
        {
            /*
            Input: - User space address of the name of the file (char*)
            Output: - Process ID of the new process (int)
            Purpose: Create a new process to execute a program
            */
            int virtAddr;
            char *filename;

            // Get the name of the file
            virtAddr = machine->ReadRegister(4);

            // Copy the name from user space to kernel space
            filename = User2System(virtAddr, MaxFileLength + 1); // MaxFileLength là = 32

            if (filename == NULL)
            {
                printf("\nNot enough memory in system");
                // If not enough memory, return -1
                machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }

            // Check if the file exists
            if (fileSystem->IsExist(filename) == FALSE)
            {
                printf("\nFile %s not found", filename);
                // If file not found, return -1
                machine->WriteRegister(2, -1);
                delete[] filename;
                IncreasePC();
                break;
            }

            int processID = pTab->ExecUpdate(filename);
            machine->WriteRegister(2, processID);

            delete[] filename;
            IncreasePC();
            break;
        }

        case SC_Join:
        {
            /*
            Input: - Process ID of the child process (SpaceId)
            Output: - Exit code of the child process (int)
            Purpose: Wait for a child process to exit
            */
            int childID = machine->ReadRegister(4);

            // The checking process is done in the JoinUpdate function
            // Because there is when pcb table is existed
            int joinCode = pTab->JoinUpdate(childID);
            machine->WriteRegister(2, joinCode);

            IncreasePC();
            break;
        }

        case SC_Create:
        {
            int virtAddr;
            char *filename;

            // Get the name of the file
            virtAddr = machine->ReadRegister(4);

            // Copy the name from user space to kernel space
            filename = User2System(virtAddr, MaxFileLength + 1); // MaxFileLength là = 32
            if (filename == NULL)
            {
                printf("\n Not enough memory in system");
                // If not enough memory, return -1
                machine->WriteRegister(2, -1);
                delete filename;
                IncreasePC();
                return;
            }

            // Create file with size = 0
            // Dùng đối tượng fileSystem của lớp OpenFile để tạo file, việc tạo file này là sử dụng các
            // thủ tục tạo file của hệ điều hành Linux, chúng ta không quản ly trực tiếp các block trên
            // đĩa cứng cấp phát cho file, việc quản ly các block của file trên ổ đĩa là một đồ án khác
            if (!fileSystem->Create(filename, 0))
            {
                printf("\n Error create file '%s'", filename);
                machine->WriteRegister(2, -1);
                delete filename;
                IncreasePC();
                return;
            }

            // Return 0 if success
            machine->WriteRegister(2, 0);
            delete[] filename;
            IncreasePC();
            return;
        }

            /* case SC_Open:
            {
                // Input: - User space address of the name of the file (char*)
                // Output: - File id that user can write and read (OpenFileId)
                // Purpose: Open a file

                int virtAddr;
                char *filename;

                // Get the name of the file
                virtAddr = machine->ReadRegister(4);

                // Copy the name from user space to kernel space
                filename = User2System(virtAddr, MaxFileLength + 1); // MaxFileLength là = 32

                if (filename == NULL)
                {
                    printf("\n Not enough memory in system");
                    // If not enough memory, return -1
                    machine->WriteRegister(2, -1);
                    delete filename;
                    IncreasePC();
                    return;
                }

                for (int i = 2; i < MAX_OPEN_FILE; i++)
                    if (fileSystem->openList[i] == NULL)
                    {
                        OpenFile *file = fileSystem->Open(filename); // Open file and add to openList

                        if (file == NULL)
                        {
                            printf("\n Error open file '%s'", filename);
                            machine->WriteRegister(2, -1);
                            delete[] filename;
                            IncreasePC();
                            return;
                        }

                        machine->WriteRegister(2, i);
                        delete[] filename;
                        IncreasePC();
                        return;
                    }

                delete[] filename;
                printf("\n Error open file '%s'", filename);
                machine->WriteRegister(2, -1);
                IncreasePC();
                return;
            } */

        case SC_Open:
        {
            /*
            Input: - User space address of the name of the file (char*)
            - Type of the file (int)
            Output: - File id that user can write and read (OpenFileId)
            Purpose: Open a file
            */
            int virtAddr;
            char *filename;
            int type;

            // Get the name of the file
            virtAddr = machine->ReadRegister(4);
            type = machine->ReadRegister(5);

            // Copy the name from user space to kernel space
            filename = User2System(virtAddr, MaxFileLength + 1); // MaxFileLength là = 32

            if (filename == NULL)
            {
                printf("\n Not enough memory in system");
                // If not enough memory, return -1
                machine->WriteRegister(2, -1);
                delete filename;
                IncreasePC();
                return;
            }

            if (strcmp(filename, "stdin") == 0) // If file is stdin we return fileID = 0 with no type
            {
                machine->WriteRegister(2, 0);
                delete[] filename;
                IncreasePC();
                return;
            }

            if (strcmp(filename, "stdout") == 0) // If file is stdout we return fileID = 1 with no type
            {
                machine->WriteRegister(2, 1);
                delete[] filename;
                IncreasePC();
                return;
            }

            if (type < 0 || type > 2)
            {
                printf("\n There is no type %d", type);
                machine->WriteRegister(2, -1);
                delete[] filename;
                IncreasePC();
                return;
            }

            // Find an empty slot in openList
            for (int i = 2; i < MAX_OPEN_FILE; i++)
            {
                if (fileSystem->openList[i] == NULL)
                {
                    OpenFile *file = fileSystem->Open(filename, type); // Open file and add to openList

                    if (file == NULL)
                    {
                        printf("\n Error open file '%s'", filename);
                        machine->WriteRegister(2, -1);
                        delete[] filename;
                        IncreasePC();
                        return;
                    }

                    fileSystem->openList[i] = file;

                    machine->WriteRegister(2, i);
                    delete[] filename;
                    IncreasePC();
                    return;
                }
            }

            delete[] filename;
            printf("\n Error open file %s", filename);
            machine->WriteRegister(2, -1);
            IncreasePC();
            return;
        }

        case SC_Write:
        {
            /*
            Input:
            - User space address of the buffer (char*)
            - Size of buffer (int)
            - File id (OpenFileId)
            Output: No output
            Purpose: Write a buffer to a file
            */
            int virtAddr = machine->ReadRegister(4);
            int size = machine->ReadRegister(5);
            int fileID = machine->ReadRegister(6);
            char *buffer;

            // Copy the buffer from user space to kernel space
            buffer = User2System(virtAddr, size);

            if (fileID < 1 || fileID >= MAX_OPEN_FILE) // Expect fileID = 0 (stdin)
            {
                printf("\n Error write file");
                machine->WriteRegister(2, -1);
                delete[] buffer;
                IncreasePC();
                return;
            }

            OpenFile *file = fileSystem->openList[fileID];

            if (file == NULL)
            {
                printf("\n Error write file");
                machine->WriteRegister(2, -1);
                delete[] buffer;
                IncreasePC();
                return;
            }

            if (fileID == 1) // Console output (stdout), this doesn't have type
            {
                int trueByteWrite = gSynchConsole->Write(buffer, size);
                machine->WriteRegister(2, trueByteWrite);
                delete[] buffer;
                IncreasePC();
                return;
            }

            if (file->type == 1)
            {
                printf("\n Error write file (file is read only)");
                machine->WriteRegister(2, -1);
                delete[] buffer;
                IncreasePC();
                return;
            }

            int position = file->getPosition();
            int trueByteWrite = file->Write(buffer, size);

            if (trueByteWrite < 0)
            {
                printf("\n Error write file");
                machine->WriteRegister(2, -1);
                delete[] buffer;
                IncreasePC();
                return;
            }

            if (position + trueByteWrite == file->Length())
                // Write to the end of file
                machine->WriteRegister(2, -2);
            else
                // Normal write to file
                machine->WriteRegister(2, trueByteWrite);

            delete[] buffer;
            IncreasePC();
            break;
        }

        case SC_Read:
        {
            /*
            Input:
            - User space address of the buffer (char*)
            - Size of buffer (int)
            - File id (OpenFileId)
            Output: No output
            Purpose: Read a buffer from a file
            */
            int virtAddr = machine->ReadRegister(4);
            int size = machine->ReadRegister(5);
            int fileID = machine->ReadRegister(6);
            char *buffer = new char[size];

            if (fileID < 0 || fileID >= MAX_OPEN_FILE || fileID == 1) // Expect fileID = 0 (stdin)
            {
                printf("\n Error read file id");
                machine->WriteRegister(2, -1);
                delete[] buffer;
                IncreasePC();
                return;
            }

            OpenFile *file = fileSystem->openList[fileID];

            if (file == NULL)
            {
                printf("\n Error read file null");
                machine->WriteRegister(2, -1);
                delete[] buffer;
                IncreasePC();
                return;
            }

            if (fileID == 0) // Console input (stdin), this doesn't have type
            {
                int trueByteRead = gSynchConsole->Read(buffer, size);
                System2User(virtAddr, trueByteRead, buffer);
                machine->WriteRegister(2, trueByteRead);
                delete[] buffer;
                IncreasePC();
                return;
            }

            int position = file->getPosition();
            int trueByteRead = file->Read(buffer, size);

            // In any file type, we allow user to all read
            if (trueByteRead < 0)
            {
                printf("\n Error read file");
                machine->WriteRegister(2, -1);
                delete[] buffer;
                IncreasePC();
                return;
            }

            // Return the number of bytes read
            if (trueByteRead != 0)
                if (position + trueByteRead == file->Length())
                    // If we read to the end of file, return -2
                    machine->WriteRegister(2, file->Length());
                else
                    // If we read normal file, return the number of bytes read
                    machine->WriteRegister(2, trueByteRead);
            else
                // If we read to the empty file, return -2
                machine->WriteRegister(2, -2);

            System2User(virtAddr, size, buffer);

            delete[] buffer;
            IncreasePC();
            break;
        }

        case SC_Close:
        {
            /*
            Input: - File id (OpenFileId)
            Output: - Error code (int)
            Purpose: Close a file
            */
            int fileID = machine->ReadRegister(4);

            if (fileID < 0 || fileID >= MAX_OPEN_FILE || fileSystem->openList[fileID] == NULL)
            {
                printf("\n Error close file");
                machine->WriteRegister(2, -1);
                IncreasePC();
                return;
            }

            delete fileSystem->openList[fileID]; // Close file
            fileSystem->openList[fileID] = NULL; // Remove file from open list

            machine->WriteRegister(2, 0); // Return 0 if success

            IncreasePC();
            break;
        }

        case SC_Fork:
        {
            // We not learn fork yet, so we just return -1
            machine->WriteRegister(2, -1);
            IncreasePC();
            break;
        }

        case SC_Yield:
        {
            // We not learn yield yet, so we just return -1
            machine->WriteRegister(2, -1);
            IncreasePC();
            break;
        }

        case SC_ReadInt:
        {
            /*
            Input: No input
            Output: - The number that user input (int)
            Purpose: Read a number from console
            */
            int MAX_BUFFER = 255;
            char *num = new char[MAX_BUFFER + 1];
            int numbytes = gSynchConsole->Read(num, MAX_BUFFER); // Return num of characters

            // Check negative or positive
            bool isNegative = false;
            int firstIndex = 0;
            int lastIndex = 0;

            if (num[0] == '-')
            {
                isNegative = true;
                firstIndex = 1;
                lastIndex = 1;
            }

            bool isValid = true;
            // Check the number is valid
            for (int i = firstIndex; i < numbytes; i++)
            {
                int j = i + 1;
                bool isdouble = false;

                if (num[i] == '.') // check is integer or float
                {
                    for (j; j < numbytes; j++)
                    {
                        if (num[j] != '0')
                        {
                            gSynchConsole->Write("The number is not valid\n", 24);
                            isdouble = true;
                            isValid = false;
                            machine->WriteRegister(2, -1);
                            delete num;
                            break;
                        }
                    }
                    if (isdouble)
                        break;
                    lastIndex = i - 1;
                }
                else if (num[i] < '0' || num[i] > '9')
                {
                    gSynchConsole->Write("The number is not valid\n", 24);
                    machine->WriteRegister(2, -1);
                    isValid = false;
                    delete num;
                    break;
                }

                lastIndex = i;
            }
            if (isValid == false)
            {
                IncreasePC();
                break;
            }

            // Convert char to int
            int number;
            number = atoi(num);

            machine->WriteRegister(2, number);
            IncreasePC();
            delete num;
            break;
        }

        case SC_PrintInt:
        {
            /*
            Input: - The number that user want to print (int)
            Output: No output
            Purpose: Print a number to console
            */

            int number = machine->ReadRegister(4);
            if (number == 0)
            {
                gSynchConsole->Write("0", 1);
                IncreasePC();
                break;
            }

            // Convert int to char to print into screen
            // Check negative or positive
            bool isNegative = false;
            int firstIndex = 0;
            if (number < 0)
            {
                isNegative = true;
                number = number * -1;
                firstIndex = 1;
            }

            // Get the number of digits
            int numberOfNum = 0;
            int temp_number = number;
            while (temp_number != 0)
            {
                numberOfNum += 1;
                temp_number = temp_number / 10;
            }
            int MAX_BUFFER = 255;
            char *ch_num = new char[MAX_BUFFER];

            // Convert int to char
            for (int i = numberOfNum + firstIndex - 1; i >= 0; i--)
            {
                ch_num[i] = (char)((number % 10) + 48);
                number /= 10;
            }
            if (isNegative)
            {
                ch_num[0] = '-';
                ch_num[numberOfNum + 1] = '\0';
                gSynchConsole->Write(ch_num, numberOfNum + 1);
            }
            else
            {
                ch_num[numberOfNum] = '\0';
                gSynchConsole->Write(ch_num, numberOfNum);
            }

            delete ch_num;
            IncreasePC();
            break;
        }

        case SC_ReadChar:
        {
            /*
            Input: No input
            Output: - The character that user input (char)
            Purpose: Read a character from console
            */

            int MAX_BYTES = 255;
            char *buffer = new char[MAX_BYTES];
            int numbytes = gSynchConsole->Read(buffer, MAX_BYTES);

            if (numbytes == 0)
            {
                gSynchConsole->Write("The character is not valid\n", 28);
                delete buffer;
                machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }
            else if (numbytes > 1)
            {
                gSynchConsole->Write("This is a string\n", 17);
                delete buffer;
                machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }
            else
            {
                machine->WriteRegister(2, buffer[0]);
                delete buffer;
                IncreasePC();
                break;
            }
        }

        case SC_PrintChar:
        {
            /*
            Input: - The character that user want to print (char)
            Output: No output
            Purpose: Print a character to console
            */

            char c = (char)machine->ReadRegister(4); // Read char from Register 4
            gSynchConsole->Write(&c, 1);
            IncreasePC();
            break;
        }

        case SC_ReadString:
        {
            /*
            Input: - The address of buffer user want to store the string (char*)
            - The length of string that user want to read (int)
            Output: - The string that user input (char*)
            Purpose: Read a string from console
            */

            int virAdd = machine->ReadRegister(4);
            int length = machine->ReadRegister(5);
            char *buffer = User2System(virAdd, length);

            gSynchConsole->Read(buffer, length);
            System2User(virAdd, length, buffer);

            delete buffer;
            IncreasePC();
            break;
        }

        case SC_PrintString:
        {
            /*
            Input: - The string that user want to print (char*)
            Output: No output
            Purpose: Print a string to console
            */

            int virAdd = machine->ReadRegister(4);
            char *buffer = User2System(virAdd, 255);

            int length = 0;
            while (buffer[length] != 0)
                length++;

            gSynchConsole->Write(buffer, length + 1);
            delete buffer;
            IncreasePC();
            break;
        }

        case SC_Semaphore:
        {
            /*
            Input: - The name of semaphore (char*)
            - The initial value of semaphore (int)
            Output: - The semaphore ID (int)
            Purpose: Create a semaphore
            */
            int virAdd = machine->ReadRegister(4);
            int init = machine->ReadRegister(5);

            char *name = User2System(virAdd, MaxFileLength + 1);

            if (name == NULL)
            {
                printf("\nNot enough memory in system");
                machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }

            int semID = semTab->Create(name, init);

            if (semID == -1)
            {
                printf("\nCreate semaphore failed");
                machine->WriteRegister(2, -1);
                delete[] name;
                IncreasePC();
                break;
            }

            machine->WriteRegister(2, semID);
            delete[] name;
            IncreasePC();
            break;
        }

        case SC_Wait:
        {
            /*
            Input: - The semaphore name (char*)
            Output: No output
            Purpose: Wait for a semaphore
            */
            int virAdd = machine->ReadRegister(4);
            char *name = User2System(virAdd, MaxFileLength + 1);
            if (name == NULL)
            {
                printf("\nNot enough memory in system");
                machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }

            int code = semTab->Wait(name); // Wait for a semaphore, if there is no semaphore with this name, return -1
            if (code == -1)
            {
                printf("\nWait semaphore failed");
                machine->WriteRegister(2, -1);
                delete[] name;
                IncreasePC();
                break;
            }

            delete[] name;
            machine->WriteRegister(2, code);
            IncreasePC();
            break;
        }

        case SC_Signal:
        {
            /*
            Input: - The semaphore name (char*)
            Output: No output
            Purpose: Signal a semaphore
            */
            int virAdd = machine->ReadRegister(4);
            char *name = User2System(virAdd, MaxFileLength + 1);
            if (name == NULL)
            {
                printf("\nNot enough memory in system");
                machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }

            int code = semTab->Signal(name); // Signal a semaphore, if there is no semaphore with this name, return -1
            if (code == -1)
            {
                printf("\nSignal semaphore failed");
                machine->WriteRegister(2, -1);
                delete[] name;
                IncreasePC();
                break;
            }

            delete[] name;
            machine->WriteRegister(2, code);
            IncreasePC();
            break;
        }

        case SC_Seek:
        {
            /*
            Input: - The position that user want to seek (int)
            - The file descriptor (int)
            Output: No output
            Purpose: Seek the position of file
            */
            int pos = machine->ReadRegister(4);
            int fd = machine->ReadRegister(5);

            if (fd < 0 || fd >= MAX_OPEN_FILE || fileSystem->openList[fd] == NULL)
            {
                gSynchConsole->Write("The file is not valid\n", 22);
                machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }
            else
            {
                // If the position is -1, then seek to the end of file
                pos = (pos == -1) ? fileSystem->openList[fd]->Length() : pos;

                // Check the position is valid or not
                if (pos < 0 || pos > fileSystem->openList[fd]->Length())
                {
                    gSynchConsole->Write("The position is not valid\n", 26);
                    machine->WriteRegister(2, -1);
                    IncreasePC();
                    break;
                }

                fileSystem->openList[fd]->Seek(pos);
                machine->WriteRegister(2, pos);
                IncreasePC();
                break;
            }
        }

        case SC_Ctoi:
        {
            /*
            Input: - The character we want to convert (char)
            Output: - The integer that user read (int)
            Purpose: Convert a character to integer
            */
            char c = (char)machine->ReadRegister(4);

            if (c < '0' || c > '9')
            {
                printf("\nCharacter is not valid: %d", c);
                machine->WriteRegister(2, (int)c);
                IncreasePC();
                break;
            }

            int res = c - '0';
            machine->WriteRegister(2, res);
            IncreasePC();
            break;
        }

        case SC_Itoa:
        {
            /* 
            Input: - The integer that user want to convert (int)
            - The address of buffer user want to store the string (char*)
            Output: - Number of characters in string (int)
            Purpose: Convert an integer to string
             */
            int num = machine->ReadRegister(4);
            int virAdd = machine->ReadRegister(5);

            char *buffer = User2System(virAdd, MaxFileLength + 1);

            if (buffer == NULL)
            {
                printf("\nNot enough memory in system");
                machine->WriteRegister(2, -1);
                IncreasePC();
                break;
            }

            int length = 0;
            int temp = num;
            bool isNegative = false;

            if (num < 0)
            {
                isNegative = true;
                num = -num;
            }

            do
            {
                length++;
                temp /= 10;
            } while (temp != 0);

            if (isNegative)
            {
                length++;
                buffer[0] = '-';
            }

            for (int i = length - 1; i >= 0 + isNegative; i--)
            {
                buffer[i] = (num % 10) + '0';
                num /= 10;
            }

            buffer[length] = '\0';
            System2User(virAdd, length + 1, buffer);
            machine->WriteRegister(2, length);
            IncreasePC();
            break;
        }

        default:
            break;
        }

        break;
    default:
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}
