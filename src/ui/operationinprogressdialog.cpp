#include "operationinprogressdialog.h"
#include "ui_operationinprogressdialog.h"

OperationInProgressDialog::OperationInProgressDialog(const QString &title, QWidget *parent) :
  QDialog{parent},
  ui{new Ui::OperationInProgressDialog},
  m_isExecable{true}
{
  ui->setupUi(this);
  setWindowTitle(title);

  ui->qprbar_inProgress->setRange(0, 0);
}

OperationInProgressDialog::~OperationInProgressDialog()
{
  delete ui;
}

int OperationInProgressDialog::exec()
{
  if (m_isExecable)
    return QDialog::exec();

  return QDialog::Accepted;
}

void OperationInProgressDialog::onOperationCompleted()
{
  m_isExecable = false;

  accept();
}

void OperationInProgressDialog::reject()
{
  if (!m_isExecable)
    QDialog::reject();

  return;
}
