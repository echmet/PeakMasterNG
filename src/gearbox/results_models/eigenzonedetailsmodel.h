#ifndef EIGENZONEDETAILSMODEL_H
#define EIGENZONEDETAILSMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class EigenzoneDetailsModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  class EigenzoneProps {
  public:
    bool tainted;
    bool isAnalyte;
    double time;
    double pH;
    double uEMD;
    QVector<double> concentrations;
  };

  explicit EigenzoneDetailsModel(QObject *parent = nullptr);

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Basic functionality:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  void refreshData(QVector<QString> &&constituents, QVector<EigenzoneProps> &&eigenzones) noexcept;

private:
  double constituentsConcentrations(const EigenzoneProps &zone, const int idx) const;
  QString constituentsHeader(const int idx) const;

  QVector<QString> m_constituents;
  QVector<EigenzoneProps> m_eigenzones;
};

#endif // EIGENZONEDETAILSMODEL_H
