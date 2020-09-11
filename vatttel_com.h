





#define VATT_CMD_STATUS_ALTAZ        (0x0a10)
#define ALT_INDEX 2
#define VATT_CMD_STATUS_TEMPS (0x0a15)
#define STRUT_INDEX 2
#define VATT_CMD_STATUS_SECONDARY1   (0x0a13)


//extern "C" double GetStrutTemp(char *);
double GetStrutTemp(char *);

//extern "C" double GetAlt(char *);
double GetAlt(char *);

