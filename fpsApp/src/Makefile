TOP=../..

INC +=fps3010.h
ifeq (windows-x64, $(findstring windows-x64, $(T_A)))
LIB_INSTALLS += ../fps3010.lib

BIN_INSTALLS += ../fps3010.dll
BIN_INSTALLS += ../libusb0.dll
endif

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = fps
# fps.dbd will be created and installed
DBD += fps.dbd

# fps.dbd will be made up from these files:
fps_DBD += base.dbd
fps_DBD += asyn.dbd
fps_DBD += fpsSupport.dbd

# Include dbd files from all support applications:
#fps_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#fps_LIBS += xxx
fps_LIBS += asyn
fps_LIBS += fps3010
fps_LIBS +=

fps_SRCS += drvfps.cpp
# fps_registerRecordDeviceDriver.cpp derives from fps.dbd
fps_SRCS += fps_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
fps_SRCS_DEFAULT += fpsMain.cpp
fps_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#fps_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
fps_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

