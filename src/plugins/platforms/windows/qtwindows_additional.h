/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTWINDOWS_ADDITIONAL_H
#define QTWINDOWS_ADDITIONAL_H

#include <QtCore/QtGlobal> // get compiler define
#include <QtCore/qt_windows.h>

/* Complement the definitions and declarations missing
 * when using MinGW or older Windows SDKs. */

#if defined(Q_CC_MINGW)
#    if !defined(ULW_ALPHA)
#        define ULW_ALPHA 0x00000002
#        define LWA_ALPHA 0x00000002
#    endif // !defined(ULW_ALPHA)
#    define SPI_GETFONTSMOOTHINGTYPE 0x200A
#    define FE_FONTSMOOTHINGCLEARTYPE 0x0002
#    define CLEARTYPE_QUALITY       5
#    define SPI_GETDROPSHADOW 0x1024

#    define CF_DIBV5 17

#define CO_E_NOT_SUPPORTED               _HRESULT_TYPEDEF_(0x80004021L)

#define IFMETHOD HRESULT STDMETHODCALLTYPE
#define IFACEMETHODIMP STDMETHODIMP
#define IFACEMETHODIMP_(type) STDMETHODIMP_(type)

#if !defined(__MINGW64_VERSION_MAJOR)

#define STATE_SYSTEM_HASPOPUP 0x40000000
#define STATE_SYSTEM_PROTECTED 0x20000000

typedef struct tagUPDATELAYEREDWINDOWINFO {
  DWORD               cbSize;
  HDC                 hdcDst;
  const POINT         *pptDst;
  const SIZE          *psize;
  HDC                 hdcSrc;
  const POINT         *pptSrc;
  COLORREF            crKey;
  const BLENDFUNCTION *pblend;
  DWORD               dwFlags;
  const RECT          *prcDirty;
} UPDATELAYEREDWINDOWINFO, *PUPDATELAYEREDWINDOWINFO;

#endif // if !defined(__MINGW64_VERSION_MAJOR)

// OpenGL Pixelformat flags.
#define PFD_SUPPORT_DIRECTDRAW      0x00002000
#define PFD_DIRECT3D_ACCELERATED    0x00004000
#define PFD_SUPPORT_COMPOSITION     0x00008000

// IME.
#define IMR_CONFIRMRECONVERTSTRING      0x0005

#endif // if defined(Q_CC_MINGW)

/* Touch is supported from Windows 7 onwards and data structures
 * are present in the Windows SDK's, but not in older MSVC Express
 * versions. */

#if defined(Q_CC_MINGW) || !defined(TOUCHEVENTF_MOVE)

#define WM_TOUCH 0x0240

typedef struct tagTOUCHINPUT {
    LONG x;
    LONG y;
    HANDLE hSource;
    DWORD dwID;
    DWORD dwFlags;
    DWORD dwMask;
    DWORD dwTime;
    ULONG_PTR dwExtraInfo;
    DWORD cxContact;
    DWORD cyContact;
} TOUCHINPUT, *PTOUCHINPUT;
typedef TOUCHINPUT const * PCTOUCHINPUT;

#    define TOUCHEVENTF_MOVE 0x0001
#    define TOUCHEVENTF_DOWN 0x0002
#    define TOUCHEVENTF_UP 0x0004
#    define TOUCHEVENTF_INRANGE 0x0008
#    define TOUCHEVENTF_PRIMARY 0x0010
#    define TOUCHEVENTF_NOCOALESCE 0x0020
#    define TOUCHEVENTF_PALM 0x0080
#    define TOUCHINPUTMASKF_CONTACTAREA 0x0004
#    define TOUCHINPUTMASKF_EXTRAINFO 0x0002

#endif // if defined(Q_CC_MINGW) || !defined(TOUCHEVENTF_MOVE)

#endif // QTWINDOWS_ADDITIONAL_H
