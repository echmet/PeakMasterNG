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
      m_uEMDStr = QObject::tr("u EMD (. 1e-9)");
    } else {
      m_condMaxStr = QObject::tr("\xCE\xBA Max (S/m)");
      m_uEffStr = QObject::tr("\xCE\xBC Eff (\xE2\x8B\x85 1e-9)");
      m_uEMDStr = QObject::tr("\xCE\xBC EMD (\xE2\x8B\x85 1e-9)");
    }

    connect(h_analytesEXIModel, &AnalytesExtraInfoModel::dataChanged, this, &AnalytesConstituentsModel::onExtraInfoChanged);
  }

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
  {
    const int baseColumnCount = ConstituentsModelImpl::columnCount(QModelIndex{});
    const int col = index.column();

    if (col < baseColumnCount)
      return ConstituentsModelImpl::data(index, role);

    if (role != Qt::DisplayRole)
      return {};

    const QString name = ConstituentsModelImpl::data(createIndex(index.row(), 2)).toString();

    const auto &exInfo = h_analytesEXIModel->info(name);
    if (!exInfo.valid)
      return {};

    if (col == baseColumnCount)
      return exInfo.uEff;
    else if (col == baseColumnCount + 1) {
      if (!exInfo.detected)
        return {};
      return exInfo.time;
    } else if (col == baseColumnCount + 2) {
      if (!exInfo.detected)
        return {};
      return exInfo.concentrationMax;
   } else if (col == baseColumnCount + 3) {
      if (!exInfo.detected)
        return {};
      return exInfo.conductivityMax;
   } else if (col == baseColumnCount + 4)
      return exInfo.uEMD;

    return {};
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

    if (section == baseColumnCount)
      return m_uEffStr;
    else if (section == baseColumnCount + 1)
      return QObject::tr("Time Max (min)");
    else if (section == baseColumnCount + 2)
      return QObject::tr("c Max (mM)");
    else if (section == baseColumnCount + 3)
      return m_condMaxStr;
    else if (section == baseColumnCount + 4)
      return m_uEMDStr;

    return {};
  }

  virtual int columnCount(const QModelIndex &parent) const override
  {
    return ConstituentsModelImpl::columnCount(parent) + 5;
  }

private:
  const AnalytesExtraInfoModel * h_analytesEXIModel;

  QString m_condMaxStr;
  QString m_uEffStr;
  QString m_uEMDStr;

private slots:
  void onExtraInfoChanged()
  {
    const int bcc = ConstituentsModelImpl::columnCount(QModelIndex{});
    const QModelIndex topLeft = createIndex(0, bcc);
    const QModelIndex bottomRight = createIndex(rowCount() - 1, bcc + 4);

    emit dataChanged(topLeft, bottomRight, { Qt::DisplayRole });
  }
};

#endif // ANALYTESCONSTITUENTSMODEL_H
