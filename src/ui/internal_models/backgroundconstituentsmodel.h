#ifndef BACKGROUNDCONSTITUENTSMODEL_H
#define BACKGROUNDCONSTITUENTSMODEL_H

#include "constituentsmodelimpl.h"
#include "../../gearbox/results_models/backgroundeffectivemobilitiesmodel.h"

class BackgroundConstituentsModel : public ConstituentsModelImpl<2>
{
public:
  explicit BackgroundConstituentsModel(const BackgroundEffectiveMobilitiesModel * const BGEEffMobsModel,
                                       const QVector<QString> &concentrationHeaders, GDMProxy &GDMProxy, ComplexationManager &cpxMgr, QObject *parent = nullptr) :
    ConstituentsModelImpl{concentrationHeaders, GDMProxy, cpxMgr, parent},
    h_BGEEffMobsModel{BGEEffMobsModel}
  {
    connect(BGEEffMobsModel, &BackgroundEffectiveMobilitiesModel::dataChanged, this, &BackgroundConstituentsModel::onEffectiveMobilitiesChanged);
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

    if (col == baseColumnCount)
      return h_BGEEffMobsModel->effectiveMobility(name);

    return {};
  }

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override
  {
    const int baseColumnCount = ConstituentsModelImpl::columnCount(QModelIndex{});

    if (section < baseColumnCount)
      return ConstituentsModelImpl::headerData(section, orientation, role);

    if (section == baseColumnCount)
      return QObject::tr("\xCE\xBC Eff (\xE2\x8B\x85 1e-9)");

    return {};
  }

  virtual int columnCount(const QModelIndex &index) const override
  {
    return ConstituentsModelImpl::columnCount(index) + 1;
  }

  int firstExtraInfoColumn() const
  {
    return ConstituentsModelImpl::columnCount(QModelIndex{});
  }

private:
  const BackgroundEffectiveMobilitiesModel * const h_BGEEffMobsModel;

private slots:
  void onEffectiveMobilitiesChanged()
  {
    const int bcc = ConstituentsModelImpl::columnCount(QModelIndex{});
    const QModelIndex topLeft = createIndex(0, bcc);
    const QModelIndex bottomRight = createIndex(rowCount() - 1, bcc + 1);

    emit dataChanged(topLeft, bottomRight, { Qt::DisplayRole });
  }
};

#endif // BACKGROUNDCONSTITUENTSMODEL_H
