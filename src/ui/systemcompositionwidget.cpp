#include "systemcompositionwidget.h"
#include "ui_systemcompositionwidget.h"

#include "../gearbox/gdmproxy.h"
#include "internal_models/analytesconstituentsmodel.h"
#include "internal_models/backgroundconstituentsmodel.h"
#include "editconstituentdialog.h"
#include "../gearbox/constituentmanipulator.h"
#include "../gearbox/complexationmanager.h"
#include "../gearbox/databaseproxy.h"
#include "analytescompositionwidget.h"
#include "backgroundcompositionwidget.h"
#include "compositioneditorwidget.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSplitter>

SystemCompositionWidget::SystemCompositionWidget(GDMProxy &backgroundGDM, GDMProxy &sampleGDM, ComplexationManager &cpxMgr, DatabaseProxy &dbProxy,
                                                 const AnalytesExtraInfoModel * const analytesEXIModel,
                                                 const BackgroundEffectiveMobilitiesModel * const BGEEffMobsModel,
                                                 QWidget *parent) :
  QWidget{parent},
  h_backgroundGDM{backgroundGDM},
  h_sampleGDM{sampleGDM},
  h_cpxMgr{cpxMgr},
  h_dbProxy{dbProxy},
  m_viscosityCorrectionEnabled{false}
{
  m_backgroundConstituentsModel = new BackgroundConstituentsModel{BGEEffMobsModel, { "BGE", "Sample" }, backgroundGDM, cpxMgr, this};
  m_analytesModel = new AnalytesConstituentsModel{analytesEXIModel, { "Sample" }, sampleGDM, cpxMgr, this};

  m_background = new BackgroundCompositionWidget{m_backgroundConstituentsModel, this};
  m_analytes = new AnalytesCompositionWidget{m_analytesModel, this};

  connect(m_background, &CompositionEditorWidget::addConstituent, this, &SystemCompositionWidget::onAddBGE);
  connect(m_background, &CompositionEditorWidget::removeConstituent, this, &SystemCompositionWidget::onRemoveBGE);
  connect(m_background, &CompositionEditorWidget::editRequested, this, &SystemCompositionWidget::onBGEEditRequested);

  connect(m_analytes, &CompositionEditorWidget::addConstituent, this, &SystemCompositionWidget::onAddAnalyte);
  connect(m_analytes, &CompositionEditorWidget::removeConstituent, this, &SystemCompositionWidget::onRemoveAnalyte);
  connect(m_analytes, &CompositionEditorWidget::editRequested, this, &SystemCompositionWidget::onAnalytesEditRequested);

  connect(&cpxMgr, &ComplexationManager::complexationStatusChanged, m_backgroundConstituentsModel, &AbstractConstituentsModelBase::onComplexationStatusUpdated);
  connect(&cpxMgr, &ComplexationManager::complexationStatusChanged, m_analytesModel, &AbstractConstituentsModelBase::onComplexationStatusUpdated);
  connect(&cpxMgr, &ComplexationManager::complexationStatusChanged, this, &SystemCompositionWidget::onCompositionChanged);

  connect(m_backgroundConstituentsModel, &AbstractConstituentsModelBase::dataChanged, this, &SystemCompositionWidget::onBackgroundChanged);
  connect(m_backgroundConstituentsModel, &AbstractConstituentsModelBase::rowsInserted, this, &SystemCompositionWidget::onCompositionChanged);
  connect(m_backgroundConstituentsModel, &AbstractConstituentsModelBase::rowsRemoved, this, &SystemCompositionWidget::onCompositionChanged);

  connect(m_analytesModel, &AbstractConstituentsModelBase::dataChanged, this, &SystemCompositionWidget::onAnalytesDataChanged);
  connect(m_analytesModel, &AbstractConstituentsModelBase::rowsInserted, this, &SystemCompositionWidget::onCompositionChanged);
  connect(m_analytesModel, &AbstractConstituentsModelBase::rowsRemoved, this, &SystemCompositionWidget::onCompositionChanged);

  auto layout = new QVBoxLayout{this};
  auto splitter = new QSplitter{Qt::Horizontal, this};
  layout->addWidget(splitter);
  splitter->addWidget(m_background);
  splitter->addWidget(m_analytes);

  splitter->setStretchFactor(0, 2);
  splitter->setStretchFactor(1, 3);

  setLayout(layout);
}

SystemCompositionWidget::~SystemCompositionWidget()
{
}

bool SystemCompositionWidget::addConstituent(GDMProxy &proxy, AbstractConstituentsModelBase *model)
{
  static QSize m_dlgSize{};

  EditConstituentDialog dlg{h_dbProxy, m_viscosityCorrectionEnabled, this};
  ConstituentManipulator manipulator{proxy, m_viscosityCorrectionEnabled};

  if (!m_dlgSize.isEmpty())
    dlg.resize(m_dlgSize);

  connect(&dlg, &EditConstituentDialog::validateInput, &manipulator, &ConstituentManipulator::onValidateConstituentInput);
  connect(&dlg, &EditConstituentDialog::addToDatabase, this, &SystemCompositionWidget::onAddToDatabase);

  if (dlg.exec() == QDialog::Accepted) {
    gdm::Constituent constituent = manipulator.makeConstituent(&dlg);

    proxy.insert(constituent);

    model->addConstituent(dlg.name());
    m_dlgSize = dlg.size();

    return true;
  }

  return false;
}

AbstractConstituentsModelBase * SystemCompositionWidget::analytesModel() noexcept
{
  return m_analytesModel;
}

AbstractConstituentsModelBase * SystemCompositionWidget::backgroundModel() noexcept
{
  return m_backgroundConstituentsModel;
}

void SystemCompositionWidget::commit()
{
  m_background->forceCommit();
  m_analytes->forceCommit();
}

void SystemCompositionWidget::editComplexation(const QString &name)
{
  if (!h_cpxMgr.complexes(name.toStdString()))
    return;

  h_cpxMgr.editComplexation(name.toStdString());
}

void SystemCompositionWidget::editConstituent(const QString &name, GDMProxy &proxy, AbstractConstituentsModelBase *model)
{
  ConstituentManipulator manipulator{proxy, m_viscosityCorrectionEnabled};

  EditConstituentDialog *dlg = manipulator.makeEditDialog(name.toStdString(), proxy, h_dbProxy);
  if (dlg == nullptr)
    return;

  connect(dlg, &EditConstituentDialog::validateInput, &manipulator, &ConstituentManipulator::onValidateConstituentInputUpdate);
  connect(dlg, &EditConstituentDialog::addToDatabase, this, &SystemCompositionWidget::onAddToDatabase);

  if (dlg->exec() == QDialog::Accepted) {
    gdm::Constituent updatedCtuent = manipulator.makeConstituent(dlg);

    try {
      if (!proxy.update(name.toStdString(), updatedCtuent)) {
        QMessageBox mbox{QMessageBox::Warning, tr("Operation failed"), tr("Failed to update the constituent properties")};
        mbox.exec();
      } else {
        model->updateName(name, dlg->name());
        h_cpxMgr.refreshAll();
      }
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
  if (addConstituent(h_sampleGDM, m_analytesModel))
    m_analytes->resizeHeader();
}

void SystemCompositionWidget::onAddBGE()
{
  if (addConstituent(h_backgroundGDM, m_backgroundConstituentsModel))
    m_background->resizeHeader();
}

void SystemCompositionWidget::onAddToDatabase(const EditConstituentDialog *dlg)
{
  if (!ConstituentManipulator::validateConstituentProperties(dlg))
    return;

  const auto ctuent = ConstituentManipulator::makeConstituent(dlg);
  h_dbProxy.addConstituent(ctuent.name(), ctuent.physicalProperties().pKas(), ctuent.physicalProperties().mobilities(), ctuent.physicalProperties().charges().low(), ctuent.physicalProperties().charges().high());
}

void SystemCompositionWidget::onAnalytesDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
  Q_UNUSED(roles);
  Q_UNUSED(bottomRight);

  const int firstExtraCol = m_analytesModel->firstExtraInfoColumn();

  if (topLeft.column() < firstExtraCol)
    onCompositionChanged();
}

void SystemCompositionWidget::onAnalytesEditRequested(const QModelIndex &idx)
{
  if (!idx.isValid())
    return;

  const QVariant v = m_analytesModel->data(m_analytesModel->index(idx.row(), 0), Qt::UserRole);
  handleDoubleClick(idx.column(), v, h_sampleGDM, m_analytesModel);
  m_analytes->resizeHeader();
}

void  SystemCompositionWidget::onBackgroundChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
  Q_UNUSED(roles);
  Q_UNUSED(bottomRight);

  if (topLeft.column() < m_backgroundConstituentsModel->firstExtraInfoColumn())
    onCompositionChanged();
}

void SystemCompositionWidget::onBGEEditRequested(const QModelIndex &idx)
{
  if (!idx.isValid())
    return;

  const QVariant v = m_backgroundConstituentsModel->data(m_backgroundConstituentsModel->index(idx.row(), 0), Qt::UserRole);
  handleDoubleClick(idx.column(), v, h_backgroundGDM, m_backgroundConstituentsModel);
  m_background->resizeHeader();
}

void SystemCompositionWidget::onCompositionChanged()
{
  emit compositionChanged();
}

void SystemCompositionWidget::onRemoveAnalyte(const QModelIndexList &indices)
{
  removeConstituent(indices, h_sampleGDM, m_analytesModel);
}

void SystemCompositionWidget::onRemoveBGE(const QModelIndexList &indices)
{
  removeConstituent(indices, h_backgroundGDM, m_backgroundConstituentsModel);
}

void SystemCompositionWidget::setViscosityCorrectionEnabled(const bool enabled)
{
  m_viscosityCorrectionEnabled = enabled;
}
