/*############################################################################
#  Title: testprojectsoft.c
#  Author: C. Johnson
#  Date: 5/30/24
#  Description: test projectsoft communications routines
#
#############################################################################*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include "projectsoft.h"

int main()
{

float el=0, tstrut=0;
int err=projectsoft_el_strut(&el, &tstrut);
if (err==PROJECTSOFT_ERROR)
    printf("error\n");
printf("strut temp = %f \n elevation = %f\n", tstrut, el);

}
