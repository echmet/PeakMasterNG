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

CONFIG(release, debug|release) {
    DEFINES += USE_CRASHHANDLER
}
CONFIG(debug, debug|release) {
    # Configuration specific for "debug" build

}

SOURCES += \
        src/main.cpp \
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
    src/gdm/persistence/deserialize.cpp \
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
    src/gearbox/calculatorworker.cpp

HEADERS += \
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
    src/gdm/persistence/deserialize.h \
    src/gdm/persistence/persistenceexcept.h \
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
    src/gearbox/results_models/backgroundpropertiesmodel.h \
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
    src/gearbox/calculatorworker.h

FORMS += \
        src/ui/pmngmainwindow.ui \
    src/ui/editconstituentdialog.ui \
    src/ui/commonpropertieswidget.ui \
    src/ui/signalplotwidget.ui \
    src/ui/maincontrolwidget.ui \
    src/ui/systemcompositionwidget.ui \
    src/ui/editcomplexationdialog.ui \
    src/ui/crashhandlerdialog.ui \
    src/ui/aboutdialog.ui \
    src/ui/eigenzonedetailsdialog.ui \
    src/ui/ioniccompositiondialog.ui

unix {
    QMAKE_CXXFLAGS += "-Wall -Wextra -pedantic"
}

win32 {
    LIBS += -ldbghelp
}

include($$PWD/PeakMasterNG.pri)

RESOURCES += \
    pmngres.qrc
