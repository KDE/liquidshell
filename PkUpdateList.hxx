/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _PkUpdateList_H_
#define _PkUpdateList_H_

#include <PkUpdates.hxx>
#include <QScrollArea>
#include <QQueue>
#include <QPointer>
#include <QSet>
#include <QMultiHash>
class QCheckBox;
class QProgressBar;
class QVBoxLayout;
class QToolButton;
class QPushButton;
class QLabel;
class QLineEdit;
class QMenu;

class PkUpdateList : public QWidget
{
  Q_OBJECT

  public:
    PkUpdateList(QWidget *parent);

    void setPackages(const PkUpdates::PackageList &packages);
    void setRefreshProgress(int progress);

    bool isInstallInProgress() const { return !installQ.isEmpty(); }

    QSize sizeHint() const override;

  protected:
    void hideEvent(QHideEvent *event) override;

  Q_SIGNALS:
    void refreshRequested();
    void packageInstalled(QString id);
    void packageCountToInstall(int num);

  private Q_SLOTS:
    void checkAll(bool on);
    void install();
    void installOne();
    void countChecked();
    void filterChanged(const QString &text);

    void askEULA(const QString &eulaID, const QString &packageID,
                 const QString &vendor, const QString &licenseAgreement);

  private:  // methods
    void enqueue(const QString &packageID);

  private:  // members
    QVBoxLayout *vbox;
    QScrollArea *scrollArea;
    QVBoxLayout *itemsLayout;
    QLineEdit *filterEdit;
    QProgressBar *progressBar;
    QToolButton *installButton;
    QMenu *installMenu;
    QToolButton *refreshButton;
    QCheckBox *checkAllBox;
    QSize savedSize;

    enum class AfterInstall { Nothing, Sleep, Shutdown } afterInstall = AfterInstall::Nothing;

    QQueue<QPointer<class PkUpdateListItem>> installQ;
    QPointer<PackageKit::Transaction> transaction;
    bool packageNoLongerAvailable;

    struct EulaPackage
    {
      EulaPackage(const QString &e, const QString &p) : eulaID(e), packageID(p) { }
      QString eulaID;
      QString packageID;
    };

    QMultiHash<QString, EulaPackage> eulaDialogs;  // key = license text hash
    QSet<QByteArray> acceptedEula;

    PackageKit::Transaction::Restart restart;
};

//--------------------------------------------------------------------------------

class PkUpdateListItem : public QWidget
{
  Q_OBJECT

  public:
    PkUpdateListItem(QWidget *parent, PackageKit::Transaction::Info info, const PkUpdates::PackageData &data);

    void showProgress(bool yes);

    PkUpdates::PackageData package;
    QToolButton *label;
    QCheckBox *checkBox;
    QProgressBar *progress;
    QLabel *detailsLabel;
    QLabel *errorLabel;
    QLabel *packageLabel;

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
};

#endif
