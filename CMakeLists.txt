cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(liquidshell)

set(KF_MIN_VERSION 5.62)
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR})

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMInstallIcons)

find_package(Qt5 5.6 REQUIRED COMPONENTS
    Core
    Widgets
    DBus
    X11Extras
)

find_package(Qt5ThemeSupport REQUIRED)

find_package(KF5 ${KF_MIN_VERSION} REQUIRED COMPONENTS
             WindowSystem WidgetsAddons ConfigWidgets Config KIO IconThemes ItemViews Archive
             Notifications I18n NetworkManagerQt Service Solid BluezQt KCMUtils Crash DBusAddons
             NewStuff
)

set(SOURCES
    desktop.cxx
    DesktopWidget.cxx
    DesktopPanel.cxx
    OnScreenVolume.cxx
    OnScreenBrightness.cxx
    ConfigureDesktopDialog.cxx
    StartMenu.cxx
    Launcher.cxx
    QuickLaunch.cxx
    IconButton.cxx
    AppMenu.cxx
    Pager.cxx
    PagerButton.cxx
    WindowList.cxx
    ClockWidget.cxx
    ClockWidgetConfigureDialog.cxx
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
    PopupMenu.cxx
    KdeConnect.cxx

    DesktopApplet.cxx
    WeatherApplet.cxx
    WeatherAppletConfigureDialog.cxx
    DiskUsageApplet.cxx
    DiskUsageAppletConfigureDialog.cxx
    PictureFrameApplet.cxx
    PictureFrameAppletConfigureDialog.cxx

    Moon.cxx
    liquidshell.qrc
)

find_package(packagekitqt5 QUIET)

if ( packagekitqt5_FOUND )
  set(SOURCES ${SOURCES}
      PkUpdates.cxx
      PkUpdateList.cxx
     )

  add_definitions(-DWITH_PACKAGEKIT)
else()
  message(WARNING "PackageKit integration not available. packagekitqt5 development files not found")
endif()

# e.g. on openSuse Leap 42.3 compile fails as a GLib header included uses signals as var
add_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)

ki18n_wrap_ui(UI_FILES
              ConfigureDesktopDialog.ui
              WeatherAppletConfigureDialog.ui
              DiskUsageAppletConfigureDialog.ui
              PictureFrameAppletConfigureDialog.ui
)

set(statusnotifieritem_xml ${KNOTIFICATIONS_DBUS_INTERFACES_DIR}/kf5_org.kde.StatusNotifierItem.xml)
set_source_files_properties(${statusnotifieritem_xml} PROPERTIES
   INCLUDE "DBusTypes.hxx"
   CLASSNAME OrgKdeStatusNotifierItem
)
qt5_add_dbus_interface(SOURCES ${statusnotifieritem_xml} statusnotifieritem_interface)

qt5_add_dbus_adaptor(SOURCES org.freedesktop.Notifications.xml NotificationServer.hxx NotificationServer)

set(TARGET liquidshell)

add_executable(${TARGET} ${SOURCES} ${UI_FILES})

set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${TARGET} PROPERTY AUTORCC ON)

target_link_libraries(${TARGET}
                      Qt5::Core
                      Qt5::Widgets
                      Qt5::DBus
                      Qt5::X11Extras
                      Qt5ThemeSupport::Qt5ThemeSupport
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
                      KF5::Crash
                      KF5::DBusAddons
                      KF5::ItemViews
                      KF5::Archive
                      KF5::NewStuffWidgets
)

if ( packagekitqt5_FOUND )
  target_link_libraries(${TARGET} PK::packagekitqt5)
endif()

install(TARGETS ${TARGET} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.${TARGET}.desktop DESTINATION ${KDE_INSTALL_APPDIR})

configure_file(start_liquidshell start_liquidshell @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/start_liquidshell DESTINATION ${KDE_INSTALL_BINDIR})

configure_file(liquidshell-session.desktop liquidshell-session.desktop @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/liquidshell-session.desktop DESTINATION ${KDE_INSTALL_DATADIR}/xsessions)

install(FILES liquidshell.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR})
install(FILES org.kde.liquidshell.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ecm_install_icons(ICONS
                  48-apps-liquidshell.png
                  DESTINATION ${KDE_INSTALL_ICONDIR}
                  THEME hicolor)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES)

ki18n_install(po)
