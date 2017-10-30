/*
	INDI Hexapod driver, header file. 
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




class Secondary : public INDI::DefaultDevice
{
  public:
    Secondary();
	~Secondary();
	virtual bool initProperties();
	virtual bool updateProperties();
	virtual bool ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n);
	virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n);
	
	virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n);

	bool ReadHex();
	bool MoveNext();
	int ConnectHex(const char *host, int port);
	bool fill();
	int ID;
	bool setNumber(INumberVectorProperty *, double values, char * names[], int);
	virtual void TimerHit();

	int GetTempAndEl( );

	void deepcopy(Axis *, Axis *);
	
	
	Axis Pos[6];
	Axis NextPos[6];
	Axis CorrPos[6];
	Axis CorrNextPos[6];


	//hex positions
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
	//end hex positions.


	

	ISwitchVectorProperty corrSV;
	ISwitch corrS[1];
	
	ISwitchVectorProperty refSV;
	ISwitch refS[1];

	ITextVectorProperty HexAddrTV;
	IText HexAddrT[1];


	ITextVectorProperty cmdTV;
	IText cmdT[1];

	ITextVectorProperty errTV;
	IText errT[1];
	
	double temp = 20.0;
	double el = 3.14159/4.0;


  protected:
    bool Connect();
    bool Disconnect();
    const char *getDefaultName();
	bool SetReadyState();
	bool SetMoveState();
};
