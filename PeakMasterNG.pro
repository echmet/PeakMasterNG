#-------------------------------------------------
#
# Project created by QtCreator 2017-11-30T15:19:31
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = PeakMasterNG
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

win32 {
    DEFINES += CRASHHANDLING_WIN32
}

linux {
    DEFINES += CRASHHANDLING_LINUX
}

win32 {
    contains(QT_ARCH, i386) {
        CONFIG += win32_i386
    } else {
        CONFIG += win32_x86_64
    }
}

win32_i386 {
    DEFINES += _WIN32_WINNT=0x0501

    win32-msvc2015 {
        QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
        DEFINES += _WIN32_WINNT=0x0501 _USING_V110_SDK71_
    }
}

CONFIG(release, debug|release) {
    DEFINES += USE_CRASHHANDLER

    QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

    win32-msvc2015 {
        QMAKE_CXXFLAGS_RELEASE += /GL
        QMAKE_LFLAGS_RELEASE += /LTCG
    }
    linux {
        QMAKE_LFLAGS_RELEASE += "-rdynamic"
    }
}
CONFIG(debug, debug|release) {
    # Configuration specific for "debug" build

    xsanitize {
        CONFIG += sanitizer
        CONFIG += sanitize_address
        CONFIG += sanitize_undefined
        CONFIG += sanitize_leak
    }
}

SOURCES += \
    src/gearbox/phadjusterinterface.cpp \
    src/main.cpp \
    src/ui/adjustphdialog.cpp \
    src/ui/internal_models/adjustphtablemodel.cpp \
    src/ui/pmngmainwindow.cpp \
    src/gdm/conversion/conversion.cpp \
    src/gdm/core/common/chargeinterval.cpp \
    src/gdm/core/common/mobility.cpp \
    src/gdm/core/complexation/chargecombination.cpp \
    src/gdm/core/complexation/complexation.cpp \
    src/gdm/core/complexation/complexform.cpp \
    src/gdm/core/constituent/constituent.cpp \
    src/gdm/core/constituent/physicalproperties.cpp \
    src/gdm/core/detail/complexationmap.cpp \
    src/gdm/core/detail/complexationmapkey.cpp \
    src/gdm/core/basiccomposition.cpp \
    src/gdm/core/composition.cpp \
    src/gdm/core/gdm.cpp \
    src/ui/editconstituentdialog.cpp \
    src/ui/elementaries/modifyconstituentchargepushbutton.cpp \
    src/ui/internal_models/constituentchargesmodel.cpp \
    src/gearbox/gearbox.cpp \
    src/gearbox/samplegdmproxy.cpp \
    src/gearbox/gdmproxy.cpp \
    src/gearbox/backgroundgdmproxy.cpp \
    src/gearbox/calculatorinterface.cpp \
    src/gearbox/results_models/resultsmodels.cpp \
    src/gearbox/results_models/resultsdata.cpp \
    src/gearbox/results_models/resultscontainer.cpp \
    src/gearbox/results_models/systemeigenzonestablemodel.cpp \
    src/ui/elementaries/floatingvaluelineedit.cpp \
    src/gearbox/doubletostringconvertor.cpp \
    src/gearbox/floatingvaluedelegate.cpp \
    src/metatyperegistrator.cpp \
    src/gearbox/calculatorcontext.cpp \
    src/ui/signalplotwidget.cpp \
    src/ui/doubleclickableqwtplotzoomer.cpp \
    src/ui/maincontrolwidget.cpp \
    src/ui/systemcompositionwidget.cpp \
    src/ui/internal_models/abstractconstituentsmodelbase.cpp \
    src/gearbox/constituentmanipulator.cpp \
    src/gearbox/complexationmanager.cpp \
    src/gearbox/gdmproxy_p.cpp \
    src/ui/editcomplexationdialog.cpp \
    src/ui/complexation_models/complexationrelationship.cpp \
    src/ui/complexation_models/complexationrelationshipsmodel.cpp \
    src/ui/complexation_models/ligandionicform.cpp \
    src/ui/complexation_models/nucleusionicform.cpp \
    src/ui/complexation_models/constituent_ui.cpp \
    src/ui/complexationcolorizerdelegate.cpp \
    src/crashhandling/crashhandlerbase.cpp \
    src/crashhandling/crashhandlerlinux_stacktrace.cpp \
    src/crashhandling/crashhandlerlinux.cpp \
    src/crashhandling/crashhandlerwindows.cpp \
    src/crashhandling/nullcrashhandler.cpp \
    src/crashevent.cpp \
    src/crasheventcatcher.cpp \
    src/ui/crashhandlerdialog.cpp \
    src/globals.cpp \
    src/pmngcrashhandler.cpp \
    src/ui/aboutdialog.cpp \
    src/ui/eigenzonedetailsdialog.cpp \
    src/gearbox/results_models/eigenzonedetailsmodel.cpp \
    src/gearbox/results_models/ioniccompositionmodel.cpp \
    src/ui/ioniccompositiondialog.cpp \
    src/gearbox/calculatorworker.cpp \
    src/persistence/serializer.cpp \
    src/persistence/persistence.cpp \
    src/persistence/deserializer.cpp \
    src/ui/operationinprogressdialog.cpp \
    src/database/db_constituent.cpp \
    src/database/db_constituentsdatabase.cpp \
    src/gearbox/databaseproxy.cpp \
    src/ui/pickconstituentfromdbdialog.cpp \
    src/ui/internal_models/databaseconstituentsphyspropstablemodel.cpp \
    src/ui/ploteventfilter.cpp \
    src/ui/nonidealitycorrectionsdialog.cpp \
    src/gearbox/complexationparametersdelegate.cpp \
    src/gearbox/efgdisplayer.cpp \
    src/gearbox/efgcsvexporter.cpp \
    src/ui/csvexportoptionsdialog.cpp \
    src/ui/copiableitemstableview.cpp \
    src/ui/databasetableview.cpp \
    src/gearbox/results_models/analytesdissociationmodel.cpp \
    src/ui/editchargeswidget.cpp \
    src/ui/databaseeditordialog.cpp \
    src/ui/editdatabaseconstituentdialog.cpp \
    src/ui/toggletracepointsdialog.cpp \
    src/gearbox/results_models/analytesextrainfomodel.cpp \
    src/ui/analytesconstituentsheader.cpp \
    src/gearbox/results_models/backgroundeffectivemobilitiesmodel.cpp \
    src/ui/backgroundconstituentsheader.cpp \
    src/ui/elementaries/uihelpers.cpp \
    src/ui/checkforupdatedialog.cpp \
    src/ui/softwareupdatewidget.cpp \
    src/softwareupdater.cpp \
    src/softwareupdateresult.cpp \
    src/ui/autoupdatecheckdialog.cpp \
    src/gearbox/additionalfloatingvalidator.cpp \
    src/ui/returnacceptabletableview.cpp \
    src/ui/refocuser.cpp \
    src/persistence/swsettings.cpp \
    src/ui/adjustueffoverkbgedialog.cpp \
    src/ui/internal_models/adjustueffoverkbgetablemodel.cpp \
    src/gearbox/ueffoverkbgecalculatorinterface.cpp \
    src/ui/compositioneditorwidget.cpp \
    src/ui/backgroundcompositionwidget.cpp \
    src/ui/analytescompositionwidget.cpp

HEADERS += \
    src/gearbox/phadjusterinterface.h \
    src/ui/adjustphdialog.h \
    src/ui/internal_models/adjustphtablemodel.h \
    src/ui/pmngmainwindow.h \
    src/gdm/conversion/conversion.h \
    src/gdm/core/common/chargeinterval.h \
    src/gdm/core/common/chargenumber.h \
    src/gdm/core/common/constituenttype.h \
    src/gdm/core/common/gdmexcept.h \
    src/gdm/core/common/mobility.h \
    src/gdm/core/complexation/chargecombination.h \
    src/gdm/core/complexation/complexation.h \
    src/gdm/core/complexation/complexform.h \
    src/gdm/core/constituent/constituent.h \
    src/gdm/core/constituent/physicalproperties.h \
    src/gdm/core/detail/complexationmap.h \
    src/gdm/core/detail/complexationmapkey.h \
    src/gdm/core/basiccomposition.h \
    src/gdm/core/composition.h \
    src/gdm/core/gdm.h \
    src/ui/editconstituentdialog.h \
    src/ui/elementaries/modifyconstituentchargepushbutton.h \
    src/ui/internal_models/constituentchargesmodel.h \
    src/gearbox/gearbox.h \
    src/mappers/floatmappermodel.h \
    src/gearbox/gdmproxy.h \
    src/gearbox/samplegdmproxy.h \
    src/gearbox/backgroundgdmproxy.h \
    src/mappers/abstractmappermodel.h \
    src/gearbox/calculatorinterface.h \
    src/gearbox/results_models/resultsmodels.h \
    src/gearbox/results_models/resultsdata.h \
    src/gearbox/results_models/resultscontainer.h \
    src/gearbox/results_models/systemeigenzonestablemodel.h \
    src/ui/elementaries/floatingvaluelineedit.h \
    src/gearbox/doubletostringconvertor.h \
    src/gearbox/inumberformatchangeable.h \
    src/gearbox/floatingvaluedelegate.h \
    src/gearbox/calculatorcontext.h \
    src/ui/signalplotwidget.h \
    src/ui/doubleclickableqwtplotzoomer.h \
    src/ui/maincontrolwidget.h \
    src/ui/systemcompositionwidget.h \
    src/ui/internal_models/abstractconstituentsmodelbase.h \
    src/ui/internal_models/abstractconstituentsmodel.h \
    src/gearbox/constituentmanipulator.h \
    src/gearbox/complexationmanager.h \
    src/gearbox/gdmproxy_p.h \
    src/ui/editcomplexationdialog.h \
    src/ui/complexation_models/complexationrelationship.h \
    src/ui/complexation_models/complexationrelationshipsmodel.h \
    src/ui/complexation_models/ligandionicform.h \
    src/ui/complexation_models/nucleusionicform.h \
    src/ui/complexation_models/constituent_ui.h \
    src/ui/complexationcolorizerdelegate.h \
    src/ui/internal_models/constituentsmodelimpl.h \
    src/crashhandling/crashhandlerbase.h \
    src/crashhandling/crashhandlerfinalizer.h \
    src/crashhandling/crashhandlerlinux_stacktrace.h \
    src/crashhandling/crashhandlerlinux.h \
    src/crashhandling/crashhandlerwindows_stacktrace.h \
    src/crashhandling/crashhandlerwindows.h \
    src/crashhandling/crashhandlingprovider.h \
    src/crashhandling/nullcrashhandler.h \
    src/crashhandling/rawmemblock.h \
    src/crashevent.h \
    src/crasheventcatcher.h \
    src/pmngcrashhandler.h \
    src/ui/crashhandlerdialog.h \
    src/globals.h \
    src/ui/aboutdialog.h \
    src/ui/eigenzonedetailsdialog.h \
    src/gearbox/results_models/eigenzonedetailsmodel.h \
    src/gearbox/results_models/ioniccompositionmodel.h \
    src/ui/ioniccompositiondialog.h \
    src/gearbox/calculatorworker.h \
    src/persistence/serializer.h \
    src/persistence/persistence.h \
    src/persistence/deserializer.h \
    src/ui/operationinprogressdialog.h \
    src/database/db_constituent.h \
    src/database/db_constituentsdatabase.h \
    src/gearbox/databaseproxy.h \
    src/ui/pickconstituentfromdbdialog.h \
    src/ui/internal_models/databaseconstituentsphyspropstablemodel.h \
    src/ui/ploteventfilter.h \
    src/ui/nonidealitycorrectionsdialog.h \
    src/gearbox/complexationparametersdelegate.h \
    src/gearbox/efgdisplayer.h \
    src/gearbox/efgcsvexporter.h \
    src/ui/csvexportoptionsdialog.h \
    src/ui/copiableitemstableview.h \
    src/ui/databasetableview.h \
    src/gearbox/results_models/analytesdissociationmodel.h \
    src/ui/editchargeswidget.h \
    src/ui/databaseeditordialog.h \
    src/ui/editdatabaseconstituentdialog.h \
    src/gearbox/iconstituenteditor.h \
    src/ui/toggletracepointsdialog.h \
    src/ui/internal_models/analytesconstituentsmodel.h \
    src/gearbox/results_models/analytesextrainfomodel.h \
    src/ui/analytesconstituentsheader.h \
    src/ui/hacks.h \
    src/ui/internal_models/backgroundconstituentsmodel.h \
    src/gearbox/results_models/backgroundeffectivemobilitiesmodel.h \
    src/ui/backgroundconstituentsheader.h \
    src/ui/elementaries/uihelpers.h \
    src/ui/checkforupdatedialog.h \
    src/ui/softwareupdatewidget.h \
    src/softwareupdater.h \
    src/softwareupdateresult.h \
    src/ui/autoupdatecheckdialog.h \
    src/gearbox/results_models/backgroundpropertiesmapping.h \
    src/gearbox/additionalfloatingvalidator.h \
    src/msvc_hacks.h \
    src/ui/returnacceptabletableview.h \
    src/ui/refocuser.h \
    src/persistence/swsettings.h \
    src/ui/adjustueffoverkbgedialog.h \
    src/ui/internal_models/adjustueffoverkbgetablemodel.h \
    src/gearbox/ueffoverkbgecalculatorinterface.h \
    src/ui/compositioneditorwidget.h \
    src/ui/backgroundcompositionwidget.h \
    src/ui/analytescompositionwidget.h \
    src/gearbox/pmngdataroles.h

FORMS += \
    src/ui/adjustphdialog.ui \
    src/ui/pmngmainwindow.ui \
    src/ui/editconstituentdialog.ui \
    src/ui/signalplotwidget.ui \
    src/ui/maincontrolwidget.ui \
    src/ui/editcomplexationdialog.ui \
    src/ui/crashhandlerdialog.ui \
    src/ui/aboutdialog.ui \
    src/ui/eigenzonedetailsdialog.ui \
    src/ui/ioniccompositiondialog.ui \
    src/ui/operationinprogressdialog.ui \
    src/ui/pickconstituentfromdbdialog.ui \
    src/ui/nonidealitycorrectionsdialog.ui \
    src/ui/csvexportoptionsdialog.ui \
    src/ui/editchargeswidget.ui \
    src/ui/databaseeditordialog.ui \
    src/ui/editdatabaseconstituentdialog.ui \
    src/ui/toggletracepointsdialog.ui \
    src/ui/checkforupdatedialog.ui \
    src/ui/softwareupdatewidget.ui \
    src/ui/autoupdatecheckdialog.ui \
    src/ui/adjustueffoverkbgedialog.ui \
    src/ui/compositioneditorwidget.ui

unix {
    QMAKE_CXXFLAGS += "-Wall -Wextra -pedantic"
}

LIBS += -lsqlite3

win32 {
    LIBS += -ldbghelp
}

win32-msvc* {
    DEFINES += ECHMET_COMPILER_MSVC
} win32-g++ {
    DEFINES += ECHMET_COMPILER_GCC_LIKE
} linux-g++ {
    DEFINES += ECHMET_COMPILER_GCC_LIKE
} linux-clang {
    DEFINES += ECHMET_COMPILER_GCC_LIKE
} macx-g++ {
    DEFINES += ECHMET_COMPILER_GCC_LIKE
} macx-clang {
    DEFINES += ECHMET_COMPILER_GCC_LIKE
}

include($$PWD/PeakMasterNG.pri)

RESOURCES += \
    pmngres.qrc

win32 {
    RC_FILE = PeakMasterNG.rc
    CONFIG += embed_manifest_exe
}

flatpak_build {
    DEFINES += PMNG_FLATPAK_BUILD

    package.path = /app/bin
    package.files += PeakMasterNG
    package.CONFIG = no_check_exist
    INSTALLS += package
}
