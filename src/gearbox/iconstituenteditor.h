#ifndef ICONSTITUENTEDITOR_H
#define ICONSTITUENTEDITOR_H

#include <vector>
#include <QString>
#include <QObject>
#include <QMetaObject>

class IConstituentEditor {
public:
  enum class ConstituentType {
    NUCLEUS,
    LIGAND
  };

  virtual int chargeLow() const = 0;
  virtual int chargeHigh() const = 0;
  virtual std::vector<double> mobilities() const = 0;
  virtual QString name() const = 0;
  virtual std::vector<double> pKas() const = 0;
  virtual ConstituentType type() const = 0;
  virtual double viscosityCoefficient() const = 0;
};

Q_DECLARE_METATYPE(IConstituentEditor::ConstituentType)
#endif // ICONSTITUENTEDITOR_H
