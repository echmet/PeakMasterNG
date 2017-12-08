#include "gui/mainwindow.h"
#include <QApplication>

#include "constituent_ui.h"
#include "complexationrelationshipsmodel.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;
  ComplexationRelationshipsModel model;

  w.setComplexationModel(&model);

  w.show();

  std::shared_ptr<Constituent> ct_a = std::make_shared<Constituent>(0, 2, "A", Constituent::Type::NUCLEUS);
  std::shared_ptr<Constituent> ct_x = std::make_shared<Constituent>(-2, 0, "X", Constituent::Type::LIGAND);
  std::shared_ptr<Constituent> ct_z = std::make_shared<Constituent>(0, 1, "Z", Constituent::Type::LIGAND);

  ComplexationRelationship relAX(ct_a, ct_x);
  ComplexationRelationship relAZ(ct_a, ct_z);

  w.addRelationship(relAX);
  w.addRelationship(relAZ);

  w.setWindowTitle(QString("Edit complexation for nucleus %1").arg(ct_a->name));

  return a.exec();
}
