// vim: sw=2 ts=2 sts=2 expandtab

#include "backgroundcompositionwidget.h"

#include "backgroundconstituentsheader.h"
#include "internal_models/backgroundconstituentsmodel.h"
#include "../gearbox/floatingvaluedelegate.h"

BackgroundCompositionWidget::BackgroundCompositionWidget(BackgroundConstituentsModel *model, QWidget *parent) :
  CompositionEditorWidget{"Background electrolyte:", "BGE", model, parent}
{
  ui->qtbv_constituents->setItemDelegateForColumn(4, m_fltDelegate);

  ui->qtbv_constituents->setMinimumHeight(100);

  auto bch = new BackgroundConstituentsHeader{Qt::Horizontal, ui->qtbv_constituents};
  ui->qtbv_constituents->setHorizontalHeader(bch);
  for (int col= 0; col < model->firstExtraInfoColumn(); col++)
    ui->qtbv_constituents->horizontalHeader()->setSectionResizeMode(col, QHeaderView::ResizeToContents);
  for (int col = model->firstExtraInfoColumn(); col < model->columnCount(QModelIndex{}); col++)
    ui->qtbv_constituents->setColumnWidth(col, bch->sizeHintForColumn(col));
}
