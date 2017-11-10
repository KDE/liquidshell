#include <DiskUsageAppletConfigureDialog.hxx>
#include <DiskUsageApplet.hxx>

//--------------------------------------------------------------------------------

DiskUsageAppletConfigureDialog::DiskUsageAppletConfigureDialog(DiskUsageApplet *parent)
  : QDialog(parent), applet(parent)
{
  ui.setupUi(this);
  ui.textColor->setColor(applet->palette().color(applet->foregroundRole()));
  ui.backgroundColor->setColor(applet->palette().color(applet->backgroundRole()));
}

//--------------------------------------------------------------------------------

void DiskUsageAppletConfigureDialog::accept()
{
  QPalette pal = applet->palette();
  pal.setColor(applet->foregroundRole(), ui.textColor->color());
  pal.setColor(applet->backgroundRole(), ui.backgroundColor->color());
  applet->setPalette(pal);

  QDialog::accept();
}

//--------------------------------------------------------------------------------
