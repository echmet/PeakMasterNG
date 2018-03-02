#include "gearbox/gearbox.h"
#include "gearbox/doubletostringconvertor.h"
#include "ui/pmngmainwindow.h"
#include "ui/systemcompositionwidget.h"
#include "pmngcrashhandler.h"

#include <QApplication>

void registerMetatypes();

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  DoubleToStringConvertor::initialize();

  Gearbox gbox{};

  registerMetatypes();

  PMNGCrashHandler::installCrashHandler();

  SystemCompositionWidget *scompWidget = new SystemCompositionWidget{gbox.backgroundGDMProxy(), gbox.sampleGDMProxy(),
                                                                     gbox.complexationManager(), gbox.databaseProxy()};
  gbox.setUICompositionModels(scompWidget->analytesModel(), scompWidget->backgroundModel());

  PMNGMainWindow *w = new PMNGMainWindow{scompWidget, gbox.calculatorInterface(), gbox.resultsModels(), gbox.persistence(), gbox.databaseProxy()};
  QObject::connect(w, &PMNGMainWindow::clearAll, &gbox, &Gearbox::onClearAll);

  PMNGCrashHandler::checkForCrash();

  w->show();

  int ret = a.exec();

  PMNGCrashHandler::uninstallCrashHandler();

  return ret;
}
