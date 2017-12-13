#include "pickconstituentfromdbdialog.h"
#include "ui_pickconstituentfromdbdialog.h"

#include "../gearbox/databaseproxy.h"

#include <QMessageBox>

PickConstituentFromDBDialog::PickConstituentFromDBDialog(DatabaseProxy &dbProxy, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::PickConstituentFromDBDialog},
  h_dbProxy{dbProxy}
{
  ui->setupUi(this);
  connect(ui->qle_constituentName, &QLineEdit::textChanged, this, &PickConstituentFromDBDialog::onConstituentNameChanged);
}

PickConstituentFromDBDialog::~PickConstituentFromDBDialog()
{
  delete ui;
}

void PickConstituentFromDBDialog::onConstituentNameChanged(const QString &name)
{
  if (!h_dbProxy.isAvailable())
    return;

  try {
    std::string _name = name.toStdString();
    const auto results = h_dbProxy.search(_name);
  } catch (const DatabaseException &ex) {
    QMessageBox mbox{QMessageBox::Warning, tr("Database lookup failed"), ex.what()};
    mbox.exec();
  }
}

