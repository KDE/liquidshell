#ifndef _PkUpdateList_H_
#define _PkUpdateList_H_

#include <PkUpdates.hxx>
#include <QWidget>
#include <QQueue>
class QCheckBox;
class QProgressBar;
class QVBoxLayout;
class QToolButton;
class QPushButton;
class QLabel;

class PkUpdateList : public QWidget
{
  Q_OBJECT

  public:
    PkUpdateList(QWidget *parent);

    void setPackages(const PkUpdates::PackageList &packages);

  Q_SIGNALS:
    void refreshRequested();
    void packageInstalled(QString id);

  private Q_SLOTS:
    void checkAll(bool on);
    void install();
    void countChecked();
    void filterChanged(const QString &text);

  private:
    QVBoxLayout *itemsLayout;
    QPushButton *installButton;
};

//--------------------------------------------------------------------------------

class PkUpdateListItem : public QWidget
{
  Q_OBJECT

  public:
    PkUpdateListItem(QWidget *parent, PackageKit::Transaction::Info info, const PkUpdates::PackageData &data);

    void showProgress(bool yes);

    PkUpdates::PackageData package;
    QCheckBox *checkBox;
    QProgressBar *progress;
    QToolButton *cancelButton;
    QLabel *detailsLabel;
    QLabel *errorLabel;

  Q_SIGNALS:
    void toggled();

  private Q_SLOTS:
    void getUpdateDetails();

    void updateDetail(const QString &packageID,
                      const QStringList &updates,
                      const QStringList &obsoletes,
                      const QStringList &vendorUrls,
                      const QStringList &bugzillaUrls,
                      const QStringList &cveUrls,
                      PackageKit::Transaction::Restart restart,
                      const QString &updateText,
                      const QString &changelog,
                      PackageKit::Transaction::UpdateState state,
                      const QDateTime &issued,
                      const QDateTime &updated);

  private:
    QQueue<QString> installQ;
};

#endif
