#include "analytesextrainfomodel.h"

AnalytesExtraInfoModel::ExtraInfo::ExtraInfo() :
  analyte{},
  uEff{0},
  time{-1},
  uEMD{0},
  concentrationMax{0},
  conductivityMax{0},
  detected{false},
  valid{false}
{}

AnalytesExtraInfoModel::ExtraInfo::ExtraInfo(QString analyte, const double uEff, const double kappaBGE, const double time, const double uEMD,
                                             const double concentrationMax, const double conductivityMax,
                                             const bool detected) :
  analyte{std::move(analyte)},
  uEff{uEff},
  time{time},
  uEMD{uEMD},
  uEffOverKappaBGE{kappaBGE != 0 ? (uEff / kappaBGE) : (0)},
  concentrationMax{concentrationMax},
  conductivityMax{conductivityMax},
  detected{detected},
  valid{true}
{
}

AnalytesExtraInfoModel::ExtraInfo::ExtraInfo(const ExtraInfo &other) :
  analyte{other.analyte},
  uEff{other.uEff},
  time{other.time},
  uEMD{other.uEMD},
  uEffOverKappaBGE{other.uEffOverKappaBGE},
  concentrationMax{other.concentrationMax},
  conductivityMax{other.conductivityMax},
  detected{other.detected},
  valid{true}
{}

AnalytesExtraInfoModel::ExtraInfo & AnalytesExtraInfoModel::ExtraInfo::operator=(const ExtraInfo & other)
{
  const_cast<QString&>(analyte) = other.analyte;
  const_cast<double&>(uEff) = other.uEff;
  const_cast<double&>(time) = other.time;
  const_cast<double&>(uEMD) = other.uEMD;
  const_cast<double&>(uEffOverKappaBGE) = other.uEffOverKappaBGE;
  const_cast<double&>(concentrationMax) = other.concentrationMax;
  const_cast<double&>(conductivityMax) = other.conductivityMax;
  const_cast<bool&>(detected) = other.detected;
  const_cast<bool&>(valid) = other.valid;

  return *this;
}

AnalytesExtraInfoModel::AnalytesExtraInfoModel(QObject *parent) :
  QObject{parent}
{
}

const AnalytesExtraInfoModel::ExtraInfo & AnalytesExtraInfoModel::info(const QString &analyte) const noexcept
{
  if (m_data.contains(analyte))
    return m_data.find(analyte).value();

  return m_defaultInfo;
}

void AnalytesExtraInfoModel::setData(const QVector<ExtraInfo> &newData)
{
  m_data.clear();

  for (const auto &item : newData)
    m_data.insert(item.analyte, item);

  emit dataChanged();
}
