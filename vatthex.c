#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vatthex.h"
#ifndef cpp
#define false 0
#define true 1
#define MICRONS2MM 1/1000.0
#endif


/*A convenience wrapper for the PI_GCS2 stuff.

	Here we use an array of structs to access
	each axis rather than a single character
	string.
*/


/* -------The following comments and code are from -----
   -------From secmove.c in the vatt tcs teld dir. -------
   -------You can read more information about this from---
https://lavinia.as.arizona.edu/~tscopewiki/doku.php?id=vatt:legacy_psuedo-hexpod

 * The following matrix of (T,L) corrections (across)
 * {focus_delta_temperature,       focus_delta_sin_elevation,
 *  collimation_delta_temperature, collimation_delta_sin_elevation}
 * to user motion corrections (down)
 *
 * These were derived from Cromwell's original (June 10, 1996) coefficients
 * for ELCdv and TCdv by multiplying by v2m.
 * 0.2230000    0.0914000   -0.1024000   -0.0426000    0.0031000    0.1702000
 * 0.2148000    0.5129000    0.0363000    0.1717000   -0.0712000    0.4409000
 */

#define C       4       /* number of corrections */
#define FTC     0		//focus temp correction
#define FLC     1		//focus elevation correction
#define CTC     2		//collimation temp correction
#define CLC     3		//collimation elevation correction

#define V       6       /* number of motors/sensors */
#define M       6       /* number of user motions */
#define TIPX    0       /* arcseconds */
#define TIPY    1
#define TIPZ    2
#define DECENX  3       /* microns */
#define DECENY  4
#define FOCUS   5


double sec_c2m[M][C] = {
//    FTC,   FLC,  CTC , CLC
	{ 0.0  , 0.0  , 0.0 , 141.9   , }, //DECENX == X Microns
	{ 0.0  , 0.0  , 0.0 , 0.0     , }, //DECENY == Y Microns
	{ 22.6 , 57.0 , 0.0 , 0.0     , }, //DECENZ == Z (Focus)
	{ 0.0  , 0.0  , 0.0 , 0.0     , }, //TIPX == W (Arcseconds)
	{ 0.0  , 0.0  , 0.0 , 0.0     , }, //TIPY == V
	{ 0.0  , 0.0  , 0.0 , 0.0     , }, //TIPZ == U
};

double sec_z[M] = {
	0.0 ,  //X
	0.0 ,  //Y
	57.0,  //Z (Focus)
	0.0 ,  //W
	0.0 ,  //V
	0.0 ,  //U
};

//----------End secmove.c stuff------------------------------


//Global vars for convenience
const char * AXIS_NAMES[] = {"X", "Y", "Z", "W", "V", "U"};
const int NAXES = sizeof(AXIS_NAMES)/sizeof(AXIS_NAMES[0]);


/*********************************
* sec_update_corr
*
* Copied from secmove.c in from the
* the vatt tcs that runs on vatttel
* 
*
*
void sec_update_corr ()
{
        int     m, c;
        double  sc;
        double  temp, cosel;
        double d[C];
        double cos();

        sec_update_tl ();
        temp = sec_temperature;
        cosel = cos(sec_elevation);

        //sanity checks
        
        if (sec_min_temp < temp && temp < sec_max_temp) {
            sec_lastgood_temp = temp;
        } else {
            temp = sec_lastgood_temp;
        }
        ifif (sec_min_cosel < cosel && cosel
            sec_lastgood_coselsec_lastgood_cosel = ;
        } else {
            cosel = ;
        }

        d[FTC] = sec_autofocus ? temp : 0.0;
        d[FLC] = sec_autofocus ? cosel : 0.0;
        d[CTC] = sec_autocoll ? temp : 0.0;
        d[CLC] = sec_autocoll ? cosel : 0.0;

        for (m = 0; m < M; m++) {
            sc = 0.0;
            for (c = 0; c < C; c++) {
                sc += sec_c2m[m][c] * d[c];
            }
            sec_c[m] = sc;
        }
}
*/


int GetError(int ID, char errBuff[], int buffSize )
{
	//Error instructions on pg 250
	// of C_887 hexapod Manual
	int errno;
			

	errno = PI_GetError(ID);
	PI_TranslateError( errno, errBuff, buffSize );

	return errno;

}


/**********************************
*correct
* Adds corections due to strut 
* temp and the elevation of the 
* telescope. 
*
*
*
**********************************/

BOOL correct( Axis xp[], double el, double temp )
{
	double eltemp[4] = { temp, cos(el), temp, cos(el) };

	for(int ii=0; ii<6; ii++)
	{

		for(int cc=0; cc<4; cc++)
		{
			xp[ii].pos += sec_c2m[ii][cc]*eltemp[cc]*MICRONS2MM;
		}
	}
	return true;

}



BOOL uncorrect( Axis xp[], double el, double temp )
{
	double eltemp[4] = { temp, cos(el), temp, cos(el) };
	for(int ii=0; ii<6; ii++)
	{
		for(int cc=0; cc<4; cc++)
		{
			xp[ii].pos -= sec_c2m[ii][cc]*eltemp[cc]*MICRONS2MM;
      printf("%s", xp[ii]);
		}
	}
	return true;
}

/************************************
 *  ReferenceIfNeeded
 *	args: ID == PI communcatiion id
 *		axis == letter of axis to reference
 * 	ID->PI identifier, 
 *	xp-> Axis struct to determine which axis to move
 *  dval-> position to move the axis to in MM
 * Descr: move one axis using the Axis struct
 *
 *
 *
 *
 ***************************************/
BOOL ReferenceIfNeeded(int ID, Axis *xp)
{
    BOOL bReferenced;
    BOOL bFlag;
    if(!PI_qFRF(ID, xp->letter, &bReferenced))
        return false;
    if(!bReferenced)
    {// if needed,
        // reference the axis using the refence switch
        printf("Referencing axis %s...\n",xp->letter);
        if(!PI_FRF(ID, xp->letter))
            return false;

        // Wait until the reference move is done.
        bFlag = false;
        while(bFlag != TRUE)
        {
            if(!PI_IsControllerReady(ID, &bFlag))
                return false;
        }
    }
    return true;
}

BOOL MoveOneAxis(int ID, Axis *xp )
{
    printf("Moving axis %s to %f...\n",xp->letter, xp->pos);

    if(!PI_MOV(ID, xp->letter, &xp->pos))
        return false;
    // Wait until the closed loop move is done.
    return true;
}

BOOL MoveAbs(int ID, Axis *next )
{
	Axis *iter_next;
	Axis *iter_curr;
	Axis current[6];
	double offset;
	double thresh = 0.001;
	BOOL test = true;
	GetHexPos( ID, current );

	//Move then
	for (iter_next=next; iter_next!=next+NAXES; iter_next++)
	{
		if( !PI_MOV(ID, iter_next->letter, &iter_next->pos ) )
		{
			printf("Move not working!!\n");
			return false;
		}
	}
	while(test)
	{
		test = false;
		for(int ii=0; ii<NAXES; ii++)
		{
			iter_next = next + ii;
			iter_curr = current + ii;
			offset = iter_next->pos-iter_curr->pos;

			printf("%s:%f\t", iter_next->letter, offset);
			if( fabs(offset) > thresh )
				test = true;
		}
		printf("\n");
		GetHexPos(ID, current);

	}
	return 0;
}


int GenericCommand(int ID, const char* cmd, char resp[], int respSize)
{

	char temp[200];
	int  iAnswerSize = 0;
	strcpy(resp, "");
	if( !PI_GcsCommandset( ID, cmd )  ) 
	{
		printf( "Could not send command!\n" );
		return 0;
	}
	

	while( iAnswerSize == 0 )
	{
		PI_GcsGetAnswerSize( ID, &iAnswerSize );
		//check again in a quarter of a second
		usleep(250000);
	}

	while( 1 )
	{
		
		PI_GcsGetAnswerSize( ID, &iAnswerSize );
		if(iAnswerSize == 0)
			break;
		
		if( !PI_GcsGetAnswer(ID, temp, iAnswerSize ) )
		{
			break;
			
		}
		else
		{
			
			strncat(resp, temp, strlen(temp) );
		}
	}

	return 1;
}


/********************************
*GetHexPos
*Args: ID => identifier for PI communcation
*	  xp => pointer to Axis array
* Descr: Populates the hexpos struct 
*
*
*
*	with positions
* Author: Scott Swindell
********************************/
BOOL GetHexPos( int ID, Axis *xp )
{
	for(int ii=0; ii<NAXES; ii++)
	{
		xp->letter = (char *)AXIS_NAMES[ii];
		
		if( !PI_qPOS(ID, xp->letter, &xp->pos ) )
		{
			printf("Failing at letter %s\n", xp->letter);
			return false;
		}
		usleep(1e4);
		xp++;
	}

	return true;
}


/******************************************
*	InitAllAxes
*	arg: xp ==  Axis pointer to be populated
*
*	Descr: Initiialize all axes. 
*	So the array is ready to be populated
*	
*	
***************************************/
void InitAllAxes(Axis *xp)
{
	for(int ii=0; ii<NAXES; ii++)
	{
		xp->letter = (char *)AXIS_NAMES[ii];
		
		xp++;
	}

}

/******************************************
*	InitAxis
*	arg: xp ==  Axis pointer to be populated
*		ii == enum associated with axis letter
*		like XX, YY, ZZ, UU etc. 
*	Descr: Initiialize a single axis. 
*	If you don't want to deal with 
*	the whole array and jus tplay with one
*	
***************************************/
BOOL InitAxis(Axis *xp, axis_ii ii)
{
	xp->letter = (char *) AXIS_NAMES[ii];
	xp->ii = ii;

}


/*********************************
*	PrintHexPos
*	Args: xp == Array of axis postions
*	Description: Print the axis names
*and positions. 
*
*
*
*************************************/
void PrintHexPos( Axis *xp )
{
	Axis *iter;
	for(iter=xp; iter!=xp+NAXES; iter++)
	{
		printf( "%s = %f\n", iter->letter, iter->pos );
	}
}

void usage(char exe[])
{
	printf("Does a closed loop move of the z-axis of the hexapod\n");
	printf("zpos must be an float less greater than -1 and less than 1\n");
	printf("Usage: %s <port_number> <zpos>\n", exe);
	printf("Like: %s 5200 0.5\n", exe);
}

/*
int main(int argc, char * argv[])
{
	int port;
	double zpos;
	if (argc != 3)
	{
				usage(argv[0]);
				exit(-1);
	}
	else
	{
		port = atoi(argv[1]);
		zpos = atof(argv[2]);
		
		if(zpos > 0.99 || zpos < -0.99)
		{
			usage(argv[0]);
			exit(-1);
		}
			
	}
	const char *szAddressToConnect = "localhost";
	int ID = PI_ConnectTCPIP(szAddressToConnect,port);
	printf("iD is %d\n", ID);
	char szIDN[200];
	char szPOS[200];
	double dPos;
	Axis Pos[NAXES];
	Axis nPos[NAXES];


	
    if(PI_qIDN(ID,szIDN,199) == FALSE)
    {
        printf("qIDN failed. Exiting.\n");
        return FALSE;
    }
	else
		printf("the qIDN %s\n", szIDN);
	
	//Populate the position array
	if ( !GetHexPos( ID, Pos ) )
		return -1;	
	for(Axis *iter=Pos; iter!=Pos+NAXES; iter++)
	{
		printf("%s = %f\n", iter->letter, iter->pos );
	}
	
	// Reference Z axis
	if(ReferenceIfNeeded(ID, Pos[ZZ].letter) == false)
    {
        printf( "Not referenced, Referencing failed.\n" );
        return FALSE;
    }
	
	//move z axis
	printf("Trying to move %s axis\n", Pos[ZZ].letter);
	Pos[ZZ].pos = zpos;
	MoveAbs(ID, Pos );
	//MoveOneAxis( ID, Pos+ZZ, zpos );
	PrintHexPos(Pos);
	return 0;
	

}
*/
