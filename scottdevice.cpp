/*
   INDI Developers Manual
   Tutorial #1

   "Hello INDI"

   We construct a most basic (and useless) device driver to illustrate INDI.

   Refer to README, which contains instruction on how to build this driver, and use it
   with an INDI-compatible client.

*/

/** \file simpledevice.cpp
    \brief Construct a basic INDI device with only one property to connect and disconnect.
    \author Jasem Mutlaq

    \example simpledevice.cpp
    A very minimal device! It also allows you to connect/disconnect and performs no other functions.
*/

#include "simpledevice.h"

#include <memory>

std::unique_ptr<SimpleDevice> simpleDevice(new SimpleDevice());




/**************************************************************************************
** Return properties of device.
***************************************************************************************/
void ISGetProperties(const char *dev)
{
    simpleDevice->ISGetProperties(dev);
}

/**************************************************************************************
** Process new switch from client
***************************************************************************************/
void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    simpleDevice->ISNewSwitch(dev, name, states, names, n);
}

/**************************************************************************************
** Process new text from client
***************************************************************************************/
void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    simpleDevice->ISNewText(dev, name, texts, names, n);
}

/**************************************************************************************
** Process new number from client
***************************************************************************************/
void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    simpleDevice->ISNewNumber(dev, name, values, names, n);
}

/**************************************************************************************
** Process new blob from client
***************************************************************************************/
void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[],
               char *names[], int n)
{
    simpleDevice->ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

/**************************************************************************************
** Process snooped property from another driver
***************************************************************************************/
void ISSnoopDevice(XMLEle *root)
{
    INDI_UNUSED(root);
}




SimpleDevice::SimpleDevice()
{
	ID=-1;
	IDLog("construct\n");
}

SimpleDevice::~SimpleDevice()
{
	PI_CloseConnection(ID);

	IDLog("construct\n");
}
bool SimpleDevice::initPropeties()
{
	
	return true;
}



bool SimpleDevice::updateProperties()
{	
	fill();
	IDMessage( getDeviceName(), "A messagge from updateProperties!" );
	SetTimer(1000);
	return true;
}


/**************************************************************************************
** Client is asking us to establish connection to the device
***************************************************************************************/
bool SimpleDevice::Connect()
{
	
	ConnectHex("jefftest4", 5251);
	GetHexPos(ID, Pos);
	GetHexPos(ID, NextPos);

    return true;
}

/**************************************************************************************
** Client is asking us to terminate connection to the device
***************************************************************************************/
bool SimpleDevice::Disconnect()
{
		
	PI_CloseConnection(ID);
    IDMessage(getDeviceName(), "Simple device disconnected successfully!");
    return true;
}

/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char *SimpleDevice::getDefaultName()
{
    return "Simple Device";
}


bool SimpleDevice::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
	INumberVectorProperty * myv = getNumber(name);
	if( strcmp(name, "PosX" ) == 0 )
	{
		
		NextPos[XX].pos = values[0];
		MoveOneAxis( ID, &NextPos[XX] );
	}
	if( strcmp(name, "PosY" ) == 0 )
	{
		
		NextPos[YY].pos = values[0];
		MoveOneAxis( ID, &NextPos[YY] );
	}
	if( strcmp(name, "PosZ" ) == 0 )
	{
		
		NextPos[ZZ].pos = values[0];
		MoveOneAxis( ID, &NextPos[ZZ] );
	}
	if( strcmp(name, "PosW" ) == 0 )
	{
		
		NextPos[WW].pos = values[0];
		MoveOneAxis( ID, &NextPos[WW] );
	}
	if( strcmp(name, "PosV" ) == 0 )
	{
		
		NextPos[VV].pos = values[0];
		MoveOneAxis( ID, &NextPos[VV] );
	}
	if( strcmp(name, "PosU" ) == 0 )
	{
		
		NextPos[UU].pos = values[0];
		MoveOneAxis( ID, &NextPos[UU] );
	}
	

	
	for(int ii=0; ii<n; ii++)
	{
		
		IDMessage(getDeviceName(), "dev=%s, name=%s, names[%i]=%s, values[%i]=%f\n", dev, name, ii, names[ii], ii, values[ii] );
		
			}
	IUUpdateNumber(myv, values, names, n);
	return true;

}

bool SimpleDevice::ISNewSwitch(const char *dev, const char * name, ISState *states, char *names[], int n)
{
	DefaultDevice::ISNewSwitch(dev, name, states, names, n);
	ISwitchVectorProperty *mysvp;
	mysvp = getSwitch(name);
	if(strcmp("ReadHex", mysvp->name) == 0 )
	{
		
		IDMessage(getDeviceName(), "we should be reading now... %i", mysvp->sp->s);
		ReadHex();
	}
	else if( strcmp("correct", mysvp->name) == 0 )
	{
		mysvp->sp[0].s == states[0];
		IUUpdateSwitch(mysvp, states, names, n);
	}
	else if( strcmp("ref", mysvp->name) == 0 )
	{
		ReferenceIfNeeded(ID, Pos);
	}
}

int SimpleDevice::ConnectHex( const char *host="localhost", int port=5200 ) 
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
	}
}

bool SimpleDevice::ReadHex()
{
	if(ID<0)
	{
		IDMessage(getDeviceName(), "Hexapod not connected!");
		return false;
	}
	GetHexPos(ID, Pos);

}
bool SimpleDevice::fill()
{

	/*Translational numbers*/
	IUFillNumber(&PosLatN_X[0] , "X", "X Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosLatNV_X,  PosLatN_X, 1, getDeviceName(), "PosX", "Linear Position X", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_X );
	
	IUFillNumber(&PosLatN_Y[0] , "Y", "Y Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosLatNV_Y,  PosLatN_Y, 1, getDeviceName(), "PosY", "Linear Position Y", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_Y );

	IUFillNumber(&PosLatN_Z[0] , "Z", "Z Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosLatNV_Z,  PosLatN_Z, 1, getDeviceName(), "PosZ", "Linear Position Z", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosLatNV_Z );


	/*rotational numbers*/
	IUFillNumber(&PosRotN_W[0] , "W", "W Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosRotNV_W,  PosRotN_W, 1, getDeviceName(), "PosW", "Rotational Position W", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_W );


	IUFillNumber(&PosRotN_V[0] , "V", "V Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosRotNV_V,  PosRotN_V, 1, getDeviceName(), "PosV", "Rotational Position V", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_V );


	IUFillNumber(&PosRotN_U[0] , "U", "U Axis ", "%5.4f", -5.0, 5.0, 0.001, 0);
	IUFillNumberVector( &PosRotNV_U,  PosRotN_U, 1, getDeviceName(), "PosU", "Rotational Position U", "ALL", IP_RW, 0.5, IPS_IDLE );
	defineNumber( &PosRotNV_U );
	
	IUFillSwitch( &readS[0], "read", "Read Hex", ISS_OFF );
	IUFillSwitchVector( &readSV, readS, 1, getDeviceName(), "ReadHex", "Read Hex Pos", "ALL", IP_RW, ISR_1OFMANY, 0.5, IPS_IDLE );
	defineSwitch(&readSV);

	IUFillSwitch( &corrS[0], "correct", "correct", ISS_OFF );
	IUFillSwitchVector( &corrSV, corrS, 1, getDeviceName(), "correct", "correct", "ALL", IP_WO, ISR_1OFMANY, 0.5, IPS_IDLE );
	defineSwitch(&corrSV);

	IUFillSwitch( &refS[0], "ref", "Reference", ISS_OFF );
	IUFillSwitchVector( &refSV, refS, 1, getDeviceName(), "ref", "Reference", "ALL", IP_WO, ISR_1OFMANY, 0.5, IPS_IDLE );
	defineSwitch(&refSV);



/*
UFillSwitchVector(
ISwitchVectorProperty *svp, 
ISwitch *sp, 
int nsp, 
const char 
*dev, 
const char *name,
const char *label, 
const char *group, 
IPerm p, 
ISRule r, 
double timeout, 
IPState s);
*/
}

void SimpleDevice::TimerHit()
{
	if(ID<0)
		return;

		
	Axis *iter;
	INumberVectorProperty *IAxis;
	
	GetHexPos(ID, Pos);

	char name[] = "PosX";
	for(iter=Pos; iter!=&Pos[6]; iter++)
	{
		name[3] = iter->letter[0];
		IAxis = getNumber(name);
		IAxis->np->value = iter->pos;
		IDSetNumber(IAxis, NULL);
		//IDMessage(getDeviceName(), "%s", name);
		//axis = getNumber();
	}

	//IDMessage(getDeviceName(), "x=%f, y=%f, z=%f, w=%f,v=%f, y=%f", Pos[XX].pos, Pos[YY].pos, Pos[ZZ].pos, Pos[WW].pos, Pos[VV].pos, Pos[UU].pos);

	SetTimer(1000);
}

bool SimpleDevice::MoveNext()
{
	MoveAbs(ID, NextPos );
}
