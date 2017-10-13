cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

find_package(ECM 1.3.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    DBus
    X11Extras
)

find_package(KF5 REQUIRED COMPONENTS
             WindowSystem WidgetsAddons ConfigWidgets Config KIO IconThemes
             Notifications I18n NetworkManagerQt Service Solid BluezQt KCMUtils
)

set(SOURCES
    desktop.cxx
    DesktopWidget.cxx
    DesktopPanel.cxx
    StartMenu.cxx
    AppMenu.cxx
    Pager.cxx
    PagerButton.cxx
    WindowList.cxx
    ClockWidget.cxx
    TaskBar.cxx
    TaskBarButton.cxx
    LockLogout.cxx
    SysTray.cxx
    SysTrayItem.cxx
    SysTrayNotifyItem.cxx
    DBusTypes.cxx
    SysLoad.cxx
    NotificationServer.cxx
    NotificationList.cxx
    Network.cxx
    NetworkList.cxx
    DeviceNotifier.cxx
    DeviceList.cxx
    Battery.cxx
    Bluetooth.cxx
)

set(statusnotifieritem_xml ${KNOTIFICATIONS_DBUS_INTERFACES_DIR}/kf5_org.kde.StatusNotifierItem.xml)
set_source_files_properties(${statusnotifieritem_xml} PROPERTIES
   INCLUDE "DBusTypes.hxx"
   CLASSNAME OrgKdeStatusNotifierItem
)
qt5_add_dbus_interface(SOURCES ${statusnotifieritem_xml} statusnotifieritem_interface)

qt5_add_dbus_adaptor(SOURCES org.freedesktop.Notifications.xml NotificationServer.hxx NotificationServer)

add_executable(desktop ${SOURCES})

target_link_libraries(desktop
                      Qt5::Core
                      Qt5::Widgets
                      Qt5::DBus
                      Qt5::X11Extras
                      KF5::WindowSystem
                      KF5::WidgetsAddons
                      KF5::ConfigWidgets
                      KF5::ConfigCore
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::IconThemes
                      KF5::Notifications
                      KF5::I18n
                      KF5::NetworkManagerQt
                      KF5::Service
                      KF5::Solid
                      KF5::BluezQt
                      KF5::KCMUtils
)
