#ifndef _AppMenu_H_
#define _AppMenu_H_

#include <QPushButton>
#include <QFrame>
#include <QFileSystemWatcher>
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

class AppMenu : public QPushButton
{
  Q_OBJECT

  public:
    AppMenu(QWidget *parent, const QString &dirPath);

    void setDir(const QString &dirPath);

  private slots:
    void fill();
    void showMenu();

  private:
    QString dirPath;
    QFileSystemWatcher dirWatcher;
    Menu *popup;
};

#endif
