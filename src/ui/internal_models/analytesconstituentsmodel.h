#ifndef ANALYTESCONSTITUENTSMODEL_H
#define ANALYTESCONSTITUENTSMODEL_H

#include "../../gearbox/results_models/analytesextrainfomodel.h"
#include "constituentsmodelimpl.h"

class AnalytesConstituentsModel : public ConstituentsModelImpl<1>
{
public:
  explicit AnalytesConstituentsModel(const AnalytesExtraInfoModel * const analytesEXIModel,
                                     const QVector<QString> &concentrationHeaders, GDMProxy &GDMProxy, ComplexationManager &cpxMgr, QObject *parent = nullptr) :
    ConstituentsModelImpl{concentrationHeaders, GDMProxy, cpxMgr, parent},
    h_analytesEXIModel{analytesEXIModel}
  {
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
      if (exInfo.time <= 0.0)
        return {};
      return exInfo.time;
    } else if (col == baseColumnCount + 2) {
      if (exInfo.time <= 0.0)
        return {};
      return exInfo.concentrationMax;
   } else if (col == baseColumnCount + 3) {
      if (exInfo.time <= 0.0)
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
      return QObject::tr("\u03BC Eff (\u00B7 1e-9)");
    else if (section == baseColumnCount + 1)
      return QObject::tr("Time (min)");
    else if (section == baseColumnCount + 2)
      return QObject::tr("Concentration Max (mM)");
    else if (section == baseColumnCount + 3)
      return QObject::tr("Conductivity Max (S/m)");
    else if (section == baseColumnCount + 4)
      return QObject::tr("\u03BC EMD (\u00B7 1e-9)");

    return {};
  }

  virtual int columnCount(const QModelIndex &index) const override
  {
    return ConstituentsModelImpl::columnCount(index) + 5;
  }

private:
  const AnalytesExtraInfoModel * h_analytesEXIModel;

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
