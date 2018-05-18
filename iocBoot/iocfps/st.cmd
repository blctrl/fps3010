#!../../bin/windows-x64/fps

## You may have to change fps to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/fps.dbd"
fps_registerRecordDeviceDriver pdbbase

## Load record instances
#dbLoadRecords("db/xxx.db","user=blctrlHost")
dbLoadTemplate("fpsApp/Db/fps.substitution")

var fpsDebug 1
blcfpsConfigure("blc",0)

cd ${TOP}/iocBoot/${IOC}
iocInit

## Start any sequence programs
#seq sncxxx,"user=blctrlHost"
