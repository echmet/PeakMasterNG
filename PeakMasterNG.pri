ECHMET_CORE_LIBS_DIR = "/home/madcat/Devel/ECHMET/ECHMETCoreLibs-bin/"
LEMNG_DIR = "/home/madcat/Devel/ECHMET/LEMNG-bin/"
QWTPATH = "/home/madcat/Devel/ECHMET/qwt-6.1.3-bin"

include("$$QWTPATH/features/qwt.prf")

#Do not touch anything below this line!

INCLUDEPATH += "$$ECHMET_CORE_LIBS_DIR/include/ECHMET/CoreLibs"
INCLUDEPATH += "$$LEMNG_DIR/include/ECHMET/LEMNG"
LIBS += -L"$$ECHMET_CORE_LIBS_DIR/lib" -lSysComp -lECHMETShared -lCAES -lIonProps
LIBS += -L"$$LEMNG_DIR/lib" -lLEMNG
