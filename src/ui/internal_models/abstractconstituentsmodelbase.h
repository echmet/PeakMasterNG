#ifndef ABSTRACTCONSTITUENTSMODELBASE_H
#define ABSTRACTCONSTITUENTSMODELBASE_H

#include <QAbstractTableModel>
#include <QColor>

class ComplexationManager;
class GDMProxy;

class AbstractConstituentsModelBase : public QAbstractTableModel
{
public:
  explicit AbstractConstituentsModelBase(GDMProxy &GDMProxy, ComplexationManager &cpxMgr, QObject *parent = nullptr);
  void addConstituent(const QString &name);
  virtual bool dropMimeData(const QMimeData *mdata, Qt::DropAction action, int row, int col, const QModelIndex &parent) override;
  virtual QMimeData * mimeData(const QModelIndexList &indexes) const override;
  virtual QStringList mimeTypes() const override;
  QString removeConstituent(const int row);
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  void refreshAll(QVector<QString> &&constituents) noexcept;
  virtual void updateName(const QString &oldName, const QString &newName);

public slots:
  virtual void onComplexationStatusUpdated();

protected:
  QList<QColor> complexationStatus(const std::string &name) const;

  QVector<QString> m_constituentNames;
  ComplexationManager &h_cpxMgr;
  GDMProxy &h_GDMProxy;

private:
  static const QString MIME_FORMAT;
};

#endif // ABSTRACTCONSTITUENTSMODELBASE_H
