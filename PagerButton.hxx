/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <martin@kollix.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _PagerButton_H_
#define _PagerButton_H_

#include <QPushButton>
#include <QTimer>
class DesktopPanel;

#include <KWinCompat.hxx>

class PagerButton : public QPushButton
{
  Q_OBJECT

  public:
    PagerButton(int num, DesktopPanel *panel, bool showIcon);

    int getDesktop() const { return desktop; }

    QSize sizeHint() const override;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  private Q_SLOTS:
    void createPixmap();
    void windowChanged(WId id, NET::Properties props, NET::Properties2 props2);

  private:
    int desktop;
    DesktopPanel *panel;
    bool showIcon;
    QPixmap firstPixmap;
    QTimer dragDropTimer;
    QTimer pixmapTimer;
};

#endif
