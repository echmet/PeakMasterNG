#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <stdexcept>
#include <QObject>
#include <QString>

namespace gdm {
  class GDM;
}

namespace persistence {

class DeserializationException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class MalformedJSONException : public DeserializationException {
public:
  MalformedJSONException(const std::string &msg) :
    DeserializationException{std::string{"Malformed JSON file: "} + msg}
  {}
};

class SerializationException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class System {
public:
  double totalLength;
  double detectorPosition;
  double drivingVoltage;
  bool positiveVoltage;
  QString eofType;
  double eofValue;
  bool correctForDebyeHuckel;
  bool correctForOnsagerFuoss;
  bool correctForViscosity;
  double injectionZoneLength;
};

class Persistence : public QObject {
  Q_OBJECT

public:
  Persistence(gdm::GDM &bgeGDM, gdm::GDM &sampleGDM);
  void deserialize(const QString &filepath, System &system);
  void serialize(const QString &filepath, const System &system);

private:
  gdm::GDM &m_bgeGDM;
  gdm::GDM &m_sampleGDM;

signals:
  void deserialized();
};

} // namespace persistence

#endif // PERSISTENCE_H
