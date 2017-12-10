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
  Gearbox gbox{};

  registerMetatypes();
  DoubleToStringConvertor::initialize();

  PMNGCrashHandler::installCrashHandler();

  SystemCompositionWidget *scompWidget = new SystemCompositionWidget{gbox.backgroundGDMProxy(), gbox.sampleGDMProxy(), gbox.complexationManager()};

  PMNGMainWindow *w = new PMNGMainWindow{scompWidget, gbox.calculatorInterface(), gbox.resultsModels(), gbox.persistence()};

  PMNGCrashHandler::checkForCrash();

  w->show();

  int ret = a.exec();

  PMNGCrashHandler::uninstallCrashHandler();

  return ret;
}
