#ifndef ANALYTESCONSTITUENTSMODEL_H
#define ANALYTESCONSTITUENTSMODEL_H

#include "../../gearbox/results_models/analytesextrainfomodel.h"
#include "constituentsmodelimpl.h"

#include "../../globals.h"

class AnalytesConstituentsModel : public ConstituentsModelImpl<1>
{
public:
  explicit AnalytesConstituentsModel(const AnalytesExtraInfoModel * const analytesEXIModel,
                                     const QVector<QString> &concentrationHeaders, GDMProxy &GDMProxy, ComplexationManager &cpxMgr, QObject *parent = nullptr) :
    ConstituentsModelImpl{concentrationHeaders, GDMProxy, cpxMgr, parent},
    h_analytesEXIModel{analytesEXIModel}
  {
    if (Globals::isZombieOS()) {
      m_condMaxStr = QObject::tr("K Max (S/m)");
      m_uEffStr = QObject::tr("u Eff (. 1e-9)");
      m_uEffOverKappaBGEStr = QObject::tr("u Eff / kappa BGE (1/A)");
      m_uEMDStr = QObject::tr("u EMD (. 1e-9)");
    } else {
      m_condMaxStr = QObject::tr("\xCE\xBA Max (S/m)");
      m_uEffStr = QObject::tr("\xCE\xBC Eff (\xE2\x8B\x85 1e-9)");
      m_uEffOverKappaBGEStr = QObject::tr("\xCE\xBC Eff/\xCE\xBA BGE (A\xE2\x81\xBB\xC2\xB9)");
      m_uEMDStr = QObject::tr("\xCE\xBC EMD (\xE2\x8B\x85 1e-9)");
    }

    connect(h_analytesEXIModel, &AnalytesExtraInfoModel::dataChanged, this, &AnalytesConstituentsModel::onExtraInfoChanged);
  }

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
  {
    const QVariant null = {};
    const int baseColumnCount = ConstituentsModelImpl::columnCount(QModelIndex{});
    const int col = index.column();

    if (col < baseColumnCount)
      return ConstituentsModelImpl::data(index, role);

    if (role != Qt::DisplayRole)
      return null;

    const QString name = ConstituentsModelImpl::data(createIndex(index.row(), 2)).toString();

    const auto &exInfo = h_analytesEXIModel->info(name);
    if (!exInfo.valid)
      return null;

    switch(int idx = col - baseColumnCount){
    case 0:
      return exInfo.uEff;
    case 1:
      return exInfo.detected ? exInfo.time : null;
    case 2:
      return exInfo.detected ? exInfo.concentrationMax : null;
    case 3:
      return exInfo.detected ? exInfo.conductivityMax : null;
    case 4:
      return exInfo.uEMD;
    case 5:
      return exInfo.uEffOverKappaBGE;
    default:
      assert(idx >= m_extraInfoParamsCount);
      return null;
    }
  }

  int firstExtraInfoColumn() const noexcept
  {
    return ConstituentsModelImpl::columnCount(QModelIndex{});
  }

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override
  {
    const int baseColumnCount = ConstituentsModelImpl::columnCount(QModelIndex{});

    if (section < baseColumnCount)
      return ConstituentsModelImpl::headerData(section, orientation, role);

    switch(int sec = section - baseColumnCount)
    {
    case 0:
      return m_uEffStr;
    case 1:
      return QObject::tr("Time Max (min)");
    case 2:
      return QObject::tr("c Max (mM)");
    case 3:
      return m_condMaxStr;
    case 4:
      return m_uEMDStr;
    case 5:
      return m_uEffOverKappaBGEStr;
    default:
      assert(sec >= m_extraInfoParamsCount);
      return {};
    }
  }

  virtual int columnCount(const QModelIndex &parent) const override
  {
    return ConstituentsModelImpl::columnCount(parent) + m_extraInfoParamsCount;
  }

private:
  const AnalytesExtraInfoModel * h_analytesEXIModel;
  const int m_extraInfoParamsCount = 6;

  QString m_condMaxStr;
  QString m_uEffStr;
  QString m_uEffOverKappaBGEStr;
  QString m_uEMDStr;

private slots:
  void onExtraInfoChanged()
  {
    const int bcc = ConstituentsModelImpl::columnCount(QModelIndex{});
    const QModelIndex topLeft = createIndex(0, bcc);
    const QModelIndex bottomRight = createIndex(rowCount() - 1, bcc + (m_extraInfoParamsCount - 1));

    emit dataChanged(topLeft, bottomRight, { Qt::DisplayRole });
  }
};

#endif // ANALYTESCONSTITUENTSMODEL_H
