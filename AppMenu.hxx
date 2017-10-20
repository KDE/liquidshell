#ifndef _AppMenu_H_
#define _AppMenu_H_

#include <Launcher.hxx>

#include <QPushButton>
#include <QFrame>
#include <QEventLoop>

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

class AppMenu : public Launcher
{
  Q_OBJECT

  public:
    AppMenu(QWidget *parent);

  private slots:
    void fill() override;
    void showMenu();

  private:
    QPushButton *button;
    Menu *popup;
};

#endif
