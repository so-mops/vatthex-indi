#include <stdlib.h>
#include <stdio.h>
#include "vatthex.h"
#include <argp.h>



int TestCorrections( int ID )
{
	double temp = 20.0;
	double el = 3.14159/4.0;
	Axis xp[6];
	
	GetHexPos(ID, xp);
	printf("Hexapod Postion!!!!\n");
	PrintHexPos(xp);
	printf("Corrected POSTIONS_________________\n");
	uncorrect(xp, el, temp);
	PrintHexPos(xp);
	GetHexPos(ID, xp);
	correct(xp, el, temp);
	printf("The zero postion__________________n\n");
	PrintHexPos(xp);
	xp[XX].pos = 0.0;
	MoveOneAxis(ID, &xp[XX]);
	return 1;

}

void TestGCS(int ID, const char * cmd )
{
	char resp[200];
	
	
	printf( "sending commnad '%s'\n", cmd );
	if ( !GenericCommand(ID, cmd, resp, 200) )
		printf("ERROR!\n");
	else
		printf( "Got Response:\n%s\n", resp );
	
}

void TestError(int ID)
{
	char buff[200];
	int errno;
	errno = GetError(ID, buff, sizeof(buff));
	printf("Error no. is %i and description is %s\n", errno, buff );
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("Wrong number of args!\n");
		exit(-1);
	}
	
	int port = atoi(argv[2]);
	char szIDN[200];
	char szDescription[500];
	int nPIdevices=PI_EnumerateTCPIPDevices( szDescription, 500, "" );
	if(nPIdevices == 0)
	{
		printf("No Devices found\n");
	}
	else
	{
		printf("%i PI devices found: \n%s\n", nPIdevices, szDescription);
	}
	int ID = PI_ConnectTCPIP(argv[1], port);
	Axis xp[6];
	if(PI_qIDN(ID,szIDN,199) == FALSE)
	{
    	printf("qIDN failed. Exiting.\n");
	    return -1;
	}

	printf("ID == %i\n IDN = %s\n", ID, szIDN);
	//TestGCS(ID, "POS?" );
	
	GetHexPos(ID, xp);
	ReferenceIfNeeded(ID, xp);
	//TestGCS(ID, "*IDN?");
	TestError(ID);

}


