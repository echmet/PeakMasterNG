#include "copiableitemstableview.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>

#include "../gearbox/doubletostringconvertor.h"

CopiableItemsTableView::CopiableItemsTableView(QWidget *parent) :
  QTableView{parent}
{
  m_rightClickMenu = new QMenu{this};

  QAction *a = new QAction{tr("Copy"), this};
  connect(a, &QAction::triggered, this, &CopiableItemsTableView::onCopyActionTriggered);
  m_rightClickMenu->addAction(a);
}

void CopiableItemsTableView::keyPressEvent(QKeyEvent *evt)
{
  if (evt->key() == Qt::Key_C && evt->modifiers() == Qt::ControlModifier)
    selectionToClipboard(selectedIndexes());
}

void CopiableItemsTableView::mousePressEvent(QMouseEvent *evt)
{
  QTableView::mousePressEvent(evt);

  if (evt->button() == Qt::RightButton) {
    if (selectedIndexes().empty())
      return;

    m_rightClickMenu->exec(evt->globalPos());
  }
}

void CopiableItemsTableView::onCopyActionTriggered()
{
  selectionToClipboard(selectedIndexes());
}

void CopiableItemsTableView::selectionToClipboard(const QModelIndexList &indexes)
{
  if (indexes.empty())
    return;

  auto varToStr = [](const QVariant &var) {
    bool ok;
    var.toDouble(&ok);
    if (ok)
      return DoubleToStringConvertor::convert(var.toDouble());
    return var.toString();
  };

  QString out;
  for (int ni = 0; ni < indexes.size() - 1; ni++) {
    const auto &idx = indexes.at(ni);
    out += varToStr(idx.data())  + ";";
  }

  out += varToStr(indexes.back().data());

  QApplication::clipboard()->setText(out);
}
