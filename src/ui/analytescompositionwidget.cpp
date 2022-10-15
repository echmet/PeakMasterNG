// vim: sw=2 ts=2 sts=2 expandtab

#include "analytescompositionwidget.h"

#include "analytesconstituentsheader.h"
#include "internal_models/analytesconstituentsmodel.h"

AnalytesCompositionWidget::AnalytesCompositionWidget(AnalytesConstituentsModel *model, QWidget *parent) :
  CompositionEditorWidget{"Analytes:", "analyte", model, parent}
{
  ui->qtbv_constituents->setMinimumHeight(100);

  auto ach = new AnalytesConstituentsHeader{Qt::Horizontal, ui->qtbv_constituents};
  ui->qtbv_constituents->setHorizontalHeader(ach);

  for (int col = 0; col < model->firstExtraInfoColumn(); col++)
    ui->qtbv_constituents->horizontalHeader()->setSectionResizeMode(col, QHeaderView::ResizeToContents);
  for (int col = model->firstExtraInfoColumn(); col < model->columnCount(QModelIndex{}); col++)
    ui->qtbv_constituents->setColumnWidth(col, ach->sizeHintForColumn(col));
}
