/*
   INDI device driver for the VATT Hexapod (secondary controller)
*/


#include "vatt_secondary.h"

#include <memory>
#include <cmath>
#include "vatttel_com.h"
#include "ngclient.h"
#include <unistd.h>


#define REFRESH  1000 //refresh time in milliseconds


//When to refresh Temerature from vatttel as a multiple of REFRESH time. 
#define MAX_VATTTEL_CNT 120 
#define SLEEP_BTWN_CALLS 1e4 //10 ms

#define PI_PORTNUM 50000
#define MILLI2MICRON 1e3
#define DEG2ASEC 3600.0
#define MAX_COMMERR 0
#define PFILENAME "posfile.dat"
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


//Constructor
Secondary::Secondary()
{
	//negative ID means we haven't connected or 
	//the connection was broken. 
	ID=-1;

	//Create the Zero position
	InitAllAxes(ZeroPos);
	//This should be read froma config file.
	ZeroPos[XX].pos = -1000/MILLI2MICRON;
	ZeroPos[YY].pos = -700/MILLI2MICRON;
	ZeroPos[ZZ].pos = -1750/MILLI2MICRON;


	ZeroPos[VV].pos = 40/DEG2ASEC;
	ZeroPos[UU].pos = -60/DEG2ASEC;
	ZeroPos[WW].pos = 0;
}

Secondary::~Secondary()
{
	PI_CloseConnection(ID);

	IDLog("construct\n");
}



bool Secondary::initProperties()
{
	DefaultDevice::initProperties();

	fill();
	
	//excede the vatttel_counter so we can 
	//update temperature immediately

	return true;
}


bool Secondary::updateProperties()
{	

	if(TimerID != -1)
	{
		RemoveTimer(TimerID);
	}
	TimerID = SetTimer(1000);
	return true;
}


/**************************************************************************************
** Client is asking us to establish connection to the device
***************************************************************************************/
bool Secondary::Connect()
{

	//The below line caused a crash on the emulator.
	setConnected(true, IPS_BUSY, "Attempting to connect");

	//the port number is hardcoded because the 
	//PI C-887 only allows you to talk to it on
	//port 50000.

	char szDescription[500];
	if ( !controllerIsAlive(szDescription) )
	{//Cant find controller on network let user know
		return false;
		setConnected(false, IPS_ALERT);
	}
	if( !ConnectHex( (const char *) szDescription ) )
	{//Cant connect let user know;
		
		return false;
	}
	_GetHexPos( Pos);
	_GetHexPos( NextPos);
	if(!isReferenced(Pos))
	{
		refSV.s = IPS_ALERT;
		IDSetSwitch(&refSV, "You need to reference the secondary");
	}

    return true;
}

/**************************************************************************************
** Client is asking us to terminate connection to the device
***************************************************************************************/
bool Secondary::Disconnect()
{
	DefaultDevice::Disconnect();

	PI_CloseConnection(ID);
	ID=-1;

	//Let gui know correciton is off
	corrS[0].s = ISS_OFF;
	corrSV.s = IPS_IDLE;
	IDSetSwitch(&corrSV, NULL);

	//Let Gui know reference may be wrong
	refS[0].s = ISS_OFF;
	refSV.s = IPS_IDLE;
	IDSetSwitch(&refSV, NULL);
	
	IDMessage(getDeviceName(), "log file is %s", INDI::Logger::getLogFile().c_str());
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
	if( strcmp( name, "PosX" ) == 0 )
	{
		
		NextPos[XX].pos = values[0]/MILLI2MICRON;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );
		
		if( corrS[0].s == ISS_ON )
			_MoveOneAxis(  &CorrNextPos[XX] );
		else
			_MoveOneAxis( &NextPos[XX] );

	}
	if( strcmp(name, "PosY" ) == 0 )
	{
		NextPos[YY].pos = values[0]/MILLI2MICRON;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );
		
		if( corrS[0].s == ISS_ON )
				
				_MoveOneAxis( &CorrNextPos[YY] );
		else
				_MoveOneAxis( &NextPos[YY] );
		
	}
	if( strcmp(name, "PosZ" ) == 0 )
	{
		NextPos[ZZ].pos = values[0]/MILLI2MICRON;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );
		
		if( corrS[0].s == ISS_ON )
				
				_MoveOneAxis( &CorrNextPos[ZZ] );
		else
				
				_MoveOneAxis( &NextPos[ZZ] );
	
	
	}
	if( strcmp(name, "PosW" ) == 0 )
	{
		NextPos[WW].pos = values[0]/DEG2ASEC;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );
		
		if( corrS[0].s == ISS_ON )
				
				_MoveOneAxis( &CorrNextPos[WW] );
		else
				
				_MoveOneAxis( &NextPos[WW] );

	}
	if( strcmp(name, "PosV" ) == 0 )
	{
		NextPos[VV].pos = values[0]/DEG2ASEC;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );

		if( corrS[0].s == ISS_ON )
				
				_MoveOneAxis(  &CorrNextPos[VV] );
		else
				
				_MoveOneAxis(  &NextPos[VV] );

	}
	if( strcmp(name, "PosU" ) == 0 )
	{
		NextPos[UU].pos = values[0]/DEG2ASEC;
		deepcopy( CorrNextPos, NextPos );
		correct( CorrNextPos, el, temp );
		
		if( corrS[0].s == ISS_ON )
				
				_MoveOneAxis(  &CorrNextPos[UU] );
		else
				
				_MoveOneAxis(  &NextPos[UU] );
	}

	if( strcmp(name, "temp") == 0 )
	{
		TempElN[0].value=values[0];
		TempElN[1].value=values[1];
		IDSetNumber(&TempElNV, "Setting Temp or El from user %f %f", values[0], values[1]);
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
		if (ID < 0)
		{
			mysvp->sp[0].s = ISS_OFF;
			IDSetSwitch( mysvp, "Hexapod is not connected please connect." );
			setConnected(false, IPS_BUSY, "");
		}

		else if( mysvp->sp[0].s == ISS_ON )
		{


			/*
			 * Corrections Have Been Switched
			 * First thing we do is align the 
			 * optics by sending each axis
			 * to its nominal zero point.
			 * */

			if ( access(PFILENAME, F_OK) != -1)
			{//If position file exists go to that position
				posfile = fopen(PFILENAME, "r");
				fscanf(posfile, "%lf %lf %lf %lf %lf", 
					&NextPos[XX].pos, 
					&NextPos[YY].pos, 
					&NextPos[ZZ].pos, 
					&NextPos[VV].pos, 
					&NextPos[UU].pos );
			}
			else
			
			{//We don't have a posfile go to zero. 
				IDMessage(getDeviceName(), "No position file moving to zero.");
				deepcopy(NextPos, ZeroPos);
			}

			//Let the user know that 
			//the axes will start moving.
			PosLatNV_X.s=IPS_BUSY;
			PosLatNV_Y.s=IPS_BUSY;
			PosLatNV_Z.s=IPS_BUSY;

			PosRotNV_W.s=IPS_BUSY;
			PosRotNV_V.s=IPS_BUSY;
			PosRotNV_U.s=IPS_BUSY;

			IDSetNumber(&PosLatNV_X, NULL);
			IDSetNumber(&PosLatNV_Y, NULL);
			IDSetNumber(&PosLatNV_Z, NULL);
			IDSetNumber(&PosRotNV_W, NULL);
			IDSetNumber(&PosRotNV_V, NULL);
			IDSetNumber(&PosRotNV_U, NULL);

			//TODO move all axes at once.
			//NextPos[XX].pos = -1000/MILLI2MICRON;
			//
			_MoveOneAxis( &NextPos[XX] );
		
			usleep( SLEEP_BTWN_CALLS );

			//NextPos[YY].pos = -700/MILLI2MICRON;
			_MoveOneAxis( &NextPos[YY] );
			
			usleep(SLEEP_BTWN_CALLS);

			//NextPos[ZZ].pos = -1750/MILLI2MICRON;
			_MoveOneAxis( &NextPos[XX]);

			usleep(SLEEP_BTWN_CALLS);

			//NextPos[VV].pos = 40/DEG2ASEC;
			_MoveOneAxis( &NextPos[VV] );

			usleep(SLEEP_BTWN_CALLS);

			//NextPos[UU].pos = -60/DEG2ASEC;
			_MoveOneAxis( &NextPos[UU]);

			//Get Temp and elevation now
			GetTempAndEl();
			deepcopy(CorrNextPos, NextPos);
			correct( CorrNextPos, el, temp);
			mysvp->s = IPS_OK;
			
			// we just turned on autocollimate lets update
			// temp
			vatttel_counter = MAX_VATTTEL_CNT + 1;
		}

		else
		{
			//Corrections are off so NextPos and 
			//Corrected next pos are the same.
			deepcopy( CorrNextPos, NextPos );
			mysvp->s = IPS_IDLE;
			_GetHexPos( Pos );

			//Remember the Pos of the hexapod 
			//So we can get back there when
			//we run auto collimate next. 
			posfile = fopen( PFILENAME, "w");
			fprintf(posfile, "%lf %lf %lf %lf %lf", 
				NextPos[XX].pos*MILLI2MICRON, 
				NextPos[YY].pos*MILLI2MICRON, 
				NextPos[ZZ].pos*MILLI2MICRON, 
				NextPos[VV].pos*DEG2ASEC, 
				NextPos[UU].pos*DEG2ASEC);
		}

		IDSetSwitch( &corrSV, NULL);
	}
	else if( strcmp("ref", mysvp->name) == 0 )
	{
		if(ID < 0)
		{
			mysvp->s = IPS_IDLE;
			mysvp->sp[0].s=ISS_OFF;
			IDSetSwitch(mysvp, "Can't reference Not connected!");
			setConnected(false, IPS_BUSY);
		
		}
		else
		{
						mysvp->s = IPS_BUSY;
			IDSetSwitch(mysvp, "Referencing Switch");
			ReferenceIfNeeded(ID, Pos);
			mysvp->s = IPS_OK;
			IDSetSwitch(mysvp, NULL);


		}

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
 * Args host=> hostname of the hexapod controller.
 * 	port=> port number the hexapod listens on.
 * 	
 *Descr: Use PI_ConnectTCPIP to connect to the
 *	controller. 
 *
 *
 *
 * **********************************************/

int Secondary::ConnectHex( const char *hostDescription ) 
{
	char szIDN[200];
	IDMessage(getDeviceName(), "Attempting to connect to %s", hostDescription );
	ID = PI_ConnectTCPIPByDescription(hostDescription);
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



/***************************************************
 *Name: controllIsAlive
 * Args: szDescription=>PI identifier string to be
 * 	populated by PI_EnumerateTCPIPDevices
 *
 * Descr: Uses the hard coded serial numbers
 *	from the header file to scan for 
 *	the VATT hexapod PI controller. The serial
 *	Numbers are that for the actual controller
 *	and the PI Hexapod Emulator. It first looks
 *	for the actual hexapod then the emulator.
 *	Once a controller is found it populates
 *	the szDescription argument, which is later
 *	used for the connection. In this way the
 *	only information we need to connect is the 
 *	serial number. 
 *
 *Returns: True on finding the controller otherwise
 * 	False
 *
 * **************************************************/
bool Secondary::controllerIsAlive(char *szDescription)
{
	int nPIdevices;
	bool ignore_serial = true;
	for ( int ii=0; ii<2; ii++ )
	{
		nPIdevices=PI_EnumerateTCPIPDevices( szDescription, 500, serial_numbers[ii] );
		if( nPIdevices == 0 )
			continue;
		else if(nPIdevices == 1)
		{
			IDMessage(getDeviceName(), "Found Device %s", szDescription );
			return true;
		}
		else if(nPIdevices<0)
		{	
			IDMessage(getDeviceName(), "There was an error when scanning for hexapod controller");
			return false;
		}

	}
	
	IDMessage(getDeviceName(), "We could not find the hexapod controller on the network. Check network cables, power, then try a restart.");
	return false;
}




/*******************************************************************
* _MoveOneAxis
* arg: ax=> pointer to single Axis of hexapod
* returns true on success and false on failure
* Description:
*	A simple wrapper for the MoveOneAxis in the libvatthex.so
*
*
* Scott Swindell 2017/11/29
*
*******************************************************************/
bool Secondary::_MoveOneAxis( Axis *ax )
{
	int retn = true;
	if(ID<0)
	{
		IDMessage( getDeviceName(), "Secondary controller not connected!" );
		return false;
	}
	if(!MoveOneAxis(ID, ax))
	{
		IDMessage( getDeviceName(), "Axis %s failed to move to %f", ax->letter, ax->pos );
		isConnected();
		commerr_count++;
		retn=false;
	}
	else 
		commerr_count=0;

	return retn;
}



/*******************************************************************
* _GetHexPos
* arg: hexpos=> pointer to array of hex axes to populate
* returns true on success and false on failure
* Description:
*	A simple wrapper for the GetHexPos in the libvatthex.so
*
* Scott Swindell 2017/11/29
*
*******************************************************************/

bool Secondary::_GetHexPos(Axis *hexpos)
{
	int retn = true;
	if(ID<0)
	{
		IDMessage(getDeviceName(), "Hexapod not connected!");
		return false;
	}
	if( !GetHexPos( ID, hexpos) )
	{
		retn = false;
		IDMessage( getDeviceName(), "Failed to retrieve position" );
		isConnected();
		commerr_count++;
	}
	else
		commerr_count=0;
	return retn;

}

/*************************************************
* isConnected
* returns: boolean weather or not the hex 
*	controller is connected
*
* Description:
*	If the controller is not connected
* we set various INDI switches and states to 
* reflect this unfortunate occurance.
* 
* At first I tried the PI_IsConnected to test
* for the connection. This function seems to
* only tell you if the PI was connected. You 
* unplug the network cable and this lovely function
* will still return true!!
*
*
* Scott Swindell 2017/11/29
*
*************************************************/

bool Secondary::isConnected()
{	
	int isConned = true;
	char szDescription[500];
	if(!DefaultDevice::isConnected())
	{//User hasn't connected
		return false;
	}

	if (!controllerIsAlive(szDescription))
	{//Gui has set connected but connection
	//to the controller has been lost. 
		PI_CloseConnection(ID);
		ID=-1;
		
		//Let gui know correction is off
		corrS[0].s = ISS_OFF;
		IDSetSwitch(&corrSV, NULL);

		//Let Gui know reference maybe wrong
		refS[0].s = ISS_OFF;
		IDSetSwitch(&refSV, NULL);
		
		setConnected(false, IPS_ALERT, "Could not communicate with the Secondary Controller, you may have to restart it.");
		connectionWentBad=true;
		
		isConned=false;

	}
	return isConned;
}

bool Secondary::isReferenced(Axis xp[])
{
	int isRefed;
	if(ID<0)
	{
		IDMessage(getDeviceName(), "You are not connected!");
		return false;
	}
	for(int ii=0; ii<6; ii++)
	{
		if(!PI_qFRF(ID, xp[ii].letter, &isRefed))
		{
			commerr_count++;
			return false;
		}
		if(!isRefed)
			return false;
		
	}
	return true;
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
	const char miscgrp[] = "Miscellaneous";
	const char tegroup[] = "Temp El";
			

	//X axis for corrections is Y axis of PI hexapod
	IUFillNumber(&PosLatN_X[0] , "Y", "X Axis ", "%5.0f", -5.0*MILLI2MICRON, 5.0*MILLI2MICRON, 1, 0);
	IUFillNumberVector( &PosLatNV_X,  PosLatN_X, 1, getDeviceName(), "PosY", "Linear Position Y", linposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_X );
	
	//Y axis for corrections (auto collimation) is X axis of PI Hexapod
	IUFillNumber(&PosLatN_Y[0] , "X", "Y Axis ", "%5.0f", -5.0*MILLI2MICRON, 5.0*MILLI2MICRON, 1, 0);
	IUFillNumberVector( &PosLatNV_Y,  PosLatN_Y, 1, getDeviceName(), "PosX", "Linear Position X", linposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_Y );

	IUFillNumber(&PosLatN_Z[0] , "Z", "Focus ", "%5.0f", -5.0*MILLI2MICRON, 5.0*MILLI2MICRON, 1, 0);
	IUFillNumberVector( &PosLatNV_Z,  PosLatN_Z, 1, getDeviceName(), "PosZ", "Linear Position Z", linposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_Z );


	/*rotational numbers*/
	IUFillNumber(&PosRotN_W[0] , "W", "W Axis ", "%5.0f", -2.5*DEG2ASEC, 2.5*DEG2ASEC, 1, 0);
	IUFillNumberVector( &PosRotNV_W,  PosRotN_W, 1, getDeviceName(), "PosW", "Rotational Position W", rotposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_W );


	IUFillNumber(&PosRotN_V[0] , "V", "Tip X", "%5.0f", -2.5*DEG2ASEC, 2.5*DEG2ASEC, 1, 0);
	IUFillNumberVector( &PosRotNV_V,  PosRotN_V, 1, getDeviceName(), "PosV", "Rotational Position V", rotposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_V );


	IUFillNumber(&PosRotN_U[0] , "U", "Tip Y", "%5.0f", -2.5*DEG2ASEC, 2.5*DEG2ASEC, 1, 0);
	IUFillNumberVector( &PosRotNV_U,  PosRotN_U, 1, getDeviceName(), "PosU", "Rotational Position U", rotposgrp, IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_U );
	
	//These numbers were used for testing purposes. I am going to leave it in for future testing.
	IUFillNumber(&TempElN[0] , "temp", "Strut Temp", "%4.1f", -20, 20, 0.1, 0);
	IUFillNumber(&TempElN[1] , "el", "Elevation", "%4.1f", 0, 90, 0.01, 0);
	IUFillNumberVector( &TempElNV,  TempElN, 2, getDeviceName(), "temp", "The fing temp", tegroup, IP_RW, 0.5, IPS_IDLE );
	//if you want to use them uncomment the below line. 
	defineNumber( &TempElNV );
	IDSetNumber(&TempElNV, NULL );

	//Corrections
	IUFillSwitch( &corrS[0], "correct", "Auto Collimate", ISS_OFF );
	IUFillSwitchVector( &corrSV, corrS, 1, getDeviceName(), "correct", "correct", miscgrp, IP_WO, ISR_NOFMANY, 0.5, IPS_IDLE );
	defineSwitch(&corrSV);

	//Reference the struts
	IUFillSwitch( &refS[0], "ref", "Reference", ISS_OFF );
	IUFillSwitchVector( &refSV, refS, 1, getDeviceName(), "ref", "Reference", miscgrp, IP_WO, ISR_NOFMANY, 0.5, IPS_IDLE );
	defineSwitch(&refSV);


	IUFillText( cmdT, "cmd", "Command", "" );
	IUFillTextVector( &cmdTV, cmdT, 1, getDeviceName(), "cmdv", "Command", miscgrp, IP_RW, 0.5, IPS_IDLE );
	defineText( &cmdTV );


	IUFillText( errT, "err", "Error", "No Error" );
	IUFillTextVector( &errTV, errT, 1, getDeviceName(), "err", "Error", miscgrp, IP_RO, 0.5, IPS_IDLE );
	defineText( &errTV );

	IUFillText(&HexAddrT[0], "hexip", "Hexapod IP", "10.0.3.10" );
	IUFillTextVector( &HexAddrTV, HexAddrT, 1, getDeviceName(), "hexipvp", "Hexapod IP Addr", "Main Control", IP_RW, 0.5, IPS_IDLE );
	defineText(&HexAddrTV);
	IDSetText(&HexAddrTV, NULL);

}



/******************************
* Secondary::TimerHit
* Descritpion:
*	Idle time updates
*	This Includes updating 
*	the values for the 
*	client and doing the 
*	autocollimation routine.
*
********************************/
void Secondary::TimerHit()
{
	char szDescription[500];
	if( ID < 0 )
	{
		//if we get here, it is likely that the hexapod lost connection
		//to the network or lost power while we were connected. 
		if( connectionWentBad && controllerIsAlive(szDescription) )
		{
			//Let the user know we can connect again.
			setConnected(false, IPS_BUSY, "Controller is back up try connecting.");
			connectionWentBad = false;
		}
		
		TimerID = SetTimer( (int) 2000 );
		return;
	}
	
	//We are connected so let the gui know.
	setConnected(true, IPS_OK, NULL);
		
	Axis *iter;
	INumberVectorProperty *IAxis;
	bool isMoving;
	int isReady = 0;
	int errno;
	char err[300];
	double unitconversion;
	

	_GetHexPos(  Pos );
	deepcopy( CorrPos, Pos );

	isMoving = SetMoveState();
	isReady = SetReadyState();
	if (isReady)
	{
		//refSV.s = IPS_OK;
		refS[0].s = ISS_OFF;
	}
	else
	{
		IDMessage(getDeviceName(), "The hexapod is not ready, it may need to be restarted");
		refSV.s = IPS_ALERT;
	}
	IDSetSwitch(&refSV, NULL);
	
		
	if ( corrS[0].s == ISS_ON )
	{
		corrSV.s = IPS_OK;
		GetTempAndEl();


		//******Active Auto Collimation stuff********
		//if we are not moving and the difference
		//between the next pos and the corrected next
		//pos is high enough (because el or temp changed) 
		//update the position
		if(!isMoving)
		{
			deepcopy(CorrNextPos, NextPos);
			correct(CorrNextPos, el, temp);
			for(int ii=0; ii<6; ii++)
			{
			
				//we have veered too far from the correct position... lets move
				if( fabs( CorrNextPos[ii].pos - CorrPos[ii].pos ) > 0.0001 )
				{
					_MoveOneAxis( &CorrNextPos[ii] );

					usleep(SLEEP_BTWN_CALLS);

				}
			
			}
		}
		

		//remove correction for display purpose.
		uncorrect( Pos, el, temp );

	}
	else
	{
		corrSV.s = IPS_IDLE;
	}
	IDSetSwitch(&corrSV, NULL);

	/*
	*
	* The below loop is how we match the INDI IAxis	
	* type to the vatthex.so Axis type. Basically
	* I use the last character of the name of the 
	* IAxis (X, Y etc) to match the letter member
	* of the Axis structure. To make this even 
	* more confusing the X and Y axes are reversed.
	*	
	*/
	
	char name[] = "PosX";
	for( iter=Pos; iter!=&Pos[6]; iter++ )
	{
		
		if ( iter->letter[0] == 'X' || iter->letter[0] == 'Y' || iter->letter[0] == 'Z' )
			unitconversion = MILLI2MICRON;
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


	/**********************************************************************
	* TLDR; desperate last measure to see if we are not connected to the Hexapod.
	*
	* It seems likely that the controllerIsAlive
	* method could return true even when the GCS connection
	* is dead. PI uses a TCP port 50000 for GCS commands
	* but the controllerIsAlive uses PI_EnumerateTCPIPDevices
	* to see if we can find the secondary controller/hexapod
	* on the network. I would guess that PI_EnumerateTCPIPDevices
	* uses some sort of multicast packet to see if the hexapod
	* is on the network. All this to say that the GCS connection
	* could have gone bad while, the broadcast packet happily
	* gets a "I am here" response. If this is the case
	* we use the number of times we missed a GCS response
	* to let the user know that we are no longer communicating
	* with the hexapod. This logic is done in the folowing if
	* statement.
	**********************************************************************/

	if(commerr_count > MAX_COMMERR)
	{
		PI_CloseConnection(ID);
		ID=-1;
		
		//Let gui know correciton is off
		corrS[0].s = ISS_OFF;
		IDSetSwitch(&corrSV, NULL);

		//Let Gui know reference maybe wrong
		refS[0].s = ISS_OFF;
		IDSetSwitch(&refSV, NULL);
		
		setConnected(false, IPS_ALERT, "Could not communicate with the Secondary Controller. Too many comm errors, you may have to restart it.");
		connectionWentBad=true;
	
	}
	//End desperate attempt.

	//Do it again in 1 second.
	TimerID = SetTimer( (int) 1000 );
	
	
}


/********************************************
* SetReadyState
* returns: boolean if hexapod is ready
*
* Description:
*
* Checks to see if the controller is ready 
* to move if not alerts the user with the
* INDI state propert. 
*
*
*
*
********************************************/

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
		if(!isReady)
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
			IAxis->s = IPS_OK;
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


/**********************************************
* GetTempAndEl
*	Retrieves the temperature and elevation
*	from vatttel.
*
*
*
* Scott Swindell 10/2017
*
**********************************************/

int Secondary::GetTempAndEl()
{

	//get Elevation from NG server on vatttel
	char rqbuff[200];


	//fake it for now 
	//ng_request( (char *) "ALL ", rqbuff );
	strcpy( rqbuff, "VATT TCS 123 12:34:56 +78:91:12  23:34:45 90.0, 85.0  ...  "  );

	bool gettingTemp = false;
	
	std::string rqstr = rqbuff;
	char elerr[100];
	char temperr[100];

	double dummy_temp;
	double dummy_el;
	bool badread = true;

	
	size_t str_begin = rqstr.find( " ", 40 );
	dummy_el = std::stof( rqstr.substr( str_begin+1,4 ) )*3.14159/180.0;
	

	//only grab the temp every 120 seconds
	vatttel_counter++;
	if( vatttel_counter > MAX_VATTTEL_CNT )
	{
		vatttel_counter=0;
		gettingTemp=true;
		dummy_temp = GetStrutTemp( temperr );
		IDMessage( getDeviceName(), "I just grabbed temp");
	}
	else
	{
		dummy_temp = TempElN[0].value;
	}
	
	IDMessage(getDeviceName(), "dummy_temp is %f", dummy_temp);
	//if the read temp is reasonable
	if( dummy_temp > -17.0 && dummy_temp < 17.0 )
	{
		if (gettingTemp)
		{
			temp = dummy_temp;
			badread = false;

			// it was a good read 
			// so let the user know
			// by updating temp
			TempElN[0].value = temp;
			IDSetNumber( &TempElNV, NULL );
		}
	}

	//if not reasonable
	else
	{//use the users temperature if its reasonable
		if(TempElN[0].value > -17.0 && TempElN[0].value < 17.0)
			temp = TempElN[0].value;
	}

	if( dummy_el >0 && dummy_el < 3.14159/2.0 + 0.1*3.14159/180 )
	{//if read_el is resonable
		//update el for autocollimation and user
		el = dummy_el;
		TempElN[1].value=el*180/3.14159;
		IDSetNumber(&TempElNV, NULL);
		

		badread = false;
	}
	else
	{
		//read the old value or 
		// the user input. 
		el = TempElN[1].value;
	}

	//el = TempElN[1].value*3.14159/180.0;
	if( badread )
		IDMessage(getDeviceName(), "Erroneous temp or elevation read temp=%f el=%f", dummy_temp, dummy_el );
	
}


bool Secondary::MoveNext()
{
	MoveAbs(ID, NextPos );
}
