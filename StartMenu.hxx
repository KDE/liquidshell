#ifndef _StartMenu_H_
#define _StartMenu_H_

#include <QPushButton>
#include <QMenu>

#include <KServiceGroup>

class StartMenu : public QPushButton
{
  Q_OBJECT

  Q_PROPERTY(QString themeIcon READ getThemeIcon WRITE setThemeIcon)

  public:
    StartMenu(QWidget *parent);

    QString getThemeIcon() const { return themeIcon; }
    void setThemeIcon(const QString &icon);

  private slots:
    void fill();

  private:
    void fillFromGroup(QMenu *menu, KServiceGroup::Ptr group);

  private:
    QString themeIcon;
};

#endif
