#ifndef CONSTITUENTSMODELBASE_H
#define CONSTITUENTSMODELBASE_H

#include "constituentsmodelimpl.h"
#include "../../gearbox/results_models/extrainfomodelbase.h"

// ---
// DCL

template<typename EXIMODEL, int N>
class ConstituentsModelBase : public ConstituentsModelImpl<N>
{
public:
  using ExtraInfoModel = EXIMODEL;
  using ConstituentsModelImpl = ConstituentsModelImpl<N>;

  explicit ConstituentsModelBase(
    const ExtraInfoModel * const EXIModel,
    const QVector<QString> &concentrationHeaders,
    GDMProxy &GDMProxy,
    ComplexationManager &cpxMgr,
    QObject *parent = nullptr
  );

  int firstExtraInfoColumn() const noexcept;

  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  virtual int columnCount(const QModelIndex &parent) const override;

private:
  const ExtraInfoModel * h_EXIModel;

private slots:
  void onExtraInfoChanged();
};

// ---
// DEF

template<typename EXIMODEL, int N>
ConstituentsModelBase<EXIMODEL, N>::ConstituentsModelBase(
  const ExtraInfoModel * const EXIModel,
  const QVector<QString> &concentrationHeaders,
  GDMProxy &GDMProxy,
  ComplexationManager &cpxMgr,
  QObject *parent
) :
  ConstituentsModelImpl{concentrationHeaders, GDMProxy, cpxMgr, parent},
  h_EXIModel{EXIModel}
{
  connect(h_EXIModel, &ExtraInfoModel::dataChanged, this, &ConstituentsModelBase::onExtraInfoChanged);
}

template<typename EXIMODEL, int N>
int ConstituentsModelBase<EXIMODEL, N>::firstExtraInfoColumn() const noexcept
{
  return ConstituentsModelImpl::columnCount(QModelIndex{});
}

template<typename EXIMODEL, int N>
QVariant ConstituentsModelBase<EXIMODEL, N>::data(const QModelIndex &index, int role) const
{
  static const QVariant null = {};
  const int col = index.column();

  if (col < firstExtraInfoColumn())
    return ConstituentsModelImpl::data(index, role);

  if (role != Qt::DisplayRole)
    return null;

  const QString name = ConstituentsModelImpl::name(index.row());

  return (*h_EXIModel)[name].data(col - firstExtraInfoColumn());
}

template<typename EXIMODEL, int N>
QVariant ConstituentsModelBase<EXIMODEL, N>::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (section < firstExtraInfoColumn())
    return ConstituentsModelImpl::headerData(section, orientation, role);

  return ExtraInfoModel::ExtraInfo::header(section - firstExtraInfoColumn());
}

template<typename EXIMODEL, int N>
int ConstituentsModelBase<EXIMODEL, N>::columnCount(const QModelIndex &parent) const
{
  return ConstituentsModelImpl::columnCount(parent) + ExtraInfoModel::ExtraInfo::size();
}

template<typename EXIMODEL, int N>
void ConstituentsModelBase<EXIMODEL, N>::onExtraInfoChanged()
{
  const QModelIndex topLeft = createIndex(0, firstExtraInfoColumn());
  const QModelIndex bottomRight = createIndex(rowCount() - 1, firstExtraInfoColumn() + (ExtraInfoModel::ExtraInfo::size() - 1));

  emit dataChanged(topLeft, bottomRight, { Qt::DisplayRole });
}

#endif // CONSTITUENTSMODEL_H
