#ifndef _AppMenu_H_
#define _AppMenu_H_

#include <Launcher.hxx>
#include <DesktopPanel.hxx>

#include <QPushButton>
#include <QToolButton>
#include <QFrame>
#include <QEventLoop>

//--------------------------------------------------------------------------------

class AppMenu : public Launcher
{
  Q_OBJECT

  public:
    AppMenu(DesktopPanel *parent);

  private slots:
    void fill() override;
    void showMenu();

  private:
    QToolButton *button;
    class Menu *popup;
};

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

class Menu : public QFrame
{
  Q_OBJECT

  public:
    Menu(QWidget *parent);
    void exec();

  protected:
    void hideEvent(QHideEvent *event) override;

  private:
    QEventLoop eventLoop;
};

//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

class AppButton : public QToolButton
{
  public:
    AppButton(QWidget *parent, const QIcon &icon, const QString &name);

    QSize sizeHint() const override;
};

//--------------------------------------------------------------------------------

#endif
