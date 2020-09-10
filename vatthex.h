#include "PI/PI_GCS2_DLL.h"

typedef enum axis_II {      
    XX=0,           
    YY=1,           
    ZZ=2,           
    WW=3,           
    VV=4,           
    UU=5

} axis_ii;

typedef struct axis
{
    double pos;
    int ii;
    char * letter;
} Axis;


//prototypes
void usage( char[] );
BOOL MoveAbs(int ID, Axis *next);
BOOL GetHexPos( int ID, Axis *xp );
BOOL MoveOneAxis(int ID, Axis *xp );
void PrintHexPos( Axis *xp );
BOOL uncorrect( Axis xp[], double el, double temp );
BOOL correct( Axis xp[], double el, double temp );
BOOL InitAxis(Axis xp[], axis_ii ii);
void InitAllAxes( Axis xp[] );
BOOL ReferenceIfNeeded(int ID, Axis *xp);
int GenericCommand(int ID, const char* cmd, char resp[], int respSize);
int GetError(int ID, char errBuff[], int buffSize );
