#ifndef COMPLEXATIONCOLORIZERDELEGATE_H
#define COMPLEXATIONCOLORIZERDELEGATE_H

#include <QItemDelegate>
#include "internal_models/abstractconstituentsmodelbase.h"

class ComplexationColorizerDelegate : public QItemDelegate
{
public:
  explicit ComplexationColorizerDelegate(AbstractConstituentsModelBase *model, QObject *parent = nullptr);
  virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
  AbstractConstituentsModelBase *h_model;
};

#endif // COMPLEXATIONCOLORIZERDELEGATE_H
