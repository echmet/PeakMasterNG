#ifndef NONIDEALITYCORRECTIONSDIALOG_H
#define NONIDEALITYCORRECTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class NonidealityCorrectionsDialog;
}

class NonidealityCorrectionsDialog : public QDialog {
  Q_OBJECT

  class State {
  public:
    bool debyeHuckel;
    bool onsagerFuoss;
    bool viscosity;
  };

public:
  explicit NonidealityCorrectionsDialog(QWidget *parent = nullptr);
  ~NonidealityCorrectionsDialog();

  int exec();
  void setState(const bool debyeHuckel, const bool onsagerFuoss, const bool viscosity);
  void setState(const State &state);
  State state() const;

private:
  Ui::NonidealityCorrectionsDialog *ui;

  State m_lastState;

private slots:
  void onAccept();
  void onReject();
};

#endif // NONIDEALITYCORRECTIONSDIALOG_H
