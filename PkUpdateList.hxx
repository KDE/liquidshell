// SPDX-License-Identifier: GPL-3.0-or-later
/*
  Copyright 2017 - 2019 Martin Koller, kollix@aon.at

  This file is part of liquidshell.

  liquidshell is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  liquidshell is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with liquidshell.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _PkUpdateList_H_
#define _PkUpdateList_H_

#include <PkUpdates.hxx>
#include <QScrollArea>
#include <QQueue>
#include <QPointer>
class QCheckBox;
class QProgressBar;
class QVBoxLayout;
class QToolButton;
class QPushButton;
class QLabel;
class QLineEdit;

class PkUpdateList : public QWidget
{
  Q_OBJECT

  public:
    PkUpdateList(QWidget *parent);

    void setPackages(const PkUpdates::PackageList &packages);
    void setRefreshProgress(int progress);

    QSize sizeHint() const override;

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

  private:
    QVBoxLayout *vbox;
    QScrollArea *scrollArea;
    QVBoxLayout *itemsLayout;
    QLineEdit *filterEdit;
    QProgressBar *progressBar;
    QPushButton *installButton;
    QPushButton *refreshButton;
    QCheckBox *checkAllBox;

    QQueue<QPointer<class PkUpdateListItem>> installQ;
    QPointer<PackageKit::Transaction> transaction;
    bool packageNoLongerAvailable;

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
    QToolButton *cancelButton;
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
