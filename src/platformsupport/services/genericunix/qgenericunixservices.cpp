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

#include "qgenericunixservices_p.h"

#include <QtCore/QStandardPaths>
#include <QtCore/QProcess>
#include <QtCore/QUrl>
#include <QtCore/QDebug>

#include <stdlib.h>

QT_BEGIN_NAMESPACE

enum { debug = 0 };

static inline QGenericUnixServices::DesktopEnvironment detectDesktopEnvironment()
{
    if (!qgetenv("KDE_FULL_SESSION").isEmpty())
        return QGenericUnixServices::DE_KDE;
    // GNOME_DESKTOP_SESSION_ID is deprecated for some reason, but still check it
    if (qgetenv("DESKTOP_SESSION") == "gnome" || !qgetenv("GNOME_DESKTOP_SESSION_ID").isEmpty())
        return QGenericUnixServices::DE_GNOME;
    return QGenericUnixServices::DE_UNKNOWN;
}

static inline bool checkExecutable(const QString &candidate, QString *result)
{
    *result = QStandardPaths::findExecutable(candidate);
    return !result->isEmpty();
}

static inline bool detectWebBrowser(QGenericUnixServices::DesktopEnvironment desktop,
                                    bool checkBrowserVariable,
                                    QString *browser)
{
    const char *browsers[] = {"google-chrome", "firefox", "mozilla", "opera"};

    browser->clear();
    if (checkExecutable(QStringLiteral("xdg-open"), browser))
        return true;

    if (checkBrowserVariable) {
        QByteArray browserVariable = qgetenv("DEFAULT_BROWSER");
        if (browserVariable.isEmpty())
            browserVariable = qgetenv("BROWSER");
        if (!browserVariable.isEmpty() && checkExecutable(QString::fromLocal8Bit(browserVariable), browser))
            return true;
    }

    switch (desktop) {
    case QGenericUnixServices::DE_UNKNOWN:
        break;
    case QGenericUnixServices::DE_KDE:
        // Konqueror launcher
        if (checkExecutable(QStringLiteral("kfmclient"), browser)) {
            browser->append(QStringLiteral(" exec"));
            return true;
        }
    case QGenericUnixServices::DE_GNOME:
        if (checkExecutable(QStringLiteral("gnome-open"), browser))
            return true;
        break;
    }

    for (size_t i = 0; i < sizeof(browsers)/sizeof(char *); ++i)
        if (checkExecutable(QLatin1String(browsers[i]), browser))
            return true;
    return false;
}

static inline bool launch(const QString &launcher, const QUrl &url)
{
    const QString command = launcher + QLatin1Char(' ') + QLatin1String(url.toEncoded());
    if (debug)
        qDebug("Launching %s", qPrintable(command));
#if defined(QT_NO_PROCESS)
    const bool ok = ::system(qPrintable(command + QStringLiteral(" &")));
#else
    const bool ok = QProcess::startDetached(command);
#endif
    if (!ok)
        qWarning("Launch failed (%s)", qPrintable(command));
    return ok;
}

QGenericUnixServices::DesktopEnvironment QGenericUnixServices::desktopEnvironment()
{
    static const DesktopEnvironment result = detectDesktopEnvironment();
    return result;
}

bool QGenericUnixServices::openUrl(const QUrl &url)
{
    if (url.scheme() == QStringLiteral("mailto"))
        return openDocument(url);

    if (m_webBrowser.isEmpty() && !detectWebBrowser(desktopEnvironment(), true, &m_webBrowser)) {
        qWarning("%s: Unable to detect a web browser to launch '%s'", Q_FUNC_INFO, qPrintable(url.toString()));
        return false;
    }
    return launch(m_webBrowser, url);
}

bool QGenericUnixServices::openDocument(const QUrl &url)
{
    if (m_documentLauncher.isEmpty() && !detectWebBrowser(desktopEnvironment(), false, &m_documentLauncher)) {
        qWarning("%s: Unable to detect a launcher for '%s'", Q_FUNC_INFO, qPrintable(url.toString()));
        return false;
    }
    return launch(m_documentLauncher, url);
}

QT_END_NAMESPACE
