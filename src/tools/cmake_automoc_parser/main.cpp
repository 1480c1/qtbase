/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qglobal.h>

#include <cstdio>
#include <cstdlib>

#include <qfile.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qdir.h>
#include <qstring.h>
#include <qhash.h>
#include <qvector.h>
#include <qstack.h>
#include <qdebug.h>
#include <qset.h>
#include <qmap.h>
#include <qcoreapplication.h>
#include <qcommandlineoption.h>
#include <qcommandlineparser.h>

QT_BEGIN_NAMESPACE

using AutoGenHeaderMap = QMap<QString, QString>;
using AutoGenSourcesList = QVector<QString>;

static bool readAutogenInfoJson(AutoGenHeaderMap &headers, AutoGenSourcesList &sources,
                                QStringList &headerExts, const QString &autoGenInfoJsonPath)
{
    QFile file(autoGenInfoJsonPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fprintf(stderr, "Could not open: %s\n", qPrintable(autoGenInfoJsonPath));
        return false;
    }

    const QByteArray contents = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(contents, &error);

    if (error.error != QJsonParseError::NoError) {
        fprintf(stderr, "Failed to parse json file: %s\n", qPrintable(autoGenInfoJsonPath));
        return false;
    }

    QJsonObject rootObject = doc.object();
    QJsonValue headersValue = rootObject.value(QLatin1String("HEADERS"));
    QJsonValue sourcesValue = rootObject.value(QLatin1String("SOURCES"));
    QJsonValue headerExtValue = rootObject.value(QLatin1String("HEADER_EXTENSIONS"));

    if (!headersValue.isArray() || !sourcesValue.isArray() || !headerExtValue.isArray()) {
        fprintf(stderr,
                "%s layout does not match the expected layout. This most likely means that file "
                "format changed or this file is not a product of CMake's AutoGen process.\n",
                qPrintable(autoGenInfoJsonPath));
        return false;
    }

    QJsonArray headersArray = headersValue.toArray();
    QJsonArray sourcesArray = sourcesValue.toArray();
    QJsonArray headerExtArray = headerExtValue.toArray();

    for (const auto &value : headersArray) {
        QJsonArray entry_array = value.toArray();
        if (entry_array.size() > 2) {
            // Array[0] : header path
            // Array[2] : Location of the generated moc file for this header
            // if no source file includes it
            headers.insert(entry_array[0].toString(), entry_array[2].toString());
        }
    }

    sources.reserve(sourcesArray.size());
    for (const auto &value : sourcesArray) {
        QJsonArray entry_array = value.toArray();
        if (entry_array.size() > 1) {
            sources.push_back(entry_array[0].toString());
        }
    }

    headerExts.reserve(headerExtArray.size());
    for (const auto &value : headerExtArray) {
        headerExts.push_back(value.toString());
    }

    return true;
}

struct ParseCacheEntry
{
    QStringList mocFiles;
    QStringList mocIncludes;
};

using ParseCacheMap = QMap<QString, ParseCacheEntry>;

static bool readParseCache(ParseCacheMap &entries, const QString &parseCacheFilePath)
{

    QFile file(parseCacheFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fprintf(stderr, "Could not open: %s\n", qPrintable(parseCacheFilePath));
        return false;
    }

    QString source;
    QStringList mocEntries;
    QStringList mocIncludes;

    // File format
    // ....
    // header/source path N
    //  mmc:Q_OBJECT| mcc:Q_GADGET # This file has been mocked
    //  miu:moc_....cpp # Path of the moc.cpp file generated for the above file
    //  relative to TARGET_BINARY_DIR/TARGET_autgen/include directory. Not
    //  present for headers.
    //  mid: ....moc # Path of .moc file generated for the above file relative
    //  to TARGET_BINARY_DIR/TARGET_autogen/include directory.
    //  uic: UI related info, ignored
    //  mdp: Moc dependencies, ignored
    //  udp: UI dependencies, ignored
    // header/source path N + 1
    // ....

    QTextStream textStream(&file);
    const QString mmcKey = QString(QLatin1String(" mmc:"));
    const QString miuKey = QString(QLatin1String(" miu:"));
    const QString uicKey = QString(QLatin1String(" uic:"));
    const QString midKey = QString(QLatin1String(" mid:"));
    const QString mdpKey = QString(QLatin1String(" mdp:"));
    const QString udpKey = QString(QLatin1String(" udp:"));
    QString line;
    bool mmc_key_found = false;
    while (textStream.readLineInto(&line)) {
        if (!line.startsWith(QLatin1Char(' '))) {
            if (!mocEntries.isEmpty() || mmc_key_found || !mocIncludes.isEmpty()) {
                entries.insert(source,
                               ParseCacheEntry { std::move(mocEntries), std::move(mocIncludes) });
                source.clear();
                mmc_key_found = false;
            }
            source = line;
        } else if (line.startsWith(mmcKey)) {
            mmc_key_found = true;
        } else if (line.startsWith(miuKey)) {
            mocIncludes.push_back(line.right(line.size() - miuKey.size()));
        } else if (line.startsWith(midKey)) {
            mocEntries.push_back(line.right(line.size() - midKey.size()));
        } else if (line.startsWith(uicKey) || line.startsWith(mdpKey) || line.startsWith(udpKey)) {
            // nothing to do ignore
            continue;
        } else {
            fprintf(stderr, "Unhandled line entry \"%s\" in %s\n", qPrintable(line),
                    qPrintable(parseCacheFilePath));
            return false;
        }
    }

    // Check if last entry has any data left to processed
    if (!mocEntries.isEmpty() || !mocIncludes.isEmpty() || mmc_key_found) {
        entries.insert(source, ParseCacheEntry { std::move(mocEntries), std::move(mocIncludes) });
    }

    file.close();
    return true;
}

static bool readJsonFiles(QVector<QString> &entries, const QString &filePath)
{

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fprintf(stderr, "Could not open: %s\n", qPrintable(filePath));
        return false;
    }

    QTextStream textStream(&file);
    QString line;
    while (textStream.readLineInto(&line)) {
        entries.push_back(line);
    }
    file.close();
    return true;
}

static bool writeJsonFiles(const QVector<QString> &fileList, const QString &fileListFilePath)
{
    QFile file(fileListFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        fprintf(stderr, "Could not open: %s\n", qPrintable(fileListFilePath));
        return false;
    }

    QTextStream textStream(&file);
    for (const auto &file : fileList) {
        textStream << file << Qt::endl;
    }

    file.close();
    return true;
}

int main(int argc, char **argv)
{

    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Qt CMake Autogen parser tool"));

    parser.addHelpOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    QCommandLineOption outputFileOption(QStringLiteral("output-file-path"));
    outputFileOption.setDescription(
            QStringLiteral("Output file where the meta type file list will be written."));
    outputFileOption.setValueName(QStringLiteral("output file"));
    parser.addOption(outputFileOption);

    QCommandLineOption cmakeAutogenCacheFileOption(QStringLiteral("cmake-autogen-cache-file"));
    cmakeAutogenCacheFileOption.setDescription(
            QStringLiteral("Location of the CMake AutoGen ParseCache.txt file."));
    cmakeAutogenCacheFileOption.setValueName(QStringLiteral("CMake AutoGen ParseCache.txt file"));
    parser.addOption(cmakeAutogenCacheFileOption);

    QCommandLineOption cmakeAutogenInfoFileOption(QStringLiteral("cmake-autogen-info-file"));
    cmakeAutogenInfoFileOption.setDescription(
            QStringLiteral("Location of the CMake AutoGen AutogenInfo.json file."));
    cmakeAutogenInfoFileOption.setValueName(QStringLiteral("CMake AutoGen AutogenInfo.json file"));
    parser.addOption(cmakeAutogenInfoFileOption);

    QCommandLineOption cmakeAutogenIncludeDirOption(
            QStringLiteral("cmake-autogen-include-dir-path"));
    cmakeAutogenIncludeDirOption.setDescription(
            QStringLiteral("Location of the CMake AutoGen include directory."));
    cmakeAutogenIncludeDirOption.setValueName(QStringLiteral("CMake AutoGen include directory"));
    parser.addOption(cmakeAutogenIncludeDirOption);

    QCommandLineOption isMultiConfigOption(
            QStringLiteral("cmake-multi-config"));
    isMultiConfigOption.setDescription(
            QStringLiteral("Set this option when using CMake with a multi-config generator"));
    parser.addOption(isMultiConfigOption);

    QStringList arguments = QCoreApplication::arguments();
    parser.process(arguments);

    if (!parser.isSet(outputFileOption) || !parser.isSet(cmakeAutogenInfoFileOption)
        || !parser.isSet(cmakeAutogenCacheFileOption)
        || !parser.isSet(cmakeAutogenIncludeDirOption)) {
        parser.showHelp(1);
        return EXIT_FAILURE;
    }

    // Read source files from AutogenInfo.json
    AutoGenHeaderMap autoGenHeaders;
    AutoGenSourcesList autoGenSources;
    QStringList headerExtList;
    if (!readAutogenInfoJson(autoGenHeaders, autoGenSources, headerExtList,
                             parser.value(cmakeAutogenInfoFileOption))) {
        return EXIT_FAILURE;
    }

    ParseCacheMap parseCacheEntries;
    if (!readParseCache(parseCacheEntries, parser.value(cmakeAutogenCacheFileOption))) {
        return EXIT_FAILURE;
    }

    const QString cmakeIncludeDir = parser.value(cmakeAutogenIncludeDirOption);

    // Algorithm description
    // 1) For each source from the AutoGenSources list check if there is a parse
    // cache entry.
    // 1a) If an entry was wound there exists an moc_...cpp file somewhere.
    // Remove the header file from the AutoGenHeader files
    // 1b) For every matched source entry, check the moc includes as it is
    // possible for a source file to include moc files from other headers.
    // Remove the header from AutoGenHeaders
    // 2) For every remaining header in AutoGenHeaders, check if there is an
    // entry for it in the parse cache. Use the value for the location of the
    // moc.json file

    QVector<QString> jsonFileList;
    QDir dir(cmakeIncludeDir);
    jsonFileList.reserve(autoGenSources.size());

    // 1) Process sources
    for (const auto &source : autoGenSources) {
        auto it = parseCacheEntries.find(source);
        if (it == parseCacheEntries.end()) {
            continue;
        }

        const QFileInfo fileInfo(source);
        const QString base = fileInfo.path() + fileInfo.completeBaseName();
        // 1a) erase header
        for (const auto &ext : headerExtList) {
            const QString headerPath = base + QLatin1Char('.') + ext;
            auto it = autoGenHeaders.find(headerPath);
            if (it != autoGenHeaders.end()) {
                autoGenHeaders.erase(it);
                break;
            }
        }
        // Add extra moc files
        for (const auto &mocFile : it.value().mocFiles) {
            jsonFileList.push_back(dir.filePath(mocFile) + QLatin1String(".json"));
        }
        // Add main moc files
        for (const auto &mocFile : it.value().mocIncludes) {
            jsonFileList.push_back(dir.filePath(mocFile) + QLatin1String(".json"));
            // 1b) Locate this header and delete it
            constexpr int mocKeyLen = 4; // length of "moc_"
            const QString headerBaseName =
                    QFileInfo(mocFile.right(mocFile.size() - mocKeyLen)).completeBaseName();
            bool breakFree = false;
            for (auto &ext : headerExtList) {
                const QString headerSuffix = headerBaseName + QLatin1Char('.') + ext;
                for (auto it = autoGenHeaders.begin(); it != autoGenHeaders.end(); ++it) {
                    if (it.key().endsWith(headerSuffix)
                        && QFileInfo(it.key()).completeBaseName() == headerBaseName) {
                        autoGenHeaders.erase(it);
                        breakFree = true;
                        break;
                    }
                }
                if (breakFree) {
                    break;
                }
            }
        }
    }

    // 2) Process headers
    const bool isMultiConfig = parser.isSet(isMultiConfigOption);
    for (auto mapIt = autoGenHeaders.begin(); mapIt != autoGenHeaders.end(); ++mapIt) {
        auto it = parseCacheEntries.find(mapIt.key());
        if (it == parseCacheEntries.end()) {
            continue;
        }
        const QString pathPrefix = !isMultiConfig
            ? QStringLiteral("../")
            : QString();
        const QString jsonPath =
                dir.filePath(pathPrefix + mapIt.value() + QLatin1String(".json"));
        jsonFileList.push_back(jsonPath);
    }

    // Sort for consistent checks across runs
    jsonFileList.sort();

    // Read Previous file list (if any)
    const QString fileListFilePath = parser.value(outputFileOption);
    QVector<QString> previousList;
    QFile prev_file(fileListFilePath);

    // Only try to open file if it exists to avoid error messages
    if (prev_file.exists()) {
        (void)readJsonFiles(previousList, fileListFilePath);
    }

    if (previousList != jsonFileList || !QFile(fileListFilePath).exists()) {
        if (!writeJsonFiles(jsonFileList, fileListFilePath)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

QT_END_NAMESPACE
