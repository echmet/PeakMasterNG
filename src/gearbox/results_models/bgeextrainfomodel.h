#ifndef BGEEXTRAINFOMODEL_H
#define BGEEXTRAINFOMODEL_H

#include <QObject>
#include <QVector>
#include <QMap>

class BGEExtraInfoModel : public QObject
{
  Q_OBJECT

public:
  class ExtraInfo {
  public:
    ExtraInfo();
    explicit ExtraInfo(QString constituent, const double uEff, const double kappaBGE);
    ExtraInfo(const ExtraInfo &other) = default;
    ExtraInfo(ExtraInfo &&other) = default;

    ExtraInfo & operator=(const ExtraInfo &other);

    const QString constituent;
    const double uEff;
    const double uEffOverKappaBGE;
  };

  BGEExtraInfoModel() = delete;
  explicit BGEExtraInfoModel(QObject *parent = nullptr);

  const ExtraInfo & info(const QString &constituent) const noexcept;
  void setData(const QVector<ExtraInfo> &newData);

private:
  typedef QMap<QString, ExtraInfo> DataMap;

  DataMap m_data;
  ExtraInfo m_defaultInfo;

signals:
  void dataChanged();

};

#endif // BGEEXTRAINFOMODEL_H
