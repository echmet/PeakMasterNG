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
  auto isArrowKey = [](const int key) {
    return (key == Qt::LeftArrow) || (key == Qt::RightArrow) ||
           (key == Qt::UpArrow)|| (key == Qt::DownArrow);
  };

  auto moveSelection = [&](const QModelIndex &idx, const int right, const int down) {
    auto newIdx = idx.model()->index(idx.row() + right , idx.column() + down);

    selectionModel()->clearSelection();
    selectionModel()->select(newIdx, QItemSelectionModel::Select);
  };

  const int key = evt->key();

  if (key == Qt::Key_C && evt->modifiers() == Qt::ControlModifier)
    selectionToClipboard(selectedIndexes());
  else if (isArrowKey(key)) {
    auto selIdxs = selectedIndexes();
    if (selIdxs.size() > 0 && selIdxs.at(0).isValid()) {
      auto sel = selIdxs[0];

      switch (key) {
      case Qt::LeftArrow:
        if (sel.column() > 1)
          moveSelection(sel, -1, 0);
        break;
      case Qt::RightArrow:
        if (sel.column() < model()->columnCount() - 1)
          moveSelection(sel, 1, 0);
        break;
      case Qt::UpArrow:
        if (sel.row() > 1)
          moveSelection(sel, 0, -1);
        break;
      case Qt::DownArrow:
        if (sel.row() <  model()->rowCount() - 1)
          moveSelection(sel, 0, 1);
        break;
      }
    }
  } else if (key == Qt::Key_Return || key == Qt::Key_Enter) {
    auto selIdxs = selectedIndexes();
    if (selIdxs.size() > 0 && selIdxs.at(0).isValid())
      edit(selIdxs[0]);
  }

  QTableView::keyPressEvent(evt);
}

void CopiableItemsTableView::mousePressEvent(QMouseEvent *evt)
{
  if (evt->button() == Qt::RightButton) {
    if (selectedIndexes().empty())
      return;

    m_rightClickMenu->exec(evt->globalPos());
  } else if (evt->button() == Qt::LeftButton) {
        const auto &idx = indexAt(evt->pos());
    if (idx.isValid()) {
      selectionModel()->clearSelection();
      selectionModel()->select(idx, QItemSelectionModel::Select);
      edit(idx);
    }
  }

  QTableView::mousePressEvent(evt);
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
      return DoubleToStringConvertor::locale().toString(var.toDouble(nullptr));
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
