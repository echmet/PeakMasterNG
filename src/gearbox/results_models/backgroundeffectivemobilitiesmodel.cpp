#include "backgroundeffectivemobilitiesmodel.h"

#include <QVariant>

BackgroundEffectiveMobilitiesModel::BackgroundEffectiveMobilitiesModel(QObject *parent) :
  QObject{parent}
{
}

QVariant BackgroundEffectiveMobilitiesModel::effectiveMobility(const QString &name) const
{
  if (m_data.contains(name))
    return m_data.value(name);

  return {};
}

void BackgroundEffectiveMobilitiesModel::setData(const QMap<QString, double> &newData)
{
  m_data = newData;

  emit dataChanged();
}
