# Path to location where the ECHMETCoreLibs are installed
ECHMET_CORE_LIBS_DIR = "/home/madcat/Devel/ECHMET/ECHMETCoreLibs-bin_OF"

# Path to location where the LEMNG library is insalled
LEMNG_DIR = "/home/madcat/Devel/ECHMET/LEMNG-bin_OF"

# Path to where the Qwt toolkit is installed
QWT_DIR = "/home/madcat/Devel/ECHMET/qwt-6.1.3-bin"

# Path to ECHMETUpdateCheck library installation
EUPD_DIR = /home/madcat/Devel/ECHMET/ECHMETUpdateCheck-bin

include("$$QWT_DIR/features/qwt.prf")

#Do not touch anything below this line!

INCLUDEPATH += "$$ECHMET_CORE_LIBS_DIR/include/ECHMET/CoreLibs"
INCLUDEPATH += "$$LEMNG_DIR/include/ECHMET/LEMNG"
INCLUDEPATH += "$$EUPD_DIR/include"
LIBS += -L"$$ECHMET_CORE_LIBS_DIR/lib" -lSysComp -lECHMETShared
LIBS += -L"$$LEMNG_DIR/lib" -lLEMNG
LIBS += -L"$$EUPD_DIR/lib" -lECHMETUpdateCheck
QMAKE_LFLAGS += -Wl,-rpath-link,"$$ECHMET_CORE_LIBS_DIR/lib"
