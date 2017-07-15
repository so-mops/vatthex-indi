/*
   INDI Developers Manual
   Tutorial #1

   "Hello INDI"

   We construct a most basic (and useless) device driver to illustate INDI.

   Refer to README, which contains instruction on how to build this driver, and use it
   with an INDI-compatible client.

*/

/** \file simpledevice.h
    \brief Construct a basic INDI device with only one property to connect and disconnect.
    \author Jasem Mutlaq

    \example simpledevice.h
    A very minimal device! It also allows you to connect/disconnect and performs no other functions.
*/

#pragma once
#include "defaultdevice.h"
#include <string.h>
extern "C" {
#include "PI_GCS2_DLL.h"

#include <vatthex.h> 
}
class indiNVP
{
	public:
		indiNVP(INumberVectorProperty *);
		indiNVP(INumber * num0, int n, const char * name, const char *label, const char * group, IPerm perm, double timeout, IPState state  );
		INumberVectorProperty *nvp;

	private: 
		bool fill();
};




class SimpleDevice : public INDI::DefaultDevice
{
  public:
    SimpleDevice();
	~SimpleDevice();
	virtual bool initPropeties();
	virtual bool updateProperties();
	virtual bool ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n);
	virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n);
	bool ReadHex();
	bool MoveNext();
	int ConnectHex(const char *host, int port);
	bool fill();
	int ID;
	bool setNumber(INumberVectorProperty *, double values, char * names[], int);
	virtual void TimerHit();
	
	Axis Pos[6];
	Axis NextPos[6];

	INumberVectorProperty PosLatNV_X;
	INumber PosLatN_X[1];
	INumberVectorProperty PosLatNV_Y;
	INumber PosLatN_Y[1];
	INumberVectorProperty PosLatNV_Z;
	INumber PosLatN_Z[1];


	INumberVectorProperty PosRotNV_W;
	INumber PosRotN_W[1];
	INumberVectorProperty PosRotNV_V;
	INumber PosRotN_V[1];
	INumberVectorProperty PosRotNV_U;
	INumber PosRotN_U[1];

	ISwitchVectorProperty readSV;
	ISwitch readS[1];

	ISwitchVectorProperty corrSV;
	ISwitch corrS[1];
	
	ISwitchVectorProperty refSV;
	ISwitch refS[1];
	
	

  protected:
    bool Connect();
    bool Disconnect();
    const char *getDefaultName();
	
};
