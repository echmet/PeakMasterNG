#include "toggletracepointsdialog.h"
#include "ui_toggletracepointsdialog.h"

#include <QFileDialog>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QMessageBox>

ToggleTracepointsDialog::ToggleTracepointsDialog(const std::vector<CalculatorInterface::TracepointInfo> &tracepointInformation, const TracingSetup &tracingSetup,
                                                 QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ToggleTracepointsDialog)
{
  ui->setupUi(this);

  m_qvlay_tracepoints = new QVBoxLayout{this};

  QWidget *w = new QWidget{this};
  w->setLayout(m_qvlay_tracepoints);

  ui->qscrArea_tracepoints->setWidget(w);
  w->show();

  for (const auto &item : tracepointInformation) {
    QCheckBox *cbox = new QCheckBox{this};
    cbox->setText(item.description);
    cbox->setProperty("TPID", item.TPID);
    m_tracepoints.emplace_back(cbox, false);

    m_qvlay_tracepoints->addWidget(cbox);
  }

  if (tracingSetup.tracepointStates.size() > 0) {
    Q_ASSERT(tracepointInformation.size() == tracingSetup.tracepointStates.size());

    for (size_t idx = 0; idx < tracingSetup.tracepointStates.size(); idx++) {
      const auto &item = tracingSetup.tracepointStates.at(idx);
      auto cbox = std::get<0>(m_tracepoints.at(idx));

      Q_ASSERT(item.TPID == cbox->property("TPID"));
      cbox->setChecked(item.enabled);
      std::get<1>(m_tracepoints.at(idx)) = item.enabled;
    }
  }

  ui->qle_outputFile->setText(tracingSetup.outputFilePath);
  ui->qcb_enableTracing->setChecked(tracingSetup.tracingEnabled);
  ui->qle_outputFile->setEnabled(tracingSetup.tracingEnabled);
  m_outputFilePath = tracingSetup.outputFilePath;

  connect(ui->qpb_chooseOutputFile, &QPushButton::clicked, this, &ToggleTracepointsDialog::onSetOutputFile);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ToggleTracepointsDialog::reject);
  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ToggleTracepointsDialog::onAccepted);
  connect(ui->qcb_enableTracing, &QCheckBox::toggled, this, &ToggleTracepointsDialog::onEnableTracingToggled);
}

ToggleTracepointsDialog::~ToggleTracepointsDialog()
{
  delete ui;
}

void ToggleTracepointsDialog::onAccepted()
{
  if (m_outputFilePath.length() < 1 && ui->qcb_enableTracing->isChecked()) {
    QMessageBox errBox{QMessageBox::Warning, tr("Invalid input"), tr("Output file must be set")};
    errBox.exec();
    return;
  }

  for (auto &item : m_tracepoints) {
    const auto cbox = std::get<0>(item);
    std::get<1>(item) = cbox->isChecked();
  }

  accept();
}

void ToggleTracepointsDialog::onEnableTracingToggled(const bool enabled)
{
  ui->qle_outputFile->setEnabled(enabled);
}

void ToggleTracepointsDialog::onSetOutputFile()
{
  if (!ui->qcb_enableTracing->isChecked())
    return;

  QFileDialog dlg{this};

  dlg.setAcceptMode(QFileDialog::AcceptSave);
  if (dlg.exec() != QDialog::Accepted)
    return;

  m_outputFilePath = dlg.selectedFiles()[0];
  ui->qle_outputFile->setText(m_outputFilePath);
}

ToggleTracepointsDialog::TracingSetup ToggleTracepointsDialog::result() const
{
  TracingSetup ts{
    m_outputFilePath,
    tracepointStates(),
    ui->qcb_enableTracing->isChecked()
  };

  return ts;
}

std::vector<CalculatorInterface::TracepointState> ToggleTracepointsDialog::tracepointStates() const
{
  std::vector<CalculatorInterface::TracepointState> states{};

  states.reserve(m_tracepoints.size());

  for (size_t idx = 0; idx < m_tracepoints.size(); idx++) {
    const auto item = m_tracepoints.at(idx);
    const int32_t TPID = std::get<0>(item)->property("TPID").value<int32_t>();

    states.emplace_back(TPID, std::get<1>(item));
  }

  return states;
}
