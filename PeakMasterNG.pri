# Path to location where the ECHMETCoreLibs are installed
ECHMET_CORE_LIBS_DIR = "/home/madcat/Devel/ECHMET/ECHMETCoreLibs-bin"

# Path to location where the LEMNG library is insalled
LEMNG_DIR = "/home/madcat/Devel/ECHMET/LEMNG-bin"

# Path to where the Qwt toolkit is installed
QWT_DIR = "/home/madcat/Devel/ECHMET/qwt-6.1.3-bin"

include("$$QWT_DIR/features/qwt.prf")

#Do not touch anything below this line!

INCLUDEPATH += "$$ECHMET_CORE_LIBS_DIR/include/ECHMET/CoreLibs"
INCLUDEPATH += "$$LEMNG_DIR/include/ECHMET/LEMNG"
LIBS += -L"$$ECHMET_CORE_LIBS_DIR/lib" -lSysComp -lECHMETShared
LIBS += -L"$$LEMNG_DIR/lib" -lLEMNG
QMAKE_LFLAGS += -Wl,-rpath-link,"$$ECHMET_CORE_LIBS_DIR/lib"
