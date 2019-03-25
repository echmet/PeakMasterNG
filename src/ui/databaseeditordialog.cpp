#include "databaseeditordialog.h"
#include "ui_databaseeditordialog.h"

#include "internal_models/databaseconstituentsphyspropstablemodel.h"
#include "editdatabaseconstituentdialog.h"
#include "../gearbox/constituentmanipulator.h"

#include <QMessageBox>

DatabaseEditorDialog::DatabaseEditorDialog(DatabaseProxy &dbProxy, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::DatabaseEditorDialog},
  h_dbProxy{dbProxy}
{
  ui->setupUi(this);

  m_model = new DatabaseConstituentsPhysPropsTableModel{};
  m_proxyModel.setSourceModel(m_model);

  ui->qtbv_constituents->setModel(&m_proxyModel);
  ui->qtbv_constituents->setSortingEnabled(true);

  ui->qcbox_matchType->addItem(tr("Name begins with..."), QVariant::fromValue(DatabaseProxy::MatchType::BEGINS_WITH));
  ui->qcbox_matchType->addItem(tr("Name contains..."), QVariant::fromValue(DatabaseProxy::MatchType::CONTAINS));

  connect(ui->qpb_add, &QPushButton::clicked, this, &DatabaseEditorDialog::onAddConstituent);
  connect(ui->qpb_allCompounds, &QPushButton::clicked, this, &DatabaseEditorDialog::onAllCompounds);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DatabaseEditorDialog::accept);
  connect(ui->qpb_delete, &QPushButton::clicked, this, &DatabaseEditorDialog::onDeleteConstituent);
  connect(ui->qpb_edit, &QPushButton::clicked, this, &DatabaseEditorDialog::onEditConstituent);
  connect(ui->qle_constituentName, &QLineEdit::textChanged, this, &DatabaseEditorDialog::onConstituentNameChanged);
  connect(ui->qtbv_constituents, &DatabaseTableView::itemSelected, this, &DatabaseEditorDialog::editConstituent);
  connect(ui->qcbox_matchType, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &DatabaseEditorDialog::onMatchTypeActivated);
}

DatabaseEditorDialog::~DatabaseEditorDialog()
{
  delete ui;
  delete m_model;
}

void DatabaseEditorDialog::editConstituent(const QModelIndex &index)
{
  if (!index.isValid())
    return;

  const int row = getConstituentRow(index);
  if (row < 0 || row >= m_model->rowCount())
    return;

  const auto &ctuent = m_model->constituentAt(row);

  EditDatabaseConstituentDialog dlg{ctuent.name, ctuent.pKas, ctuent.mobilities, ctuent.chargeLow, ctuent.chargeHigh, this};

  connect(&dlg, &EditDatabaseConstituentDialog::validateInput, this,
          [](const EditDatabaseConstituentDialog *me, bool *ok) {
            *ok = ConstituentManipulator::validateConstituentProperties(me);
          }
  );

  if (dlg.exec() != QDialog::Accepted)
    return;

  const QString msg = QString{tr("Are you sure you want to edit constituent %1 in the database?\n"
                                 "This action cannot be undone!")}.arg(ctuent.name);
  QMessageBox aboutToEdit{QMessageBox::Warning,
                         tr("Confirm action"),
                         msg,
                         QMessageBox::Yes | QMessageBox::No};
  if (aboutToEdit.exec() != QMessageBox::Yes)
    return;

  if (!h_dbProxy.editConstituent(ctuent.id, dlg.name().toStdString(), dlg.pKas(), dlg.mobilities(), dlg.chargeLow(), dlg.chargeHigh())) {
      QMessageBox errBox{QMessageBox::Warning,
                         tr("Database operation failed"),
                         tr("Failed to update constituent in the database")};
      errBox.exec();
      return;
  }

  onConstituentNameChanged(ui->qle_constituentName->text());
}

void DatabaseEditorDialog::executeSearch(const QString &name, const QVariant &matchVar)
{
  if (!h_dbProxy.isAvailable())
    return;

  try {
     const auto match = [matchVar]() {
      if (!matchVar.canConvert<DatabaseProxy::MatchType>())
        return DatabaseProxy::MatchType::BEGINS_WITH;
      return matchVar.value<DatabaseProxy::MatchType>();
    }();

    std::string _name = name.toStdString();
    auto results = h_dbProxy.search(_name, match);

    m_model->refreshData(std::move(results));
    ui->qtbv_constituents->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->qtbv_constituents->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
  } catch (const DatabaseException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Database lookup failed"), ex.what()};
    mbox.exec();
  }
}

int DatabaseEditorDialog::getConstituentRow(const QModelIndex &index) const
{
  if (!index.isValid())
    return -1;

  const int row = m_proxyModel.mapToSource(index).row();
  if (row < 0 || row >= m_model->rowCount())
    return -1;

  return row;
}

QModelIndex DatabaseEditorDialog::getIndex() const
{
  const auto &selection = ui->qtbv_constituents->selectionModel()->selectedIndexes();
  if (selection.empty())
      return {};

  const auto &idx = selection.first();
  if (!idx.isValid())
      return {};

  return idx;
}

void DatabaseEditorDialog::onAddConstituent()
{
  if (!h_dbProxy.isAvailable())
    return;

  EditDatabaseConstituentDialog dlg{this};

  connect(&dlg, &EditDatabaseConstituentDialog::validateInput,
          [](const EditDatabaseConstituentDialog *me, bool *ok) {
            *ok = ConstituentManipulator::validateConstituentProperties(me);
          }
  );

  if (dlg.exec() != QDialog::Accepted)
    return;

  if (!h_dbProxy.addConstituent(dlg.name().toStdString(), dlg.pKas(), dlg.mobilities(), dlg.chargeLow(), dlg.chargeHigh())) {
    QMessageBox errBox{QMessageBox::Warning,
                       tr("Database operation failed"),
                       tr("Failed to add constituent to the database")};
    errBox.exec();
    return;
  }

  onConstituentNameChanged(dlg.name());
}

void DatabaseEditorDialog::onAllCompounds()
{
  if (!h_dbProxy.isAvailable())
    return;

  try {
    auto results = h_dbProxy.fetchAll();

    m_model->refreshData(std::move(results));
  } catch (const DatabaseException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Database lookup failed"), ex.what()};
    mbox.exec();
  }
}

void DatabaseEditorDialog::onConstituentNameChanged(const QString &name)
{
  executeSearch(name, ui->qcbox_matchType->currentData());
}

void DatabaseEditorDialog::onDeleteConstituent()
{
  if (!h_dbProxy.isAvailable())
    return;

  const int idx = getConstituentRow(getIndex());
  if (idx < 0)
    return;

  const auto &ctuent = m_model->constituentAt(idx);

  const QString msg = QString{tr("Are you sure you want to delete constituent %1 from the database?\n"
                                   "This action cannot be undone!")}.arg(ctuent.name);
  QMessageBox aboutToDel{QMessageBox::Warning,
                         tr("Confirm action"),
                         msg,
                         QMessageBox::Yes | QMessageBox::No};

  if (aboutToDel.exec() == QMessageBox::Yes) {
    if (!h_dbProxy.deleteById(ctuent.id)) {
        QMessageBox errBox{QMessageBox::Warning,
                        tr("Database operation failed"),
                        tr("Failed to delete constituent from the database")};
      errBox.exec();
      return;
    }

    onConstituentNameChanged(ui->qle_constituentName->text());
  }
}

void DatabaseEditorDialog::onEditConstituent()
{
  if (!h_dbProxy.isAvailable())
    return;

  editConstituent(getIndex());
}

void DatabaseEditorDialog::onMatchTypeActivated(const int)
{
  executeSearch(ui->qle_constituentName->text(), ui->qcbox_matchType->currentData());
}
