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
ECHMET_CORE_LIBS_DIR = "/home/madcat/Devel/ECHMET/ECHMETCoreLibs-bin"

# Path to location where the LEMNG library is insalled
LEMNG_DIR = "/home/madcat/Devel/ECHMET/LEMNG-bin"

# Path to where the Qwt toolkit is installed
QWT_DIR = "/home/madcat/Devel/qwt-6.2.0-dev"

# Path to ECHMETUpdateCheck library installation
EUPD_DIR = "/home/madcat/Devel/ECHMET/ECHMETUpdateCheck-bin"

# Path to Sqlite installation
SQLITE_DIR = ""

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
   LIBS += -L"$$ECHMET_CORE_LIBS_DIR/lib64" -lSysComp -lECHMETShared -lCAES -lIonProps
   LIBS += -L"$$LEMNG_DIR/lib64" -lLEMNG
}

unix {
    QMAKE_LFLAGS += -Wl,-rpath-link,"$$ECHMET_CORE_LIBS_DIR/lib64"
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
