/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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

#include "qwindowsfontdatabase.h"
#include "qwindowscontext.h"
#include "qwindowsfontengine.h"
#include "qwindowsfontenginedirectwrite.h"
#include "qtwindows_additional.h"

#include <QtGui/QFont>
#include <QtGui/QGuiApplication>

#include <QtCore/qmath.h>
#include <QtCore/QDebug>

#if !defined(QT_NO_DIRECTWRITE)
#    include <dwrite.h>
#    include <d2d1.h>
#endif

QT_BEGIN_NAMESPACE

/*!
    \struct QWindowsFontEngineData
    \brief Static constant data shared by the font engines.
    \ingroup qt-lighthouse-win
*/

QWindowsFontEngineData::QWindowsFontEngineData()
#if !defined(QT_NO_DIRECTWRITE)
    : directWriteFactory(0)
    , directWriteGdiInterop(0)
#endif
{
    // from qapplication_win.cpp
    UINT result = 0;
    if (SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &result, 0))
        clearTypeEnabled = (result == FE_FONTSMOOTHINGCLEARTYPE);

    int winSmooth;
    if (SystemParametersInfo(0x200C /* SPI_GETFONTSMOOTHINGCONTRAST */, 0, &winSmooth, 0)) {
        fontSmoothingGamma = winSmooth / qreal(1000.0);
    } else {
        fontSmoothingGamma = 1.0;
    }

    // Safeguard ourselves against corrupt registry values...
    if (fontSmoothingGamma > 5 || fontSmoothingGamma < 1)
        fontSmoothingGamma = qreal(1.4);

    const qreal gray_gamma = 2.31;
    for (int i=0; i<256; ++i)
        pow_gamma[i] = uint(qRound(qPow(i / qreal(255.), gray_gamma) * 2047));

    HDC displayDC = GetDC(0);
    hdc = CreateCompatibleDC(displayDC);
    ReleaseDC(0, displayDC);
}

QWindowsFontEngineData::~QWindowsFontEngineData()
{
    if (hdc)
        ReleaseDC(0, hdc);
#if !defined(QT_NO_DIRECTWRITE)
    if (directWriteGdiInterop)
        directWriteGdiInterop->Release();
    if (directWriteFactory)
        directWriteFactory->Release();
#endif
}

#if !defined(QT_NO_DIRECTWRITE)
static inline bool initDirectWrite(QWindowsFontEngineData *d)
{
    if (!d->directWriteFactory) {
        const HRESULT hr = DWriteCreateFactory(
                    DWRITE_FACTORY_TYPE_SHARED,
                    __uuidof(IDWriteFactory),
                    reinterpret_cast<IUnknown **>(&d->directWriteFactory)
                    );
        if (FAILED(hr)) {
            qErrnoWarning("%s: DWriteCreateFactory failed", __FUNCTION__);
            return false;
        }
    }
    if (!d->directWriteGdiInterop) {
        const HRESULT  hr = d->directWriteFactory->GetGdiInterop(&d->directWriteGdiInterop);
        if (FAILED(hr)) {
            qErrnoWarning("%s: GetGdiInterop failed", __FUNCTION__);
            return false;
        }
    }
    return true;
}

#endif // !defined(QT_NO_DIRECTWRITE)

/*!
    \class QWindowsFontDatabase
    \brief Font database for Windows

    \note The Qt 4.8 WIndows font database employed a mechanism of
    delayed population of the database again passing a font name
    to EnumFontFamiliesEx(), working around the fact that
    EnumFontFamiliesEx() does not list all fonts by default.
    This should be introduced to Lighthouse as well?

    \ingroup qt-lighthouse-win
*/

QDebug operator<<(QDebug d, const QFontDef &def)
{
    d.nospace() << "Family=" << def.family << " Stylename=" << def.styleName
                << " pointsize=" << def.pointSize << " pixelsize=" << def.pixelSize
                << " styleHint=" << def.styleHint << " weight=" << def.weight
                << " stretch=" << def.stretch << " hintingPreference="
                << def.hintingPreference << ' ';
    return d;
}

/* From QFontDatabase.cpp, qt_determine_writing_systems_from_truetype_bits().
 * Fixme: Make public? */

// see the Unicode subset bitfields in the MSDN docs
static int requiredUnicodeBits[QFontDatabase::WritingSystemsCount][2] = {
        // Any,
    { 127, 127 },
        // Latin,
    { 0, 127 },
        // Greek,
    { 7, 127 },
        // Cyrillic,
    { 9, 127 },
        // Armenian,
    { 10, 127 },
        // Hebrew,
    { 11, 127 },
        // Arabic,
    { 13, 127 },
        // Syriac,
    { 71, 127 },
    //Thaana,
    { 72, 127 },
    //Devanagari,
    { 15, 127 },
    //Bengali,
    { 16, 127 },
    //Gurmukhi,
    { 17, 127 },
    //Gujarati,
    { 18, 127 },
    //Oriya,
    { 19, 127 },
    //Tamil,
    { 20, 127 },
    //Telugu,
    { 21, 127 },
    //Kannada,
    { 22, 127 },
    //Malayalam,
    { 23, 127 },
    //Sinhala,
    { 73, 127 },
    //Thai,
    { 24, 127 },
    //Lao,
    { 25, 127 },
    //Tibetan,
    { 70, 127 },
    //Myanmar,
    { 74, 127 },
        // Georgian,
    { 26, 127 },
        // Khmer,
    { 80, 127 },
        // SimplifiedChinese,
    { 126, 127 },
        // TraditionalChinese,
    { 126, 127 },
        // Japanese,
    { 126, 127 },
        // Korean,
    { 56, 127 },
        // Vietnamese,
    { 0, 127 }, // same as latin1
        // Other,
    { 126, 127 },
        // Ogham,
    { 78, 127 },
        // Runic,
    { 79, 127 },
        // Nko,
    { 14, 127 },
};

enum
{
    SimplifiedChineseCsbBit = 18,
    TraditionalChineseCsbBit = 20,
    JapaneseCsbBit = 17,
    KoreanCsbBit = 21
};

static inline void writingSystemsFromTrueTypeBits(quint32 unicodeRange[4],
                                                  quint32 codePageRange[2],
                                                  QSupportedWritingSystems *ws)
{
    bool hasScript = false;
    for(int i = 0; i < QFontDatabase::WritingSystemsCount; i++) {
        int bit = requiredUnicodeBits[i][0];
        int index = bit/32;
        int flag =  1 << (bit&31);
        if (bit != 126 && unicodeRange[index] & flag) {
            bit = requiredUnicodeBits[i][1];
            index = bit/32;

            flag =  1 << (bit&31);
            if (bit == 127 || unicodeRange[index] & flag) {
                ws->setSupported(QFontDatabase::WritingSystem(i), true);
                hasScript = true;
            }
        }
    }
    if(codePageRange[0] & (1 << SimplifiedChineseCsbBit)) {
        ws->setSupported(QFontDatabase::SimplifiedChinese, true);
        hasScript = true;
    }
    if(codePageRange[0] & (1 << TraditionalChineseCsbBit)) {
        ws->setSupported(QFontDatabase::TraditionalChinese, true);
        hasScript = true;
    }
    if(codePageRange[0] & (1 << JapaneseCsbBit)) {
        ws->setSupported(QFontDatabase::Japanese, true);
        hasScript = true;
        //qDebug("font %s supports Japanese", familyName.latin1());
    }
    if(codePageRange[0] & (1 << KoreanCsbBit)) {
        ws->setSupported(QFontDatabase::Korean, true);
        hasScript = true;
    }
    if (!hasScript)
        ws->setSupported(QFontDatabase::Symbol, true);
}

// convert 0 ~ 1000 integer to QFont::Weight
static inline QFont::Weight weightFromInteger(long weight)
{
    if (weight < 400)
        return QFont::Light;
    if (weight < 600)
        return QFont::Normal;
    if (weight < 700)
        return QFont::DemiBold;
    if (weight < 800)
        return QFont::Bold;
    return QFont::Black;
}

static inline QFontDatabase::WritingSystem writingSystemFromScript(const QString &scriptName)
{
    if (scriptName == QStringLiteral("Western")
            || scriptName == QStringLiteral("Baltic")
            || scriptName == QStringLiteral("Central European")
            || scriptName == QStringLiteral("Turkish")
            || scriptName == QStringLiteral("Vietnamese")
            || scriptName == QStringLiteral("OEM/Dos"))
        return QFontDatabase::Latin;
    if (scriptName == QStringLiteral("Thai"))
        return QFontDatabase::Thai;
    if (scriptName == QStringLiteral("Symbol")
        || scriptName == QStringLiteral("Other"))
        return QFontDatabase::Symbol;
    if (scriptName == QStringLiteral("CHINESE_GB2312"))
        return QFontDatabase::SimplifiedChinese;
    if (scriptName == QStringLiteral("CHINESE_BIG5"))
        return QFontDatabase::TraditionalChinese;
    if (scriptName == QStringLiteral("Cyrillic"))
        return QFontDatabase::Cyrillic;
    if (scriptName == QStringLiteral("Hangul"))
        return QFontDatabase::Korean;
    if (scriptName == QStringLiteral("Hebrew"))
        return QFontDatabase::Hebrew;
    if (scriptName == QStringLiteral("Greek"))
        return QFontDatabase::Greek;
    if (scriptName == QStringLiteral("Japanese"))
        return QFontDatabase::Japanese;
    if (scriptName == QStringLiteral("Arabic"))
        return QFontDatabase::Arabic;
    return QFontDatabase::Any;
}

static bool addFontToDatabase(QString familyName, const QString &scriptName,
                              const TEXTMETRIC *textmetric,
                              const FONTSIGNATURE *signature,
                              int type)
{
    // the "@family" fonts are just the same as "family". Ignore them.
    if (familyName.at(0) == QLatin1Char('@') || familyName.startsWith(QStringLiteral("WST_")))
        return false;

    static const int SMOOTH_SCALABLE = 0xffff;
    const QString foundryName; // No such concept.
    const NEWTEXTMETRIC *tm = (NEWTEXTMETRIC *)textmetric;
    const bool fixed = !(tm->tmPitchAndFamily & TMPF_FIXED_PITCH);
    const bool ttf = (tm->tmPitchAndFamily & TMPF_TRUETYPE);
    const bool scalable = tm->tmPitchAndFamily & (TMPF_VECTOR|TMPF_TRUETYPE);
    const int size = scalable ? SMOOTH_SCALABLE : tm->tmHeight;
    const QFont::Style style = tm->tmItalic ? QFont::StyleItalic : QFont::StyleNormal;
    const bool antialias = false;
    const QFont::Weight weight = weightFromInteger(tm->tmWeight);
    const QFont::Stretch stretch = QFont::Unstretched;

    Q_UNUSED(fixed)

    if (QWindowsContext::verboseFonts > 2) {
        QDebug nospace = qDebug().nospace();
        nospace << __FUNCTION__ << familyName << scriptName
                 << "TTF=" << ttf;
        if (type & DEVICE_FONTTYPE)
            nospace << " DEVICE";
        if (type & RASTER_FONTTYPE)
            nospace << " RASTER";
        if (type & TRUETYPE_FONTTYPE)
            nospace << " TRUETYPE";
        nospace << " scalable=" << scalable << " Size=" << size
                << " Style=" << style << " Weight=" << weight
                << " stretch=" << stretch;
    }

/* Fixme: omitted for the moment
    if(ttf && localizedName(familyName) && family->english_name.isEmpty())
        family->english_name = getEnglishName(familyName);
*/
    QSupportedWritingSystems writingSystems;
    if (type & TRUETYPE_FONTTYPE) {
        quint32 unicodeRange[4] = {
            signature->fsUsb[0], signature->fsUsb[1],
            signature->fsUsb[2], signature->fsUsb[3]
        };
        quint32 codePageRange[2] = {
            signature->fsCsb[0], signature->fsCsb[1]
        };
        writingSystemsFromTrueTypeBits(unicodeRange, codePageRange, &writingSystems);
        // ### Hack to work around problem with Thai text on Windows 7. Segoe UI contains
        // the symbol for Baht, and Windows thus reports that it supports the Thai script.
        // Since it's the default UI font on this platform, most widgets will be unable to
        // display Thai text by default. As a temporary work around, we special case Segoe UI
        // and remove the Thai script from its list of supported writing systems.
        if (writingSystems.supported(QFontDatabase::Thai) &&
                familyName == QStringLiteral("Segoe UI"))
            writingSystems.setSupported(QFontDatabase::Thai, false);
    } else {
        const QFontDatabase::WritingSystem ws = writingSystemFromScript(scriptName);
        if (ws != QFontDatabase::Any)
            writingSystems.setSupported(ws);
    }

    QPlatformFontDatabase::registerFont(familyName, foundryName, weight,
                                        style, stretch, antialias, scalable, size, writingSystems, 0);
    // add fonts windows can generate for us:
    if (weight <= QFont::DemiBold)
        QPlatformFontDatabase::registerFont(familyName, foundryName, QFont::Bold,
                                            style, stretch, antialias, scalable, size, writingSystems, 0);
    if (style != QFont::StyleItalic)
        QPlatformFontDatabase::registerFont(familyName, foundryName, weight,
                                            QFont::StyleItalic, stretch, antialias, scalable, size, writingSystems, 0);
    if (weight <= QFont::DemiBold && style != QFont::StyleItalic)
        QPlatformFontDatabase::registerFont(familyName, foundryName, QFont::Bold,
                                            QFont::StyleItalic, stretch, antialias, scalable, size, writingSystems, 0);
    return true;
}

static int CALLBACK storeFont(ENUMLOGFONTEX* f, NEWTEXTMETRICEX *textmetric,
                              int type, LPARAM namesSetIn)
{
    typedef QSet<QString> StringSet;
    const QString familyName = QString::fromWCharArray(f->elfLogFont.lfFaceName);
    const QString script = QString::fromWCharArray(f->elfScript);

    const FONTSIGNATURE signature = textmetric->ntmFontSig;

    // NEWTEXTMETRICEX is a NEWTEXTMETRIC, which according to the documentation is
    // identical to a TEXTMETRIC except for the last four members, which we don't use
    // anyway
    if (addFontToDatabase(familyName, script, (TEXTMETRIC *)textmetric, &signature, type))
        reinterpret_cast<StringSet *>(namesSetIn)->insert(familyName);

    // keep on enumerating
    return 1;
}

void QWindowsFontDatabase::populateFontDatabase()
{
    if (m_families.isEmpty()) {
        QPlatformFontDatabase::populateFontDatabase();
        populate(); // Called multiple times.
        // Work around EnumFontFamiliesEx() not listing the system font, see below.
        const QString sysFontFamily = QGuiApplication::font().family();
        if (!m_families.contains(sysFontFamily))
             populate(sysFontFamily);
    }
}

/*!
    \brief Populate font database using EnumFontFamiliesEx().

    Normally, leaving the name empty should enumerate
    all fonts, however, system fonts like "MS Shell Dlg 2"
    are only found when specifying the name explicitly.
*/

void QWindowsFontDatabase::populate(const QString &family)
    {

    if (QWindowsContext::verboseFonts)
        qDebug() << __FUNCTION__ << m_families.size() << family;

    HDC dummy = GetDC(0);
    LOGFONT lf;
    lf.lfCharSet = DEFAULT_CHARSET;
    if (family.size() >= LF_FACESIZE) {
        qWarning("%s: Unable to enumerate family '%s'.",
                 __FUNCTION__, qPrintable(family));
        return;
    }
    wmemcpy(lf.lfFaceName, reinterpret_cast<const wchar_t*>(family.utf16()),
            family.size() + 1);
    lf.lfPitchAndFamily = 0;
    EnumFontFamiliesEx(dummy, &lf, (FONTENUMPROC)storeFont,
                       (LPARAM)&m_families, 0);
    ReleaseDC(0, dummy);
}

QWindowsFontDatabase::QWindowsFontDatabase() :
    m_fontEngineData(new QWindowsFontEngineData)
{
    if (QWindowsContext::verboseFonts)
        qDebug() << __FUNCTION__ << "Clear type: "
                 << m_fontEngineData->clearTypeEnabled << "gamma: "
                 << m_fontEngineData->fontSmoothingGamma;
}

QWindowsFontDatabase::~QWindowsFontDatabase()
{
}

QFontEngine * QWindowsFontDatabase::fontEngine(const QFontDef &fontDef,
                                              QUnicodeTables::Script script,
                                              void *handle)
{
    QFontEngine *fe = QWindowsFontDatabase::createEngine(script, fontDef,
                                              0, QWindowsContext::instance()->defaultDPI(), false,
                                              QStringList(), m_fontEngineData);
    if (QWindowsContext::verboseFonts)
        qDebug() << __FUNCTION__ << "FONTDEF" << fontDef << script << fe << handle;
    return fe;
}

QFontEngine *QWindowsFontDatabase::fontEngine(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference)
{
    QFontEngine *fe = QPlatformFontDatabase::fontEngine(fontData, pixelSize, hintingPreference);
    if (QWindowsContext::verboseFonts)
        qDebug() << __FUNCTION__ << "FONTDATA" << fontData << pixelSize << hintingPreference << fe;
    return fe;
}

QStringList QWindowsFontDatabase::fallbacksForFamily(const QString family, const QFont::Style &style, const QFont::StyleHint &styleHint, const QUnicodeTables::Script &script) const
{
    QStringList result = QPlatformFontDatabase::fallbacksForFamily(family, style, styleHint, script);
    if (!result.isEmpty())
        return result;
    if (QWindowsContext::verboseFonts)
        qDebug() << __FUNCTION__ << family << style << styleHint
                 << script << result << m_families.size();
    return result;
}

QStringList QWindowsFontDatabase::addApplicationFont(const QByteArray &fontData, const QString &fileName)
{
    const QStringList result = QPlatformFontDatabase::addApplicationFont(fontData, fileName);
    Q_UNIMPLEMENTED();
    return result;
}

void QWindowsFontDatabase::releaseHandle(void *handle)
{
    if (handle && QWindowsContext::verboseFonts)
        qDebug() << __FUNCTION__ << handle;
}

QString QWindowsFontDatabase::fontDir() const
{
    const QString result = QPlatformFontDatabase::fontDir();
    if (QWindowsContext::verboseFonts)
        qDebug() << __FUNCTION__ << result;
    return result;
}

HFONT QWindowsFontDatabase::systemFont()
{
    static const HFONT stock_sysfont = (HFONT)GetStockObject(SYSTEM_FONT);
    return stock_sysfont;
}

// Creation functions

static inline bool scriptRequiresOpenType(int script)
{
    return ((script >= QUnicodeTables::Syriac && script <= QUnicodeTables::Sinhala)
            || script == QUnicodeTables::Khmer || script == QUnicodeTables::Nko);
}

static const char *other_tryFonts[] = {
    "Arial",
    "MS UI Gothic",
    "Gulim",
    "SimSun",
    "PMingLiU",
    "Arial Unicode MS",
    0
};

static const char *jp_tryFonts [] = {
    "MS UI Gothic",
    "Arial",
    "Gulim",
    "SimSun",
    "PMingLiU",
    "Arial Unicode MS",
    0
};

static const char *ch_CN_tryFonts [] = {
    "SimSun",
    "Arial",
    "PMingLiU",
    "Gulim",
    "MS UI Gothic",
    "Arial Unicode MS",
    0
};

static const char *ch_TW_tryFonts [] = {
    "PMingLiU",
    "Arial",
    "SimSun",
    "Gulim",
    "MS UI Gothic",
    "Arial Unicode MS",
    0
};

static const char *kr_tryFonts[] = {
    "Gulim",
    "Arial",
    "PMingLiU",
    "SimSun",
    "MS UI Gothic",
    "Arial Unicode MS",
    0
};

static const char **tryFonts = 0;

QFontEngine *QWindowsFontDatabase::createEngine(int script, const QFontDef &request,
                                                HDC fontHdc, int dpi, bool rawMode,
                                                const QStringList &family_list,
                                                const QSharedPointer<QWindowsFontEngineData> &data)
{
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));

    const bool useDevice = (request.styleStrategy & QFont::PreferDevice) && fontHdc;

    const HDC hdc = useDevice ? fontHdc : data->hdc;

    bool stockFont = false;
    bool preferClearTypeAA = false;

    HFONT hfont = 0;

#if !defined(QT_NO_DIRECTWRITE)
    bool useDirectWrite = (request.hintingPreference == QFont::PreferNoHinting)
                       || (request.hintingPreference == QFont::PreferVerticalHinting);
    IDWriteFont *directWriteFont = 0;
#else
    bool useDirectWrite = false;
#endif

    if (rawMode) {                        // will choose a stock font
        int f = SYSTEM_FONT;
        const QString fam = request.family.toLower();
        if (fam == QStringLiteral("default") || fam == QStringLiteral("system"))
            f = SYSTEM_FONT;
        else if (fam == QStringLiteral("system_fixed"))
            f = SYSTEM_FIXED_FONT;
        else if (fam == QStringLiteral("ansi_fixed"))
            f = ANSI_FIXED_FONT;
        else if (fam == QStringLiteral("ansi_var"))
            f = ANSI_VAR_FONT;
        else if (fam == QStringLiteral("device_default"))
            f = DEVICE_DEFAULT_FONT;
        else if (fam == QStringLiteral("oem_fixed"))
            f = OEM_FIXED_FONT;
        else if (fam.at(0) == QLatin1Char('#'))
            f = fam.right(fam.length()-1).toInt();
        hfont = (HFONT)GetStockObject(f);
        if (!hfont) {
            qErrnoWarning("%s: GetStockObject failed", __FUNCTION__);
            hfont = QWindowsFontDatabase::systemFont();
        }
        stockFont = true;
    } else {
        int hint = FF_DONTCARE;
        switch (request.styleHint) {
            case QFont::Helvetica:
                hint = FF_SWISS;
                break;
            case QFont::Times:
                hint = FF_ROMAN;
                break;
            case QFont::Courier:
                hint = FF_MODERN;
                break;
            case QFont::OldEnglish:
                hint = FF_DECORATIVE;
                break;
            case QFont::System:
                hint = FF_MODERN;
                break;
            default:
                break;
        }

        lf.lfHeight = -qRound(request.pixelSize);
        lf.lfWidth                = 0;
        lf.lfEscapement        = 0;
        lf.lfOrientation        = 0;
        if (request.weight == 50)
            lf.lfWeight = FW_DONTCARE;
        else
            lf.lfWeight = (request.weight*900)/99;
        lf.lfItalic         = request.style != QFont::StyleNormal;
        lf.lfCharSet        = DEFAULT_CHARSET;

        int strat = OUT_DEFAULT_PRECIS;
        if (request.styleStrategy & QFont::PreferBitmap) {
            strat = OUT_RASTER_PRECIS;
        } else if (request.styleStrategy & QFont::PreferDevice) {
            strat = OUT_DEVICE_PRECIS;
        } else if (request.styleStrategy & QFont::PreferOutline) {
            strat = OUT_OUTLINE_PRECIS;
        } else if (request.styleStrategy & QFont::ForceOutline) {
            strat = OUT_TT_ONLY_PRECIS;
        }

        lf.lfOutPrecision   = strat;

        int qual = DEFAULT_QUALITY;

        if (request.styleStrategy & QFont::PreferMatch)
            qual = DRAFT_QUALITY;
        else if (request.styleStrategy & QFont::PreferQuality)
            qual = PROOF_QUALITY;

        if (request.styleStrategy & QFont::PreferAntialias) {
            if (QSysInfo::WindowsVersion >= QSysInfo::WV_XP) {
                qual = CLEARTYPE_QUALITY;
                preferClearTypeAA = true;
            } else {
                qual = ANTIALIASED_QUALITY;
            }
        } else if (request.styleStrategy & QFont::NoAntialias) {
            qual = NONANTIALIASED_QUALITY;
        }

        lf.lfQuality        = qual;

        lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
        lf.lfPitchAndFamily = DEFAULT_PITCH | hint;

        QString fam = request.family;

        if(fam.isEmpty())
            fam = QStringLiteral("MS Sans Serif");

        if ((fam == QStringLiteral("MS Sans Serif"))
            && (request.style == QFont::StyleItalic || (-lf.lfHeight > 18 && -lf.lfHeight != 24))) {
            fam = QStringLiteral("Arial"); // MS Sans Serif has bearing problems in italic, and does not scale
        }
        if (fam == QStringLiteral("Courier") && !(request.styleStrategy & QFont::PreferBitmap))
            fam = QStringLiteral("Courier New");

        memcpy(lf.lfFaceName, fam.utf16(), sizeof(wchar_t) * qMin(fam.length() + 1, 32));  // 32 = Windows hard-coded

        hfont = CreateFontIndirect(&lf);
        if (!hfont)
            qErrnoWarning("%s: CreateFontIndirect failed", __FUNCTION__);

        stockFont = (hfont == 0);
        bool ttf = false;
        int avWidth = 0;
        BOOL res;
        HGDIOBJ oldObj = SelectObject(hdc, hfont);

        TEXTMETRIC tm;
        res = GetTextMetrics(hdc, &tm);
        avWidth = tm.tmAveCharWidth;
        ttf = tm.tmPitchAndFamily & TMPF_TRUETYPE;
        SelectObject(hdc, oldObj);

        if (!ttf || !useDirectWrite) {
            useDirectWrite = false;

            if (hfont && (!ttf || request.stretch != 100)) {
                DeleteObject(hfont);
                if (!res)
                    qErrnoWarning("QFontEngine::loadEngine: GetTextMetrics failed");
                lf.lfWidth = avWidth * request.stretch/100;
                hfont = CreateFontIndirect(&lf);
                if (!hfont)
                    qErrnoWarning("%s: CreateFontIndirect with stretch failed", __FUNCTION__);
            }

            if (hfont == 0) {
                hfont = (HFONT)GetStockObject(ANSI_VAR_FONT);
                stockFont = true;
            }
        }

#if !defined(QT_NO_DIRECTWRITE)
        else {
            // Default to false for DirectWrite (and re-enable once/if everything
            // turns out okay)
            useDirectWrite = false;
            if (initDirectWrite(data.data())) {
                const QString nameSubstitute = QWindowsFontEngineDirectWrite::fontNameSubstitute(QString::fromWCharArray(lf.lfFaceName));
                memcpy(lf.lfFaceName, nameSubstitute.utf16(),
                       sizeof(wchar_t) * qMin(nameSubstitute.length() + 1, LF_FACESIZE));

                HRESULT hr = data->directWriteGdiInterop->CreateFontFromLOGFONT(
                            &lf,
                            &directWriteFont);
                if (FAILED(hr)) {
                    qErrnoWarning("%s: CreateFontFromLOGFONT failed", __FUNCTION__);
                } else {
                    DeleteObject(hfont);
                    useDirectWrite = true;
                }
        }
        }
#endif
    }

    QFontEngine *fe = 0;
    if (!useDirectWrite)  {
        QWindowsFontEngine *few = new QWindowsFontEngine(request.family, hfont, stockFont, lf, data);
        few->setObjectName(QStringLiteral("QWindowsFontEngine_") + request.family);
        if (preferClearTypeAA)
            few->glyphFormat = QFontEngineGlyphCache::Raster_RGBMask;

        // Also check for OpenType tables when using complex scripts
        // ### TODO: This only works for scripts that require OpenType. More generally
        // for scripts that do not require OpenType we should just look at the list of
        // supported writing systems in the font's OS/2 table.
        if (scriptRequiresOpenType(script)) {
            HB_Face hbFace = few->harfbuzzFace();
            if (!hbFace || !hbFace->supported_scripts[script]) {
                qWarning("  OpenType support missing for script\n");
                delete few;
                return 0;
            }
        }

        few->initFontInfo(request, fontHdc, dpi);
        fe = few;
    }

#if !defined(QT_NO_DIRECTWRITE)
    else {
        IDWriteFontFace *directWriteFontFace = NULL;
        HRESULT hr = directWriteFont->CreateFontFace(&directWriteFontFace);
        if (SUCCEEDED(hr)) {
            QWindowsFontEngineDirectWrite *fedw = new QWindowsFontEngineDirectWrite(directWriteFontFace,
                                                                                    request.pixelSize,
                                                                                    data);
            fedw->initFontInfo(request, dpi, directWriteFont);
            fedw->setObjectName(QStringLiteral("QWindowsFontEngineDirectWrite_") + request.family);
            fe = fedw;
        } else {
            qErrnoWarning("%s: CreateFontFace failed", __FUNCTION__);
        }
    }

    if (directWriteFont != 0)
        directWriteFont->Release();
#endif

    if(script == QUnicodeTables::Common
       && !(request.styleStrategy & QFont::NoFontMerging)) {
       QFontDatabase db;
       if (!db.writingSystems(request.family).contains(QFontDatabase::Symbol)) {
           if(!tryFonts) {
               LANGID lid = GetUserDefaultLangID();
               switch( lid&0xff ) {
               case LANG_CHINESE: // Chinese (Taiwan)
                   if ( lid == 0x0804 ) // Taiwan
                       tryFonts = ch_TW_tryFonts;
                   else
                       tryFonts = ch_CN_tryFonts;
                   break;
               case LANG_JAPANESE:
                   tryFonts = jp_tryFonts;
                   break;
               case LANG_KOREAN:
                   tryFonts = kr_tryFonts;
                   break;
               default:
                   tryFonts = other_tryFonts;
                   break;
               }
           }
           QStringList fm = QFontDatabase().families();
           QStringList list = family_list;
           const char **tf = tryFonts;
           while(tf && *tf) {
               if(fm.contains(QLatin1String(*tf)))
                   list << QLatin1String(*tf);
               ++tf;
           }
           QFontEngine *mfe = new QWindowsMultiFontEngine(fe, list);
           mfe->setObjectName(QStringLiteral("QWindowsMultiFontEngine_") + request.family);
           mfe->fontDef = fe->fontDef;
           fe = mfe;
       }
    }
    return fe;
}

static inline int verticalDPI()
{
    return GetDeviceCaps(QWindowsContext::instance()->displayContext(), LOGPIXELSY);
}

QFont QWindowsFontDatabase::defaultFont() const
{
    LOGFONT lf;
    GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(lf), &lf);
    QFont systemFont =  QWindowsFontDatabase::LOGFONT_to_QFont(lf);
    // "MS Shell Dlg 2" is the correct system font >= Win2k
    if (systemFont.family() == QStringLiteral("MS Shell Dlg"))
        systemFont.setFamily(QStringLiteral("MS Shell Dlg 2"));
    if (QWindowsContext::verboseFonts)
        qDebug() << __FUNCTION__ << systemFont;
    return systemFont;
}

QHash<QByteArray, QFont> QWindowsFontDatabase::defaultFonts() const
{
    QHash<QByteArray, QFont> result;
    NONCLIENTMETRICS ncm;
    ncm.cbSize = FIELD_OFFSET(NONCLIENTMETRICS, lfMessageFont) + sizeof(LOGFONT);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize , &ncm, 0);

    const int verticalRes = verticalDPI();

    const QFont menuFont = LOGFONT_to_QFont(ncm.lfMenuFont, verticalRes);
    const QFont messageFont = LOGFONT_to_QFont(ncm.lfMessageFont, verticalRes);
    const QFont statusFont = LOGFONT_to_QFont(ncm.lfStatusFont, verticalRes);
    const QFont titleFont = LOGFONT_to_QFont(ncm.lfCaptionFont, verticalRes);

    LOGFONT lfIconTitleFont;
    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfIconTitleFont), &lfIconTitleFont, 0);
    const QFont iconTitleFont = LOGFONT_to_QFont(lfIconTitleFont, verticalRes);

    result.insert(QByteArray("QMenu"), menuFont);
    result.insert(QByteArray("QMenuBar"), menuFont);
    result.insert(QByteArray("QMessageBox"), messageFont);
    result.insert(QByteArray("QTipLabel"), statusFont);
    result.insert(QByteArray("QStatusBar"), statusFont);
    result.insert(QByteArray("Q3TitleBar"), titleFont);
    result.insert(QByteArray("QWorkspaceTitleBar"), titleFont);
    result.insert(QByteArray("QAbstractItemView"), iconTitleFont);
    result.insert(QByteArray("QDockWidgetTitle"), iconTitleFont);
    if (QWindowsContext::verboseFonts) {
        typedef QHash<QByteArray, QFont>::const_iterator CIT;
        QDebug nsp = qDebug().nospace();
        nsp << __FUNCTION__ << " DPI=" << verticalRes << "\n";
        const CIT cend = result.constEnd();
        for (CIT it = result.constBegin(); it != cend; ++it)
            nsp << it.key() << ' ' << it.value() << '\n';
    }
    return result;
}

QFont QWindowsFontDatabase::LOGFONT_to_QFont(const LOGFONT& logFont, int verticalDPI_In)
{
    if (verticalDPI_In <= 0)
        verticalDPI_In = verticalDPI();
    QFont qFont(QString::fromWCharArray(logFont.lfFaceName));
    qFont.setItalic(logFont.lfItalic);
    if (logFont.lfWeight != FW_DONTCARE)
        qFont.setWeight(weightFromInteger(logFont.lfWeight));
    const qreal logFontHeight = qAbs(logFont.lfHeight);
    qFont.setPointSizeF(logFontHeight * 72.0 / qreal(verticalDPI_In));
    qFont.setUnderline(false);
    qFont.setOverline(false);
    qFont.setStrikeOut(false);
    return qFont;
}

QT_END_NAMESPACE
