#ifndef UI_CONSTITUENT_H
#define UI_CONSTITUENT_H

#include "complexationrelationship.h"
#include <QMap>
#include <QString>

class Constituent
{
public:
  enum class Type {
    NUCLEUS,
    LIGAND
  };

  Constituent(const int lowCharge, const int highCharge, QString name, const Type ctype);

  const int lowCharge;
  const int highCharge;
  const QString name;
  const Type ctype;
};

#endif // UI_CONSTITUENT_H
