#ifndef COMPLEXATIONMANAGER_H
#define COMPLEXATIONMANAGER_H

#include "../ui/complexation_models/complexationrelationship.h"

#include <QObject>
#include <map>
#include <vector>

namespace gdm {
  class Constituent;
  class GDM;
}

class EditComplexationDialog;
class GDMProxy;

class ComplexationManager : public QObject
{
  Q_OBJECT

public:
  explicit ComplexationManager(gdm::GDM &backgroundGDM, gdm::GDM &sampleGDM, QObject *parent = nullptr);
  std::vector<int> complexationStatus(const std::string &name) const;
  bool complexes(const std::string &name);
  void editComplexation(const std::string &name);
  void handleUserInput(EditComplexationDialog *dlg, const std::string &nucleusName, const std::string &ligandName);
  void makeComplexation(const std::string &first, const std::string &second);
  void notifyConstituentRemoved();

private:
  typedef std::map<std::string, std::vector<int>> ComplexationStatusMap;
  typedef std::map<std::string, int> ComplexingNucleiMap;

  void addNucleus(const std::string &nucleusName);
  void updateComplexingNuclei();
  void updateComplexationStatus();

  gdm::GDM &h_backgroundGDM;
  gdm::GDM &h_sampleGDM;
  ComplexingNucleiMap m_complexingNuclei;
  ComplexationStatusMap m_complexationStatus;

signals:
  void complexationStatusChanged();
};

#endif // COMPLEXATIONMANAGER_H
