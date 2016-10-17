// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <conio.h>
#include <stdio.h>
#include "stringUtilities.h"

int main(int argc, char* argv[])
{
	printf("Hello this World!\n");
	char *textFileTxt=STRUTILEReadTxtFile("..\\Dump.txt",-1); // 2048
	if (textFileTxt)
		printf("The text file contains the following data\n%s",textFileTxt);
	delete [] textFileTxt;
	textFileTxt = NULL;

	// the following...
	int *myInts = new int[1];
	myInts[0]=1;
	delete [] myInts;
	// works?? ^!!

	getch();
	return 0;
}