/*
   INDI device driver for the VATT Hexapod (secondary controller)
*/

#include "vatt_secondary.h"

#include <memory>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <time.h>

#define REFRESH  1000
#define MAX_VATTTEL_CNT 5
#define SLEEP_BTWN_CALLS 1e4
#define PI_PORTNUM 50000
#define MILLI2MICRON 1e3
#define DEG2ASEC 3600.0
#define MAX_COMMERR 0

namespace
{
    // Returns the base directory used for driver runtime files.
    // The location can be overridden by the VATTHEX_DATA_DIR environment variable
    // (typically set through the docker run command).
    std::string getDataDir()
    {
        const char *env = std::getenv("VATTHEX_DATA_DIR");
        if (env && *env)
            return std::string(env);

        return "/var/lib/vatthex";
    }

    // Returns the path to the saved position file.
    // The VATTHEX_POS_FILE environment variable can override the default location.
    std::string getPosFilePath()
    {
        const char *env = std::getenv("VATTHEX_POS_FILE");
        if (env && *env)
            return std::string(env);

        return getDataDir() + "/posfile.dat";
    }

    // Returns the path to the correction log file.
    // The VATTHEX_CORR_FILE environment variable can override the default location.
    std::string getCorrFilePath()
    {
        const char *env = std::getenv("VATTHEX_CORR_FILE");
        if (env && *env)
            return std::string(env);

        return getDataDir() + "/corrections.log";
    }

    // Ensures the directory containing the specified file path exists.
    bool ensureParentDirExists(const std::string &path)
    {
        try
        {
            const auto parent = std::filesystem::path(path).parent_path();
            if (!parent.empty())
                std::filesystem::create_directories(parent);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
}

std::unique_ptr<Secondary> secondary(new Secondary());

// INDI entry points
// These functions forward requests from indiserver to the Secondary driver instance.

// Return properties of device.
void ISGetProperties(const char *dev)
{
    secondary->ISGetProperties(dev);
}

// Process new switch from client.
void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    secondary->ISNewSwitch(dev, name, states, names, n);
}

// Process new text from client.
void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    secondary->ISNewText(dev, name, texts, names, n);
}

// Process new number from client.
void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    secondary->ISNewNumber(dev, name, values, names, n);
}

// Process new blob from client.
void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[],
               char *names[], int n)
{
    secondary->ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

// Process snooped property from another driver.
void ISSnoopDevice(XMLEle *root)
{
    INDI_UNUSED(root);
}

// Constructor
// Initializes connection state, ensures runtime directories exist,
// and defines the nominal zero position of the hexapod axes.
Secondary::Secondary()
{
    ID = -1;

    // Ensure directories for runtime files exist.
    ensureParentDirExists(getPosFilePath());
    ensureParentDirExists(getCorrFilePath());

    // Initialize all axes to their default structure values.
    InitAllAxes(ZeroPos);

    // Nominal optical zero positions for the hexapod.
    ZeroPos[XX].pos = -1000 / MILLI2MICRON;
    ZeroPos[YY].pos = -700 / MILLI2MICRON;
    ZeroPos[ZZ].pos = -1750 / MILLI2MICRON;

    ZeroPos[VV].pos = 40 / DEG2ASEC;
    ZeroPos[UU].pos = -60 / DEG2ASEC;
    ZeroPos[WW].pos = 0;
}

// Destructor
// Ensures the PI controller connection is closed if still active.
Secondary::~Secondary()
{
    PI_CloseConnection(ID);
    IDLog("construct\n");
}

/*
 * Initialize the INDI properties for this driver.
 * The base device properties are created first, then
 * fill() defines the driver-specific properties.
 */
bool Secondary::initProperties()
{
    DefaultDevice::initProperties();
    fill();
    return true;
}

/*
 * Ensure the driver update timer is running.
 * The timer periodically calls TimerHit() to poll the
 * controller and update INDI properties.
 */
bool Secondary::updateProperties()
{
    if (TimerID != -1)
        RemoveTimer(TimerID);

    TimerID = SetTimer(1000);
    return true;
}

/*
 * Establish a TCP connection to the PI hexapod controller.
 * The controller IP is taken from the HexAddr INDI property.
 * After connecting, the controller identity is verified and
 * the current hexapod position is retrieved.
 */
bool Secondary::Connect()
{
    setConnected(true, IPS_BUSY, "Attempting to connect");

    const char *ip = (HexAddrT[0].text && *HexAddrT[0].text) ? HexAddrT[0].text : "10.0.3.10";
    const int port = 50000;

    IDMessage(getDeviceName(), "Attempting direct connect to %s:%d", ip, port);

    ID = PI_ConnectTCPIP(ip, port);
    if (ID < 0)
    {
        IDMessage(getDeviceName(), "PI_ConnectTCPIP failed (ID=%d)", ID);
        setConnected(false, IPS_ALERT, "Connection failed");
        return false;
    }

    IDMessage(getDeviceName(), "PI_ConnectTCPIP (ID=%d)", ID);

    char szIDN[200] = {0};
    if (PI_qIDN(ID, szIDN, (int)sizeof(szIDN) - 1) == FALSE)
    {
        IDMessage(getDeviceName(), "PI_qIDN failed after connect; closing connection");
        PI_CloseConnection(ID);
        ID = -1;
        setConnected(false, IPS_ALERT, "Controller did not respond");
        return false;
    }

    IDMessage(getDeviceName(), "Connected: %s", szIDN);

    _GetHexPos(Pos);
    if (ID < 0)
        return false;

    _GetHexPos(NextPos);
    if (ID < 0)
        return false;

    if (!isReferenced(Pos))
    {
        refSV.s = IPS_ALERT;
        IDSetSwitch(&refSV, "You need to reference the secondary");
    }

    connectionWentBad = false;
    setConnected(true, IPS_OK, "Connected");
    return true;
}

/*
 * Close the connection to the hexapod controller and reset
 * the driver state so the client reflects a disconnected device.
 */
bool Secondary::Disconnect()
{
    DefaultDevice::Disconnect();

    PI_CloseConnection(ID);
    ID = -1;

    corrS[0].s = ISS_OFF;
    corrSV.s = IPS_IDLE;
    IDSetSwitch(&corrSV, NULL);

    refS[0].s = ISS_OFF;
    refSV.s = IPS_IDLE;
    IDSetSwitch(&refSV, "Not referenced");

    SetIAxisState(IPS_IDLE);
    return true;
}

const char *Secondary::getDefaultName()
{
    return "VATT Secondary";
}

/*
 * Handle numeric property updates from an INDI client.
 *
 * This method processes position commands for each hexapod axis.
 * The requested position is stored in NextPos and optionally
 * corrected using the current temperature and elevation if
 * autocollimation is enabled. The corresponding axis move
 * command is then issued to the controller.
 */
bool Secondary::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    INDI_UNUSED(dev);
    INDI::PropertyViewNumber *myv = getProperty(name).getNumber();

    if (strcmp(name, "PosX") == 0)
    {
        IDMessage(getDeviceName(), "User input PosX: %lf", values[0]);
        NextPos[XX].pos = values[0] / MILLI2MICRON;
        deepcopy(CorrNextPos, NextPos);
        correct(CorrNextPos, el, temp);

        if (corrS[0].s == ISS_ON)
            _MoveOneAxis(&CorrNextPos[XX]);
        else
            _MoveOneAxis(&NextPos[XX]);
    }

    if (strcmp(name, "PosY") == 0)
    {
        IDMessage(getDeviceName(), "User input PosY: %lf", values[0]);
        NextPos[YY].pos = values[0] / MILLI2MICRON;
        deepcopy(CorrNextPos, NextPos);
        correct(CorrNextPos, el, temp);

        if (corrS[0].s == ISS_ON)
            _MoveOneAxis(&CorrNextPos[YY]);
        else
            _MoveOneAxis(&NextPos[YY]);
    }

    if (strcmp(name, "PosZ") == 0)
    {
        IDMessage(getDeviceName(), "User input PosZ: %lf", values[0]);
        NextPos[ZZ].pos = values[0] / MILLI2MICRON;
        deepcopy(CorrNextPos, NextPos);
        correct(CorrNextPos, el, temp);

        if (corrS[0].s == ISS_ON)
            _MoveOneAxis(&CorrNextPos[ZZ]);
        else
            _MoveOneAxis(&NextPos[ZZ]);
    }

    if (strcmp(name, "PosW") == 0)
    {
        IDMessage(getDeviceName(), "User input PosW: %lf", values[0]);
        NextPos[WW].pos = values[0] / DEG2ASEC;
        deepcopy(CorrNextPos, NextPos);
        correct(CorrNextPos, el, temp);

        if (corrS[0].s == ISS_ON)
            _MoveOneAxis(&CorrNextPos[WW]);
        else
            _MoveOneAxis(&NextPos[WW]);
    }

    if (strcmp(name, "PosV") == 0)
    {
        IDMessage(getDeviceName(), "User input PosV: %lf", values[0]);
        NextPos[VV].pos = values[0] / DEG2ASEC;
        deepcopy(CorrNextPos, NextPos);
        correct(CorrNextPos, el, temp);

        if (corrS[0].s == ISS_ON)
            _MoveOneAxis(&CorrNextPos[VV]);
        else
            _MoveOneAxis(&NextPos[VV]);
    }

    if (strcmp(name, "PosU") == 0)
    {
        IDMessage(getDeviceName(), "User input PosU: %lf", values[0]);
        NextPos[UU].pos = values[0] / DEG2ASEC;
        deepcopy(CorrNextPos, NextPos);
        correct(CorrNextPos, el, temp);

        if (corrS[0].s == ISS_ON)
            _MoveOneAxis(&CorrNextPos[UU]);
        else
            _MoveOneAxis(&NextPos[UU]);
    }

    if (strcmp(name, "temp") == 0)
    {
        TempElN[0].value = values[0];
        TempElN[1].value = values[1];
        IDSetNumber(&TempElNV, "Setting Temp or El from user %f %f", values[0], values[1]);
    }

    IUUpdateNumber(myv, values, names, n);
    return true;
}

/*
 * Handle switch property updates from an INDI client.
 *
 * This processes control actions such as enabling or disabling
 * autocollimation and referencing the hexapod. When autocollimation
 * is enabled the driver moves the axes to the stored position or
 * nominal zero position and begins applying temperature and
 * elevation corrections during normal operation.
 */
bool Secondary::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    DefaultDevice::ISNewSwitch(dev, name, states, names, n);
    INDI::PropertyViewSwitch *mysvp = getProperty(name).getSwitch();

    IUUpdateSwitch(mysvp, states, names, n);

    if (strcmp("correct", mysvp->name) == 0)
    {
        if (ID < 0)
        {
            mysvp->sp[0].s = ISS_OFF;
            IDSetSwitch(mysvp, "Hexapod is not connected please connect.");
            setConnected(false, IPS_ALERT, "");
        }
        else if (mysvp->sp[0].s == ISS_ON)
        {
            const std::string posFile = getPosFilePath();

            if (access(posFile.c_str(), F_OK) != -1)
            {
                posfile = fopen(posFile.c_str(), "r");
                if (posfile == nullptr)
                {
                    IDMessage(getDeviceName(), "Failed to open %s: %s", posFile.c_str(), strerror(errno));
                    deepcopy(NextPos, ZeroPos);
                }
                else
                {
                    int fail = fscanf(posfile, "%lf %lf %lf %lf %lf",
                                      &NextPos[XX].pos,
                                      &NextPos[YY].pos,
                                      &NextPos[ZZ].pos,
                                      &NextPos[VV].pos,
                                      &NextPos[UU].pos);

                    IDMessage(getDeviceName(), "Reading position file %i", fail);

                    if (fail != 5)
                    {
                        IDMessage(getDeviceName(), "Position file exists but could not be read");
                        deepcopy(NextPos, ZeroPos);
                    }

                    fclose(posfile);
                }
            }
            else
            {
                IDMessage(getDeviceName(), "No position file moving to zero.");
                deepcopy(NextPos, ZeroPos);
            }

            PosLatNV_X.s = IPS_BUSY;
            PosLatNV_Y.s = IPS_BUSY;
            PosLatNV_Z.s = IPS_BUSY;
            PosRotNV_W.s = IPS_BUSY;
            PosRotNV_V.s = IPS_BUSY;
            PosRotNV_U.s = IPS_BUSY;

            IDSetNumber(&PosLatNV_X, NULL);
            IDSetNumber(&PosLatNV_Y, NULL);
            IDSetNumber(&PosLatNV_Z, NULL);
            IDSetNumber(&PosRotNV_W, NULL);
            IDSetNumber(&PosRotNV_V, NULL);
            IDSetNumber(&PosRotNV_U, NULL);

            _MoveOneAxis(&NextPos[XX]);
            if (ID < 0)
            {
                mysvp->sp[0].s = ISS_OFF;
                IDSetSwitch(mysvp, "Lost communication while enabling autocollimation.");
                return true;
            }
            usleep(SLEEP_BTWN_CALLS);

            _MoveOneAxis(&NextPos[YY]);
            if (ID < 0)
            {
                mysvp->sp[0].s = ISS_OFF;
                IDSetSwitch(mysvp, "Lost communication while enabling autocollimation.");
                return true;
            }
            usleep(SLEEP_BTWN_CALLS);

            _MoveOneAxis(&NextPos[ZZ]);
            if (ID < 0)
            {
                mysvp->sp[0].s = ISS_OFF;
                IDSetSwitch(mysvp, "Lost communication while enabling autocollimation.");
                return true;
            }
            usleep(SLEEP_BTWN_CALLS);

            _MoveOneAxis(&NextPos[VV]);
            if (ID < 0)
            {
                mysvp->sp[0].s = ISS_OFF;
                IDSetSwitch(mysvp, "Lost communication while enabling autocollimation.");
                return true;
            }
            usleep(SLEEP_BTWN_CALLS);

            _MoveOneAxis(&NextPos[UU]);
            if (ID < 0)
            {
                mysvp->sp[0].s = ISS_OFF;
                IDSetSwitch(mysvp, "Lost communication while enabling autocollimation.");
                return true;
            }

            GetTempAndEl();
            deepcopy(CorrNextPos, NextPos);
            correct(CorrNextPos, el, temp);
            mysvp->s = IPS_OK;
            vatttel_counter = MAX_VATTTEL_CNT + 1;
        }
        else
        {
            deepcopy(CorrNextPos, NextPos);
            mysvp->s = IPS_IDLE;
            _GetHexPos(Pos);

            const std::string posFile = getPosFilePath();
            if (!ensureParentDirExists(posFile))
            {
                IDMessage(getDeviceName(), "Failed to create directory for %s", posFile.c_str());
            }
            else
            {
                posfile = fopen(posFile.c_str(), "w");
                if (posfile == nullptr)
                {
                    IDMessage(getDeviceName(), "Failed to open %s for writing: %s", posFile.c_str(), strerror(errno));
                }
                else
                {
                    fprintf(posfile, "%lf %lf %lf %lf %lf",
                            NextPos[XX].pos,
                            NextPos[YY].pos,
                            NextPos[ZZ].pos,
                            NextPos[VV].pos,
                            NextPos[UU].pos);
                    fclose(posfile);
                    IDMessage(getDeviceName(), "Wrote position file %s", posFile.c_str());
                }
            }
        }

        IDSetSwitch(&corrSV, NULL);
    }
    else if (strcmp("ref", mysvp->name) == 0)
    {
        if (ID < 0)
        {
            mysvp->s = IPS_IDLE;
            mysvp->sp[0].s = ISS_OFF;
            IDSetSwitch(mysvp, "Can't reference Not connected!");
            setConnected(false, IPS_BUSY);
        }
        else
        {
            mysvp->s = IPS_BUSY;
            IDSetSwitch(mysvp, "Referencing Hexapod");
            ReferenceIfNeeded(ID, Pos);
            mysvp->s = IPS_OK;
            IDSetSwitch(mysvp, NULL);
        }
    }

    return true;
}

/*
 * Handle text property updates from an INDI client.
 *
 * This allows clients to send raw commands to the controller
 * and update the configured hexapod controller IP address.
 */
bool Secondary::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    INDI_UNUSED(dev);
    INDI_UNUSED(names);
    INDI_UNUSED(n);

    char resp[300];

    if (strcmp(name, "cmdv") == 0)
    {
        GenericCommand(ID, texts[0], resp, 300);
        strncpy(cmdT[0].text, resp, 39);
        cmdT[0].text[39] = '\0';
        IDSetText(&cmdTV, "%s", resp);
    }

    if (strcmp(name, "hexipvp") == 0)
    {
        IDMessage(getDeviceName(), "Setting ip address %s", texts[0]);
        strcpy(HexAddrT[0].text, texts[0]);
        IDSetText(&HexAddrTV, NULL);
    }

    return true;
}

/*
 * Move a single hexapod axis to the requested position.
 * This wraps the low-level motion call and updates the
 * driver connection state if communication fails.
 */
bool Secondary::_MoveOneAxis(Axis *ax)
{
    int retn = true;

    if (ID < 0)
    {
        IDMessage(getDeviceName(), "Please connect the secondary controller.");
        setConnected(false, IPS_ALERT, NULL);
        return false;
    }

    if (!MoveOneAxis(ID, ax))
    {
        IDMessage(getDeviceName(), "Axis %s failed to move to %f", ax->letter, ax->pos);
        isConnected();
        retn = false;
    }
    else
    {
        commerr_count = 0;
    }

    return retn;
}

/*
 * Read the current hexapod position from the controller.
 * This wraps the low-level position query and tracks
 * communication failures for connection handling.
 */
bool Secondary::_GetHexPos(Axis *hexpos)
{
    int retn = true;

    if (ID < 0)
    {
        IDMessage(getDeviceName(), "Hexapod not connected!");
        return false;
    }

    if (!GetHexPos(ID, hexpos))
    {
        retn = false;
        IDMessage(getDeviceName(), "Failed to retrieve position");
        isConnected();
        commerr_count++;
    }
    else
    {
        commerr_count = 0;
    }

    return retn;
}

/*
 * Verify that the current PI controller connection is still valid.
 * If communication fails, the connection is closed, the driver state
 * is reset, and the client is told to reconnect.
 */
bool Secondary::isConnected()
{
    if (!DefaultDevice::isConnected())
        return false;

    if (ID < 0)
        return false;

    char szIDN[200] = {0};
    if (PI_qIDN(ID, szIDN, (int)sizeof(szIDN) - 1) == FALSE)
    {
        PI_CloseConnection(ID);
        ID = -1;

        corrS[0].s = ISS_OFF;
        corrSV.s = IPS_IDLE;
        IDSetSwitch(&corrSV, NULL);

        refS[0].s = ISS_OFF;
        refSV.s = IPS_IDLE;
        IDSetSwitch(&refSV, NULL);

        setConnected(false, IPS_ALERT,
                     "Lost communication with the Secondary Controller. Press Connect to reconnect.");
        connectionWentBad = true;

        return false;
    }

    return true;
}

/*
 * Check whether all hexapod axes have been referenced.
 * If any axis is not referenced, or the controller cannot
 * be queried, the function returns false.
 */
bool Secondary::isReferenced(Axis xp[])
{
    int isRefed;

    if (ID < 0)
    {
        IDMessage(getDeviceName(), "You are not connected!");
        return false;
    }

    for (int ii = 0; ii < 6; ii++)
    {
        if (!PI_qFRF(ID, xp[ii].letter, &isRefed))
        {
            commerr_count++;
            return false;
        }

        if (!isRefed)
            return false;
    }

    return true;
}

/*
 * Define the INDI properties exposed by this driver.
 * This includes axis position controls, temperature and
 * elevation values, correction and reference switches,
 * command text fields, and the configurable hexapod IP.
 */
bool Secondary::fill()
{
    const char linposgrp[] = "Decenter";
    const char rotposgrp[] = "Tip Tilt";
    const char miscgrp[] = "Miscellaneous";
    const char tegroup[] = "Temp El";

    IUFillNumber(&PosLatN_X[0], "X", "X Axis ", "%5.0f", -5.0 * MILLI2MICRON, 5.0 * MILLI2MICRON, 1, 0);
    IUFillNumberVector(&PosLatNV_X, PosLatN_X, 1, getDeviceName(), "PosX", "Linear Position X", linposgrp, IP_RW, 0.5, IPS_IDLE);
    defineProperty(&PosLatNV_X);

    IUFillNumber(&PosLatN_Y[0], "Y", "Y Axis ", "%5.0f", -5.0 * MILLI2MICRON, 5.0 * MILLI2MICRON, 1, 0);
    IUFillNumberVector(&PosLatNV_Y, PosLatN_Y, 1, getDeviceName(), "PosY", "Linear Position Y", linposgrp, IP_RW, 0.5, IPS_IDLE);
    defineProperty(&PosLatNV_Y);

    IUFillNumber(&PosLatN_Z[0], "Z", "Focus ", "%5.0f", -5.0 * MILLI2MICRON, 5.0 * MILLI2MICRON, 1, 0);
    IUFillNumberVector(&PosLatNV_Z, PosLatN_Z, 1, getDeviceName(), "PosZ", "Linear Position Z", linposgrp, IP_RW, 0.5, IPS_IDLE);
    defineProperty(&PosLatNV_Z);

    IUFillNumber(&PosRotN_W[0], "W", "W Axis ", "%5.0f", -2.5 * DEG2ASEC, 2.5 * DEG2ASEC, 1, 0);
    IUFillNumberVector(&PosRotNV_W, PosRotN_W, 1, getDeviceName(), "PosW", "Rotational Position W", rotposgrp, IP_RW, 0.5, IPS_IDLE);
    defineProperty(&PosRotNV_W);

    IUFillNumber(&PosRotN_V[0], "V", "Tip X", "%5.0f", -2.5 * DEG2ASEC, 2.5 * DEG2ASEC, 1, 0);
    IUFillNumberVector(&PosRotNV_V, PosRotN_V, 1, getDeviceName(), "PosV", "Rotational Position V", rotposgrp, IP_RW, 0.5, IPS_IDLE);
    defineProperty(&PosRotNV_V);

    IUFillNumber(&PosRotN_U[0], "U", "Tip Y", "%5.0f", -2.5 * DEG2ASEC, 2.5 * DEG2ASEC, 1, 0);
    IUFillNumberVector(&PosRotNV_U, PosRotN_U, 1, getDeviceName(), "PosU", "Rotational Position U", rotposgrp, IP_RW, 0.5, IPS_IDLE);
    defineProperty(&PosRotNV_U);

    IUFillNumber(&TempElN[0], "temp", "Strut Temp", "%4.1f", -20, 20, 0.1, 0);
    IUFillNumber(&TempElN[1], "el", "Elevation", "%4.1f", 0, 90, 0.01, 0);
    IUFillNumberVector(&TempElNV, TempElN, 2, getDeviceName(), "temp", "The fing temp", tegroup, IP_RW, 0.5, IPS_IDLE);
    defineProperty(&TempElNV);
    IDSetNumber(&TempElNV, NULL);

    IUFillSwitch(&corrS[0], "correct", "Auto Collimate", ISS_OFF);
    IUFillSwitchVector(&corrSV, corrS, 1, getDeviceName(), "correct", "correct", miscgrp, IP_WO, ISR_NOFMANY, 0.5, IPS_IDLE);
    defineProperty(&corrSV);

    IUFillSwitch(&refS[0], "ref", "Reference", ISS_OFF);
    IUFillSwitchVector(&refSV, refS, 1, getDeviceName(), "ref", "Reference", miscgrp, IP_WO, ISR_NOFMANY, 0.5, IPS_IDLE);
    defineProperty(&refSV);

    IUFillText(cmdT, "cmd", "Command", "");
    IUFillTextVector(&cmdTV, cmdT, 1, getDeviceName(), "cmdv", "Command", miscgrp, IP_RW, 0.5, IPS_IDLE);
    defineProperty(&cmdTV);

    IUFillText(errT, "err", "Error", "No Error");
    IUFillTextVector(&errTV, errT, 1, getDeviceName(), "err", "Error", miscgrp, IP_RO, 0.5, IPS_IDLE);
    defineProperty(&errTV);

    IUFillText(&HexAddrT[0], "hexip", "Hexapod IP", "10.0.3.10");
    IUFillTextVector(&HexAddrTV, HexAddrT, 1, getDeviceName(), "hexipvp", "Hexapod IP Addr", "Main Control", IP_RW, 0.5, IPS_IDLE);
    defineProperty(&HexAddrTV);
    IDSetText(&HexAddrTV, NULL);

    return true;
}

/*
 * Periodic driver update loop.
 *
 * This polls the controller for position and status, updates
 * the INDI properties shown to clients, and applies correction
 * moves when autocollimation is enabled. If communication is
 * lost, the driver marks itself disconnected and waits for the
 * user to reconnect.
 */
void Secondary::TimerHit()
{
    if (ID < 0)
    {
        if (connectionWentBad)
        {
            setConnected(false, IPS_ALERT,
                         "Lost communication with the Secondary Controller. Press Connect to reconnect.");
            connectionWentBad = false;
        }

        TimerID = SetTimer(2000);
        return;
    }

    setConnected(true, IPS_OK, NULL);

    Axis *iter;
    INDI::PropertyViewNumber *IAxis;
    bool isMoving = false;
    int isReady = 0;
    int errno_val;
    char err[300];
    double unitconversion;
    bool axisMoveState;

    _GetHexPos(Pos);
    if (ID < 0)
    {
        TimerID = SetTimer(2000);
        return;
    }

    deepcopy(CorrPos, Pos);

    isMoving = SetMoveState();
    if (ID < 0)
    {
        TimerID = SetTimer(2000);
        return;
    }

    isReady = SetReadyState();
    if (ID < 0)
    {
        TimerID = SetTimer(2000);
        return;
    }

    if (isReady)
    {
        refS[0].s = ISS_OFF;
    }
    else
    {
        IDMessage(getDeviceName(), "The hexapod is not ready, it may need to be restarted");
        refSV.s = IPS_ALERT;
    }
    IDSetSwitch(&refSV, NULL);

    if (corrS[0].s == ISS_ON)
    {
        corrSV.s = IPS_OK;
        GetTempAndEl();

        if (!isMoving)
        {
            deepcopy(CorrNextPos, NextPos);
            correct(CorrNextPos, el, temp);

            for (int ii = 0; ii < 6; ii++)
            {
                if (fabs(CorrNextPos[ii].pos - CorrPos[ii].pos) > 0.0001)
                {
                    IDMessage(getDeviceName(), "Applying correction for autocollimation");

                    const std::string corrFile = getCorrFilePath();
                    if (!ensureParentDirExists(corrFile))
                    {
                        IDMessage(getDeviceName(), "Failed to create directory for %s", corrFile.c_str());
                    }
                    else
                    {
                        corrfile = fopen(corrFile.c_str(), "a");
                        if (corrfile == nullptr)
                        {
                            IDMessage(getDeviceName(), "Failed to open %s: %s", corrFile.c_str(), strerror(errno));
                        }
                        else
                        {
                            time_t rawtime;
                            struct tm *timeinfo;
                            time(&rawtime);
                            timeinfo = localtime(&rawtime);
                            char buffer[26];
                            strftime(buffer, 26, "%F %H:%M:%S", timeinfo);

                            fprintf(corrfile, "%s\t%lf\t%lf\t%lf\t%lf\t%lf\t%i\t%lf\t%lf\t%lf\n",
                                    buffer,
                                    NextPos[XX].pos,
                                    NextPos[YY].pos,
                                    NextPos[ZZ].pos,
                                    NextPos[VV].pos,
                                    NextPos[UU].pos,
                                    ii,
                                    el,
                                    temp,
                                    CorrNextPos[ii].pos);

                            fclose(corrfile);
                        }
                    }

                    axisMoveState = _MoveOneAxis(&CorrNextPos[ii]);
                    if (ID < 0)
                    {
                        TimerID = SetTimer(2000);
                        return;
                    }

                    if (axisMoveState == false)
                    {
                        corrS[0].s = ISS_OFF;
                        corrSV.s = IPS_IDLE;

                        const std::string posFile = getPosFilePath();
                        IDMessage(getDeviceName(), "Deleting %s", posFile.c_str());
                        remove(posFile.c_str());

                        IDSetSwitch(&corrSV, NULL);
                    }

                    usleep(SLEEP_BTWN_CALLS);
                }
            }
        }

        uncorrect(Pos, el, temp);
    }
    else
    {
        corrSV.s = IPS_IDLE;
    }
    IDSetSwitch(&corrSV, NULL);

    char name[] = "PosX";
    for (iter = Pos; iter != &Pos[6]; iter++)
    {
        if (iter->letter[0] == 'X' || iter->letter[0] == 'Y' || iter->letter[0] == 'Z')
            unitconversion = MILLI2MICRON;
        else
            unitconversion = DEG2ASEC;

        name[3] = iter->letter[0];
        IAxis = getProperty(name).getNumber();
        IAxis->np->value = iter->pos * unitconversion;
        IDSetNumber(IAxis, NULL);
    }

    if (ID < 0)
    {
        TimerID = SetTimer(2000);
        return;
    }

    errno_val = GetError(ID, err, 300);
    if (errno_val > 0)
        errTV.s = IPS_ALERT;
    else
        errTV.s = IPS_IDLE;

    sprintf(errT[0].text, "%i: %s", errno_val, err);
    IDSetText(&errTV, NULL);

    if (commerr_count > MAX_COMMERR)
    {
        PI_CloseConnection(ID);
        ID = -1;

        corrS[0].s = ISS_OFF;
        corrSV.s = IPS_IDLE;
        IDSetSwitch(&corrSV, NULL);

        refS[0].s = ISS_OFF;
        refSV.s = IPS_IDLE;
        IDSetSwitch(&refSV, NULL);

        setConnected(false, IPS_ALERT,
                     "Lost communication with the Secondary Controller. Press Connect to reconnect.");
        connectionWentBad = true;
    }

    TimerID = SetTimer(1000);
}

/*
 * Set the INDI state for all axis position properties.
 * This is used to keep the client display in sync with
 * the overall motion state of the hexapod.
 */
void Secondary::SetIAxisState(IPState state)
{
    PosLatNV_X.s = state;
    IDSetNumber(&PosLatNV_X, NULL);

    PosLatNV_Y.s = state;
    IDSetNumber(&PosLatNV_Y, NULL);

    PosLatNV_Z.s = state;
    IDSetNumber(&PosLatNV_Z, NULL);

    PosRotNV_W.s = state;
    IDSetNumber(&PosRotNV_W, NULL);

    PosRotNV_U.s = state;
    IDSetNumber(&PosRotNV_U, NULL);

    PosRotNV_V.s = state;
    IDSetNumber(&PosRotNV_V, "SETTING THE STATES AS REQUESTED");
}

/*
 * Query whether the controller is ready to accept motion commands.
 * The corresponding INDI axis properties are updated to reflect
 * the ready state reported by the controller.
 */
bool Secondary::SetReadyState()
{
    INDI::PropertyViewNumber *IAxis;
    Axis *iter;
    char name[] = "PosX";
    int isReady = 0;

    if (ID < 0)
        return false;

    if (!PI_IsControllerReady(ID, &isReady))
    {
        isConnected();
        return false;
    }

    for (iter = Pos; iter != &Pos[6]; iter++)
    {
        name[3] = iter->letter[0];
        IAxis = getProperty(name).getNumber();
        if (!isReady)
            IAxis->s = IPS_BUSY;
        IDSetNumber(IAxis, NULL);
    }

    return (bool)isReady;
}

/*
 * Query whether each hexapod axis is currently moving.
 * The corresponding INDI axis properties are updated so
 * clients can see which axes are busy.
 */
bool Secondary::SetMoveState()
{
    INDI::PropertyViewNumber *IAxis;
    Axis *iter;
    char name[] = "PosX";
    int isMoving = 0;

    if (ID < 0)
        return false;

    for (iter = Pos; iter != &Pos[6]; iter++)
    {
        name[3] = iter->letter[0];

        if (!PI_IsMoving(ID, iter->letter, &isMoving))
        {
            isConnected();
            return false;
        }

        IAxis = getProperty(name).getNumber();
        if (isMoving)
            IAxis->s = IPS_BUSY;
        else
            IAxis->s = IPS_OK;
        IDSetNumber(IAxis, NULL);
    }

    return (bool)isMoving;
}

/*
 * Copy the contents of one axis array into another.
 * This is used to preserve requested, corrected, and
 * current hexapod positions as separate state buffers.
 */
void Secondary::deepcopy(Axis *copyto, Axis *copyfrom)
{
    for (int ii = 0; ii < 6; ii++)
    {
        copyto[ii].letter = copyfrom[ii].letter;
        copyto[ii].ii = copyfrom[ii].ii;
        copyto[ii].pos = copyfrom[ii].pos;
    }
}

/*
 * Update the temperature and elevation values used for
 * autocollimation. These values are read from projectsoft
 * and copied into the corresponding INDI properties when
 * the returned values are valid.
 */
int Secondary::GetTempAndEl()
{
    float ps_el = 0;
    float ps_strut = 0;

    if (vatttel_counter <= MAX_VATTTEL_CNT)
    {
        vatttel_counter++;
        return 0;
    }
    else
    {
        vatttel_counter = 0;
    }

    int pserr = projectsoft_el_strut(&ps_el, &ps_strut);
    if (pserr == PROJECTSOFT_ERROR)
    {
        IDMessage(getDeviceName(), "**************************");
        corrS[0].s = ISS_OFF;
        corrSV.s = IPS_IDLE;
        IDSetSwitch(&corrSV, "Communication Error with Projectsoft, turning off autocollimation");
        return -1;
    }

    double dummy_temp;
    double dummy_el;
    bool badread = true;

    dummy_el = (double)ps_el;

    if (dummy_el > 90.0)
        dummy_el = 89.9999;
    dummy_el *= (double)3.14159 / 180.0;

    dummy_temp = ps_strut;

    if (dummy_temp > -30.0 && dummy_temp < 30.0)
    {
        temp = dummy_temp;
        badread = false;
        TempElN[0].value = temp;
        IDSetNumber(&TempElNV, NULL);
    }
    else
    {
        IDMessage(getDeviceName(), "Temperature read was bad: %f", dummy_temp);
        if (TempElN[0].value > -30.0 && TempElN[0].value < 30.0)
            temp = TempElN[0].value;
    }

    if (dummy_el > 0 && dummy_el < 3.14159 / 2.0 + 0.1 * 3.14159 / 180)
    {
        el = dummy_el;
        TempElN[1].value = el * 180 / 3.14159;
        IDSetNumber(&TempElNV, NULL);
        badread = false;
    }
    else
    {
        IDMessage(getDeviceName(), "Elevation read was bad: %f", dummy_el);
        el = TempElN[1].value * 3.14159 / 180.0;
    }

    if (badread)
    {
        IDMessage(getDeviceName(), "Erroneous temp or elevation read temp=%f el=%f", dummy_temp, dummy_el);
        return -1;
    }

    return 0;
}

/*
 * Move the hexapod to the position currently stored in NextPos.
 */
bool Secondary::MoveNext()
{
    MoveAbs(ID, NextPos);
    return true;
}