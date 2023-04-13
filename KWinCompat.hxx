// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _KWINCOMPAT_H_
#define _KWINCOMPAT_H_

// make porting to KF5 >= 5.101 simpler

#include <kwindowsystem_version.h>
#include <KWindowSystem>

#if KWINDOWSYSTEM_VERSION >= QT_VERSION_CHECK(5, 101, 0)
#  include <kx11extras.h>
#  define KWinCompat KX11Extras
#else
#  define KWinCompat KWindowSystem
#endif

#endif
