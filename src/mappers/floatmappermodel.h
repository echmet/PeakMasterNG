#ifndef FLOATMAPPERMODEL_H
#define FLOATMAPPERMODEL_H

#include "abstractmappermodel.h"
#include "../gearbox/doubletostringconvertor.h"

template <typename I>
class FloatMapperModel : public AbstractMapperModel<double, I>
{
public:
  FloatMapperModel(QObject *parent) :
    AbstractMapperModel<double, I>{parent}
  {
  }

  virtual QVariant data(const QModelIndex &index, int role) const override
  {
    if (!this->isParametersValid(index, role, { Qt::EditRole, Qt::UserRole + 1 }))
      return false;

    if (role == Qt::UserRole + 1)
      return this->m_decimalDigits.at(index.column());

    return AbstractMapperModel<double, I>::data(index, role);
  }

  virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override
  {
    if (!this->isParametersValid(index, role, { Qt::EditRole, Qt::UserRole + 1 }))
      return false;

    if (this->m_data == nullptr)
      return false;

    switch (role) {
    case Qt::EditRole:
    {
      const auto currData = this->m_data->operator[](index.column());
      const auto newData = value.toReal();
      if (currData == newData)
          return false;

      (*(this->m_data))[index.column()] = value.toReal();
      emit this->dataChanged(this->createIndex(0, index.column()), this->createIndex(0, index.column()), { role });
      return true;
    }
    case Qt::UserRole + 1:
      {
      bool ok;
      const int prec = value.toInt(&ok);
      if (!ok)
        return false;
      this->m_decimalDigits[index.column()] = prec;
      return true;
      }
    }

    return false;
  }

  virtual void notifyAllDataChanged(const QVector<int> &roles = { Qt::DisplayRole }) override
  {
    setPrecision(0, this->m_data->size() - 1);
    AbstractMapperModel<double, I>::notifyAllDataChanged(roles);
  }

  virtual void notifyDataChanged(const I &fromIndex, const I &toIndex, const QVector<int> &roles = { Qt::DisplayRole }) override
  {
    const int fromIdx = this->indexFromItem(fromIndex);
    const int toIdx = this->indexFromItem(toIndex);

    setPrecision(fromIdx, toIdx);
    AbstractMapperModel<double, I>::notifyDataChanged(fromIndex, toIndex, roles);
  }

  virtual void notifyDataChangedFromStart(const I &toIndex, const QVector<int> &roles = { Qt::DisplayRole }) override
  {
    const int toIdx = this->indexFromItem(toIndex);

    setPrecision(0, toIdx);
    AbstractMapperModel<double, I>::notifyDataChangedFromStart(toIndex, roles);
  }

  virtual void notifyDataChangedToEnd(const I &fromIndex, const QVector<int> &roles = { Qt::DisplayRole }) override
  {
    const int fromIdx = this->indexFromItem(fromIndex);

    setPrecision(fromIdx, this->m_data->size());
    AbstractMapperModel<double, I>::notifyDataChangedToEnd(fromIndex, roles);
  }

  virtual void setUnderlyingData(QVector<double> *data) override
  {
    this->m_decimalDigits.resize(data->size());

    AbstractMapperModel<double, I>::setUnderlyingData(data);
    setPrecision(0, data->size()-1);
  }

private:
  void setPrecision(const int from, const int to)
  {
    Q_ASSERT(from >= 0 && to < this->m_data->size());
    Q_ASSERT(from <= to);

    for (int idx = from; idx <= to; idx++) {
      const int prec = DoubleToStringConvertor::guessPrecision(this->m_data->at(idx));
      this->m_decimalDigits[idx] = prec;
    }
  }

  QVector<int> m_decimalDigits;
};

#endif // FLOATMAPPERMODEL_H
