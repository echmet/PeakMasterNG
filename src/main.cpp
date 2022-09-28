#include "gearbox/gearbox.h"
#include "gearbox/doubletostringconvertor.h"
#include "ui/pmngmainwindow.h"
#include "ui/systemcompositionwidget.h"
#include "pmngcrashhandler.h"
#include "softwareupdater.h"
#include "globals.h"
#include "persistence/swsettings.h"

#include <QApplication>

void registerMetatypes();

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  QCoreApplication::setOrganizationDomain(Globals::ORG_DOMAIN());
  QCoreApplication::setOrganizationName(Globals::ORG_NAME());
  QCoreApplication::setApplicationName(Globals::SOFTWARE_NAME());
  QCoreApplication::setApplicationVersion(Globals::VERSION_STRING());

  persistence::SWSettings::initialize();

  DoubleToStringConvertor::initialize();
  SoftwareUpdater updater;

  Gearbox gbox{};

  registerMetatypes();

  PMNGCrashHandler::installCrashHandler();

  SystemCompositionWidget *scompWidget = new SystemCompositionWidget{gbox.backgroundGDMProxy(), gbox.sampleGDMProxy(),
                                                                     gbox.complexationManager(), gbox.databaseProxy(),
                                                                     gbox.resultsModels().analytesExtraInfoModel(), gbox.resultsModels().backgroundEffectiveMobilitiesModel()};
  gbox.setUICompositionModels(scompWidget->analytesModel(), scompWidget->backgroundModel());

  PMNGMainWindow *w = new PMNGMainWindow{scompWidget, gbox.calculatorInterface(), gbox.resultsModels(), gbox.persistence(), gbox.databaseProxy(),
                                         gbox.resultsModels().analytesExtraInfoModel(), gbox.resultsModels().eigenzoneDetailsModel(),
                                         gbox.backgroundGDMProxy(), gbox.sampleGDMProxy()};
  QObject::connect(w, &PMNGMainWindow::clearAll, &gbox, &Gearbox::onClearAll);
  w->connectUpdater(&updater);

  updater.checkAutomatically();

  w->show();

  PMNGCrashHandler::checkForCrash();

  int ret = a.exec();

  persistence::SWSettings::destroy();

  PMNGCrashHandler::uninstallCrashHandler();

  delete w;

  return ret;
}
