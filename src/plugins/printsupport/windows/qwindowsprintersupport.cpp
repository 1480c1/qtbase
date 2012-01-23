/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwindowsprintersupport.h"

#include <QtCore/QList>
#include <QtPrintSupport/QPrinterInfo>
#include <qprintengine_win_p.h>
#include <private/qpaintengine_alpha_p.h>
#include <private/qprinterinfo_p.h>

QWindowsPrinterSupport::QWindowsPrinterSupport()
    : QPlatformPrinterSupport()
{
    DWORD needed = 0;
    DWORD returned = 0;
    if (!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 4, 0, 0, &needed, &returned)) {
        LPBYTE buffer = new BYTE[needed];
        if (EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 4, buffer, needed, &needed, &returned)) {
            PPRINTER_INFO_4 infoList = reinterpret_cast<PPRINTER_INFO_4>(buffer);
            QPrinterInfo defPrn = defaultPrinter();
            for (uint i = 0; i < returned; ++i) {
                QString printerName(QString::fromWCharArray(infoList[i].pPrinterName));

                QPrinterInfo printerInfo(printerName);
                if (printerInfo.printerName() == defPrn.printerName())
                    printerInfo.d_ptr->isDefault = true;
                mPrinterList.append(printerInfo);
            }
        }
        delete [] buffer;
    }
}

QWindowsPrinterSupport::~QWindowsPrinterSupport()
{
}

QPrintEngine *QWindowsPrinterSupport::createNativePrintEngine(QPrinter::PrinterMode printerMode)
{
        return new QWin32PrintEngine(printerMode);
}

QPaintEngine *QWindowsPrinterSupport::createPaintEngine(QPrintEngine *engine, QPrinter::PrinterMode printerMode)
{
    Q_UNUSED(printerMode)
    return static_cast<QWin32PrintEngine *>(engine);
}

QList<QPrinter::PaperSize> QWindowsPrinterSupport::supportedPaperSizes(const QPrinterInfo &) const
{
    QList<QPrinter::PaperSize> returnList;
    foreach (const QPrinterInfo &info, mPrinterList) {
        foreach (const QPrinter::PaperSize supportedSize, info.supportedPaperSizes())
            if (!returnList.contains(supportedSize))
                returnList.append(supportedSize);
    }
    return returnList;
}

QList<QPrinterInfo> QWindowsPrinterSupport::availablePrinters()
{
    return mPrinterList;
}
