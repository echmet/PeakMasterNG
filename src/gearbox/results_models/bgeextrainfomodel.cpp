#include "bgeextrainfomodel.h"

BGEExtraInfoModel::ExtraInfo::ExtraInfo() :
  constituent{},
  uEff{0},
  uEffOverKappaBGE{0}
{}

BGEExtraInfoModel::ExtraInfo::ExtraInfo(QString constituent, const double uEff, const double kappaBGE) :
  constituent{std::move(constituent)},
  uEff{uEff},
  uEffOverKappaBGE{kappaBGE != 0 ? ((uEff / kappaBGE) * 1E-9) : (0)}
{}

BGEExtraInfoModel::ExtraInfo & BGEExtraInfoModel::ExtraInfo::operator=(const BGEExtraInfoModel::ExtraInfo &other)
{
    if (&other == this) return *this;

    const_cast<QString&>(constituent) = other.constituent;
    const_cast<double&>(uEff) = other.uEff;
    const_cast<double&>(uEffOverKappaBGE) = other.uEffOverKappaBGE;

    return *this;
}

BGEExtraInfoModel::BGEExtraInfoModel(QObject *parent) :
    QObject{parent}
{}

const BGEExtraInfoModel::ExtraInfo & BGEExtraInfoModel::info(const QString &constituent) const noexcept
{
  if (m_data.contains(constituent))
    return m_data.find(constituent).value();

  return m_defaultInfo;
}

void BGEExtraInfoModel::setData(const QVector<ExtraInfo> &newData)
{
  m_data.clear();

  for(const auto &item : newData)
    m_data.insert(item.constituent, item);

  emit dataChanged();
}
