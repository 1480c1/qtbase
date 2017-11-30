/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2015 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author David Faure <david.faure@kdab.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QMIMEPROVIDER_P_H
#define QMIMEPROVIDER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qmimedatabase_p.h"

#ifndef QT_NO_MIMETYPE

#include "qmimeglobpattern_p.h"
#include <QtCore/qdatetime.h>
#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE

class QMimeMagicRuleMatcher;

class QMimeProviderBase
{
public:
    QMimeProviderBase(QMimeDatabasePrivate *db);
    virtual ~QMimeProviderBase() {}

    virtual bool isValid() = 0;
    virtual QMimeType mimeTypeForName(const QString &name) = 0;
    virtual QMimeGlobMatchResult findByFileName(const QString &fileName) = 0;
    virtual QStringList parents(const QString &mime) = 0;
    virtual QString resolveAlias(const QString &name) = 0;
    virtual QStringList listAliases(const QString &name) = 0;
    virtual QMimeType findByMagic(const QByteArray &data, int *accuracyPtr) = 0;
    virtual QList<QMimeType> allMimeTypes() = 0;
    virtual void loadMimeTypePrivate(QMimeTypePrivate &) {}
    virtual void loadIcon(QMimeTypePrivate &) {}
    virtual void loadGenericIcon(QMimeTypePrivate &) {}
    virtual void ensureLoaded() {}

    QMimeDatabasePrivate *m_db;
};

/*
   Parses the files 'mime.cache' and 'types' on demand
 */
class QMimeBinaryProvider : public QMimeProviderBase
{
public:
    QMimeBinaryProvider(QMimeDatabasePrivate *db);
    virtual ~QMimeBinaryProvider();

    virtual bool isValid() override;
    virtual QMimeType mimeTypeForName(const QString &name) override;
    virtual QMimeGlobMatchResult findByFileName(const QString &fileName) override;
    virtual QStringList parents(const QString &mime) override;
    virtual QString resolveAlias(const QString &name) override;
    virtual QStringList listAliases(const QString &name) override;
    virtual QMimeType findByMagic(const QByteArray &data, int *accuracyPtr) override;
    virtual QList<QMimeType> allMimeTypes() override;
    virtual void loadMimeTypePrivate(QMimeTypePrivate &) override;
    virtual void loadIcon(QMimeTypePrivate &) override;
    virtual void loadGenericIcon(QMimeTypePrivate &) override;
    void ensureLoaded() override;

private:
    struct CacheFile;

    void matchGlobList(QMimeGlobMatchResult &result, CacheFile *cacheFile, int offset, const QString &fileName);
    bool matchSuffixTree(QMimeGlobMatchResult &result, CacheFile *cacheFile, int numEntries, int firstOffset, const QString &fileName, int charPos, bool caseSensitiveCheck);
    bool matchMagicRule(CacheFile *cacheFile, int numMatchlets, int firstOffset, const QByteArray &data);
    QLatin1String iconForMime(CacheFile *cacheFile, int posListOffset, const QByteArray &inputMime);
    void loadMimeTypeList();

    class CacheFileList : public QList<CacheFile *>
    {
    public:
        CacheFile *findCacheFile(const QString &fileName) const;
        bool checkCacheChanged();
    };
    CacheFileList m_cacheFiles;
    QStringList m_cacheFileNames;
    QSet<QString> m_mimetypeNames;
    bool m_mimetypeListLoaded;
};

/*
   Parses the raw XML files (slower)
 */
class QMimeXMLProvider : public QMimeProviderBase
{
public:
    QMimeXMLProvider(QMimeDatabasePrivate *db);
    ~QMimeXMLProvider();

    virtual bool isValid() override;
    virtual QMimeType mimeTypeForName(const QString &name) override;
    virtual QMimeGlobMatchResult findByFileName(const QString &fileName) override;
    virtual QStringList parents(const QString &mime) override;
    virtual QString resolveAlias(const QString &name) override;
    virtual QStringList listAliases(const QString &name) override;
    virtual QMimeType findByMagic(const QByteArray &data, int *accuracyPtr) override;
    virtual QList<QMimeType> allMimeTypes() override;
    void ensureLoaded() override;

    bool load(const QString &fileName, QString *errorMessage);

    // Called by the mimetype xml parser
    void addMimeType(const QMimeType &mt);
    void addGlobPattern(const QMimeGlobPattern &glob);
    void addParent(const QString &child, const QString &parent);
    void addAlias(const QString &alias, const QString &name);
    void addMagicMatcher(const QMimeMagicRuleMatcher &matcher);

private:
    void load(const QString &fileName);

    typedef QHash<QString, QMimeType> NameMimeTypeMap;
    NameMimeTypeMap m_nameMimeTypeMap;

    typedef QHash<QString, QString> AliasHash;
    AliasHash m_aliases;

    typedef QHash<QString, QStringList> ParentsHash;
    ParentsHash m_parents;
    QMimeAllGlobPatterns m_mimeTypeGlobs;

    QList<QMimeMagicRuleMatcher> m_magicMatchers;
    QStringList m_allFiles;
};

QT_END_NAMESPACE

#endif // QT_NO_MIMETYPE
#endif // QMIMEPROVIDER_P_H
