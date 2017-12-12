#include "systemcompositionwidget.h"
#include "ui_systemcompositionwidget.h"

#include "../gearbox/gdmproxy.h"
#include "internal_models/constituentsmodelimpl.h"
#include "editconstituentdialog.h"
#include "../gearbox/constituentmanipulator.h"
#include "../gearbox/complexationmanager.h"
#include "complexationcolorizerdelegate.h"
#include "../gearbox/floatingvaluedelegate.h"

#include <QMessageBox>

void enableDragDrop(QTableView *v)
{
 v->setDragDropMode(QAbstractItemView::DragDrop);
 v->setDragDropOverwriteMode(false);
 v->setDropIndicatorShown(true);
 v->setDragEnabled(true);
 v->setAcceptDrops(true);
}

SystemCompositionWidget::SystemCompositionWidget(GDMProxy &backgroundGDM, GDMProxy &sampleGDM, ComplexationManager &cpxMgr, QWidget *parent) :
  QWidget{parent},
  ui{new Ui::SystemCompositionWidget},
  h_backgroundGDM{backgroundGDM},
  h_sampleGDM{sampleGDM},
  h_cpxMgr{cpxMgr}
{
  ui->setupUi(this);

  m_backgroundConstituentsModel = new ConstituentsModelImpl<2>{{ "BGE", "Sample" }, backgroundGDM, cpxMgr, this};
  ui->qtbv_backgroudConstituents->setModel(m_backgroundConstituentsModel);
  enableDragDrop(ui->qtbv_backgroudConstituents);
  m_ccDelegateBGE = new ComplexationColorizerDelegate{m_backgroundConstituentsModel, this};
  m_fltDelegateBGE = new FloatingValueDelegate{this};

  m_analytesModel = new ConstituentsModelImpl<1>{{ "Sample" }, sampleGDM, cpxMgr, this};
  ui->qtbv_analytes->setModel(m_analytesModel);
  enableDragDrop(ui->qtbv_analytes);
  m_ccDelegateAnalytes = new ComplexationColorizerDelegate{m_analytesModel, this};
  m_fltDelegateAnalytes = new FloatingValueDelegate{this};

  connect(ui->qpb_addAnalyte, &QPushButton::clicked, this, &SystemCompositionWidget::onAddAnalyte);
  connect(ui->qpb_addBGE, &QPushButton::clicked, this, &SystemCompositionWidget::onAddBGE);
  connect(ui->qpb_removeAnalyte, &QPushButton::clicked, this, &SystemCompositionWidget::onRemoveAnalyte);
  connect(ui->qpb_removeBGE, &QPushButton::clicked, this, &SystemCompositionWidget::onRemoveBGE);

  connect(ui->qtbv_analytes, &QTableView::doubleClicked, this, &SystemCompositionWidget::onAnalytesDoubleClicked);
  connect(ui->qtbv_backgroudConstituents, &QTableView::doubleClicked, this, &SystemCompositionWidget::onBGEDoubleClicked);

  connect(&cpxMgr, &ComplexationManager::complexationStatusChanged, m_backgroundConstituentsModel, &AbstractConstituentsModelBase::onComplexationStatusUpdated);
  connect(&cpxMgr, &ComplexationManager::complexationStatusChanged, m_analytesModel, &AbstractConstituentsModelBase::onComplexationStatusUpdated);
  connect(&cpxMgr, &ComplexationManager::complexationStatusChanged, this, &SystemCompositionWidget::onCompositionChanged);

  connect(m_backgroundConstituentsModel, &AbstractConstituentsModelBase::dataChanged, this, &SystemCompositionWidget::onCompositionChanged);
  connect(m_backgroundConstituentsModel, &AbstractConstituentsModelBase::rowsInserted, this, &SystemCompositionWidget::onCompositionChanged);
  connect(m_backgroundConstituentsModel, &AbstractConstituentsModelBase::rowsRemoved, this, &SystemCompositionWidget::onCompositionChanged);

  connect(m_analytesModel, &AbstractConstituentsModelBase::dataChanged, this, &SystemCompositionWidget::onCompositionChanged);
  connect(m_analytesModel, &AbstractConstituentsModelBase::rowsInserted, this, &SystemCompositionWidget::onCompositionChanged);
  connect(m_analytesModel, &AbstractConstituentsModelBase::rowsRemoved, this, &SystemCompositionWidget::onCompositionChanged);

  ui->qtbv_backgroudConstituents->setItemDelegateForColumn(0, m_ccDelegateBGE);
  ui->qtbv_backgroudConstituents->setItemDelegateForColumn(3, m_fltDelegateBGE);
  ui->qtbv_backgroudConstituents->setItemDelegateForColumn(4, m_fltDelegateBGE);
  ui->qtbv_analytes->setItemDelegateForColumn(0, m_ccDelegateAnalytes);
  ui->qtbv_analytes->setItemDelegateForColumn(3, m_fltDelegateAnalytes);

  ui->qtbv_backgroudConstituents->setMinimumHeight(100);
  ui->qtbv_analytes->setMinimumHeight(100);

  ui->qtbv_backgroudConstituents->resizeColumnsToContents();
  ui->qtbv_analytes->resizeColumnsToContents();

  setControlsIcons();
}

SystemCompositionWidget::~SystemCompositionWidget()
{
  delete ui;
}

void SystemCompositionWidget::addConstituent(GDMProxy &proxy, AbstractConstituentsModelBase *model)
{
  EditConstituentDialog dlg{this};
  ConstituentManipulator manipulator{proxy};

  connect(&dlg, &EditConstituentDialog::validateInput, &manipulator, &ConstituentManipulator::onValidateConstituentInput);

  if (dlg.exec() == QDialog::Accepted) {
    gdm::Constituent constituent = manipulator.makeConstituent(&dlg);

    proxy.insert(std::move(constituent));

    model->addConstituent(dlg.name());
  }
}

AbstractConstituentsModelBase * SystemCompositionWidget::analytesModel() noexcept
{
  return m_analytesModel;
}

AbstractConstituentsModelBase * SystemCompositionWidget::backgroundModel() noexcept
{
  return m_backgroundConstituentsModel;
}

void SystemCompositionWidget::editComplexation(const QString &name)
{
  if (!h_cpxMgr.complexes(name.toStdString()))
    return;

  h_cpxMgr.editComplexation(name.toStdString());
}

void SystemCompositionWidget::editConstituent(const QString &name, GDMProxy &proxy, AbstractConstituentsModelBase *model)
{
  ConstituentManipulator manipulator{proxy};

  EditConstituentDialog *dlg = manipulator.makeEditDialog(name.toStdString(), proxy);
  if (dlg == nullptr)
    return;

  connect(dlg, &EditConstituentDialog::validateInput, &manipulator, &ConstituentManipulator::onValidateConstituentInputUpdate);

  if (dlg->exec() == QDialog::Accepted) {
    gdm::Constituent updatedCtuent = manipulator.makeConstituent(dlg);

    try {
      if (!proxy.update(name.toStdString(), updatedCtuent)) {
        QMessageBox mbox{QMessageBox::Warning, tr("Operation failed"), tr("Failed to update the constituent properties")};
        mbox.exec();
      } else
        model->updateName(name, dlg->name());
    } catch (GDMProxyException &ex) {
      QMessageBox mbox{QMessageBox::Warning, tr("Operation failed"), ex.what()};
      mbox.exec();
    }
  }

  delete dlg;
}

void SystemCompositionWidget::handleDoubleClick(const int column, const QVariant &v, GDMProxy &GDM, AbstractConstituentsModelBase *model)
{
  if (!v.isValid())
   return;

  switch (column) {
  case 0:
    editComplexation(v.toString());
    break;
  case 1:
  case 2:
    editConstituent(v.toString(), GDM, model);
    break;
  }
}

void SystemCompositionWidget::removeConstituent(const QModelIndexList &indexes, GDMProxy &proxy, AbstractConstituentsModelBase *model)
{
  /* Remove only one constituent at a time */
  const int max = indexes.size() > 1 ? 1 : indexes.size();

  for (int idx = 0; idx < max; idx++) {
    const QModelIndex &midx = indexes.at(idx);
    if (midx.isValid()) {
      const QString name = model->removeConstituent(midx.row());
      const auto _name = name.toStdString();
      proxy.erase(_name);
      h_cpxMgr.notifyConstituentRemoved();
    }
  }
}

void SystemCompositionWidget::onAddAnalyte()
{
  addConstituent(h_sampleGDM, m_analytesModel);
}

void SystemCompositionWidget::onAddBGE()
{
  addConstituent(h_backgroundGDM, m_backgroundConstituentsModel);
}

void SystemCompositionWidget::onAnalytesDoubleClicked(const QModelIndex &idx)
{
  if (!idx.isValid())
    return;

  const QVariant v = m_analytesModel->data(m_analytesModel->index(idx.row(), 0), Qt::UserRole);
  handleDoubleClick(idx.column(), v, h_sampleGDM, m_analytesModel);
  ui->qtbv_analytes->resizeColumnToContents(2);
}

void SystemCompositionWidget::onBGEDoubleClicked(const QModelIndex &idx)
{
  if (!idx.isValid())
    return;

  const QVariant v = m_backgroundConstituentsModel->data(m_backgroundConstituentsModel->index(idx.row(), 0), Qt::UserRole);
  handleDoubleClick(idx.column(), v, h_backgroundGDM, m_backgroundConstituentsModel);
  ui->qtbv_backgroudConstituents->resizeColumnToContents(2);
}

void SystemCompositionWidget::onCompositionChanged()
{
  ui->qtbv_backgroudConstituents->resizeColumnToContents(2);
  ui->qtbv_analytes->resizeColumnToContents(2);

  emit compositionChanged();
}

void SystemCompositionWidget::onRemoveAnalyte()
{
  const auto indexes = ui->qtbv_analytes->selectionModel()->selectedIndexes();

  removeConstituent(indexes, h_sampleGDM, m_analytesModel);
}

void SystemCompositionWidget::onRemoveBGE()
{
  const auto indexes = ui->qtbv_backgroudConstituents->selectionModel()->selectedIndexes();

  removeConstituent(indexes, h_backgroundGDM, m_backgroundConstituentsModel);
}

void SystemCompositionWidget::setControlsIcons()
{
#ifdef Q_OS_LINUX
  ui->qpb_addAnalyte->setIcon(QIcon::fromTheme("list-add"));
  ui->qpb_addBGE->setIcon(QIcon::fromTheme("list-add"));

  ui->qpb_removeAnalyte->setIcon(QIcon::fromTheme("list-remove"));
  ui->qpb_removeBGE->setIcon(QIcon::fromTheme("list-remove"));
#else
  ui->qpb_addAnalyte->setIcon(style()->standardIcon(QStyle::SP_DialogOkButton));
  ui->qpb_addBGE->setIcon(style()->standardIcon(QStyle::SP_DialogOkButton));

  ui->qpb_removeAnalyte->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
  ui->qpb_removeBGE->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
#endif // Q_OS_
}

