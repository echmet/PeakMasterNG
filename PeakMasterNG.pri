ECHMET_CORE_LIBS_DIR = "/home/madcat/Devel/ECHMET/ECHMETCoreLibs-bin/"
LEMNG_INCLUDE = "/home/madcat/Devel/ECHMET/LEMNG/include"
LEMNG_BIN = "/home/madcat/Devel/ECHMET/LEMNG/build"
QWTPATH = "/home/madcat/Devel/ECHMET/qwt-6.1.3-bin"

include("$$QWTPATH/features/qwt.prf")

DEFINES += ECHMET_COMPILER_GCC_LIKE


#Do not touch anything below this line!

INCLUDEPATH += $$ECHMET_CORE_LIBS_DIR/include/ECHMET/CoreLibs
INCLUDEPATH += $$LEMNG_INCLUDE
LIBS += -L"$$ECHMET_CORE_LIBS_DIR/lib" -lSysComp -lECHMETShared
LIBS += -L"$$LEMNG_BIN" -lLEMNG
