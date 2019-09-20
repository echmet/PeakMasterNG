WINARCH=""
win32_i386 {
    WINARCH=32
}
win32_x86_64 {
    WINARCH=64
}

# Path to location where the ECHMETCoreLibs are installed
ECHMET_CORE_LIBS_DIR = "/home/madcat/Devel/ECHMET/ECHMETCoreLibs-bin"

# Path to location where the LEMNG library is insalled
LEMNG_DIR = "/home/madcat/Devel/ECHMET/LEMNG-bin"

# Path to where the Qwt toolkit is installed
QWT_DIR = "/home/madcat/Devel/ECHMET/qwt-6.1.3-bin"

# Path to ECHMETUpdateCheck library installation
EUPD_DIR = "/home/madcat/Devel/ECHMET/ECHMETUpdateCheck-bin"

include("$$QWT_DIR/features/qwt.prf")

#Do not touch anything below this line!
INCLUDEPATH += "$$ECHMET_CORE_LIBS_DIR/include/ECHMET/CoreLibs"
INCLUDEPATH += "$$LEMNG_DIR/include/ECHMET/LEMNG"
LIBS += -L"$$ECHMET_CORE_LIBS_DIR/lib" -lSysComp -lECHMETShared -lCAES -lIonProps
LIBS += -L"$$LEMNG_DIR/lib" -lLEMNG
QMAKE_LFLAGS += -Wl,-rpath-link,"$$ECHMET_CORE_LIBS_DIR/lib"

SQLITE_DIR = "C:/Building/sqlite3-w$$WINARCH"
INCLUDEPATH += $$SQLITE_DIR
LIBS += -L"$$SQLITE_DIR" -lsqlite3

INCLUDEPATH += "$$EUPD_DIR/include"
LIBS += -L"$$EUPD_DIR/lib" -lECHMETUpdateCheck
