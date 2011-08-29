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

#include "qopenglframebufferobject.h"
#include "qopenglframebufferobject_p.h"

#include <qdebug.h>
#include <private/qopengl_p.h>
#include <private/qopenglcontext_p.h>
#include <private/qopenglextensions_p.h>
#include <private/qfont_p.h>
#include <private/qpaintengineex_opengl2_p.h>

#include <qwindow.h>
#include <qlibrary.h>
#include <qimage.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_DEBUG
#define QT_RESET_GLERROR()                                \
{                                                         \
    while (glGetError() != GL_NO_ERROR) {}                \
}
#define QT_CHECK_GLERROR()                                \
{                                                         \
    GLenum err = glGetError();                            \
    if (err != GL_NO_ERROR) {                             \
        qDebug("[%s line %d] GL Error: %d",               \
               __FILE__, __LINE__, (int)err);             \
    }                                                     \
}
#else
#define QT_RESET_GLERROR() {}
#define QT_CHECK_GLERROR() {}
#endif

/*!
    \class QOpenGLFramebufferObjectFormat
    \brief The QOpenGLFramebufferObjectFormat class specifies the format of an OpenGL
    framebuffer object.

    \since 4.6

    \ingroup painting-3D

    A framebuffer object has several characteristics:
    \list
    \i \link setSamples() Number of samples per pixels.\endlink
    \i \link setAttachment() Depth and/or stencil attachments.\endlink
    \i \link setTextureTarget() Texture target.\endlink
    \i \link setInternalTextureFormat() Internal texture format.\endlink
    \endlist

    Note that the desired attachments or number of samples per pixels might not
    be supported by the hardware driver. Call QOpenGLFramebufferObject::format()
    after creating a QOpenGLFramebufferObject to find the exact format that was
    used to create the frame buffer object.

    \sa QOpenGLFramebufferObject
*/

/*!
    \internal
*/
void QOpenGLFramebufferObjectFormat::detach()
{
    if (d->ref != 1) {
        QOpenGLFramebufferObjectFormatPrivate *newd
            = new QOpenGLFramebufferObjectFormatPrivate(d);
        if (!d->ref.deref())
            delete d;
        d = newd;
    }
}

/*!
    Creates a QOpenGLFramebufferObjectFormat object for specifying
    the format of an OpenGL framebuffer object.

    By default the format specifies a non-multisample framebuffer object with no
    attachments, texture target \c GL_TEXTURE_2D, and internal format \c GL_RGBA8.
    On OpenGL/ES systems, the default internal format is \c GL_RGBA.

    \sa samples(), attachment(), internalTextureFormat()
*/

QOpenGLFramebufferObjectFormat::QOpenGLFramebufferObjectFormat()
{
    d = new QOpenGLFramebufferObjectFormatPrivate;
}

/*!
    Constructs a copy of \a other.
*/

QOpenGLFramebufferObjectFormat::QOpenGLFramebufferObjectFormat(const QOpenGLFramebufferObjectFormat &other)
{
    d = other.d;
    d->ref.ref();
}

/*!
    Assigns \a other to this object.
*/

QOpenGLFramebufferObjectFormat &QOpenGLFramebufferObjectFormat::operator=(const QOpenGLFramebufferObjectFormat &other)
{
    if (d != other.d) {
        other.d->ref.ref();
        if (!d->ref.deref())
            delete d;
        d = other.d;
    }
    return *this;
}

/*!
    Destroys the QOpenGLFramebufferObjectFormat.
*/
QOpenGLFramebufferObjectFormat::~QOpenGLFramebufferObjectFormat()
{
    if (!d->ref.deref())
        delete d;
}

/*!
    Sets the number of samples per pixel for a multisample framebuffer object
    to \a samples.  The default sample count of 0 represents a regular
    non-multisample framebuffer object.

    If the desired amount of samples per pixel is not supported by the hardware
    then the maximum number of samples per pixel will be used. Note that
    multisample framebuffer objects can not be bound as textures. Also, the
    \c{GL_EXT_framebuffer_multisample} extension is required to create a
    framebuffer with more than one sample per pixel.

    \sa samples()
*/
void QOpenGLFramebufferObjectFormat::setSamples(int samples)
{
    detach();
    d->samples = samples;
}

/*!
    Returns the number of samples per pixel if a framebuffer object
    is a multisample framebuffer object. Otherwise, returns 0.
    The default value is 0.

    \sa setSamples()
*/
int QOpenGLFramebufferObjectFormat::samples() const
{
    return d->samples;
}

/*!
    \since 4.8

    Enables mipmapping if \a enabled is true; otherwise disables it.

    Mipmapping is disabled by default.

    If mipmapping is enabled, additional memory will be allocated for
    the mipmap levels. The mipmap levels can be updated by binding the
    texture and calling glGenerateMipmap(). Mipmapping cannot be enabled
    for multisampled framebuffer objects.

    \sa mipmap(), QOpenGLFramebufferObject::texture()
*/
void QOpenGLFramebufferObjectFormat::setMipmap(bool enabled)
{
    detach();
    d->mipmap = enabled;
}

/*!
    \since 4.8

    Returns true if mipmapping is enabled.

    \sa setMipmap()
*/
bool QOpenGLFramebufferObjectFormat::mipmap() const
{
    return d->mipmap;
}

/*!
    Sets the attachment configuration of a framebuffer object to \a attachment.

    \sa attachment()
*/
void QOpenGLFramebufferObjectFormat::setAttachment(QOpenGLFramebufferObject::Attachment attachment)
{
    detach();
    d->attachment = attachment;
}

/*!
    Returns the configuration of the depth and stencil buffers attached to
    a framebuffer object.  The default is QOpenGLFramebufferObject::NoAttachment.

    \sa setAttachment()
*/
QOpenGLFramebufferObject::Attachment QOpenGLFramebufferObjectFormat::attachment() const
{
    return d->attachment;
}

/*!
    Sets the texture target of the texture attached to a framebuffer object to
    \a target. Ignored for multisample framebuffer objects.

    \sa textureTarget(), samples()
*/
void QOpenGLFramebufferObjectFormat::setTextureTarget(GLenum target)
{
    detach();
    d->target = target;
}

/*!
    Returns the texture target of the texture attached to a framebuffer object.
    Ignored for multisample framebuffer objects.  The default is
    \c GL_TEXTURE_2D.

    \sa setTextureTarget(), samples()
*/
GLenum QOpenGLFramebufferObjectFormat::textureTarget() const
{
    return d->target;
}

/*!
    Sets the internal format of a framebuffer object's texture or
    multisample framebuffer object's color buffer to
    \a internalTextureFormat.

    \sa internalTextureFormat()
*/
void QOpenGLFramebufferObjectFormat::setInternalTextureFormat(GLenum internalTextureFormat)
{
    detach();
    d->internal_format = internalTextureFormat;
}

/*!
    Returns the internal format of a framebuffer object's texture or
    multisample framebuffer object's color buffer.  The default is
    \c GL_RGBA8 on desktop OpenGL systems, and \c GL_RGBA on
    OpenGL/ES systems.

    \sa setInternalTextureFormat()
*/
GLenum QOpenGLFramebufferObjectFormat::internalTextureFormat() const
{
    return d->internal_format;
}

/*!
    Returns true if all the options of this framebuffer object format
    are the same as \a other; otherwise returns false.
*/
bool QOpenGLFramebufferObjectFormat::operator==(const QOpenGLFramebufferObjectFormat& other) const
{
    if (d == other.d)
        return true;
    else
        return d->equals(other.d);
}

/*!
    Returns false if all the options of this framebuffer object format
    are the same as \a other; otherwise returns true.
*/
bool QOpenGLFramebufferObjectFormat::operator!=(const QOpenGLFramebufferObjectFormat& other) const
{
    return !(*this == other);
}

void QOpenGLFBOGLPaintDevice::setFBO(QOpenGLFramebufferObject* f,
                                 QOpenGLFramebufferObject::Attachment attachment)
{
    fbo = f;
    m_thisFBO = fbo->d_func()->fbo(); // This shouldn't be needed

    // The context that the fbo was created in may not have depth
    // and stencil buffers, but the fbo itself might.
    fboFormat = QOpenGLContext::currentContext()->format();
    if (attachment == QOpenGLFramebufferObject::CombinedDepthStencil) {
        fboFormat.setDepthBufferSize(24);
        fboFormat.setStencilBufferSize(8);
    } else if (attachment == QOpenGLFramebufferObject::Depth) {
        fboFormat.setDepthBufferSize(24);
        fboFormat.setStencilBufferSize(0);
    } else {
        fboFormat.setDepthBufferSize(0);
        fboFormat.setStencilBufferSize(0);
    }

    GLenum format = f->format().internalTextureFormat();
    reqAlpha = (format != GL_RGB
#ifndef QT_OPENGL_ES
                && format != GL_RGB5 && format != GL_RGB8
#endif
    );
}

QOpenGLContextGroup *QOpenGLFBOGLPaintDevice::group() const
{
    QOpenGLSharedResourceGuard *fbo_guard =
        fbo->d_func()->fbo_guard;
    return fbo_guard ? fbo_guard->group() : 0;
}

bool QOpenGLFramebufferObjectPrivate::checkFramebufferStatus() const
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx)
        return false;   // Context no longer exists.
    GLenum status = ctx->functions()->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status) {
    case GL_NO_ERROR:
    case GL_FRAMEBUFFER_COMPLETE:
        return true;
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        qDebug("QOpenGLFramebufferObject: Unsupported framebuffer format.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        qDebug("QOpenGLFramebufferObject: Framebuffer incomplete attachment.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        qDebug("QOpenGLFramebufferObject: Framebuffer incomplete, missing attachment.");
        break;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT
    case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT:
        qDebug("QOpenGLFramebufferObject: Framebuffer incomplete, duplicate attachment.");
        break;
#endif
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        qDebug("QOpenGLFramebufferObject: Framebuffer incomplete, attached images must have same dimensions.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
        qDebug("QOpenGLFramebufferObject: Framebuffer incomplete, attached images must have same format.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        qDebug("QOpenGLFramebufferObject: Framebuffer incomplete, missing draw buffer.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        qDebug("QOpenGLFramebufferObject: Framebuffer incomplete, missing read buffer.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        qDebug("QOpenGLFramebufferObject: Framebuffer incomplete, attachments must have same number of samples per pixel.");
        break;
    default:
        qDebug() <<"QOpenGLFramebufferObject: An undefined error has occurred: "<< status;
        break;
    }
    return false;
}

namespace
{
    void freeFramebufferFunc(QOpenGLFunctions *funcs, GLuint id)
    {
        funcs->glDeleteFramebuffers(1, &id);
    }

    void freeRenderbufferFunc(QOpenGLFunctions *funcs, GLuint id)
    {
        funcs->glDeleteRenderbuffers(1, &id);
    }

    void freeTextureFunc(QOpenGLFunctions *, GLuint id)
    {
        glDeleteTextures(1, &id);
    }
}

void QOpenGLFramebufferObjectPrivate::init(QOpenGLFramebufferObject *q, const QSize &sz,
                                       QOpenGLFramebufferObject::Attachment attachment,
                                       GLenum texture_target, GLenum internal_format,
                                       GLint samples, bool mipmap)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();

    funcs.initializeGLFunctions();

    if (!funcs.hasOpenGLFeature(QOpenGLFunctions::Framebuffers))
        return;

    size = sz;
    target = texture_target;
    // texture dimensions

    QT_RESET_GLERROR(); // reset error state
    GLuint fbo = 0;

    funcs.glGenFramebuffers(1, &fbo);
    funcs.glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint texture = 0;
    GLuint color_buffer = 0;
    GLuint depth_buffer = 0;
    GLuint stencil_buffer = 0;

    QT_CHECK_GLERROR();
    // init texture
    if (samples == 0) {
        glGenTextures(1, &texture);
        glBindTexture(target, texture);
        glTexImage2D(target, 0, internal_format, size.width(), size.height(), 0,
                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        if (mipmap)
            funcs.glGenerateMipmap(GL_TEXTURE_2D);
#ifndef QT_OPENGL_ES
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
        glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
        funcs.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                target, texture, 0);

        QT_CHECK_GLERROR();
        valid = checkFramebufferStatus();
        glBindTexture(target, 0);

        color_buffer = 0;
    } else {
        mipmap = false;
        GLint maxSamples;
        glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

        samples = qBound(0, int(samples), int(maxSamples));

        funcs.glGenRenderbuffers(1, &color_buffer);
        funcs.glBindRenderbuffer(GL_RENDERBUFFER, color_buffer);
        if (funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample) && samples > 0) {
            funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                internal_format, size.width(), size.height());
        } else {
            samples = 0;
            funcs.glRenderbufferStorage(GL_RENDERBUFFER, internal_format,
                size.width(), size.height());
        }

        funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                             GL_RENDERBUFFER, color_buffer);

        QT_CHECK_GLERROR();
        valid = checkFramebufferStatus();

        if (valid)
            funcs.glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);
    }

    // In practice, a combined depth-stencil buffer is supported by all desktop platforms, while a
    // separate stencil buffer is not. On embedded devices however, a combined depth-stencil buffer
    // might not be supported while separate buffers are, according to QTBUG-12861.

    if (attachment == QOpenGLFramebufferObject::CombinedDepthStencil
        && funcs.hasOpenGLExtension(QOpenGLExtensions::PackedDepthStencil))
    {
        // depth and stencil buffer needs another extension
        funcs.glGenRenderbuffers(1, &depth_buffer);
        Q_ASSERT(!funcs.glIsRenderbuffer(depth_buffer));
        funcs.glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        Q_ASSERT(funcs.glIsRenderbuffer(depth_buffer));
        if (samples != 0 && funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample))
            funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                GL_DEPTH24_STENCIL8, size.width(), size.height());
        else
            funcs.glRenderbufferStorage(GL_RENDERBUFFER,
                GL_DEPTH24_STENCIL8, size.width(), size.height());

        stencil_buffer = depth_buffer;
        funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                     GL_RENDERBUFFER, depth_buffer);
        funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                     GL_RENDERBUFFER, stencil_buffer);

        valid = checkFramebufferStatus();
        if (!valid) {
            funcs.glDeleteRenderbuffers(1, &depth_buffer);
            stencil_buffer = depth_buffer = 0;
        }
    }

    if (depth_buffer == 0 && (attachment == QOpenGLFramebufferObject::CombinedDepthStencil
        || (attachment == QOpenGLFramebufferObject::Depth)))
    {
        funcs.glGenRenderbuffers(1, &depth_buffer);
        Q_ASSERT(!funcs.glIsRenderbuffer(depth_buffer));
        funcs.glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        Q_ASSERT(funcs.glIsRenderbuffer(depth_buffer));
        if (samples != 0 && funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample)) {
#ifdef QT_OPENGL_ES
            if (funcs.hasOpenGLExtension(QOpenGLExtensions::Depth24)) {
                funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                    GL_DEPTH_COMPONENT24, size.width(), size.height());
            } else {
                funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                    GL_DEPTH_COMPONENT16, size.width(), size.height());
            }
#else
            funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                GL_DEPTH_COMPONENT, size.width(), size.height());
#endif
        } else {
#ifdef QT_OPENGL_ES
            if (funcs.hasOpenGLExtension(QOpenGLExtensions::Depth24)) {
                funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                                        size.width(), size.height());
            } else {
                funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
                                        size.width(), size.height());
            }
#else
            funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.width(), size.height());
#endif
        }
        funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                     GL_RENDERBUFFER, depth_buffer);
        valid = checkFramebufferStatus();
        if (!valid) {
            funcs.glDeleteRenderbuffers(1, &depth_buffer);
            depth_buffer = 0;
        }
    }

    if (stencil_buffer == 0 && (attachment == QOpenGLFramebufferObject::CombinedDepthStencil)) {
        funcs.glGenRenderbuffers(1, &stencil_buffer);
        Q_ASSERT(!funcs.glIsRenderbuffer(stencil_buffer));
        funcs.glBindRenderbuffer(GL_RENDERBUFFER, stencil_buffer);
        Q_ASSERT(funcs.glIsRenderbuffer(stencil_buffer));
        if (samples != 0 && funcs.hasOpenGLExtension(QOpenGLExtensions::FramebufferMultisample)) {
#ifdef QT_OPENGL_ES
            funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                GL_STENCIL_INDEX8, size.width(), size.height());
#else
            funcs.glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                GL_STENCIL_INDEX, size.width(), size.height());
#endif
        } else {
#ifdef QT_OPENGL_ES
            funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8,
                                  size.width(), size.height());
#else
            funcs.glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX,
                                  size.width(), size.height());
#endif
        }
        funcs.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, stencil_buffer);
        valid = checkFramebufferStatus();
        if (!valid) {
            funcs.glDeleteRenderbuffers(1, &stencil_buffer);
            stencil_buffer = 0;
        }
    }

    // The FBO might have become valid after removing the depth or stencil buffer.
    valid = checkFramebufferStatus();

    if (depth_buffer && stencil_buffer) {
        fbo_attachment = QOpenGLFramebufferObject::CombinedDepthStencil;
    } else if (depth_buffer) {
        fbo_attachment = QOpenGLFramebufferObject::Depth;
    } else {
        fbo_attachment = QOpenGLFramebufferObject::NoAttachment;
    }

    funcs.glBindFramebuffer(GL_FRAMEBUFFER, ctx->d_func()->current_fbo);
    if (valid) {
        fbo_guard = new QOpenGLSharedResourceGuard(ctx, fbo, freeFramebufferFunc);
        if (color_buffer)
            color_buffer_guard = new QOpenGLSharedResourceGuard(ctx, color_buffer, freeRenderbufferFunc);
        else
            texture_guard = new QOpenGLSharedResourceGuard(ctx, texture, freeTextureFunc);
        if (depth_buffer)
            depth_buffer_guard = new QOpenGLSharedResourceGuard(ctx, depth_buffer, freeRenderbufferFunc);
        if (stencil_buffer) {
            if (stencil_buffer == depth_buffer)
                stencil_buffer_guard = depth_buffer_guard;
            else
                stencil_buffer_guard = new QOpenGLSharedResourceGuard(ctx, stencil_buffer, freeRenderbufferFunc);
        }
    } else {
        if (color_buffer)
            funcs.glDeleteRenderbuffers(1, &color_buffer);
        else
            glDeleteTextures(1, &texture);
        if (depth_buffer)
            funcs.glDeleteRenderbuffers(1, &depth_buffer);
        if (stencil_buffer && depth_buffer != stencil_buffer)
            funcs.glDeleteRenderbuffers(1, &stencil_buffer);
        funcs.glDeleteFramebuffers(1, &fbo);
    }
    QT_CHECK_GLERROR();

    format.setTextureTarget(target);
    format.setSamples(int(samples));
    format.setAttachment(fbo_attachment);
    format.setInternalTextureFormat(internal_format);
    format.setMipmap(mipmap);

    glDevice.setFBO(q, attachment);
}

/*!
    \class QOpenGLFramebufferObject
    \brief The QOpenGLFramebufferObject class encapsulates an OpenGL framebuffer object.
    \since 4.2

    \ingroup painting-3D

    The QOpenGLFramebufferObject class encapsulates an OpenGL framebuffer
    object, defined by the \c{GL_EXT_framebuffer_object} extension. In
    addition it provides a rendering surface that can be painted on
    with a QPainter, rendered to using native GL calls, or both. This
    surface can be bound and used as a regular texture in your own GL
    drawing code.  By default, the QOpenGLFramebufferObject class
    generates a 2D GL texture (using the \c{GL_TEXTURE_2D} target),
    which is used as the internal rendering target.

    \bold{It is important to have a current GL context when creating a
    QOpenGLFramebufferObject, otherwise initialization will fail.}

    OpenGL framebuffer objects and pbuffers (see
    \l{QOpenGLPixelBuffer}{QOpenGLPixelBuffer}) can both be used to render to
    offscreen surfaces, but there are a number of advantages with
    using framebuffer objects instead of pbuffers:

    \list 1
    \o A framebuffer object does not require a separate rendering
    context, so no context switching will occur when switching
    rendering targets. There is an overhead involved in switching
    targets, but in general it is cheaper than a context switch to a
    pbuffer.

    \o Rendering to dynamic textures (i.e. render-to-texture
    functionality) works on all platforms. No need to do explicit copy
    calls from a render buffer into a texture, as was necessary on
    systems that did not support the \c{render_texture} extension.

    \o It is possible to attach several rendering buffers (or texture
    objects) to the same framebuffer object, and render to all of them
    without doing a context switch.

    \o The OpenGL framebuffer extension is a pure GL extension with no
    system dependant WGL, CGL, or GLX parts. This makes using
    framebuffer objects more portable.
    \endlist

    When using a QPainter to paint to a QOpenGLFramebufferObject you should take
    care that the QOpenGLFramebufferObject is created with the CombinedDepthStencil
    attachment for QPainter to be able to render correctly.
    Note that you need to create a QOpenGLFramebufferObject with more than one
    sample per pixel for primitives to be antialiased when drawing using a
    QPainter. To create a multisample framebuffer object you should use one of
    the constructors that take a QOpenGLFramebufferObject parameter, and set the
    QOpenGLFramebufferObject::samples() property to a non-zero value.

    When painting to a QOpenGLFramebufferObject using QPainter, the state of
    the current GL context will be altered by the paint engine to reflect
    its needs.  Applications should not rely upon the GL state being reset
    to its original conditions, particularly the current shader program,
    GL viewport, texture units, and drawing modes.

    For multisample framebuffer objects a color render buffer is created,
    otherwise a texture with the specified texture target is created.
    The color render buffer or texture will have the specified internal
    format, and will be bound to the \c GL_COLOR_ATTACHMENT0
    attachment in the framebuffer object.

    If you want to use a framebuffer object with multisampling enabled
    as a texture, you first need to copy from it to a regular framebuffer
    object using QOpenGLContext::blitFramebuffer().

    \section1 Threading

    As of Qt 4.8, it's possible to draw into a QOpenGLFramebufferObject
    using a QPainter in a separate thread. Note that OpenGL 2.0 or
    OpenGL ES 2.0 is required for this to work.

    \sa {Framebuffer Object Example}
*/


/*!
    \enum QOpenGLFramebufferObject::Attachment
    \since 4.3

    This enum type is used to configure the depth and stencil buffers
    attached to the framebuffer object when it is created.

    \value NoAttachment         No attachment is added to the framebuffer object. Note that the
                                OpenGL depth and stencil tests won't work when rendering to a
                                framebuffer object without any depth or stencil buffers.
                                This is the default value.

    \value CombinedDepthStencil If the \c GL_EXT_packed_depth_stencil extension is present,
                                a combined depth and stencil buffer is attached.
                                If the extension is not present, only a depth buffer is attached.

    \value Depth                A depth buffer is attached to the framebuffer object.

    \sa attachment()
*/


/*! \fn QOpenGLFramebufferObject::QOpenGLFramebufferObject(const QSize &size, GLenum target)

    Constructs an OpenGL framebuffer object and binds a 2D GL texture
    to the buffer of the size \a size. The texture is bound to the
    \c GL_COLOR_ATTACHMENT0 target in the framebuffer object.

    The \a target parameter is used to specify the GL texture
    target. The default target is \c GL_TEXTURE_2D. Keep in mind that
    \c GL_TEXTURE_2D textures must have a power of 2 width and height
    (e.g. 256x512), unless you are using OpenGL 2.0 or higher.

    By default, no depth and stencil buffers are attached. This behavior
    can be toggled using one of the overloaded constructors.

    The default internal texture format is \c GL_RGBA8 for desktop
    OpenGL, and \c GL_RGBA for OpenGL/ES.

    It is important that you have a current GL context set when
    creating the QOpenGLFramebufferObject, otherwise the initialization
    will fail.

    \sa size(), texture(), attachment()
*/

QOpenGLFramebufferObject::QOpenGLFramebufferObject(const QSize &size, GLenum target)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, size, NoAttachment, target, DEFAULT_FORMAT);
}

/*! \overload

    Constructs an OpenGL framebuffer object and binds a 2D GL texture
    to the buffer of the given \a width and \a height.

    \sa size(), texture()
*/
QOpenGLFramebufferObject::QOpenGLFramebufferObject(int width, int height, GLenum target)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, QSize(width, height), NoAttachment, target, DEFAULT_FORMAT);
}

/*! \overload

    Constructs an OpenGL framebuffer object of the given \a size based on the
    supplied \a format.
*/

QOpenGLFramebufferObject::QOpenGLFramebufferObject(const QSize &size, const QOpenGLFramebufferObjectFormat &format)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, size, format.attachment(), format.textureTarget(), format.internalTextureFormat(),
            format.samples(), format.mipmap());
}

/*! \overload

    Constructs an OpenGL framebuffer object of the given \a width and \a height
    based on the supplied \a format.
*/

QOpenGLFramebufferObject::QOpenGLFramebufferObject(int width, int height, const QOpenGLFramebufferObjectFormat &format)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, QSize(width, height), format.attachment(), format.textureTarget(),
            format.internalTextureFormat(), format.samples(), format.mipmap());
}

/*! \overload

    Constructs an OpenGL framebuffer object and binds a texture to the
    buffer of the given \a width and \a height.

    The \a attachment parameter describes the depth/stencil buffer
    configuration, \a target the texture target and \a internal_format
    the internal texture format. The default texture target is \c
    GL_TEXTURE_2D, while the default internal format is \c GL_RGBA8
    for desktop OpenGL and \c GL_RGBA for OpenGL/ES.

    \sa size(), texture(), attachment()
*/
QOpenGLFramebufferObject::QOpenGLFramebufferObject(int width, int height, Attachment attachment,
                                           GLenum target, GLenum internal_format)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, QSize(width, height), attachment, target, internal_format);
}

/*! \overload

    Constructs an OpenGL framebuffer object and binds a texture to the
    buffer of the given \a size.

    The \a attachment parameter describes the depth/stencil buffer
    configuration, \a target the texture target and \a internal_format
    the internal texture format. The default texture target is \c
    GL_TEXTURE_2D, while the default internal format is \c GL_RGBA8
    for desktop OpenGL and \c GL_RGBA for OpenGL/ES.

    \sa size(), texture(), attachment()
*/
QOpenGLFramebufferObject::QOpenGLFramebufferObject(const QSize &size, Attachment attachment,
                                           GLenum target, GLenum internal_format)
    : d_ptr(new QOpenGLFramebufferObjectPrivate)
{
    Q_D(QOpenGLFramebufferObject);
    d->init(this, size, attachment, target, internal_format);
}

/*!
    \fn QOpenGLFramebufferObject::~QOpenGLFramebufferObject()

    Destroys the framebuffer object and frees any allocated resources.
*/
QOpenGLFramebufferObject::~QOpenGLFramebufferObject()
{
    Q_D(QOpenGLFramebufferObject);

    delete d->engine;

    if (d->texture_guard)
        d->texture_guard->free();
    if (d->color_buffer_guard)
        d->color_buffer_guard->free();
    if (d->depth_buffer_guard)
        d->depth_buffer_guard->free();
    if (d->stencil_buffer_guard && d->stencil_buffer_guard != d->depth_buffer_guard)
        d->stencil_buffer_guard->free();
    if (d->fbo_guard)
        d->fbo_guard->free();
}

/*!
    \fn bool QOpenGLFramebufferObject::isValid() const

    Returns true if the framebuffer object is valid.

    The framebuffer can become invalid if the initialization process
    fails, the user attaches an invalid buffer to the framebuffer
    object, or a non-power of two width/height is specified as the
    texture size if the texture target is \c{GL_TEXTURE_2D}.
    The non-power of two limitation does not apply if the OpenGL version
    is 2.0 or higher, or if the GL_ARB_texture_non_power_of_two extension
    is present.

    The framebuffer can also become invalid if the QOpenGLContext that
    the framebuffer was created within is destroyed and there are
    no other shared contexts that can take over ownership of the
    framebuffer.
*/
bool QOpenGLFramebufferObject::isValid() const
{
    Q_D(const QOpenGLFramebufferObject);
    return d->valid && d->fbo_guard && d->fbo_guard->id();
}

/*!
    \fn bool QOpenGLFramebufferObject::bind()

    Switches rendering from the default, windowing system provided
    framebuffer to this framebuffer object.
    Returns true upon success, false otherwise.

    \sa release()
*/
bool QOpenGLFramebufferObject::bind()
{
    if (!isValid())
	return false;
    Q_D(QOpenGLFramebufferObject);
    QOpenGLContext *current = QOpenGLContext::currentContext();
    if (!current)
        return false;
#ifdef QT_DEBUG
    if (current->shareGroup() != d->fbo_guard->group())
        qWarning("QOpenGLFramebufferObject::bind() called from incompatible context");
#endif
    d->funcs.glBindFramebuffer(GL_FRAMEBUFFER, d->fbo());
    d->valid = d->checkFramebufferStatus();
    if (d->valid && current)
        current->d_func()->current_fbo = d->fbo();
    return d->valid;
}

/*!
    \fn bool QOpenGLFramebufferObject::release()

    Switches rendering back to the default, windowing system provided
    framebuffer.
    Returns true upon success, false otherwise.

    \sa bind()
*/
bool QOpenGLFramebufferObject::release()
{
    if (!isValid())
	return false;

    QOpenGLContext *current = QOpenGLContext::currentContext();
    if (!current)
        return false;

#ifdef QT_DEBUG
    Q_D(QOpenGLFramebufferObject);
    if (current->shareGroup() != d->fbo_guard->group())
        qWarning("QOpenGLFramebufferObject::release() called from incompatible context");
#endif

    if (current) {
        current->d_func()->current_fbo = current->d_func()->default_fbo;
        d->funcs.glBindFramebuffer(GL_FRAMEBUFFER, current->d_func()->default_fbo);
    }

    return true;
}

/*!
    \fn GLuint QOpenGLFramebufferObject::texture() const

    Returns the texture id for the texture attached as the default
    rendering target in this framebuffer object. This texture id can
    be bound as a normal texture in your own GL code.

    If a multisample framebuffer object is used then the value returned
    from this function will be invalid.
*/
GLuint QOpenGLFramebufferObject::texture() const
{
    Q_D(const QOpenGLFramebufferObject);
    return d->texture_guard ? d->texture_guard->id() : 0;
}

/*!
    \fn QSize QOpenGLFramebufferObject::size() const

    Returns the size of the texture attached to this framebuffer
    object.
*/
QSize QOpenGLFramebufferObject::size() const
{
    Q_D(const QOpenGLFramebufferObject);
    return d->size;
}

/*!
    Returns the format of this framebuffer object.
*/
QOpenGLFramebufferObjectFormat QOpenGLFramebufferObject::format() const
{
    Q_D(const QOpenGLFramebufferObject);
    return d->format;
}

namespace {
/*
   Read back the contents of the currently bound framebuffer, used in
   QGLWidget::grabFrameBuffer(), QGLPixelbuffer::toImage() and
   QGLFramebufferObject::toImage()
*/

void convertFromGLImage(QImage &img, int w, int h, bool alpha_format, bool include_alpha)
{
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        // OpenGL gives RGBA; Qt wants ARGB
        uint *p = (uint*)img.bits();
        uint *end = p + w*h;
        if (alpha_format && include_alpha) {
            while (p < end) {
                uint a = *p << 24;
                *p = (*p >> 8) | a;
                p++;
            }
        } else {
            // This is an old legacy fix for PowerPC based Macs, which
            // we shouldn't remove
            while (p < end) {
                *p = 0xff000000 | (*p>>8);
                ++p;
            }
        }
    } else {
        // OpenGL gives ABGR (i.e. RGBA backwards); Qt wants ARGB
        for (int y = 0; y < h; y++) {
            uint *q = (uint*)img.scanLine(y);
            for (int x=0; x < w; ++x) {
                const uint pixel = *q;
                if (alpha_format && include_alpha) {
                    *q = ((pixel << 16) & 0xff0000) | ((pixel >> 16) & 0xff)
                         | (pixel & 0xff00ff00);
                } else {
                    *q = 0xff000000 | ((pixel << 16) & 0xff0000)
                         | ((pixel >> 16) & 0xff) | (pixel & 0x00ff00);
                }

                q++;
            }
        }

    }
    img = img.mirrored();
}

}

Q_GUI_EXPORT QImage qt_gl_read_framebuffer(const QSize &size, bool alpha_format, bool include_alpha)
{
    QImage img(size, (alpha_format && include_alpha) ? QImage::Format_ARGB32_Premultiplied
                                                     : QImage::Format_RGB32);
    int w = size.width();
    int h = size.height();
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    convertFromGLImage(img, w, h, alpha_format, include_alpha);
    return img;
}

/*!
    \fn QImage QOpenGLFramebufferObject::toImage() const

    Returns the contents of this framebuffer object as a QImage.
*/
QImage QOpenGLFramebufferObject::toImage() const
{
    Q_D(const QOpenGLFramebufferObject);
    if (!d->valid)
        return QImage();

    // qt_gl_read_framebuffer doesn't work on a multisample FBO
    if (format().samples() != 0) {
        QOpenGLFramebufferObject temp(size(), QOpenGLFramebufferObjectFormat());

        QRect rect(QPoint(0, 0), size());
        blitFramebuffer(&temp, rect, const_cast<QOpenGLFramebufferObject *>(this), rect);

        return temp.toImage();
    }

    bool wasBound = isBound();
    if (!wasBound)
        const_cast<QOpenGLFramebufferObject *>(this)->bind();
    QImage image = qt_gl_read_framebuffer(d->size, format().internalTextureFormat() != GL_RGB, true);
    if (!wasBound)
        const_cast<QOpenGLFramebufferObject *>(this)->release();

    return image;
}

Q_GLOBAL_STATIC(QOpenGLEngineThreadStorage<QOpenGL2PaintEngineEx>, qt_buffer_2_engine)

/*! \reimp */
QPaintEngine *QOpenGLFramebufferObject::paintEngine() const
{
    Q_D(const QOpenGLFramebufferObject);
    if (d->engine)
        return d->engine;

    QPaintEngine *engine = qt_buffer_2_engine()->engine();
    if (engine->isActive() && engine->paintDevice() != this) {
        d->engine = new QOpenGL2PaintEngineEx;
        return d->engine;
    }
    return engine;
}

/*!
    \fn bool QOpenGLFramebufferObject::bindDefault()
    \internal

    Switches rendering back to the default, windowing system provided
    framebuffer.
    Returns true upon success, false otherwise.

    \sa bind(), release()
*/
bool QOpenGLFramebufferObject::bindDefault()
{
    QOpenGLContext *ctx = const_cast<QOpenGLContext *>(QOpenGLContext::currentContext());
    QOpenGLFunctions functions(ctx);

    if (ctx) {
        ctx->d_func()->current_fbo = ctx->d_func()->default_fbo;
        functions.glBindFramebuffer(GL_FRAMEBUFFER, ctx->d_func()->default_fbo);
#ifdef QT_DEBUG
    } else {
        qWarning("QOpenGLFramebufferObject::bindDefault() called without current context.");
#endif
    }

    return ctx != 0;
}

/*!
    \fn bool QOpenGLFramebufferObject::hasOpenGLFramebufferObjects()

    Returns true if the OpenGL \c{GL_EXT_framebuffer_object} extension
    is present on this system; otherwise returns false.
*/
bool QOpenGLFramebufferObject::hasOpenGLFramebufferObjects()
{
    return QOpenGLFunctions(QOpenGLContext::currentContext()).hasOpenGLFeature(QOpenGLFunctions::Framebuffers);
}

/*! \reimp */
int QOpenGLFramebufferObject::metric(PaintDeviceMetric metric) const
{
    Q_D(const QOpenGLFramebufferObject);

    float dpmx = qt_defaultDpiX()*100./2.54;
    float dpmy = qt_defaultDpiY()*100./2.54;
    int w = d->size.width();
    int h = d->size.height();
    switch (metric) {
    case PdmWidth:
        return w;

    case PdmHeight:
        return h;

    case PdmWidthMM:
        return qRound(w * 1000 / dpmx);

    case PdmHeightMM:
        return qRound(h * 1000 / dpmy);

    case PdmNumColors:
        return 0;

    case PdmDepth:
        return 32;//d->depth;

    case PdmDpiX:
        return qRound(dpmx * 0.0254);

    case PdmDpiY:
        return qRound(dpmy * 0.0254);

    case PdmPhysicalDpiX:
        return qRound(dpmx * 0.0254);

    case PdmPhysicalDpiY:
        return qRound(dpmy * 0.0254);

    default:
        qWarning("QOpenGLFramebufferObject::metric(), Unhandled metric type: %d.\n", metric);
        break;
    }
    return 0;
}

/*!
    \fn GLuint QOpenGLFramebufferObject::handle() const

    Returns the GL framebuffer object handle for this framebuffer
    object (returned by the \c{glGenFrameBuffersEXT()} function). This
    handle can be used to attach new images or buffers to the
    framebuffer. The user is responsible for cleaning up and
    destroying these objects.
*/
GLuint QOpenGLFramebufferObject::handle() const
{
    Q_D(const QOpenGLFramebufferObject);
    return d->fbo();
}

/*! \fn int QOpenGLFramebufferObject::devType() const
    \internal
*/


/*!
    Returns the status of the depth and stencil buffers attached to
    this framebuffer object.
*/

QOpenGLFramebufferObject::Attachment QOpenGLFramebufferObject::attachment() const
{
    Q_D(const QOpenGLFramebufferObject);
    if (d->valid)
        return d->fbo_attachment;
    return NoAttachment;
}

/*!
    \since 4.5

    Returns true if the framebuffer object is currently bound to a context,
    otherwise false is returned.
*/

bool QOpenGLFramebufferObject::isBound() const
{
    Q_D(const QOpenGLFramebufferObject);
    QOpenGLContext *current = QOpenGLContext::currentContext();
    return current ? current->d_func()->current_fbo == d->fbo() : false;
}

/*!
    \fn bool QOpenGLFramebufferObject::hasOpenGLFramebufferBlit()

    \since 4.6

    Returns true if the OpenGL \c{GL_EXT_framebuffer_blit} extension
    is present on this system; otherwise returns false.

    \sa blitFramebuffer()
*/
bool QOpenGLFramebufferObject::hasOpenGLFramebufferBlit()
{
    return QOpenGLExtensions(QOpenGLContext::currentContext()).hasOpenGLExtension(QOpenGLExtensions::FramebufferBlit);
}

/*!
    \since 4.6

    Blits from the \a sourceRect rectangle in the \a source framebuffer
    object to the \a targetRect rectangle in the \a target framebuffer object.

    If \a source or \a target is 0, the default framebuffer will be used
    instead of a framebuffer object as source or target respectively.

    The \a buffers parameter should be a mask consisting of any combination of
    \c GL_COLOR_BUFFER_BIT, \c GL_DEPTH_BUFFER_BIT, and
    \c GL_STENCIL_BUFFER_BIT.  Any buffer type that is not present both
    in the source and target buffers is ignored.

    The \a sourceRect and \a targetRect rectangles may have different sizes;
    in this case \a buffers should not contain \c GL_DEPTH_BUFFER_BIT or
    \c GL_STENCIL_BUFFER_BIT. The \a filter parameter should be set to
    \c GL_LINEAR or \c GL_NEAREST, and specifies whether linear or nearest
    interpolation should be used when scaling is performed.

    If \a source equals \a target a copy is performed within the same buffer.
    Results are undefined if the source and target rectangles overlap and
    have different sizes. The sizes must also be the same if any of the
    framebuffer objects are multisample framebuffers.

    Note that the scissor test will restrict the blit area if enabled.

    This function will have no effect unless hasOpenGLFramebufferBlit() returns
    true.

    \sa hasOpenGLFramebufferBlit()
*/
void QOpenGLFramebufferObject::blitFramebuffer(QOpenGLFramebufferObject *target, const QRect &targetRect,
                                           QOpenGLFramebufferObject *source, const QRect &sourceRect,
                                           GLbitfield buffers,
                                           GLenum filter)
{
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx)
        return;

    QOpenGLExtensions extensions(ctx);
    if (!extensions.hasOpenGLExtension(QOpenGLExtensions::FramebufferBlit))
        return;

    QSurface *surface = ctx->surface();

    const int height = static_cast<QWindow *>(surface)->height();

    const int sh = source ? source->height() : height;
    const int th = target ? target->height() : height;

    const int sx0 = sourceRect.left();
    const int sx1 = sourceRect.left() + sourceRect.width();
    const int sy0 = sh - (sourceRect.top() + sourceRect.height());
    const int sy1 = sh - sourceRect.top();

    const int tx0 = targetRect.left();
    const int tx1 = targetRect.left() + targetRect.width();
    const int ty0 = th - (targetRect.top() + targetRect.height());
    const int ty1 = th - targetRect.top();

    extensions.glBindFramebuffer(GL_READ_FRAMEBUFFER, source ? source->handle() : 0);
    extensions.glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target ? target->handle() : 0);

    extensions.glBlitFramebuffer(sx0, sy0, sx1, sy1,
                         tx0, ty0, tx1, ty1,
                         buffers, filter);

    extensions.glBindFramebuffer(GL_FRAMEBUFFER, ctx->d_func()->current_fbo);
}

QT_END_NAMESPACE
