/*############################################################################
#  Title: readcbw.h
#  Author: C. Johnson
#  Date: 4/20/23
#  Description: header file for readcbw.c to read strut temps from
#	Control By Web
#
#############################################################################*/

typedef struct tempstruct{
char error[30];
double temp1;
double temp2;
}tempstruct;

//int GetStrutTempCBW(tempstruct *);
//int GetStrutTempCBW(double *);
double GetStrutTempCBW(char *);
