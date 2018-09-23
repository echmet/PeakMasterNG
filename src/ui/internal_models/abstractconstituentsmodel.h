#ifndef ABSTRACTCONSTITUENTSMODEL_H
#define ABSTRACTCONSTITUENTSMODEL_H

#include "abstractconstituentsmodelbase.h"

#include <QModelIndex>

template <int _NConcs>
class AbstractConstituentsModel : public AbstractConstituentsModelBase
{
public:
  using AbstractConstituentsModelBase::AbstractConstituentsModelBase;

  virtual int columnCount(const QModelIndex &parent) const override
  {
    if (parent.isValid())
      return 0;

    return 3 + NConcs();
  }

protected:
  constexpr int NConcs() const noexcept
  {
    static_assert(_NConcs > 0, "_NConcs must be greater than zero");
    return _NConcs;
  }
};

#endif // ABSTRACTCONSTITUENTSMODEL_H
