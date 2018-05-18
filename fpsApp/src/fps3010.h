/*****************************************************************************
 *
 *  Project:        FPS3010 Control Library
 *
 *  Filename:       fps3010.h
 *
 *  Author:         NHands GmbH & Co KG
 */
/*****************************************************************************/
/** @mainpage Custom Programming Library for FPS3010
 *
 *  @ref fps3010.h "FPS3010 Control Library" is a library that allows
 *  custom programming for the FPS3010 interferometer.
 *  It can manage multiple devices that are connected to the PC via USB
 *  or ethernet. Corresponding to the very simple device handling only
 *  a very small number of functions is necessary.
 *
 *  Documentation for the functions can be found \ref fps3010.h "here".
 */
/*****************************************************************************/
/** @file fps3010.h
 *
 *  @brief Control and acquisition functions for FPS3010
 *  
 *  Defines functions for connecting and controlling the FPS3010.
 *  The functions are not thread safe!
 */
/******************************************************************/
/* $Id: fps3010.h,v 1.15 2014/09/08 13:07:50 zaphod Exp $ */

#ifndef __FPS3010_H__
#define __FPS3010_H__

/** @name Function declarations
 *
 *  Technical definitions, mainly for the Windows DLL interface.
 *  @{
 */
#ifdef __cplusplus
#define EXTC extern "C"                  /**< Storage class for C++                  */
#else
#define EXTC extern                      /**< Storage class for C                    */
#endif

#ifdef unix
#define FPS_API EXTC                     /**< Not required for Unix                  */
#define WINCC                            /**< Not required for Unix                  */
#else
#define WINCC __stdcall                  /**< Calling convention for Windows         */
#ifdef  FPS_EXPORTS
#define FPS_API EXTC __declspec(dllexport)   /**< For internal use of this header    */
#else
#define FPS_API EXTC __declspec(dllimport)   /**< For external use of this header    */
#endif
#endif
/* @} */

typedef int bln32;                       /**< Boolean; to avoid troubles with
                                              incomplete C99 implementations         */

/** @name Return values of the functions
 *
 *  All functions of this lib - as far as they can fail - return
 *  one of these constants for success control.
 *  @{
 */
#define FPS_Ok             0             /**< No error                               */
#define FPS_Error         -1             /**< Unspecified error                      */
#define FPS_Timeout        1             /**< Communication timeout                  */
#define FPS_NotConnected   2             /**< No active connection to device         */
#define FPS_DriverError    3             /**< Error in comunication with driver      */
#define FPS_DeviceLocked   7             /**< Device is already in use by other      */
#define FPS_Unknown        8             /**< Unknown error                          */
#define FPS_NoDevice       9             /**< Invalid device number in function call */
#define FPS_NoAxis        10             /**< Invalid axis number in function call   */
/* @} */


/** @name FPS Feature Flags
 *  @anchor FFlags
 *
 *  The flags describe optional features of the FPS device.
 *  They are used by @ref FPS_getDeviceConfig .
 *  @{
 */
#define FPS_FeatureSync    0x01          /**< "Sync": Ethernet enabled               */
#define FPS_FeatureAngle   0x02          /**< "Angle": Angular measurement           */
#define FPS_FeatureMarker  0x04          /**< "DataMarker": digital inputs           */
#define FPS_FeatureEcu     0x08          /**< "ECU": Environmental compensation      */
/* @} */
 

/** Interface types
 */
typedef enum { IfNone = 0x00,            /**< Device invalid / not connected         */
               IfUsb  = 0x01,            /**< Device connected via USB               */
               IfTcp  = 0x02,            /**< Device connected via ethernet (TCP/IP) */
               IfAll  = 0x03             /**< All physical interfaces                */
} FPS_InterfaceType;


/** Position callback function
 *
 *  A function of this type can be registered as callback function for new
 *  position values with @ref FPS_setPositionCallback .

 *  The position measurements are taken on a regular timebase, the results are
 *  buffered for a short time and packed together to fit the requirements of the
 *  transport medium. When the packet arrives at the PC, the callback function is
 *  called as soon as possible. If the PC cannot process the data fast enough, the
 *  delay between measurement and call will increase with the filling level of the
 *  device internal buffers. Data packets will get lost when the buffers are full.
 *
 *  The index counts the data since the begin of the measurement, i.e. it is
 *  incremented from call to call by length. It also counts data that have been
 *  lost due to performance problems of the control PC. To avoid overflow, the
 *  index is resetted from time to time.
 *
 *  The buffer that contains the data is static and will be overwritten in
 *  the next call. It must not be free()'d or used by the application to
 *  store data.
 *  @param  devNo        Number of the device that produced the data
 *  @param  length       Number of triples of position values
 *  @param  index        Sequence number of the first position of the packet
 *  @param  positions    Array of three pointers, pointing to arrays of
 *                       positions [pm] of axes 1, 2, and 3, respectively.
 *  @param  markers      Data marker Flags corresponding to the positions
 *                       at the same indices. Empty if the feature isn't enabled.
 */
typedef void (* FPS_PositionCallback) ( unsigned int   devNo,
                                        unsigned int   length,
                                        unsigned int   index,
                                        const double * const positions[3],
                                        const bln32  * const markers[3]   );


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
FPS_API int WINCC FPS_discover( FPS_InterfaceType ifaces,
                                unsigned int    * devCount );


/** Device information
 *
 *  Returns available information about a device. The function can not be called
 *  before @ref FPS_discover but the devices don't have to be @ref FPS_connect "connected" .
 *  All Pointers to output parameters may be zero to ignore the respective value.
 *  @param   devNo     Sequence number of the device
 *  @param   id        Output: programmed hardware ID of the device
 *  @param   address   Output: The device's interface address if applicable.
 *                     Returns the IP address in dotted-decimal notation or the
 *                     string "USB", respectively. The string buffer should be
 *                     at least 16 bytes long.
 *  @param   connected Output: if the device is connected
 *  @return            Error code
 */
FPS_API int WINCC FPS_getDeviceInfo( unsigned int devNo,
                                     int        * id,
                                     char       * address,
                                     bln32      * connected );


/** Connect device
 *
 *  Initializes and connects the selected device.
 *  This has to be done before any access to control variables or measured data.
 *  @param  devNo      Sequence number of the device
 *  @return            Error code
 */
FPS_API int WINCC FPS_connect( unsigned int devNo );


/** Disconnect device
 *
 *  Closes the connection to the device.
 *  @param  devNo      Sequence number of the device
 *  @return            Error code
 */
FPS_API int WINCC FPS_disconnect( unsigned int devNo );


/** Read device configuration
 *
 *  Reads static device configuration data
 *  @param  devNo      Sequence number of the device
 *  @param  axisCount  Output: Number of enabled axes
 *  @param  features   Output: Bitfield of enabled features,
 *                     see @ref FFlags "FPS Feature Flags"
 *  @return            Error code
 */
FPS_API int WINCC FPS_getDeviceConfig( unsigned int   devNo,
                                       unsigned int * axisCount,
                                       int          * features   );


/** Read device status
 *
 *  Reads status information about the device.
 *  @param  devNo      Sequence number of the device
 *  @param  adjust     Output: If the internal adjustment procedure
 *                             is carried out currently
 *  @param  align      Output: If the device is in alignment mode
 *  @return            Error code
 */
FPS_API int WINCC FPS_getDeviceStatus( unsigned int devNo,
                                       bln32      * adjust,
                                       bln32      * align );


/** Read axis status
 *
 *  Reads status information belonging to an axis from the device.
 *  @param  devNo      Sequence number of the device
 *  @param  axisNo     Axis number (0 ... 2)
 *  @param  valid      Output: If the axis is successfully aligned
 *  @param  error      Output: If the signal quality is insufficient
 *  @return            Error code
 */
FPS_API int WINCC FPS_getAxisStatus( unsigned int devNo,
                                     unsigned int axisNo,
                                     bln32      * valid,
                                     bln32      * error );


/** Read ECU sensor data
 *
 *  Reads current output of the ECU sensors and the index of refraction
 *  calculated from these data. If the ECU option is not configured or
 *  the sensor is not plugged in, default values are output.
 *  ECU data are updated internally every 100ms, it is useless to poll
 *  them more often.
 *  @param  devNo      Sequence number of the device
 *  @param  t          Output: Temperature in degree centigrade, default = 0
 *  @param  p          Output: Air pressure in Pascal, default = 0
 *  @param  h          Output: Relative humidity in Percent, default = 0
 *  @param  n          Ouptut: Calculated index of refraction, default = 1
 *  @return            Error code
 */
FPS_API int WINCC FPS_getEcuData( unsigned int devNo,
                                  double     * t,
                                  double     * p,
                                  double     * h,
                                  double     * n  );


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
FPS_API int WINCC FPS_startAdjustment( unsigned int devNo );


/** Reset axis
 *
 *  Sets the position value of an axis to 0 and clears the error flag.
 *  @param  devNo      Sequence number of the device
 *  @param  axisNo     Axis number (0 ... 2)
 *  @return            Error code
 */
FPS_API int WINCC FPS_resetAxis( unsigned int devNo,
                                 unsigned int axisNo );


/** Reset axes synchronized
 *
 *  Resets all axes at the same time.
 *  The position values are set to 0 and the error flags are cleared.
 *  @param  devNo      Sequence number of the device
 *  @return            Error code
 */
FPS_API int WINCC FPS_resetAxes( unsigned int devNo );


/** Read position
 *
 *  Reads the measured position of an axis.
 *  @param  devNo      Sequence number of the device
 *  @param  axisNo     Axis number (0 ... 2)
 *  @param  position   Output: current position (nm)
 *  @return            Error code
 */
FPS_API int WINCC FPS_getPosition( unsigned int devNo,
                                   unsigned int axisNo,
                                   double     * position );

/** Read positions synchronized
 *
 *  Reads the measured positions of all axes at the same time.
 *  @param  devNo      Sequence number of the device
 *  @param  positions  Output: current positions (nm) of axes 1, 2, and 3.
 *                     The caller must provide an array of 3 Elements.
 *  @return            Error code
 */
FPS_API int WINCC FPS_getPositions( unsigned int devNo,
                                    double     * positions );


/** Read positions and markers synchronized
 *
 *  Reads the measured positions and markers of all axes at the same time.
 *  The marker flag is always false if the Marker feature is not enabled.
 *  @param  devNo      Sequence number of the device
 *  @param  positions  Output: current positions (nm) of axes 1, 2, and 3.
 *                     The caller must provide an array of 3 Elements.
 *  @param  markers    Output: current data markers of axes 1, 2, and 3.
 *                     The caller must provide an array of 3 Elements or
 *                     a NULL pointer.
 *  @return            Error code
 */
FPS_API int WINCC FPS_getPositionsAndMarkers( unsigned int devNo,
                                              double     * positions,
                                              bln32      * markers );


/** Register callback function
 *
 *  Registers a callback function for a device that will be called when new
 *  position data are available. A callback function registered previously
 *  is unregistered.
 *  @param  devNo      Sequence number of the device
 *  @param  callback   Callback function for the device. Use NULL to
 *                     unregister a function.
 *  @param  lbSmpTime  Logarithmic time distance of two subsequent position
 *                     measurements. The actual sample time can be varied
 *                     only in powers of two:
 *                     sample time = (2 ^ lbSmpTime) * 10.24us .
 *                     Allowed range is from 0 to 20.
 *                     The sample time directly affects the data rate!
 */
FPS_API int WINCC FPS_setPositionCallback( unsigned int         devNo,
                                           FPS_PositionCallback callback,
                                           unsigned int         lbSmpTime );


/** Set position average
 *
 *  Reads position average of an axis.
 *  @param  devNo      Sequence number of the device
 *  @param  axisNo     Axis number (0 ... 2)
 *  @param  average    Time over which each position sample is averaged in ns for following functions:
 *                     FPS_getPosition, FPS_getPositions, FPS_getPositionsAndMarkers.
 *                     FPS_setPositionCallback is not affected by this parameter!
 *                     The range is from 0,08µs to 2621.44µs. The value is quantized to 2^n * 0.08µs
 *                     with n ranging from 0..15. The devices approximates the value to the next matching one.
 *				       The actual used value can be read back with FPS_getPosAverage. Values exceeding the 
 *					   allowed range are ignored.
 *  @return            Error code
 */
FPS_API int WINCC FPS_setPosAverage( unsigned int devNo,
                                     unsigned int axisNo,
                                     unsigned int average );

/** Read position average
 *
 *  Reads position average of an axis.
 *  @param  devNo      Sequence number of the device
 *  @param  axisNo     Axis number (0 ... 2)
 *  @param  average    Time over which each position sample is averaged in ns.
 *  @return            Error code
 */
FPS_API int WINCC FPS_getPosAverage( unsigned int devNo,
                                     unsigned int axisNo,
                                     unsigned int* average );



#endif 

