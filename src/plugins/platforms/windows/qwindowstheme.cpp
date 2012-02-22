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

#include "qwindowstheme.h"
#include "qwindowsdialoghelpers.h"
#include "qwindowscontext.h"
#include "qwindowsintegration.h"
#include "qt_windows.h"

#include <QtCore/QVariant>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QTextStream>
#include <QtCore/QSysInfo>
#include <QtGui/QPalette>
#include <QtGui/QGuiApplication>

QT_BEGIN_NAMESPACE

static inline QTextStream& operator<<(QTextStream &str, const QColor &c)
{
    str.setIntegerBase(16);
    str.setFieldWidth(2);
    str.setPadChar(QLatin1Char('0'));
    str << " rgb: #" << c.red()  << c.green() << c.blue();
    str.setIntegerBase(10);
    str.setFieldWidth(0);
    return str;
}

static inline QString paletteToString(const QPalette &palette)
{
    QString result;
    QTextStream str(&result);
    str << "text=" << palette.color(QPalette::WindowText)
        << " background=" << palette.color(QPalette::Window);
    return result;
}

static inline QColor mixColors(const QColor &c1, const QColor &c2)
{
    return QColor ((c1.red() + c2.red()) / 2,
                   (c1.green() + c2.green()) / 2,
                   (c1.blue() + c2.blue()) / 2);
}

static inline QColor getSysColor(int index)
{
    return qColorToCOLORREF(GetSysColor(index));
}

static inline QPalette systemPalette()
{
    QPalette result;
    result.setColor(QPalette::WindowText, getSysColor(COLOR_WINDOWTEXT));
    result.setColor(QPalette::Button, getSysColor(COLOR_BTNFACE));
    result.setColor(QPalette::Light, getSysColor(COLOR_BTNHIGHLIGHT));
    result.setColor(QPalette::Dark, getSysColor(COLOR_BTNSHADOW));
    result.setColor(QPalette::Mid, result.button().color().darker(150));
    result.setColor(QPalette::Text, getSysColor(COLOR_WINDOWTEXT));
    result.setColor(QPalette::BrightText, getSysColor(COLOR_BTNHIGHLIGHT));
    result.setColor(QPalette::Base, getSysColor(COLOR_WINDOW));
    result.setColor(QPalette::Window, getSysColor(COLOR_BTNFACE));
    result.setColor(QPalette::ButtonText, getSysColor(COLOR_BTNTEXT));
    result.setColor(QPalette::Midlight, getSysColor(COLOR_3DLIGHT));
    result.setColor(QPalette::Shadow, getSysColor(COLOR_3DDKSHADOW));
    result.setColor(QPalette::Highlight, getSysColor(COLOR_HIGHLIGHT));
    result.setColor(QPalette::HighlightedText, getSysColor(COLOR_HIGHLIGHTTEXT));
    result.setColor(QPalette::Link, Qt::blue);
    result.setColor(QPalette::LinkVisited, Qt::magenta);
    result.setColor(QPalette::Inactive, QPalette::Button, result.button().color());
    result.setColor(QPalette::Inactive, QPalette::Window, result.background().color());
    result.setColor(QPalette::Inactive, QPalette::Light, result.light().color());
    result.setColor(QPalette::Inactive, QPalette::Dark, result.dark().color());

    if (result.midlight() == result.button())
        result.setColor(QPalette::Midlight, result.button().color().lighter(110));
    if (result.background() != result.base()) {
        result.setColor(QPalette::Inactive, QPalette::Highlight, result.color(QPalette::Inactive, QPalette::Window));
        result.setColor(QPalette::Inactive, QPalette::HighlightedText, result.color(QPalette::Inactive, QPalette::Text));
    }

    const QColor disabled =
        mixColors(result.foreground().color(), result.button().color());

    result.setColorGroup(QPalette::Disabled, result.foreground(), result.button(),
                         result.light(), result.dark(), result.mid(),
                         result.text(), result.brightText(), result.base(),
                         result.background());
    result.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
    result.setColor(QPalette::Disabled, QPalette::Text, disabled);
    result.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
    result.setColor(QPalette::Disabled, QPalette::Highlight,
                    getSysColor(COLOR_HIGHLIGHT));
    result.setColor(QPalette::Disabled, QPalette::HighlightedText,
                    getSysColor(COLOR_HIGHLIGHTTEXT));
    result.setColor(QPalette::Disabled, QPalette::Base,
                    result.background().color());
    return result;
}

QPalette toolTipPalette(const QPalette &systemPalette)
{
    QPalette result(systemPalette);
    const QColor tipBgColor(getSysColor(COLOR_INFOBK));
    const QColor tipTextColor(getSysColor(COLOR_INFOTEXT));

    result.setColor(QPalette::All, QPalette::Button, tipBgColor);
    result.setColor(QPalette::All, QPalette::Window, tipBgColor);
    result.setColor(QPalette::All, QPalette::Text, tipTextColor);
    result.setColor(QPalette::All, QPalette::WindowText, tipTextColor);
    result.setColor(QPalette::All, QPalette::ButtonText, tipTextColor);
    result.setColor(QPalette::All, QPalette::Button, tipBgColor);
    result.setColor(QPalette::All, QPalette::Window, tipBgColor);
    result.setColor(QPalette::All, QPalette::Text, tipTextColor);
    result.setColor(QPalette::All, QPalette::WindowText, tipTextColor);
    result.setColor(QPalette::All, QPalette::ButtonText, tipTextColor);
    const QColor disabled =
        mixColors(result.foreground().color(), result.button().color());
    result.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
    result.setColor(QPalette::Disabled, QPalette::Text, disabled);
    result.setColor(QPalette::Disabled, QPalette::Base, Qt::white);
    result.setColor(QPalette::Disabled, QPalette::BrightText, Qt::white);
    return result;
}

static inline bool booleanSystemParametersInfo(UINT what, bool defaultValue)
{
    BOOL result;
    if (SystemParametersInfo(what, 0, &result, 0))
        return result ? true : false;
    return defaultValue;
}

static inline bool dWordSystemParametersInfo(UINT what, DWORD defaultValue)
{
    DWORD result;
    if (SystemParametersInfo(what, 0, &result, 0))
        return result;
    return defaultValue;
}

QWindowsTheme::QWindowsTheme()
{
    qFill(m_palettes, m_palettes + NPalettes, static_cast<QPalette *>(0));
    refresh();
}

QWindowsTheme::~QWindowsTheme()
{
    clearPalettes();
}

void QWindowsTheme::clearPalettes()
{
    qDeleteAll(m_palettes, m_palettes + NPalettes);
    qFill(m_palettes, m_palettes + NPalettes, static_cast<QPalette *>(0));
}

QWindowsTheme *QWindowsTheme::instance()
{
    return static_cast<QWindowsTheme *>(QWindowsIntegration::instance()->platformTheme());
}

static inline QStringList iconThemeSearchPaths()
{
    const QFileInfo appDir(QCoreApplication::applicationDirPath() + QStringLiteral("/icons"));
    return appDir.isDir() ? QStringList(appDir.absoluteFilePath()) : QStringList();
}

static inline QStringList styleNames()
{
    QStringList result;
    if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA)
        result.append(QStringLiteral("WindowsVista"));
    if (QSysInfo::WindowsVersion >= QSysInfo::WV_XP)
        result.append(QStringLiteral("WindowsXP"));
    result.append(QStringLiteral("Windows"));
    return result;
}

QVariant QWindowsTheme::themeHint(ThemeHint hint) const
{
    switch (hint) {
    case UseFullScreenForPopupMenu:
        return QVariant(true);
    case DialogButtonBoxLayout:
        return QVariant(int(0)); // QDialogButtonBox::WinLayout
    case IconThemeSearchPaths:
        return QVariant(iconThemeSearchPaths());
    case StyleNames:
        return QVariant(styleNames());
    case TextCursorWidth:
        return QVariant(int(dWordSystemParametersInfo(SPI_GETCARETWIDTH, 1u)));
    case DropShadow:
        return QVariant(booleanSystemParametersInfo(SPI_GETDROPSHADOW, false));
    case MaximumScrollBarDragDistance:
        return QVariant(qRound(qreal(QWindowsContext::instance()->defaultDPI()) * 1.375));
    case KeyboardScheme:
        return QVariant(int(WindowsKeyboardScheme));
    default:
        break;
    }
    return QPlatformTheme::themeHint(hint);
}

void QWindowsTheme::refresh()
{
    clearPalettes();
    if (QGuiApplication::desktopSettingsAware()) {
        m_palettes[SystemPalette] = new QPalette(systemPalette());
        m_palettes[ToolTipPalette] = new QPalette(toolTipPalette(*m_palettes[SystemPalette]));
        if (QWindowsContext::verboseTheming)
            qDebug() << __FUNCTION__ << '\n'
                     << "  system=" << paletteToString(*m_palettes[SystemPalette])
                     << "  tooltip=" << paletteToString(*m_palettes[ToolTipPalette]);
    }
}

bool QWindowsTheme::usePlatformNativeDialog(DialogType type) const
{
    return QWindowsDialogs::useHelper(type);
}

QPlatformDialogHelper *QWindowsTheme::createPlatformDialogHelper(DialogType type) const
{
    return QWindowsDialogs::createHelper(type);
}

void QWindowsTheme::windowsThemeChanged(QWindow * /* window */)
{
    refresh();
    // QWindowSystemInterface::handleThemeChange(window);
}

QT_END_NAMESPACE
