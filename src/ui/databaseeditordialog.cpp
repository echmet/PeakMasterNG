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
  ui->qtbv_constituents->setModel(m_model);

  connect(ui->qpb_add, &QPushButton::clicked, this, &DatabaseEditorDialog::onAddConstituent);
  connect(ui->qpb_allCompounds, &QPushButton::clicked, this, &DatabaseEditorDialog::onAllCompounds);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DatabaseEditorDialog::accept);
  connect(ui->qpb_delete, &QPushButton::clicked, this, &DatabaseEditorDialog::onDeleteConstituent);
  connect(ui->qpb_edit, &QPushButton::clicked, this, &DatabaseEditorDialog::onEditConstituent);
  connect(ui->qle_constituentName, &QLineEdit::textChanged, this, &DatabaseEditorDialog::onConstituentNameChanged);
}

DatabaseEditorDialog::~DatabaseEditorDialog()
{
  delete ui;
  delete m_model;
}

int DatabaseEditorDialog::getIndex() const
{
  const auto &selection = ui->qtbv_constituents->selectionModel()->selectedIndexes();
  if (selection.empty())
    return -1;

  const auto &idx = selection.first();
  if (!idx.isValid())
    return -1;

  const int row = idx.row();
  if (row < 0 || row >= m_model->rowCount())
    return -1;

  return row;
}

void DatabaseEditorDialog::onAddConstituent()
{
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
  if (!h_dbProxy.isAvailable())
    return;

  try {
    std::string _name = name.toStdString();
    auto results = h_dbProxy.search(_name);

    m_model->refreshData(std::move(results));
    ui->qtbv_constituents->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->qtbv_constituents->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
  } catch (const DatabaseException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Database lookup failed"), ex.what()};
    mbox.exec();
  }
}

void DatabaseEditorDialog::onDeleteConstituent()
{
  const int idx = getIndex();
  if (idx < 0)
    return;

  try {
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
  } catch (const DatabaseException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Database query error"), QString{tr("Database query error: %1")}.arg(ex.what())};
    mbox.exec();
  }
}

void DatabaseEditorDialog::onEditConstituent()
{
  const int idx = getIndex();
  if (idx < 0)
    return;

  try {
    const auto &ctuent = m_model->constituentAt(idx);
    EditDatabaseConstituentDialog dlg{ctuent.name, ctuent.pKas, ctuent.mobilities, ctuent.chargeLow, ctuent.chargeHigh, this};

    connect(&dlg, &EditDatabaseConstituentDialog::validateInput,
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
  } catch (const DatabaseException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Database query error"), QString{tr("Database query error: %1")}.arg(ex.what())};
    mbox.exec();
  }
}
