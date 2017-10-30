/*
   INDI device driver for the VATT Hexapod (secondary controller)
*/


#include "vatt_secondary.h"

#include <memory>

#define PI_PORTNUM 50000

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

	IUFillText(&HexAddrT[0], "hexip", "Hexapod IP", "128.196.208.218" );
	IUFillTextVector( &HexAddrTV, HexAddrT, 1, getDeviceName(), "hexipvp", "Hexapod IP Addr", "Main Control", IP_RO, 0.5, IPS_IDLE );
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
    IDMessage(getDeviceName(), "Simple device disconnected successfully!");
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
		
		NextPos[XX].pos = values[0];
		
		if( corrS[0].s == ISS_ON)
			correct(NextPos, el,temp);

		MoveOneAxis( ID, &NextPos[XX] );
		
		if( corrS[0].s == ISS_ON)
			uncorrect(NextPos, el,temp);
		
	}
	if( strcmp(name, "PosY" ) == 0 )
	{
		
		NextPos[YY].pos = values[0];	
		if( corrS[0].s == ISS_ON)
			correct(NextPos, el,temp);

		MoveOneAxis( ID, &NextPos[YY] );
		
		if( corrS[0].s == ISS_ON)
			uncorrect(NextPos, el,temp);

	}
	if( strcmp(name, "PosZ" ) == 0 )
	{
		
		NextPos[ZZ].pos = values[0];	
		if( corrS[0].s == ISS_ON)
			correct(NextPos, el,temp);

		MoveOneAxis( ID, &NextPos[ZZ] );
		
		if( corrS[0].s == ISS_ON)
			uncorrect(NextPos, el,temp);

	}
	if( strcmp(name, "PosW" ) == 0 )
	{
		
		NextPos[WW].pos = values[0];	
		if( corrS[0].s == ISS_ON)
			correct(NextPos, el,temp);

		MoveOneAxis( ID, &NextPos[WW] );
		
		if( corrS[0].s == ISS_ON)
			uncorrect(NextPos, el,temp);

	}
	if( strcmp(name, "PosV" ) == 0 )
	{
		
		NextPos[VV].pos = values[0];	
		if( corrS[0].s == ISS_ON)
			correct(NextPos, el,temp);

		MoveOneAxis( ID, &NextPos[VV] );
		
		if( corrS[0].s == ISS_ON)
			uncorrect(NextPos, el,temp);

	}
	if( strcmp(name, "PosU" ) == 0 )
	{
		
		NextPos[UU].pos = values[0];	
		if( corrS[0].s == ISS_ON)
			correct(NextPos, el,temp);

		MoveOneAxis( ID, &NextPos[UU] );
		
		if( corrS[0].s == ISS_ON)
			uncorrect(NextPos, el,temp);

	}
	

	
	for(int ii=0; ii<n; ii++)
	{
		
		IDMessage(getDeviceName(), "dev=%s, name=%s, names[%i]=%s, values[%i]=%f\n", dev, name, ii, names[ii], ii, values[ii] );
		
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
		if(mysvp->sp[0].s == ISS_ON)
			mysvp->s = IPS_OK;
		else
			mysvp->s = IPS_IDLE;
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
	if( strcmp(name, "cmdv") == 0)
	{
		GenericCommand( ID, texts[0], resp, 300 );
		strncpy( cmdT[0].text, resp, 39 );
		IDSetText( &cmdTV, "%s", resp );
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
	const char linposgrp[] = "Linear Position";
	const char rotposgrp[] = "Rotational Position";

			

	IUFillNumber(&PosLatN_X[0] , "X", "X Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosLatNV_X,  PosLatN_X, 1, getDeviceName(), "PosX", "Linear Position X", linposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_X );
	
	IUFillNumber(&PosLatN_Y[0] , "Y", "Y Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosLatNV_Y,  PosLatN_Y, 1, getDeviceName(), "PosY", "Linear Position Y", linposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_Y );

	IUFillNumber(&PosLatN_Z[0] , "Z", "Focus ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosLatNV_Z,  PosLatN_Z, 1, getDeviceName(), "PosZ", "Linear Position Z", linposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_Z );


	/*rotational numbers*/
	IUFillNumber(&PosRotN_W[0] , "W", "Tip X ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosRotNV_W,  PosRotN_W, 1, getDeviceName(), "PosW", "Rotational Position W", rotposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_W );


	IUFillNumber(&PosRotN_V[0] , "V", "Tip Y", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosRotNV_V,  PosRotN_V, 1, getDeviceName(), "PosV", "Rotational Position V", rotposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_V );


	IUFillNumber(&PosRotN_U[0] , "U", "U Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosRotNV_U,  PosRotN_U, 1, getDeviceName(), "PosU", "Rotational Position U", rotposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_U );
	

	//Corrections
	IUFillSwitch( &corrS[0], "correct", "correct", ISS_OFF );
	IUFillSwitchVector( &corrSV, corrS, 1, getDeviceName(), "correct", "correct", "ALL", IP_WO, ISR_NOFMANY, 0.5, IPS_IDLE );
	defineSwitch(&corrSV);

	//Reference the struts
	IUFillSwitch( &refS[0], "ref", "Reference", ISS_OFF );
	IUFillSwitchVector( &refSV, refS, 1, getDeviceName(), "ref", "Reference", "ALL", IP_WO, ISR_NOFMANY, 0.5, IPS_IDLE );
	defineSwitch(&refSV);


	IUFillText( cmdT, "cmd", "Command", "" );
	IUFillTextVector( &cmdTV, cmdT, 1, getDeviceName(), "cmdv", "Command", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineText( &cmdTV );
	//IDSetText( &cmdTV, "I AM FUCKING TRYING TO SET THE TEXT HERE ASSHOLE!!!!!" );


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
	if(ID<0)
		return;

		
	Axis *iter;
	INumberVectorProperty *IAxis;
	int isReady=0;
	int errno;
	char err[300];
	
	
	GetHexPos( ID, Pos );
	if (corrS[0].s == ISS_ON)
	{
		uncorrect(Pos, el, temp);
		
	}
		
	char name[] = "PosX";
	for( iter=Pos; iter!=&Pos[6]; iter++ )
	{
		name[3] = iter->letter[0];
		IAxis = getNumber( name );
		IAxis->np->value = iter->pos;
		IDSetNumber(IAxis, NULL);
	}

	errno = GetError( ID, err, 300 );
	if(errno>0)
		errTV.s = IPS_ALERT;
	else
		errTV.s = IPS_IDLE;

	sprintf( errT[0].text, "%i: %s",errno, err );
	IDSetText( &errTV, NULL );
	SetTimer( (int) 1000 );
	
	SetReadyState();
	SetMoveState();
}

void Secondary::SetReadyState()
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

void Secondary::SetMoveState()
{
	INumberVectorProperty *IAxis;
	
	Axis *iter;
	char name[] = "PosX";
	BOOL isMoving;
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
	
}

int Secondary::GetTempAndEl()
{

	temp = 20.0;
	el = 3.14159/4.0;
}

bool Secondary::MoveNext()
{
	MoveAbs(ID, NextPos );
}