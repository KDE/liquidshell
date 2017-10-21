#ifndef _ConfigureDesktopDialog_H_
#define _ConfigureDesktopDialog_H_

#include <QDialog>
#include <QButtonGroup>
#include <DesktopWidget.hxx>

#include <ui_ConfigureDesktopDialog.h>

class ConfigureDesktopDialog : public QDialog
{
  Q_OBJECT

  public:
    ConfigureDesktopDialog(QWidget *parent, const DesktopWidget::Wallpaper &wp);

    const DesktopWidget::Wallpaper &getWallpaper() const { return wallpaper; }

  signals:
    void changed();

  private:
    Ui::ConfigureDesktopDialog ui;
    QButtonGroup buttonGroup;
    DesktopWidget::Wallpaper wallpaper;
};

#endif
