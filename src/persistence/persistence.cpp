#include "persistence.h"

#include "../gdm/core/gdm.h"

#include "deserializer.h"
#include "serializer.h"

namespace persistence {


const QString Persistence::CTUENT_CTUENTS{"constituents"};
const QString Persistence::CTUENT_TYPE{"type"};
const QString Persistence::CTUENT_NAME{"name"};
const QString Persistence::CTUENT_CHARGE_LOW{"chargeLow"};
const QString Persistence::CTUENT_CHARGE_HIGH{"chargeHigh"};
const QString Persistence::CTUENT_PKAS{"pKas"};
const QString Persistence::CTUENT_MOBILITIES{"mobilities"};
const QString Persistence::CTUENT_VISCOSITY_COEFFICIENT{"viscosityCoefficient"};

const QString Persistence::CTUENT_TYPE_NUCLEUS{"N"};
const QString Persistence::CTUENT_TYPE_LIGAND{"L"};

const QString Persistence::CPX_NUCLEUS_CHARGE{"nucleusCharge"};
const QString Persistence::CPX_LIGANDS{"ligands"};
const QString Persistence::CPX_LIGAND_GROUPS{"ligandGroups"};
const QString Persistence::CPX_COMPLEX_FORMS{"complexForms"};
const QString Persistence::CPX_NAME{"name"};
const QString Persistence::CPX_CHARGE{"charge"};
const QString Persistence::CPX_MAX_COUNT{"maxCount"};
const QString Persistence::CPX_PBS{"pBs"};
const QString Persistence::CPX_MOBILITIES{"mobilities"};

const QString Persistence::SYS_TOTAL_LENGTH{"totalLength"};
const QString Persistence::SYS_DETECTOR_POSITION{"detectorPosition"};
const QString Persistence::SYS_DRIVING_VOLTAGE{"drivingVoltage"};
const QString Persistence::SYS_POSITIVE_VOLTAGE{"positiveVoltage"};
const QString Persistence::SYS_EOF_TYPE{"eofType"};
const QString Persistence::SYS_EOF_VALUE{"eofValue"};
const QString Persistence::SYS_CORRECT_FOR_DEBYE_HUCKEL{"correctForDebyeHuckel"};
const QString Persistence::SYS_CORRECT_FOR_ONSAGER_FUOSS{"correctForOnsagerFuoss"};
const QString Persistence::SYS_CORRECT_FOR_VISCOSITY{"correctForViscosity"};
const QString Persistence::SYS_INJECTION_ZONE_LENGTH{"injectionZoneLength"};

const QString Persistence::SYS_EOF_TYPE_NONE{"N"};
const QString Persistence::SYS_EOF_TYPE_MOBILITY{"U"};
const QString Persistence::SYS_EOF_TYPE_TIME{"T"};

const QString Persistence::ROOT_COMPOSITION_BGE{"compositionBGE"};
const QString Persistence::ROOT_COMPOSITION_SAMPLE{"compositionSample"};
const QString Persistence::ROOT_CONCENTRATIONS_BGE{"concentrationsBGE"};
const QString Persistence::ROOT_CONCENTRATIONS_SAMPLE{"concentrationsSample"};
const QString Persistence::ROOT_SYSTEM{"system"};

Persistence::Persistence(gdm::GDM &bgeGDM, gdm::GDM &sampleGDM) :
  m_bgeGDM{bgeGDM},
  m_sampleGDM{sampleGDM}
{
}

void Persistence::deserialize(const QString &filepath, System &system)
{
  gdm::GDM bgeGDM;
  gdm::GDM sampleGDM;

  Deserializer::deserialize(filepath, bgeGDM, sampleGDM, system);

  m_bgeGDM = bgeGDM;
  m_sampleGDM = sampleGDM;

  emit deserialized();
}

void Persistence::serialize(const QString &filepath, const System &system)
{
  Serializer::serialize(filepath, m_bgeGDM, m_sampleGDM, system);
}

} // namespace persistence
