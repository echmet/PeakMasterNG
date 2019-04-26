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

class Target {
public:
  enum Type {
    TT_FILE,
    TT_CLIPBOARD
  };

  Target(const Type _type, QString _path);

  const Type type;
  const QString path;
};

class Persistence : public QObject {
  Q_OBJECT

public:
  Persistence(gdm::GDM &bgeGDM, gdm::GDM &sampleGDM);
  void deserialize(const Target &target, System &system);
  void serialize(const Target &target, const System &system);

  static const QString CTUENT_CTUENTS;
  static const QString CTUENT_TYPE;
  static const QString CTUENT_NAME;
  static const QString CTUENT_CHARGE_LOW;
  static const QString CTUENT_CHARGE_HIGH;
  static const QString CTUENT_PKAS;
  static const QString CTUENT_MOBILITIES;
  static const QString CTUENT_VISCOSITY_COEFFICIENT;

  static const QString CTUENT_TYPE_NUCLEUS;
  static const QString CTUENT_TYPE_LIGAND;

  static const QString CPX_NUCLEUS_CHARGE;
  static const QString CPX_LIGANDS;
  static const QString CPX_LIGAND_GROUPS;
  static const QString CPX_COMPLEX_FORMS;
  static const QString CPX_NAME;
  static const QString CPX_CHARGE;
  static const QString CPX_MAX_COUNT;
  static const QString CPX_PBS;
  static const QString CPX_MOBILITIES;

  static const QString SYS_TOTAL_LENGTH;
  static const QString SYS_DETECTOR_POSITION;
  static const QString SYS_DRIVING_VOLTAGE;
  static const QString SYS_POSITIVE_VOLTAGE;
  static const QString SYS_EOF_TYPE;
  static const QString SYS_EOF_VALUE;
  static const QString SYS_CORRECT_FOR_DEBYE_HUCKEL;
  static const QString SYS_CORRECT_FOR_ONSAGER_FUOSS;
  static const QString SYS_CORRECT_FOR_VISCOSITY;
  static const QString SYS_INJECTION_ZONE_LENGTH;

  static const QString SYS_EOF_TYPE_NONE;
  static const QString SYS_EOF_TYPE_MOBILITY;
  static const QString SYS_EOF_TYPE_TIME;

  static const QString ROOT_COMPOSITION_BGE;
  static const QString ROOT_COMPOSITION_SAMPLE;
  static const QString ROOT_CONCENTRATIONS_BGE;
  static const QString ROOT_CONCENTRATIONS_SAMPLE;
  static const QString ROOT_SYSTEM;

private:
  gdm::GDM &m_bgeGDM;
  gdm::GDM &m_sampleGDM;

signals:
  void deserialized();
};

} // namespace persistence

#endif // PERSISTENCE_H
