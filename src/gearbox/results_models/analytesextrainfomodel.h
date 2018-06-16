#ifndef ANALYTESEXTRAINFOMODEL_H
#define ANALYTESEXTRAINFOMODEL_H

#include <QMap>
#include <QObject>
#include <QVector>

class AnalytesExtraInfoModel : public QObject
{
  Q_OBJECT

public:
  class ExtraInfo {
  public:
    ExtraInfo();
    explicit ExtraInfo(const QString &analyte, const double uEff, const double time, const double uEMD,
                       const double concentrationMax, const double conductivityMax,
                       const bool detected);
    ExtraInfo(const ExtraInfo &other);

    ExtraInfo & operator=(const ExtraInfo &other);

    const QString analyte;
    const double uEff;
    const double time;
    const double uEMD;
    const double concentrationMax;
    const double conductivityMax;
    const bool detected;
    const bool valid;
  };

  AnalytesExtraInfoModel() = delete;
  explicit AnalytesExtraInfoModel(QObject *parent = nullptr);

  const ExtraInfo & info(const QString &analyte) const noexcept;
  void setData(const QVector<ExtraInfo> &newData);

private:
  typedef QMap<QString, ExtraInfo> DataMap;

  DataMap m_data;
  ExtraInfo m_defaultInfo;

signals:
  void dataChanged();
};

#endif // ANALYTESEXTRAINFOMODEL_H
