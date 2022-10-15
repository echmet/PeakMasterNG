// vim: sw=2 ts=2 sts=2 expandtab

#include "compositioneditorwidget.h"

#include "complexationcolorizerdelegate.h"
#include "../gearbox/floatingvaluedelegate.h"
#include "internal_models/abstractconstituentsmodelbase.h"

static
void enableDragDrop(QTableView *v)
{
  v->setDragDropMode(QAbstractItemView::DragDrop);
  v->setDragDropOverwriteMode(false);
  v->setDropIndicatorShown(true);
  v->setDragEnabled(true);
  v->setAcceptDrops(true);
}

CompositionEditorWidget::CompositionEditorWidget(const QString &title, const QString &constituentType, AbstractConstituentsModelBase *model, QWidget *parent) :
  QWidget{parent},
  ui{new Ui::CompositionEditorWidget}
{
  ui->setupUi(this);

  auto ccDelegate = new ComplexationColorizerDelegate{model, this};
  m_fltDelegate = new FloatingValueDelegate{false, this};

  enableDragDrop(ui->qtbv_constituents);
  ui->qtbv_constituents->setModel(model);
  ui->qtbv_constituents->setItemDelegateForColumn(0, ccDelegate);
  ui->qtbv_constituents->setItemDelegateForColumn(3, m_fltDelegate);
  ui->qtbv_constituents->setMinimumHeight(100);

  ui->ql_title->setText(title);
  ui->qpb_add->setText(QString{tr("Add %1")}.arg(constituentType));
  ui->qpb_remove->setText(QString{tr("Remove %1")}.arg(constituentType));

  connect(ui->qpb_add, &QPushButton::clicked, this, [this]() { emit addConstituent(); });
  connect(ui->qpb_remove, &QPushButton::clicked, this, [this]() {
    const auto indices = ui->qtbv_constituents->selectionModel()->selectedIndexes();
    emit removeConstituent(indices);
  });
  connect(ui->qtbv_constituents, &QTableView::doubleClicked, this, [this](const QModelIndex &idx) { emit editRequested(idx); });

  setupIcons();
}

void CompositionEditorWidget::forceCommit()
{
  m_fltDelegate->forceCommit();
}

void CompositionEditorWidget::resizeHeader()
{
  ui->qtbv_constituents->resizeColumnToContents(2);
}

void CompositionEditorWidget::setupIcons()
{
#ifdef Q_OS_LINUX
  ui->qpb_add->setIcon(QIcon::fromTheme("list-add"));
  ui->qpb_remove->setIcon(QIcon::fromTheme("list-remove"));
#else
  ui->qpb_add->setIcon(style()->standardIcon(QStyle::SP_DialogOkButton));
  ui->qpb_remove->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
#endif // Q_OS_
}
