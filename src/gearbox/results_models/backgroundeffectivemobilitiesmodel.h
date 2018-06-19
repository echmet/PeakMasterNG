#ifndef BACKGROUNDEFFECTIVEMOBILITIESMODEL_H
#define BACKGROUNDEFFECTIVEMOBILITIESMODEL_H

#include <QMap>
#include <QObject>

class BackgroundEffectiveMobilitiesModel : public QObject
{
  Q_OBJECT

public:
  explicit BackgroundEffectiveMobilitiesModel(QObject *parent = nullptr);

  QVariant effectiveMobility(const QString &name) const;
  void setData(const QMap<QString, double> &newData);

private:
  QMap<QString, double> m_data;

signals:
  void dataChanged();
};

#endif // BACKGROUNDEFFECTIVEMOBILITIESMODEL_H
