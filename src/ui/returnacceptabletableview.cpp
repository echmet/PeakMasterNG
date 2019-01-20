#include "returnacceptabletableview.h"

#include <QKeyEvent>

ReturnAcceptableTableView::ReturnAcceptableTableView(QWidget *parent) :
  CopiableItemsTableView{parent}
{
}

void ReturnAcceptableTableView::keyPressEvent(QKeyEvent *evt)
{
  CopiableItemsTableView::keyPressEvent(evt);

  if (evt->key() == Qt::Key_Return && evt->modifiers() == Qt::CTRL) {
    /* This is necessary to make sure that
     * any edited value gets commited */
    if (!hasFocus())
      setFocus();
    clearFocus();

    emit returnPressed();
  }
}
