#ifndef FLOATMAPPERMODEL_H
#define FLOATMAPPERMODEL_H

#include "abstractmappermodel.h"

template <typename I>
class FloatMapperModel : public AbstractMapperModel<double, I>
{
public:
  FloatMapperModel(QObject *parent) :
    AbstractMapperModel<double, I>{parent}
  {
  }
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override
  {
    if (!this->isParametersValid(index, role))
      return false;

    if (this->m_data == nullptr)
      return false;

    (*(this->m_data))[index.column()] = value.toReal();
    emit this->dataChanged(this->createIndex(0, index.column()), this->createIndex(0, index.column()), { role });
    return true;
  }
};

#endif // FLOATMAPPERMODEL_H
