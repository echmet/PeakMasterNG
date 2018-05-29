#include "pickconstituentfromdbdialog.h"
#include "ui_pickconstituentfromdbdialog.h"

#include "../gearbox/databaseproxy.h"
#include "internal_models/databaseconstituentsphyspropstablemodel.h"
#include "databasetableview.h"

#include <QMessageBox>

QSize PickConstituentFromDBDialog::m_lastDlgSize = QSize{};

PickConstituentFromDBDialog::PickConstituentFromDBDialog(DatabaseConstituentsPhysPropsTableModel &model, DatabaseProxy &dbProxy, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::PickConstituentFromDBDialog},
  h_dbProxy{dbProxy},
  m_model{model},
  m_selectedIndex{-1}
{
  ui->setupUi(this);
  ui->qtbv_constituents->setModel(&m_model);

  if (!m_lastDlgSize.isEmpty())
    resize(m_lastDlgSize);

  ui->qcbox_matchType->addItem(tr("Name begins with..."), QVariant::fromValue(DatabaseProxy::MatchType::BEGINS_WITH));
  ui->qcbox_matchType->addItem(tr("Name contains..."), QVariant::fromValue(DatabaseProxy::MatchType::CONTAINS));

  connect(ui->qle_constituentName, &QLineEdit::textChanged, this, &PickConstituentFromDBDialog::onConstituentNameChanged);

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &PickConstituentFromDBDialog::onAccepted);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &PickConstituentFromDBDialog::onRejected);
  connect(ui->qtbv_constituents, &DatabaseTableView::itemSelected, this, &PickConstituentFromDBDialog::onItemSelected);
  connect(ui->qpb_allCompounds, &QPushButton::clicked, this, &PickConstituentFromDBDialog::onAllCompounds);
  connect(ui->qcbox_matchType, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &PickConstituentFromDBDialog::onMatchTypeActivated);
}

PickConstituentFromDBDialog::~PickConstituentFromDBDialog()
{
  delete ui;
}

void PickConstituentFromDBDialog::executeSearch(const QString &name, const QVariant &matchVar)
{
  if (!h_dbProxy.isAvailable())
    return;

  ui->stackedWidget->setCurrentIndex(0);

  try {
    const auto match = [matchVar]() {
      if (!matchVar.canConvert<DatabaseProxy::MatchType>())
        return DatabaseProxy::MatchType::BEGINS_WITH;
      return matchVar.value<DatabaseProxy::MatchType>();
    }();

    std::string _name = name.toStdString();
    auto results = h_dbProxy.search(_name, match);

    m_model.refreshData(std::move(results));
  } catch (const DatabaseException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Database lookup failed"), ex.what()};
    mbox.exec();
  }
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

  m_lastDlgSize = size();
  accept();
}

void PickConstituentFromDBDialog::onAllCompounds()
{
  if (!h_dbProxy.isAvailable())
    return;

  ui->stackedWidget->setCurrentIndex(0);

  try {
    auto results = h_dbProxy.fetchAll();

    m_model.refreshData(std::move(results));
  } catch (const DatabaseException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Database lookup failed"), ex.what()};
    mbox.exec();
  }
}

void PickConstituentFromDBDialog::onConstituentNameChanged(const QString &name)
{
  executeSearch(name, ui->qcbox_matchType->currentData());
}

void PickConstituentFromDBDialog::onItemSelected(const int row)
{
  if (row < 0)
    return;

  m_selectedIndex = row;
  m_lastDlgSize = size();
  accept();
}

void PickConstituentFromDBDialog::onMatchTypeActivated(const int)
{
  executeSearch(ui->qle_constituentName->text(), ui->qcbox_matchType->currentData());
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

