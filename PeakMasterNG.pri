WINARCH=""
win32_i386 {
    WINARCH=32
}
win32_x86_64 {
    WINARCH=64
}

# Allowed values are 0 and 1. Set to 1 if you built custom dependencies (ECHMETCoreLibs, LEMNG, ECHMETUpdateCheck)
# with MSVC to avoid linking errors due to MSVC different naming policy
BACKEND_BUILT_BY_MSVC=0

# Path to location where the ECHMETCoreLibs are installed
ECHMET_CORE_LIBS_DIR = "C:/Building/ECHMET/ECHMETCoreLibs-bin-w$$WINARCH"

# Path to location where the LEMNG library is insalled
LEMNG_DIR = "C:/Building/ECHMET/LEMNG-bin-w$$WINARCH"

# Path to where the Qwt toolkit is installed
QWT_DIR = "C:/Building/Qwt-6.2.0-msvc-bin-w$$WINARCH"

# Path to ECHMETUpdateCheck library installation
EUPD_DIR = "C:/Building/ECHMET/ECHMETUpdateCheck-bin-w$$WINARCH"

# Path to Sqlite installation
SQLITE_DIR = "C:/Building/sqlite3-w$$WINARCH"

include("$$QWT_DIR/features/qwt.prf")

#Do not touch anything below this line!
INCLUDEPATH += "$$ECHMET_CORE_LIBS_DIR/include/ECHMET/CoreLibs"
INCLUDEPATH += "$$LEMNG_DIR/include/ECHMET/LEMNG"

win32-msvc* {
    equals(BACKEND_BUILT_BY_MSVC, 1) {
      LIBS += -L"$$ECHMET_CORE_LIBS_DIR/lib" -lSysComp -lECHMETShared -lCAES -lIonProps
      LIBS += -L"$$LEMNG_DIR/lib" -lLEMNG
    } else {
      LIBS += -L"$$ECHMET_CORE_LIBS_DIR/lib" -llibSysComp -llibECHMETShared -llibCAES -llibIonProps
      LIBS += -L"$$LEMNG_DIR/lib" -llibLEMNG
    }
} else {
   LIBS += -L"$$ECHMET_CORE_LIBS_DIR/lib" -lSysComp -lECHMETShared -lCAES -lIonProps
   LIBS += -L"$$LEMNG_DIR/lib" -lLEMNG
}

unix {
    QMAKE_LFLAGS += -Wl,-rpath-link,"$$ECHMET_CORE_LIBS_DIR/lib"
}

INCLUDEPATH += $$SQLITE_DIR
isEmpty(SQLITE_DIR) {
    LIBS += -lsqlite3
} else {
    LIBS += -L"$$SQLITE_DIR" -lsqlite3
}

INCLUDEPATH += "$$EUPD_DIR/include"

win32-msvc* {
    equals(BACKEND_BUILT_BY_MSVC, 1) {
        LIBS += -L"$$EUPD_DIR/lib" -lECHMETUpdateCheck
    } else {
        LIBS += -L"$$EUPD_DIR/lib" -llibECHMETUpdateCheck
    }
} else {
    LIBS += -L"$$EUPD_DIR/lib" -lECHMETUpdateCheck
}
