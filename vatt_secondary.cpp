/*
   INDI device driver for the VATT Hexapod (secondary controller)
*/


#include "vatt_secondary.h"

#include <memory>
#include <cmath>
#include "vatttel_com.h"
#define PI_PORTNUM 50000
#define MILI2MICRON 1e3
#define DEG2ASEC 3600.0

std::unique_ptr<Secondary> secondary(new Secondary());


/**************************************************************************************
** Return properties of device.
***************************************************************************************/
void ISGetProperties(const char *dev)
{
    secondary->ISGetProperties(dev);
}

/**************************************************************************************
** Process new switch from client
***************************************************************************************/
void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    secondary->ISNewSwitch(dev, name, states, names, n);
}

/**************************************************************************************
** Process new text from client
***************************************************************************************/
void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    secondary->ISNewText(dev, name, texts, names, n);
}

/**************************************************************************************
** Process new number from client
***************************************************************************************/
void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    secondary->ISNewNumber(dev, name, values, names, n);
}

/**************************************************************************************
** Process new blob from client
***************************************************************************************/
void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[],
               char *names[], int n)
{
    secondary->ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

/**************************************************************************************
** Process snooped property from another driver
***************************************************************************************/
void ISSnoopDevice(XMLEle *root)
{
    INDI_UNUSED(root);
}



Secondary::Secondary()
{
	ID=-1;
	IDLog("construct\n");
}

Secondary::~Secondary()
{
	PI_CloseConnection(ID);

	IDLog("construct\n");
}
bool Secondary::initProperties()
{
	DefaultDevice::initProperties();

	IUFillText(&HexAddrT[0], "hexip", "Hexapod IP", "10.0.3.10" );
	IUFillTextVector( &HexAddrTV, HexAddrT, 1, getDeviceName(), "hexipvp", "Hexapod IP Addr", "Main Control", IP_RW, 0.5, IPS_IDLE );
	defineText(&HexAddrTV);
	IDSetText(&HexAddrTV, "should show the IP");

	return true;
}


bool Secondary::updateProperties()
{	

	fill();
	SetTimer(1000);
	return true;
}


/**************************************************************************************
** Client is asking us to establish connection to the device
***************************************************************************************/
bool Secondary::Connect()
{


	//the port number is hardcoded because the 
	//PI C-887 only allows you to talk to it on
	//port 50000. 
	if(!ConnectHex(HexAddrT[0].text, PI_PORTNUM))
		return false;
	GetHexPos(ID, Pos);
	GetHexPos(ID, NextPos);

    return true;
}

/**************************************************************************************
** Client is asking us to terminate connection to the device
***************************************************************************************/
bool Secondary::Disconnect()
{
		
	PI_CloseConnection(ID);
    return true;
}

/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char *Secondary::getDefaultName()
{
    return "VATT Secondary";
}


bool Secondary::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
	INumberVectorProperty * myv = getNumber(name);
	if( strcmp(name, "PosX" ) == 0 )
	{
		
		NextPos[XX].pos = values[0]/MILI2MICRON;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );
		
		if( corrS[0].s == ISS_ON )
				
				MoveOneAxis( ID, &CorrNextPos[XX] );
		else
				
				MoveOneAxis( ID, &NextPos[XX] );
				

		
		
	}
	if( strcmp(name, "PosY" ) == 0 )
	{
		NextPos[YY].pos = values[0]/MILI2MICRON;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );
		
		if( corrS[0].s == ISS_ON )
				
				MoveOneAxis( ID, &CorrNextPos[YY] );
		else
				
				MoveOneAxis( ID, &NextPos[YY] );
	
		
	}
	if( strcmp(name, "PosZ" ) == 0 )
	{
		NextPos[ZZ].pos = values[0]/MILI2MICRON;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );
		
		if( corrS[0].s == ISS_ON )
				
				MoveOneAxis( ID, &CorrNextPos[ZZ] );
		else
				
				MoveOneAxis( ID, &NextPos[ZZ] );
	
	
	}
	if( strcmp(name, "PosW" ) == 0 )
	{
		NextPos[WW].pos = values[0]/DEG2ASEC;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );
		
		if( corrS[0].s == ISS_ON )
				
				MoveOneAxis( ID, &CorrNextPos[WW] );
		else
				
				MoveOneAxis( ID, &NextPos[WW] );

		
	}
	if( strcmp(name, "PosV" ) == 0 )
	{
		NextPos[VV].pos = values[0]/DEG2ASEC;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );
		
		if( corrS[0].s == ISS_ON )
				
				MoveOneAxis( ID, &CorrNextPos[VV] );
		else
				
				MoveOneAxis( ID, &NextPos[VV] );

	
	}
	if( strcmp(name, "PosU" ) == 0 )
	{
		NextPos[UU].pos = values[0]/DEG2ASEC;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );
		
		if( corrS[0].s == ISS_ON )
				
				MoveOneAxis( ID, &CorrNextPos[UU] );
		else
				
				MoveOneAxis( ID, &NextPos[UU] );
	}
	

	
	for(int ii=0; ii<n; ii++)
	{
		
		//IDMessage(getDeviceName(), "dev=%s, name=%s, names[%i]=%s, values[%i]=%f\n", dev, name, ii, names[ii], ii, values[ii] );
		
			}
	IUUpdateNumber(myv, values, names, n);
	return true;

}


/*****************************
* Method: Secondary::ISNewSwitch 
*
*
*
*
*
*
*******************************/
bool Secondary::ISNewSwitch(const char *dev, const char * name, ISState *states, char *names[], int n)
{
	DefaultDevice::ISNewSwitch(dev, name, states, names, n);
	ISwitchVectorProperty *mysvp;
	mysvp = getSwitch(name);

	IUUpdateSwitch(mysvp, states, names, n);
	if( strcmp("correct", mysvp->name) == 0 )
	{
		deepcopy(CorrNextPos, NextPos);
		if( mysvp->sp[0].s == ISS_ON )
		{
			correct( CorrNextPos, el, temp);
			mysvp->s = IPS_OK;
		}
		else
		{
			mysvp->s = IPS_IDLE;
			GetHexPos( ID, Pos );
		}
		IDSetSwitch( &corrSV, NULL);
	}
	else if( strcmp("ref", mysvp->name) == 0 )
	{
		mysvp->s = IPS_BUSY;
		IDSetSwitch(mysvp, NULL);
		ReferenceIfNeeded(ID, Pos);
		mysvp->s = IPS_OK;
		IDSetSwitch(mysvp, NULL);

	}
}

bool Secondary::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
	char resp[300];


	if( strcmp( name, "cmdv" ) == 0 )
	{
		GenericCommand( ID, texts[0], resp, 300 );
		strncpy( cmdT[0].text, resp, 39 );
		IDSetText( &cmdTV, "%s", resp );
	}

	if( strcmp( name, "hexipvp" ) == 0 ) 
	{
		IDMessage( getDeviceName(), "Setting ip address %s", texts[0] );
		strcpy( HexAddrT[0].text, texts[0] );
		IDSetText( &HexAddrTV, NULL );
	}
}

/************************************************
 * ConnectHex
 * Args
 *
 *
 *
 *
 *
 *
 * **********************************************/
int Secondary::ConnectHex( const char *host="localhost", int port=5200 ) 
{
	char szIDN[200];
	IDMessage(getDeviceName(), "Attempting to connect to %s, %i", host, port);
	ID = PI_ConnectTCPIP(host,port);
	if(ID<0)
	{
		IDMessage(getDeviceName(), "Hex not connected %i", ID);
		return false;
	}
	
	if(PI_qIDN(ID,szIDN,199) == FALSE)
	{
    		IDMessage( getDeviceName(),  "qIDN failed. Exiting." );
		return false;
	}
	else
	{
		IDMessage(getDeviceName(), "Connected to hexapod ID=%i", ID);
		
		IDMessage(getDeviceName(), "%s", szIDN);
		return true;
	}
}

bool Secondary::ReadHex()
{
	if(ID<0)
	{
		IDMessage(getDeviceName(), "Hexapod not connected!");
		return false;
	}
	GetHexPos(ID, Pos);

}

/**************************************************
 * fill
 * Description:
 * 	This is where all the indi properties 
 * 	are initialized. 
 *
 *
 *
 *
 *
 * ***********************************************/

bool Secondary::fill()
{
	/*Translational numbers*/
	const char linposgrp[] = "Decenter";
	const char rotposgrp[] = "Tip Tilt";

			

	//X axis for corrections is Y axis of PI hexapod
	IUFillNumber(&PosLatN_X[0] , "Y", "X Axis ", "%5.0f", -5.0*MILI2MICRON, 5.0*MILI2MICRON, 1, 0);
	IUFillNumberVector( &PosLatNV_X,  PosLatN_X, 1, getDeviceName(), "PosY", "Linear Position Y", linposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_X );
	
	//Y axis for corrections (auto collimation) is X axis of PI Hexapod
	IUFillNumber(&PosLatN_Y[0] , "X", "Y Axis ", "%5.0f", -5.0*MILI2MICRON, 5.0*MILI2MICRON, 1, 0);
	IUFillNumberVector( &PosLatNV_Y,  PosLatN_Y, 1, getDeviceName(), "PosX", "Linear Position X", linposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_Y );

	IUFillNumber(&PosLatN_Z[0] , "Z", "Focus ", "%5.0f", -5.0*MILI2MICRON, 5.0*MILI2MICRON, 1, 0);
	IUFillNumberVector( &PosLatNV_Z,  PosLatN_Z, 1, getDeviceName(), "PosZ", "Linear Position Z", linposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_Z );


	/*rotational numbers*/
	IUFillNumber(&PosRotN_W[0] , "W", "W Axis ", "%5.0f", -2.5*DEG2ASEC, 2.5*DEG2ASEC, 1, 0);
	IUFillNumberVector( &PosRotNV_W,  PosRotN_W, 1, getDeviceName(), "PosW", "Rotational Position W", rotposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_W );


	IUFillNumber(&PosRotN_V[0] , "V", "Tip Y", "%5.0f", -2.5*DEG2ASEC, 2.5*DEG2ASEC, 1, 0);
	IUFillNumberVector( &PosRotNV_V,  PosRotN_V, 1, getDeviceName(), "PosV", "Rotational Position V", rotposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_V );


	IUFillNumber(&PosRotN_U[0] , "U", "Tip X", "%5.0f", -2.5*DEG2ASEC, 2.5*DEG2ASEC, 1, 0);
	IUFillNumberVector( &PosRotNV_U,  PosRotN_U, 1, getDeviceName(), "PosU", "Rotational Position U", rotposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_U );
	

	//Corrections
	IUFillSwitch( &corrS[0], "correct", "Auto Collimate", ISS_OFF );
	IUFillSwitchVector( &corrSV, corrS, 1, getDeviceName(), "correct", "correct", "ALL", IP_WO, ISR_NOFMANY, 0.5, IPS_IDLE );
	defineSwitch(&corrSV);

	//Reference the struts
	IUFillSwitch( &refS[0], "ref", "Reference", ISS_OFF );
	IUFillSwitchVector( &refSV, refS, 1, getDeviceName(), "ref", "Reference", "ALL", IP_WO, ISR_NOFMANY, 0.5, IPS_IDLE );
	defineSwitch(&refSV);


	IUFillText( cmdT, "cmd", "Command", "" );
	IUFillTextVector( &cmdTV, cmdT, 1, getDeviceName(), "cmdv", "Command", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineText( &cmdTV );


	IUFillText( errT, "err", "Error", "No Error" );
	IUFillTextVector( &errTV, errT, 1, getDeviceName(), "err", "Error", "ALL", IP_RO, 0.5, IPS_IDLE );
	defineText( &errTV );


}



/******************************
* Secondary::TimerHit
* Descritpion:
*	Idle time updates
*
*
*
*
*
*
*
*
********************************/
void Secondary::TimerHit()
{
	if( ID < 0 )
		return;

		
	Axis *iter;
	Axis ActualPos[6]; //The real position of the hexapod without corrections
	INumberVectorProperty *IAxis;
	bool isMoving;
	int isReady = 0;
	int errno;
	char err[300];
	double unitconversion;
	

	GetHexPos( ID, Pos );
	deepcopy(CorrPos, Pos);

	isMoving = SetMoveState();
	isReady = SetReadyState();
	
	if(vatttel_counter == 5)
	{
		GetTempAndEl();
		vatttel_counter = 0;
	}
	else
		vatttel_counter++;
	
	if ( corrS[0].s == ISS_ON )
	{
		//remove correction for display purpose.
		uncorrect(Pos, el, temp);

		//if we are not moving and the difference
		//between the next pos and the corrected next
		//pos is high enough (because el or temp changed) 
		//update the position
		if(!isMoving)
		{
				
			for(int ii=0; ii<6; ii++)
			{
				if( fabs( CorrNextPos[ii].pos - NextPos[ii].pos ) > 0.001 )
				{
					MoveOneAxis(ID, &CorrNextPos[ii] );
				}
			}
		}


	}
	

	/**********************************************
	* The below loop is how we match the INDI IAxis	
	* type to the vatthex.so Axis type. Basically
	* I use the last character of the name of the 
	* IAxis (X, Y etc) to macthe the letter member
	* of the Axis structure. To make this even 
	* more confusing the X and Y axes are reversed.
	*	
	*	
	***********************************************/
	
	char name[] = "PosX";
	for( iter=Pos; iter!=&Pos[6]; iter++ )
	{
		
		if ( iter->letter[0] == 'X' || iter->letter[0] == 'Y' || iter->letter[0] == 'Z' )
			unitconversion = MILI2MICRON;
		else
			unitconversion = DEG2ASEC;

		name[3] = iter->letter[0];
		
		IAxis = getNumber( name );
		IAxis->np->value = iter->pos*unitconversion;
		IDSetNumber( IAxis, NULL );
	}

	errno = GetError( ID, err, 300 );
	if(errno>0)
		errTV.s = IPS_ALERT;
	else
		errTV.s = IPS_IDLE;

	sprintf( errT[0].text, "%i: %s",errno, err );
	IDSetText( &errTV, NULL );
	SetTimer( (int) 1000 );
	
}




bool Secondary::SetReadyState()
{
	INumberVectorProperty *IAxis;
	
	Axis *iter;
	char name[] = "PosX";
	int isReady;
	if(  !PI_IsControllerReady(ID, &isReady) )
	 isReady = 0;
	
	for( iter=Pos; iter!=&Pos[6]; iter++ )
	{
		name[3] = iter->letter[0];
		IAxis = getNumber( name );
		if(isReady)
			IAxis->s = IPS_IDLE;
		else
			IAxis->s = IPS_BUSY;
		IDSetNumber(IAxis, NULL);
		
	}
	return (bool) isReady;	
}

/****************************
 * SetMoveState
 *
 * Sets the state of the positions
 * hexapod axes. If its moving
 * ts IPS_BUSY, if not IPS_IDLE
 * which show up as Yelow and 
 * transparent respectively in the gui
 *
 *
 *
 * ****************************/

bool Secondary::SetMoveState()
{
	INumberVectorProperty *IAxis;
	
	Axis *iter;
	char name[] = "PosX";
	int isMoving;
	for( iter=Pos; iter!=&Pos[6]; iter++ )
	{
		name[3] = iter->letter[0];
		PI_IsMoving(ID, iter->letter, &isMoving );
		IAxis = getNumber( name );
		if(isMoving)
			IAxis->s = IPS_BUSY;
		else
			IAxis->s = IPS_IDLE;
		IDSetNumber(IAxis, NULL);
		
	}
	
	return (bool) isMoving;
}

void Secondary::deepcopy(Axis *copyto, Axis *copyfrom)
{
		for(int ii=0; ii<6; ii++)
		{

			copyto[ii].letter = copyfrom[ii].letter;
			copyto[ii].ii = copyfrom[ii].ii;
			copyto[ii].pos = copyfrom[ii].pos;

		}
}

int Secondary::GetTempAndEl()
{

	char elerr[100];
	char temperr[100];
	temp = GetStrutTemp(temperr);
	el = GetAlt(elerr)*3.14159/180.0;
	//IDMessage(getDeviceName(), "the temp err is %f and the el is %f", temp, el*180/3.14159 );
}

bool Secondary::MoveNext()
{
	MoveAbs(ID, NextPos );
}
