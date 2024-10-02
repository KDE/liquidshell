/*
  This file is part of liquidshell.

  SPDX-FileCopyrightText: 2017 - 2024 Martin Koller <kollix@aon.at>

  SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef _TaskBarButton_H_
#define _TaskBarButton_H_

#include <QPushButton>
#include <QTimer>
class QLabel;
class KSqueezedTextLabel;

#include <KWinCompat.hxx>

class TaskBarButton : public QPushButton
{
  Q_OBJECT

  public:
    TaskBarButton(WId wid);

    void setIconSize(int size);

    QSize sizeHint() const override;

  Q_SIGNALS:
    void clicked();

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void moveEvent(QMoveEvent *) override { updateWMGeometry(); }
    void resizeEvent(QResizeEvent *) override { updateWMGeometry(); }

  private Q_SLOTS:
    void fill();
    void setBackground();
    void windowChanged(WId id, NET::Properties props, NET::Properties2 props2);

  private:
    void updateWMGeometry();

  private:
    WId wid;
    QLabel *iconLabel;
    KSqueezedTextLabel *textLabel;
    QTimer dragDropTimer;
    QPoint dragStartPos;
};

#endif
