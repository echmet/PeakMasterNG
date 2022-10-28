#ifndef CONSTITUENTSMODELIMPL_H
#define CONSTITUENTSMODELIMPL_H

#include "abstractconstituentsmodel.h"
#include "../../gearbox/gdmproxy.h"
#include "../../gearbox/pmngdataroles.h"

#include <cassert>

class ComplexationManager;

template <int N>
class ConstituentsModelImpl : public AbstractConstituentsModel<N>
{
public:
  explicit ConstituentsModelImpl(const QVector<QString> &concentrationHeaders, GDMProxy &GDMProxy, ComplexationManager &cpxMgr, QObject *parent = nullptr) :
    AbstractConstituentsModel<N>{GDMProxy, cpxMgr, parent},
    m_concentrationHeaders{concentrationHeaders}
  {
    assert(concentrationHeaders.size() == N);
  }

  // Basic functionality:
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
  {
    if (!(role == Qt::DisplayRole || role == ConstituentNameRole || role == ComplexationStatusRole))
      return {};

    const int row = index.row();

    if (row < 0 || row >= this->m_constituentNames.size())
      return {};

    const int col = index.column();

    if (col < 0 || col >= this->columnCount(index.parent()))
      return {};

    const auto constituentName = this->m_constituentNames.at(row).toStdString();
    const auto concentrations = this->h_GDMProxy.concentrations(constituentName);
    assert(concentrations.size() == static_cast<size_t>(this->NConcs()));

    if (role == ConstituentNameRole)
      return QString::fromStdString(constituentName);

    switch (col) {
    case 0:
      if (role == ComplexationStatusRole)
        return QVariant::fromValue(this->complexationStatus(constituentName));
      return {};
    case 1:
      return this->h_GDMProxy.isNucleus(constituentName) ? "N" : "L";
    case 2:
      return this->m_constituentNames.at(row);
    default:
      return concentrationFromIndex(concentrations, col - 3);
    }

    return {};
  }

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override
  {
    if (role != Qt::DisplayRole)
      return {};

    if (orientation == Qt::Vertical)
      return {};

    switch (section) {
    case 0:
      return QObject::tr("Complexations");
    case 1:
      return QObject::tr("Type");
    case 2:
      return QObject::tr("Name");
    default:
      return concentrationHeaderFromIndex(section - 3);
    }

    return {};
  }

  Qt::ItemFlags flags(const QModelIndex &idx) const override
  {
    Qt::ItemFlags defaultFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (!idx.isValid())
      return Qt::NoItemFlags;

    if (idx.column() < 3)
      return defaultFlags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    else if (idx.column() >= 3 && idx.column() < 3 + N)
      return defaultFlags | Qt::ItemIsEditable;

    return defaultFlags;
  }

  bool setData(const QModelIndex &idx, const QVariant &value, int role) override
  {
    if (role != Qt::EditRole)
      return false;

    const int row = idx.row();
    if (row < 0 || row >= this->m_constituentNames.size())
      return false;

    bool ok;
    double v = value.toReal(&ok);
    if (!ok)
      return false;

    const auto name = this->m_constituentNames.at(row).toStdString();
    auto concentrations = this->h_GDMProxy.concentrations(name);
    assert(concentrations.size() == static_cast<size_t>(this->NConcs()));

    const int col = idx.column();
    if (col >= 3 && col < 3 + N) {
      const double current = concentrations[col - 3];
      if (v == current)
        return false;

      concentrations[col - 3] = v;
      this->h_GDMProxy.setConcentrations(name, concentrations);
      emit this->dataChanged(idx, idx, { role } );
      return true;
    }

    return false;
  }

private:
  QVariant concentrationFromIndex(const std::vector<double> &concs, const int idx) const
  {
    if (idx >= 0 && idx < N)
      return concs.at(static_cast<size_t>(idx));
    return  {};
  }

  QVariant concentrationHeaderFromIndex(const int idx) const
  {
    if (idx >= 0 && idx < N)
      return  QString{"c %1 (mM)"}.arg(m_concentrationHeaders.at(idx));
    return {};
  }

  QVector<QString> m_concentrationHeaders;
};

#endif // CONSTITUENTSMODELIMPL_H
