/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QSURFACEFORMAT_H
#define QSURFACEFORMAT_H

#include <qglobal.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QOpenGLContext;
class QSurfaceFormatPrivate;

class Q_GUI_EXPORT QSurfaceFormat
{
public:
    enum FormatOption {
        StereoBuffers            = 0x0001,
        DebugContext             = 0x0002,
        DeprecatedFunctions      = 0x0004
    };
    Q_DECLARE_FLAGS(FormatOptions, FormatOption)

    enum SwapBehavior {
        DefaultSwapBehavior,
        SingleBuffer,
        DoubleBuffer,
        TripleBuffer
    };

    enum OpenGLContextProfile {
        NoProfile,
        CoreProfile,
        CompatibilityProfile
    };

    QSurfaceFormat();
    QSurfaceFormat(FormatOptions options);
    QSurfaceFormat(const QSurfaceFormat &other);
    QSurfaceFormat &operator=(const QSurfaceFormat &other);
    ~QSurfaceFormat();

    void setDepthBufferSize(int size);
    int depthBufferSize() const;

    void setStencilBufferSize(int size);
    int stencilBufferSize() const;

    void setRedBufferSize(int size);
    int redBufferSize() const;
    void setGreenBufferSize(int size);
    int greenBufferSize() const;
    void setBlueBufferSize(int size);
    int blueBufferSize() const;
    void setAlphaBufferSize(int size);
    int alphaBufferSize() const;

    void setSamples(int numSamples);
    int samples() const;

    void setSwapBehavior(SwapBehavior behavior);
    SwapBehavior swapBehavior() const;

    bool hasAlpha() const;

    void setProfile(OpenGLContextProfile profile);
    OpenGLContextProfile profile() const;

    void setMajorVersion(int majorVersion);
    int majorVersion() const;

    void setMinorVersion(int minorVersion);
    int minorVersion() const;

    bool stereo() const;
    void setStereo(bool enable);

    void setOption(QSurfaceFormat::FormatOptions opt);
    bool testOption(QSurfaceFormat::FormatOptions opt) const;

private:
    QSurfaceFormatPrivate *d;

    void detach();

    friend Q_GUI_EXPORT bool operator==(const QSurfaceFormat&, const QSurfaceFormat&);
    friend Q_GUI_EXPORT bool operator!=(const QSurfaceFormat&, const QSurfaceFormat&);
#ifndef QT_NO_DEBUG_STREAM
    friend Q_GUI_EXPORT QDebug operator<<(QDebug, const QSurfaceFormat &);
#endif
};

Q_GUI_EXPORT bool operator==(const QSurfaceFormat&, const QSurfaceFormat&);
Q_GUI_EXPORT bool operator!=(const QSurfaceFormat&, const QSurfaceFormat&);

#ifndef QT_NO_DEBUG_STREAM
Q_GUI_EXPORT QDebug operator<<(QDebug, const QSurfaceFormat &);
#endif

Q_DECLARE_OPERATORS_FOR_FLAGS(QSurfaceFormat::FormatOptions)

inline bool QSurfaceFormat::stereo() const
{
    return testOption(QSurfaceFormat::StereoBuffers);
}

QT_END_NAMESPACE

QT_END_HEADER

#endif //QSURFACEFORMAT_H
