/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QOPENGLFRAMEBUFFEROBJECT_H
#define QOPENGLFRAMEBUFFEROBJECT_H

#include <QtGui/qopengl.h>
#include <QtGui/qpaintdevice.h>

#include <QtCore/qscopedpointer.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QOpenGLFramebufferObjectPrivate;
class QOpenGLFramebufferObjectFormat;

class Q_GUI_EXPORT QOpenGLFramebufferObject
{
    Q_DECLARE_PRIVATE(QOpenGLFramebufferObject)
public:
    enum Attachment {
        NoAttachment,
        CombinedDepthStencil,
        Depth
    };

    QOpenGLFramebufferObject(const QSize &size, GLenum target = GL_TEXTURE_2D);
    QOpenGLFramebufferObject(int width, int height, GLenum target = GL_TEXTURE_2D);
#if !defined(QT_OPENGL_ES) || defined(Q_QDOC)
    QOpenGLFramebufferObject(const QSize &size, Attachment attachment,
                         GLenum target = GL_TEXTURE_2D, GLenum internal_format = GL_RGBA8);
    QOpenGLFramebufferObject(int width, int height, Attachment attachment,
                         GLenum target = GL_TEXTURE_2D, GLenum internal_format = GL_RGBA8);
#else
    QOpenGLFramebufferObject(const QSize &size, Attachment attachment,
                         GLenum target = GL_TEXTURE_2D, GLenum internal_format = GL_RGBA);
    QOpenGLFramebufferObject(int width, int height, Attachment attachment,
                         GLenum target = GL_TEXTURE_2D, GLenum internal_format = GL_RGBA);
#endif

    QOpenGLFramebufferObject(const QSize &size, const QOpenGLFramebufferObjectFormat &format);
    QOpenGLFramebufferObject(int width, int height, const QOpenGLFramebufferObjectFormat &format);

    virtual ~QOpenGLFramebufferObject();

    QOpenGLFramebufferObjectFormat format() const;

    bool isValid() const;
    bool isBound() const;
    bool bind();
    bool release();

    int width() const { return size().width(); }
    int height() const { return size().width(); }

    GLuint texture() const;
    QSize size() const;
    QImage toImage() const;
    Attachment attachment() const;

    QPaintEngine *paintEngine() const;
    GLuint handle() const;

    static bool bindDefault();

    static bool hasOpenGLFramebufferObjects();

    static bool hasOpenGLFramebufferBlit();
    static void blitFramebuffer(QOpenGLFramebufferObject *target, const QRect &targetRect,
                                QOpenGLFramebufferObject *source, const QRect &sourceRect,
                                GLbitfield buffers = GL_COLOR_BUFFER_BIT,
                                GLenum filter = GL_NEAREST);

private:
    Q_DISABLE_COPY(QOpenGLFramebufferObject)
    QScopedPointer<QOpenGLFramebufferObjectPrivate> d_ptr;
    friend class QOpenGLPaintDevice;
    friend class QOpenGLFBOGLPaintDevice;
};

class QOpenGLFramebufferObjectFormatPrivate;
class Q_GUI_EXPORT QOpenGLFramebufferObjectFormat
{
public:
    QOpenGLFramebufferObjectFormat();
    QOpenGLFramebufferObjectFormat(const QOpenGLFramebufferObjectFormat &other);
    QOpenGLFramebufferObjectFormat &operator=(const QOpenGLFramebufferObjectFormat &other);
    ~QOpenGLFramebufferObjectFormat();

    void setSamples(int samples);
    int samples() const;

    void setMipmap(bool enabled);
    bool mipmap() const;

    void setAttachment(QOpenGLFramebufferObject::Attachment attachment);
    QOpenGLFramebufferObject::Attachment attachment() const;

    void setTextureTarget(GLenum target);
    GLenum textureTarget() const;

    void setInternalTextureFormat(GLenum internalTextureFormat);
    GLenum internalTextureFormat() const;

    bool operator==(const QOpenGLFramebufferObjectFormat& other) const;
    bool operator!=(const QOpenGLFramebufferObjectFormat& other) const;

private:
    QOpenGLFramebufferObjectFormatPrivate *d;

    void detach();
};

QT_END_NAMESPACE

QT_END_HEADER
#endif // QOPENGLFRAMEBUFFEROBJECT_H
