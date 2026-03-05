/*############################################################################
#  Title: projectsoft.h
#  Author: C. Johnson
#  Date: 5/30/24
#  Description: header for communications routines for projectsoft 
#	plc for hexapod
#
#############################################################################*/


#define PROJECTSOFT_IP "10.0.3.25"
#define PROJECTSOFT_PORT 2000
#define PROJECTSOFT_ERROR 0
#define PROJECTSOFT_SUCCESS 1



int projectsoft_el_strut(float *el, float *tstrut);
