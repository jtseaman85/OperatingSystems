#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static void DoubleToStr(const double, char*);
static int IntLen(int);

int main(int argc, char **argv)
{
	double num = 45452.9451;
	char* str = (char*) malloc(50);
	DoubleToStr(num, str);
	fprintf(stderr, "%s\n", str);
	return 0;
}

static void DoubleToStr(const double dblNum, char* strDblNum)
{
	double dbl = dblNum;
	double decNum;
	int signFlag = 0;
	int intVal = 0;
	int intLen = 0;
	int i;
	int dCount;
	
	// check for negative number
	if (dbl < 0) {
		dbl = -1 * dbl;
	}
	
	// get whole number
	intVal = (int)dbl;
	intLen = (intVal == 0 ? 1 : IntLen(intVal));
	
	for (i = intLen - 1; i >= 0; i--) {
		strDblNum[i] = (char)(intVal % 10 + '0');
		intVal = intVal / 10;
	}
	
	strDblNum[intLen] = '.';
	
	dCount = intLen + 1;
	decNum = dbl - (int) dbl;
	
	while (dCount < 16) {
		decNum = decNum * 10;
		strDblNum[dCount] = (char)((int)decNum + '0');
		decNum = decNum - (int)decNum;
		dCount++;
	}
	
	strDblNum[dCount] = '\0';
}

static int IntLen(int i) {

	int count = 0;
	while (i > 0) {
		count++;
		i = i / 10;
	}
	return count;
}
