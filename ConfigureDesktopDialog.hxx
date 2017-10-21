#ifndef _ConfigureDesktopDialog_H_
#define _ConfigureDesktopDialog_H_

#include <QDialog>
#include <DesktopWidget.hxx>

#include <ui_ConfigureDesktopDialog.h>

class ConfigureDesktopDialog : public QDialog
{
  Q_OBJECT

  public:
    ConfigureDesktopDialog(QWidget *parent, const DesktopWidget::Wallpaper &wp);

    const DesktopWidget::Wallpaper &getWallpaper() const { return wallpaper; }

  protected:
    void resizeEvent(QResizeEvent *event) override;

  private slots:
    void fill();

  private:
    Ui::ConfigureDesktopDialog ui;
    DesktopWidget::Wallpaper wallpaper;
};

#endif
