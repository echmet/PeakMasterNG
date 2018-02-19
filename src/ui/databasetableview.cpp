#include "databasetableview.h"

#include <QMouseEvent>
#include <QHeaderView>

DatabaseTableView::DatabaseTableView(QWidget *parent) : QTableView{parent}
{
  setSelectionMode(QAbstractItemView::SingleSelection);

  connect(this->verticalHeader(), &QHeaderView::sectionDoubleClicked, this, &DatabaseTableView::onSectionSelected);
}

void DatabaseTableView::mouseDoubleClickEvent(QMouseEvent *evt)
{
  if (evt->button() != Qt::LeftButton)
    return;

  const auto &indices = selectionModel()->selectedIndexes();

  if (indices.empty())
    return;

  emit itemSelected(indices.first().row());
}

void DatabaseTableView::onSectionSelected(const int idx)
{
  if (idx >= 0 || idx < model()->rowCount())
    emit itemSelected(idx);
}
