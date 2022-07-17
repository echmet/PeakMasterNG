#include "../mappers/userroles.h"
#include "complexationcolorizerdelegate.h"

#include <cassert>
#include <cmath>
#include <QPainter>

static
void paintLigand(QPainter *painter, const QStyleOptionViewItem &option, const QList<QColor> &colors)
{
  const int w = option.rect.width();
  const int h = option.rect.height();
  const int fromY = option.rect.y();
  const int step = static_cast<int>(std::ceil(static_cast<double>(w) / colors.size()));

  int fromX = option.rect.x();
  for (const auto &clr : colors) {
    QBrush br{clr};
    const int _step = [&]() {
      const int end = fromX + step;
      if (end > w)
        return step - (end - w);
      return step;
    }();

    painter->fillRect(fromX, fromY, _step, h, br);
    fromX += step;
  }
}

static
void paintNucleus(QPainter *painter, const QStyleOptionViewItem &option, const QColor &clr)
{
  painter->fillRect(option.rect, QBrush{clr});
}

ComplexationColorizerDelegate::ComplexationColorizerDelegate(AbstractConstituentsModelBase *model, QObject *parent) :
  QItemDelegate{parent},
  h_model{model}
{
}

void ComplexationColorizerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QVariant vList = h_model->data(h_model->index(index.row(), 0), UserRoles::PrecissionRole);
  if (!vList.canConvert<QList<QColor>>())
    return;

  const auto list = qvariant_cast<QList<QColor>>(vList);
  assert(!list.empty());

  QVariant vType = h_model->data(h_model->index(index.row(), 1));
  const QString type = vType.toString();

  painter->save();
  if (type == "N")
    paintNucleus(painter, option, list.at(0));
  else if (type == "L")
    paintLigand(painter, option, list);
  painter->restore();
}
