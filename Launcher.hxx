#ifndef _Launcher_H_
#define _Launcher_H_

#include <QWidget>
#include <QFileSystemWatcher>

class Launcher : public QWidget
{
  Q_OBJECT

  public:
    Launcher(QWidget *parent, const QString &theId);

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void loadConfig(const QString &defaultDir = QString());
    void setDir(const QString &theDirPath);

  private slots:
    virtual void fill() = 0;

  protected:
    QString id;
    QString dirPath;
    QFileSystemWatcher dirWatcher;
};

#endif
