/*############################################################################
#  Title: testcbw.c
#  Author: Chris Johnson
#  Date: 4/20/2023
#  Description: quick standalone app that uses the readcbw stuff to test that
#	the routines work
#
#	csj - 5/26/23 --changed to use single float vs the struct
#
#############################################################################*/
#include <stdio.h>
#include "readcbw.h"
#include "vatttel_com.h"

void main()
{
//tempstruct mytemps;
double mytemps;
char myerr[100];
mytemps=GetStrutTempCBW(myerr);
printf("cbw temp:%f\n", mytemps);
mytemps=GetStrutTemp(myerr);
printf("tcs temp:%f\n", mytemps);

}
