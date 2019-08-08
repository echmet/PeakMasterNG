WINARCH=""
win32_i386 {
    WINARCH=32
}
win32_x86_64 {
    WINARCH=64
}

# Path to location where the ECHMETCoreLibs are installed
ECHMET_CORE_LIBS_DIR = "C:/Building/ECHMET/ECHMETCoreLibs-bin-w$$WINARCH/"

# Path to location where the LEMNG library is insalled
LEMNG_DIR = "C:/Building/ECHMET/LEMNG-bin-w$$WINARCH-msvc/"

# Path to where the Qwt toolkit is installed
QWT_DIR = "C:/Building/Qwt-6.1.3-msvc-bin-w$$WINARCH"

# Path to ECHMETUpdateCheck library installation
EUPD_DIR = "C:/Building/ECHMET/ECHMETUpdateCheck-bin-w$$WINARCH"

include("$$QWT_DIR/features/qwt.prf")

#Do not touch anything below this line!
INCLUDEPATH += "$$ECHMET_CORE_LIBS_DIR/include/ECHMET/CoreLibs"
INCLUDEPATH += "$$LEMNG_DIR/include/ECHMET/LEMNG"
LIBS += -L"$$ECHMET_CORE_LIBS_DIR/lib" -lSysComp -lECHMETShared
LIBS += -L"$$LEMNG_DIR/lib" -lLEMNG

SQLITE_DIR = "C:/Building/sqlite3-w$$WINARCH"
INCLUDEPATH += $$SQLITE_DIR
LIBS += -L"$$SQLITE_DIR" -lsqlite3

INCLUDEPATH += "$$EUPD_DIR/include"
LIBS += -L"$$EUPD_DIR/lib" -lECHMETUpdateCheck
