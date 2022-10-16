#ifndef UI_LIGANDIONICFORM_H
#define UI_LIGANDIONICFORM_H

#include <memory>
#include <QString>

class Constituent;

class LigandIonicForm
{
public:
  LigandIonicForm();
  LigandIonicForm(std::shared_ptr<Constituent> &ligand, const int charge);
  LigandIonicForm(const LigandIonicForm &other) = default;
  LigandIonicForm(LigandIonicForm &&other) = default;

  QString ligandName() const;
  bool matches(const int charge, const QString &ligandName) const;

  LigandIonicForm & operator=(const LigandIonicForm &other);

  bool operator==(const LigandIonicForm &other) const;
  bool operator!=(const LigandIonicForm &other) const;

  const int charge;

private:
  std::shared_ptr<Constituent> _ligand;

};

#endif // UI_LIGANDIONICFORM_H
