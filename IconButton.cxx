#include <IconButton.hxx>

#include <QHBoxLayout>

//--------------------------------------------------------------------------------

IconButton::IconButton(QWidget *parent)
  : QToolButton(parent)
{
  setAutoRaise(true);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

  QHBoxLayout *hbox = new QHBoxLayout(this);

  iconLabel = new QLabel;
  iconLabel->setFixedSize(iconSize());
  iconLabel->setContextMenuPolicy(Qt::PreventContextMenu);
  hbox->addWidget(iconLabel);

  textLabel = new QLabel;
  hbox->addWidget(textLabel);
}

//--------------------------------------------------------------------------------

IconButton::IconButton(QWidget *parent, const QIcon &icon, int theIconSize, const QString &name)
  : IconButton(parent)
{
  if ( theIconSize != -1 )
    setIconSize(QSize(theIconSize, theIconSize));

  iconLabel->setFixedSize(iconSize());
  iconLabel->setPixmap(icon.pixmap(iconSize()));

  textLabel->setText(name);
}

//--------------------------------------------------------------------------------

void IconButton::setText(const QString &txt)
{
  textLabel->setText(txt);
}

//--------------------------------------------------------------------------------

void IconButton::setIcon(const QIcon &icon)
{
  iconLabel->setPixmap(icon.pixmap(iconSize()));
}

//--------------------------------------------------------------------------------

QSize IconButton::sizeHint() const
{
  return layout()->sizeHint();
}

//--------------------------------------------------------------------------------
