/*This is a EPICS driver for the attocube FPS31010 module

Project: SSRF beamline Control Group ioc driver for FPS3010

Author: Zhang Zhaohong

date: 2016-1-7

*/


#include <iocsh.h>
#include <asynDriver.h>
#include <asynPortDriver.h>
#include <epicsExport.h>
#include <iostream>
#include <fps3010.h>

using namespace std;
int fpsDebug;

void fpsStatePrint(int status);


static const char* driverName = "blcfpszzhDriver";



class blcfps : public asynPortDriver 
{
	
public:
	blcfps(const char* portName, int devNo);
	~blcfps();
	virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
	virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);

protected:
	int adjust1;
	int align2;
	int axisValid3;
	int axisSignalWeak4;
	int getPosition5;
	int reset6;

private:
	FPS_InterfaceType type;
	unsigned int devNum;
	unsigned int devNo;
	bln32  adjust;
    bln32  align;
	bln32  valid;
	bln32  error;
	double position;
	
};

//the class constructor function

blcfps::blcfps(const char* portName, int devNo_):
	asynPortDriver(portName,				//port name 
		3,									//max addrs
		6, 									//max params
		asynFloat64Mask | asynInt32Mask | asynOctetMask | asynDrvUserMask,	//interfaces to be implement
		0,													//interrupt
		ASYN_MULTIDEVICE | ASYN_CANBLOCK, 					//if multidevice and if canblock
		1, 						//autoconnect
		0,						//default priority
		0),						//default stack size
	type(IfUsb)					//initiate
{
	
	devNo = devNo_;
	int status;
	
	/** Discover devices
 *
 *  The function searches for connected FPS3010 devices on USB and LAN and
 *  initializes internal data structures per device. Devices that are in use
 *  by another application or PC are not found.
 *  The function must be called before connecting to a device and must not be called
 *  as long as any devices are connected.
 *
 *  The number of devices found is returned. In subsequent functions, devices
 *  are identified by a sequence number that must be less than the number returned.
 *  @param   ifaces    Interfaces where devices are to be searched
 *  @param   devCount  Output: number of devices found
 *  @return            Error code
 */
	//discover the devices, initialize the devNum
	
	status = FPS_discover( type, &devNum );
	
/** Connect device
 *
 *  Initializes and connects the selected device.
 *  This has to be done before any access to control variables or measured data.
 *  @param  devNo      Sequence number of the device
 *  @return            Error code
 */	
	
	//connect to the device of devNo
	
	status = FPS_connect( devNo );

/** Start adjustment
 *
 *  Starts the internal adjustment procedure. The procedure will
 *  run for about one minute and finish autonomously. It cannot
 *  be stopped or interrupted by the control PC.
 *  Use @ref FPS_getDeviceStatus to inquire the adjustment status.
 *  During the adjstment, no valid position data will be delivered.
 *  @param  devNo      Sequence number of the device
 *  @return            Error code
 */	
	//start internal adjustment procedure, which will run for about one minute
	
	status = FPS_startAdjustment( devNo );

	
	createParam("adjust", asynParamInt32, &adjust1);
	createParam("align", asynParamInt32, &align2);
	createParam("axisValid", asynParamInt32, &axisValid3);
	createParam("axisSignalWeak", asynParamInt32, &axisSignalWeak4);
	createParam("getPosition", asynParamFloat64, &getPosition5);
	createParam("reset", asynParamInt32, &reset6);
		
}
//interface readInt32

asynStatus blcfps :: readInt32(asynUser *pasynUser, epicsInt32 *value)
{
	
    int function = pasynUser->reason;
    int addr=0;
    asynStatus status = asynSuccess;
    epicsTimeStamp timeStamp; getTimeStamp(&timeStamp);
    static const char *functionName = "readInt32";
	
	status = getAddress(pasynUser, &addr); if (status != asynSuccess) return(status);
    
	/*Read device status*/
	
/** Read device status
 *
 *  Reads status information about the device.
 *  @param  devNo      Sequence number of the device
 *  @param  adjust     Output: If the internal adjustment procedure
 *                             is carried out currently
 *  @param  align      Output: If the device is in alignment mode
 *  @return            Error code
 */	
 
	if( function == adjust1)
	{
	int status;
	status = FPS_getDeviceStatus( devNo, &adjust, &align );	
	setIntegerParam( addr, adjust1, adjust );
	setIntegerParam( addr, align2, align );
	}
	
/** Read axis status
 *
 *  Reads status information belonging to an axis from the device.
 *  @param  devNo      Sequence number of the device
 *  @param  axisNo     Axis number (0 ... 2)
 *  @param  valid      Output: If the axis is successfully aligned
 *  @param  error      Output: If the signal quality is insufficient
 *  @return            Error code
 */
 
	//Read the axis valid and error state
	
	if( function == axisValid3)
	{
	int status;
	status = FPS_getAxisStatus( devNo, addr, &valid, &error );	
	setIntegerParam( addr, axisValid3, valid );
	setIntegerParam( addr, axisSignalWeak4, error );
	}
	
    status = (asynStatus) getIntegerParam(addr, function, value);
    
	/* Set the timestamp */
	
    pasynUser->timestamp = timeStamp;
    
	if (status) 
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, value=%d", 
                  driverName, functionName, status, function, *value);
    else        
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, value=%d\n", 
              driverName, functionName, function, *value);
    return(status);
	
}


//interface wtiteInt32

asynStatus blcfps::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
	
    int function = pasynUser->reason;
    int addr=0;
    asynStatus status = asynSuccess;
    const char* functionName = "writeInt32";

    status = getAddress(pasynUser, &addr); if (status != asynSuccess) return(status);


/** Reset axis
 *
 *  Sets the position value of an axis to 0 and clears the error flag.
 *  @param  devNo      Sequence number of the device
 *  @param  axisNo     Axis number (0 ... 2)
 *  @return            Error code
 */	
 
	if(function==reset6)
	{
		
	int status;
	status = FPS_resetAxis( devNo, addr );
	fpsStatePrint(status);
	
	}

    /* Set the parameter in the parameter library. */
	
    status = (asynStatus) setIntegerParam(addr, function, value);

    /* Do callbacks so higher layers see any changes */
	
    status = (asynStatus) callParamCallbacks(addr, addr);
    
    if (status) 
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, value=%d", 
                  driverName, functionName, status, function, value);
    else        
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, value=%d\n", 
              driverName, functionName, function, value);
    return status;
	
}

//interface readFloat64

asynStatus blcfps :: readFloat64(asynUser *pasynUser, epicsFloat64 *value)
{
	
    int function = pasynUser->reason;
    int addr=0;
    asynStatus status = asynSuccess;
    epicsTimeStamp timeStamp; getTimeStamp(&timeStamp);
    static const char *functionName = "readFloat64";
    
    status = getAddress(pasynUser, &addr); if (status != asynSuccess) return(status);

	
	//get axis position
	
	if( function == getPosition5)
	{
		
	int status;
	status = FPS_getPosition( devNo, addr, &position );
	setDoubleParam( addr, getPosition5, position );
	
	}
/** Read position
 *
 *  Reads the measured position of an axis.
 *  @param  devNo      Sequence number of the device
 *  @param  axisNo     Axis number (0 ... 2)
 *  @param  position   Output: current position (nm)
 *  @return            Error code
 */	
	
    status = (asynStatus) getDoubleParam(addr, function, value);
	
    /* Set the timestamp */
	
    pasynUser->timestamp = timeStamp;

    
    if (status) 
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, value=%f", 
                  driverName, functionName, status, function, *value);
    else        
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, value=%f\n", 
              driverName, functionName, function, *value);
    return(status);
}



//the class destructor function

blcfps::~blcfps()
{
	//disconnect the device
	
	FPS_disconnect( devNo );
	
}


/******************The following is the code needn't to be modified*/ 
//banding to the epics iocsh shell

extern "C" int blcfpsConfigure(const char* portName, int devNo)
{
	
	blcfps *pblcfps = new blcfps(portName, devNo);
	return asynSuccess;
	
}

static const iocshArg blcfpsArg0 = {"Port name", iocshArgString};
static const iocshArg blcfpsArg1 = {"number", iocshArgInt};
static const iocshArg * const blcfpsArgs[] = {&blcfpsArg0, &blcfpsArg1};

static const iocshFuncDef blcfpsFuncDef = {"blcfpsConfigure", 2, blcfpsArgs};
static void blcfpsConfigCallFunc(const iocshArgBuf *args)
{
	
	blcfpsConfigure(args[0].sval, args[1].ival);
	
}

void drvblcfpsRegister(void)
{
	
	iocshRegister(&blcfpsFuncDef, blcfpsConfigCallFunc);
	
}

extern "C" {
	
	epicsExportRegistrar(drvblcfpsRegister);
	
}

extern "C" {epicsExportAddress(int, fpsDebug);}


void fpsStatePrint(int status){
	
	switch(status){
		
		case 0:
		
			cout << "FPS_ OK" << endl;
			break;
			
		case -1:
		
			cout << "Unspecified error" << endl;
			break;
			
		case 1:
		
			cout << "FPS timeout" << endl;
			break;
			
		case 2:
		
			cout<< "FPS_NotConnected" << endl;
			break;
			
		case 3:
		
			cout << "Error in comunication with driver" << endl;
			break;
			
		case 7:
		
			cout << "Device is already in use by other" << endl;
			break;
			
		case 8:
		
			cout << "Unknow error" << endl;
			break;
			
		case 9:
		
			cout << "Invalid device number in function call" << endl;
			break;
			
		case 10:
		
			cout << "Invalid axis number in function call" << endl;
			break;
			
	}
	
}