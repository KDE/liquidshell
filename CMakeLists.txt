cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(liquidshell)

set(KF_MIN_VERSION 6)
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR})

set(KDE_COMPILERSETTINGS_LEVEL 5.84)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(ECMInstallIcons)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    DBus
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
             WindowSystem WidgetsAddons ConfigWidgets Config KIO IconThemes ItemViews Archive
             Notifications I18n NetworkManagerQt Service Solid BluezQt KCMUtils Crash DBusAddons
             NewStuff XmlGui
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

find_package(packagekitqt6 QUIET)

if ( packagekitqt6_FOUND )
  set(SOURCES ${SOURCES}
      PkUpdates.cxx
      PkUpdateList.cxx
     )

  add_definitions(-DWITH_PACKAGEKIT)
else()
  message(WARNING "PackageKit integration not available. packagekitqt6 development files not found")
endif()

# e.g. on openSuse Leap 42.3 compile fails as a GLib header included uses signals as var
add_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)

ki18n_wrap_ui(UI_FILES
              ConfigureDesktopDialog.ui
              WeatherAppletConfigureDialog.ui
              DiskUsageAppletConfigureDialog.ui
              PictureFrameAppletConfigureDialog.ui
)

set(statusnotifieritem_xml ${KDE_INSTALL_FULL_DBUSINTERFACEDIR}/kf6_org.kde.StatusNotifierItem.xml)
set_source_files_properties(${statusnotifieritem_xml} PROPERTIES
   INCLUDE "DBusTypes.hxx"
   CLASSNAME OrgKdeStatusNotifierItem
)
qt_add_dbus_interface(SOURCES ${statusnotifieritem_xml} statusnotifieritem_interface)

qt_add_dbus_adaptor(SOURCES org.freedesktop.Notifications.xml NotificationServer.hxx NotificationServer)

set(TARGET liquidshell)

add_executable(${TARGET} ${SOURCES} ${UI_FILES})

set_property(TARGET ${TARGET} PROPERTY AUTORCC ON)

target_link_libraries(${TARGET}
                      Qt::Core
                      Qt::Gui
                      Qt::GuiPrivate
                      Qt::Widgets
                      Qt::DBus
                      KF6::WindowSystem
                      KF6::WidgetsAddons
                      KF6::ConfigWidgets
                      KF6::ConfigCore
                      KF6::KIOCore
                      KF6::KIOWidgets
                      KF6::IconThemes
                      KF6::Notifications
                      KF6::I18n
                      KF6::NetworkManagerQt
                      KF6::Service
                      KF6::Solid
                      KF6::BluezQt
                      KF6::KCMUtils
                      KF6::Crash
                      KF6::DBusAddons
                      KF6::ItemViews
                      KF6::Archive
                      KF6::NewStuffWidgets
                      KF6::XmlGui
)

if ( packagekitqt6_FOUND )
  target_link_libraries(${TARGET} PK::packagekitqt6)
endif()

install(TARGETS ${TARGET} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.${TARGET}.desktop DESTINATION ${KDE_INSTALL_APPDIR})

configure_file(start_liquidshell start_liquidshell @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/start_liquidshell DESTINATION ${KDE_INSTALL_BINDIR})

configure_file(liquidshell-session.desktop liquidshell-session.desktop @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/liquidshell-session.desktop DESTINATION ${KDE_INSTALL_DATADIR}/xsessions)

install(FILES liquidshell.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
install(FILES org.kde.liquidshell.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ecm_install_icons(ICONS
                  48-apps-liquidshell.png
                  DESTINATION ${KDE_INSTALL_ICONDIR}
                  THEME hicolor)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES)

ki18n_install(po)
