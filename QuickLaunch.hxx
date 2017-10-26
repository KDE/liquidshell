#ifndef _QuickLaunch_H_
#define _QuickLaunch_H_

#include <Launcher.hxx>
#include <DesktopPanel.hxx>

#include <QGridLayout>

class QuickLaunch : public Launcher
{
  Q_OBJECT

  public:
    QuickLaunch(DesktopPanel *parent);

  private slots:
    void fill() override;

  private:
    QGridLayout *grid;
};

#endif
