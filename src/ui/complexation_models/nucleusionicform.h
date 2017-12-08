#ifndef UI_NUCLEUSIONICFORM_H
#define UI_NUCLEUSIONICFORM_H

#include "ligandionicform.h"
#include <QVector>

class NucleusIonicForm
{
public:
  NucleusIonicForm();
  NucleusIonicForm(const int charge);

  const int charge;
  bool addLigandIon(const LigandIonicForm &ligandIF);
  bool removeLigandIon(const QString ligandName, const int charge);

private:
  QVector<LigandIonicForm> _ligandIonicForms;

};

#endif // UI_NUCLEUSIONICFORM_H
