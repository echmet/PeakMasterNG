#include "gearbox/gearbox.h"
#include "gearbox/doubletostringconvertor.h"
#include "ui/pmngmainwindow.h"
#include "ui/systemcompositionwidget.h"
#include "pmngcrashhandler.h"
#include "softwareupdater.h"

#include <QApplication>

void registerMetatypes();

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
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
                                         gbox.resultsModels().analytesExtraInfoModel(), gbox.resultsModels().eigenzoneDetailsModel()};
  QObject::connect(w, &PMNGMainWindow::clearAll, &gbox, &Gearbox::onClearAll);
  w->connectUpdater(&updater);

  PMNGCrashHandler::checkForCrash();

  updater.checkAutomatically();

  w->show();

  int ret = a.exec();

  PMNGCrashHandler::uninstallCrashHandler();

  delete w;

  return ret;
}
