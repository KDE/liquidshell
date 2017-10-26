#ifndef _StartMenu_H_
#define _StartMenu_H_

#include <QPushButton>
#include <QMenu>

#include <KServiceGroup>

#include <DesktopPanel.hxx>

class StartMenu : public QPushButton
{
  Q_OBJECT

  Q_PROPERTY(QString themeIcon READ getThemeIcon WRITE setThemeIcon)

  public:
    StartMenu(DesktopPanel *parent);

    QString getThemeIcon() const { return themeIcon; }
    void setThemeIcon(const QString &icon);

  protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

  private slots:
    void fill();

  private:
    void fillFromGroup(QMenu *menu, KServiceGroup::Ptr group);

  private:
    QString themeIcon;
};

//--------------------------------------------------------------------------------

class PopupMenu : public QMenu
{
  Q_OBJECT

  public:
    PopupMenu(QWidget *parent) : QMenu(parent), dragStartPos(-1, -1) { }

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private:
    QPoint dragStartPos;
};

#endif
