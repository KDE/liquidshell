#include <LockLogout.hxx>

#include <QGridLayout>
#include <QIcon>

#include <cstdlib>

#include <KLocalizedString>

//--------------------------------------------------------------------------------

LockLogout::LockLogout(DesktopPanel *parent)
  : QWidget(parent)
{
  setObjectName("LockLogout");

  QGridLayout *grid = new QGridLayout(this);
  grid->setContentsMargins(QMargins());
  grid->setSpacing(2);

  lock   = new QToolButton;
  logout = new QToolButton;

  lock->setIcon(QIcon::fromTheme("system-lock-screen"));
  logout->setIcon(QIcon::fromTheme("system-shutdown"));

  lock->setIconSize(QSize(22, 22));
  logout->setIconSize(QSize(22, 22));

  lock->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  logout->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  connect(lock, &QToolButton::clicked, []() { std::system("xdg-screensaver lock >/dev/null 2>/dev/null&"); });

  connect(logout, &QToolButton::clicked,
          []()
          {
            std::system("dbus-send --type=method_call --dest=org.kde.ksmserver "
                        "/KSMServer org.kde.KSMServerInterface.logout "
                        "int32:-1 int32:0 int32:0");
          });

  fill(parent->getRows());
  connect(parent, &DesktopPanel::rowsChanged, this, &LockLogout::fill);
}

//--------------------------------------------------------------------------------

void LockLogout::fill(int rows)
{
  QGridLayout *grid = static_cast<QGridLayout *>(layout());
  delete grid->takeAt(0);
  delete grid->takeAt(1);

  if ( rows == 1 )
  {
    grid->addWidget(lock, 0, 0);
    grid->addWidget(logout, 0, 1);
  }
  else
  {
    grid->addWidget(lock, 0, 0);
    grid->addWidget(logout, 1, 0);
  }
}

//--------------------------------------------------------------------------------
