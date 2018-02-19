#include "pickconstituentfromdbdialog.h"
#include "ui_pickconstituentfromdbdialog.h"

#include "../gearbox/databaseproxy.h"
#include "internal_models/databaseconstituentsphyspropstablemodel.h"
#include "databasetableview.h"

#include <QMessageBox>

PickConstituentFromDBDialog::PickConstituentFromDBDialog(DatabaseConstituentsPhysPropsTableModel &model, DatabaseProxy &dbProxy, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::PickConstituentFromDBDialog},
  h_dbProxy{dbProxy},
  m_model{model},
  m_selectedIndex{-1}
{
  ui->setupUi(this);
  ui->qtbv_constituents->setModel(&m_model);

  connect(ui->qle_constituentName, &QLineEdit::textChanged, this, &PickConstituentFromDBDialog::onConstituentNameChanged);

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &PickConstituentFromDBDialog::onAccepted);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &PickConstituentFromDBDialog::onRejected);
  connect(ui->qtbv_constituents, &DatabaseTableView::itemSelected, this, &PickConstituentFromDBDialog::onItemSelected);
}

PickConstituentFromDBDialog::~PickConstituentFromDBDialog()
{
  delete ui;
}

void PickConstituentFromDBDialog::onAccepted()
{
  m_selectedIndex = -1; /* Safety net */

  if (m_model.rowCount() < 1)
    m_selectedIndex = -1;
  else if (m_model.rowCount() == 1)
    m_selectedIndex = 0;
  else {
    const auto &mids = ui->qtbv_constituents->selectionModel()->selectedIndexes();
    if (mids.size() < 1)
      m_selectedIndex = -1;
    else {
      if (!mids.first().isValid())
        m_selectedIndex = -1;
      else
        m_selectedIndex = mids.first().row();
    }
  }

  accept();
}

void PickConstituentFromDBDialog::onConstituentNameChanged(const QString &name)
{
  if (!h_dbProxy.isAvailable())
    return;

  try {
    std::string _name = name.toStdString();
    auto results = h_dbProxy.search(_name);

    m_model.refreshData(std::move(results));
  } catch (const DatabaseException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Database lookup failed"), ex.what()};
    mbox.exec();
  }
}

void PickConstituentFromDBDialog::onItemSelected(const int row)
{
  if (row < 0)
    return;

  m_selectedIndex = row;
  accept();
}

void PickConstituentFromDBDialog::onRejected()
{
  m_selectedIndex = -1;
  reject();
}

int PickConstituentFromDBDialog::selectedIndex() const
{
  return m_selectedIndex;
}

