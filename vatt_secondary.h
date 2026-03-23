/*
    INDI Hexapod driver, header file.
*/

#pragma once

#include "defaultdevice.h"

#include <cstdio>
#include <string.h>

extern "C" {
#include "PI/PI_GCS2_DLL.h"
#include "projectsoft.h"
#include "vatthex.h"
}

class indiNVP
{
    public:
        indiNVP(INumberVectorProperty *);
        indiNVP(INumber *num0, int n, const char *name, const char *label, const char *group,
                IPerm perm, double timeout, IPState state);
        INumberVectorProperty *nvp;

    private:
        bool fill();
};

class Secondary : public INDI::DefaultDevice
{
    public:
        Secondary();
        ~Secondary() override;

        bool initProperties() override;
        bool updateProperties() override;
        bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
        bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
        bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;
        void TimerHit() override;

        bool _GetHexPos(Axis ax[]);
        bool _MoveOneAxis(Axis *ax);
        bool MoveNext();
        bool fill();
        bool isReferenced(Axis xp[]);
        bool isConnected();
        void SetIAxisState(IPState state);
        int GetTempAndEl();
        void deepcopy(Axis *, Axis *);

        int ID = -1;
        bool connectionWentBad = false;
        unsigned short int vatttel_counter = 0;
        unsigned short commerr_count = 0;
        int TimerID = -1;

        Axis Pos[6];
        Axis NextPos[6];
        Axis CorrPos[6];
        Axis CorrNextPos[6];
        Axis ZeroPos[6];

        FILE *posfile = nullptr;
        FILE *corrfile = nullptr;

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

        INumberVectorProperty TempElNV;
        INumber TempElN[2];

        double temp = 20.0;
        double el = 3.14159 / 4.0;

    protected:
        bool Connect() override;
        bool Disconnect() override;
        const char *getDefaultName() override;
        bool SetReadyState();
        bool SetMoveState();
};