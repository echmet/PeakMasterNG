#ifndef IONICCOMPOSITIONMODEL_H
#define IONICCOMPOSITIONMODEL_H

#include <QAbstractTableModel>

class IonicCompositionModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  class ConstituentConcentrations {
  public:
    ConstituentConcentrations() :
      lowestCharge{0},
      highestCharge{0}
    {}

    ConstituentConcentrations(const int lowestCharge, const int highestCharge,
                              QMap<int, double> &&plain, QMap<QString, double> &&complex) noexcept :
      lowestCharge{lowestCharge},
      highestCharge{highestCharge},
      plain(plain),
      complex(complex)
    {}

    ConstituentConcentrations(const ConstituentConcentrations &other) :
      lowestCharge{other.lowestCharge},
      highestCharge{other.highestCharge},
      plain{other.plain},
      complex{other.complex}
    {}

    ConstituentConcentrations(ConstituentConcentrations &&other) noexcept :
      lowestCharge{other.lowestCharge},
      highestCharge{other.highestCharge},
      plain(std::move(other.plain)),
      complex(std::move(other.complex))
    {}

    int lowestCharge;
    int highestCharge;
    QMap<int, double> plain;
    QMap<QString, double> complex;
  };

  explicit IonicCompositionModel(QObject *parent = nullptr);

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Basic functionality:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  void refreshData(const int lowestCharge, const int highestCharge, QVector<QString> &&constituents, QVector<QString> &&complexForms,
                   QVector<ConstituentConcentrations> &&concentrations) noexcept;

private:
  QVector<QString> m_constituents;
  QVector<QString> m_complexForms;

  QVector<ConstituentConcentrations> m_concentrations;

  int m_lowestCharge;
  int m_chargeSpan;
};

#endif // IONICCOMPOSITIONMODEL_H
