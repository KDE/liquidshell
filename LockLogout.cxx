#include <LockLogout.hxx>

#include <QToolButton>
#include <QVBoxLayout>
#include <QIcon>

#include <cstdlib>

//--------------------------------------------------------------------------------

LockLogout::LockLogout(QWidget *parent)
  : QWidget(parent)
{
  setObjectName("LockLogout");

  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(QMargins());
  vbox->setSpacing(2);

  QToolButton *lock   = new QToolButton;
  QToolButton *logout = new QToolButton;

  lock->setIcon(QIcon::fromTheme("system-lock-screen"));
  logout->setIcon(QIcon::fromTheme("system-shutdown"));

  lock->setIconSize(QSize(22, 22));
  logout->setIconSize(QSize(22, 22));

  lock->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  logout->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  vbox->addWidget(lock);
  vbox->addWidget(logout);

  connect(lock, &QToolButton::clicked, []() { std::system("xdg-screensaver lock >/dev/null 2>/dev/null&"); });

  connect(logout, &QToolButton::clicked,
          []()
          {
            std::system("dbus-send --type=method_call --dest=org.kde.ksmserver "
                        "/KSMServer org.kde.KSMServerInterface.logout "
                        "int32:-1 int32:0 int32:0");
          });
}

//--------------------------------------------------------------------------------
