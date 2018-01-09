/*############################################################################
#  Title: ngindi.h
#  Author: C. Johnson
#  Date: 12/2/12
#  Description: all of the required globals for NGINDI
#
#############################################################################*/
char NET_ADDR[30];
unsigned short int PORT;
char SYSID[20];

//NG Stuff
char TELID[20];

//tcs stuff
char TCS_NET_ADDR[16];
unsigned short int  TCS_PORT;
char TCS_SYSID[20];
double FLATFLD_POS[2];
double INST1_POS[2];
double INST2_POS[2];

//Focus stuff
char FOC_NET_ADDR[16];
unsigned short int  FOC_PORT;
char FOC_SYSID[20];
int FOC_M4K_POS;
int FOC_EYE_POS;
int FOC_SPOL_POS;
int FOC_AUX_POS;

//Dome stuff
char DOM_NET_ADDR[16];
unsigned short int  DOM_PORT;
char DOM_SYSID[20];
double DOM_FLATFLD_POS;
double DOM_AUX1_POS;
double DOM_AUX2_POS;

//local settings
int CONTROL_MODE;


//function prots from configure.c 
int configure();
