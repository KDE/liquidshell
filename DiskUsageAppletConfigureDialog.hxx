#ifndef _DiskUsageAppletConfigureDialog_H_
#define _DiskUsageAppletConfigureDialog_H_

#include <QDialog>
#include <ui_DiskUsageAppletConfigureDialog.h>
class DiskUsageApplet;

class DiskUsageAppletConfigureDialog : public QDialog
{
  Q_OBJECT

  public:
    DiskUsageAppletConfigureDialog(DiskUsageApplet *parent);

  private Q_SLOTS:
    void accept() override;

  private:
    DiskUsageApplet *applet;
    Ui::DiskUsageAppletConfigureDialog ui;
};

#endif
