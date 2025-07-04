/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <ClockWidgetConfigureDialog.hxx>

#include <QTreeWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QTimeZone>
#include <QHeaderView>
#include <QStandardPaths>
#include <QPixmap>

#include <KLocalizedString>
#include <KTreeWidgetSearchLine>

//--------------------------------------------------------------------------------

ClockWidgetConfigureDialog::ClockWidgetConfigureDialog(QWidget *parent, const QVector<QByteArray> &timeZoneIds)
  : QDialog(parent)
{
  tree = new QTreeWidget;
  QLineEdit *filter = new KTreeWidgetSearchLine(this, tree);
  filter->setClearButtonEnabled(true);
  filter->setPlaceholderText(i18n("Filter"));
  QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->addWidget(filter);
  vbox->addWidget(tree);
  vbox->addWidget(buttons);

  // fill tree
  tree->setHeaderLabels(QStringList() << i18n("Timezone") << i18n("Territory"));
  tree->setRootIsDecorated(false);

  QList<QByteArray> zones = QTimeZone::availableTimeZoneIds();

  for (const QByteArray &zone : zones)
  {
    QTimeZone timeZone(zone);
    QTreeWidgetItem *item = new QTreeWidgetItem(tree);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |
                   Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);

    item->setCheckState(0, timeZoneIds.contains(zone) ? Qt::Checked : Qt::Unchecked);
    item->setText(0, timeZone.id());
    item->setText(1, QLocale::territoryToString(timeZone.territory()));

    // Locate the flag from share/kf5/locale/countries/%1/flag.png
    QList<QLocale> matchingLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, timeZone.territory());
    if ( !matchingLocales.isEmpty() )
    {
      QString flag = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                         QString("kf5/locale/countries/%1/flag.png").arg(matchingLocales[0].name().mid(3).toLower()));
      if ( !flag.isEmpty() )
        item->setIcon(1, QPixmap(flag));
    }
  }

  tree->setSortingEnabled(true);
  tree->sortByColumn(0, Qt::AscendingOrder);
  tree->header()->resizeSections(QHeaderView::ResizeToContents);
}

//--------------------------------------------------------------------------------

QVector<QByteArray> ClockWidgetConfigureDialog::getSelectedTimeZoneIds() const
{
  QVector<QByteArray> timeZoneIds;

  for (int i = 0; i < tree->topLevelItemCount(); i++)
  {
    QTreeWidgetItem *item = tree->topLevelItem(i);
    if ( item->checkState(0) == Qt::Checked )
      timeZoneIds.append(item->text(0).toUtf8());
  }

  return timeZoneIds;
}

//--------------------------------------------------------------------------------
