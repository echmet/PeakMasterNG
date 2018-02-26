#include "databaseeditordialog.h"
#include "ui_databaseeditordialog.h"

#include "internal_models/databaseconstituentsphyspropstablemodel.h"

#include <QMessageBox>

DatabaseEditorDialog::DatabaseEditorDialog(DatabaseProxy &dbProxy, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::DatabaseEditorDialog},
  h_dbProxy{dbProxy}
{
  ui->setupUi(this);

  m_model = new DatabaseConstituentsPhysPropsTableModel{};
  ui->qtbv_constituents->setModel(m_model);

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
  } catch (const DatabaseException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Database lookup failed"), ex.what()};
    mbox.exec();
  }
}

void DatabaseEditorDialog::onDeleteConstituent()
{
  const auto &selection = ui->qtbv_constituents->selectionModel()->selectedIndexes();
  if (selection.empty())
    return;

  const auto &idx = selection.first();
  if (!idx.isValid())
    return;

  const int row = idx.row();
  if (row < 0 || row >= m_model->rowCount())
    return;

  auto &ctuent = m_model->constituentAt(row);

  const QString msg = QString{tr("Are you sure you want to delete constituent %1 from the database?\n"
                                 "This action cannot be undone!")}.arg(ctuent.name);
  QMessageBox aboutToDel{QMessageBox::Warning,
                         tr("Confirm action"),
                         msg,
                         QMessageBox::Yes | QMessageBox::No};

  if (aboutToDel.exec() == QMessageBox::Yes) {
    h_dbProxy.deleteById(ctuent.id);

    onConstituentNameChanged(ctuent.name);
  }
}

void DatabaseEditorDialog::onEditConstituent()
{

}
