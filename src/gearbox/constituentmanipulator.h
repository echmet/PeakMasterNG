#ifndef CONSTITUENTVALIDATOR_H
#define CONSTITUENTVALIDATOR_H

#include "../gdm/core/constituent/constituent.h"

#include <QObject>

class EditConstituentDialog;
class GDMProxy;

class ConstituentManipulator : public QObject
{
  Q_OBJECT

public:
  explicit ConstituentManipulator(const GDMProxy &proxy);
  gdm::Constituent makeConstituent(const EditConstituentDialog *dlg);
  EditConstituentDialog * makeEditDialog(const std::string &name, GDMProxy &proxy);

public slots:
  void onValidateConstituentInput(const EditConstituentDialog *dlg, bool *ok);
  void onValidateConstituentInputUpdate(const EditConstituentDialog *dlg, bool *ok);

private:
  const GDMProxy &h_proxy;
};

#endif // CONSTITUENTVALIDATOR_H
