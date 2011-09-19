/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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


#include <QtGui/private/qopenglcontext_p.h>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QPainter>
#include <QtGui/QScreen>
#include <QtGui/QWindow>

#include <QtTest/QtTest>

class tst_QOpenGL : public QObject
{
Q_OBJECT

private slots:
    void sharedResourceCleanup();
    void fboSimpleRendering();
    void fboRendering();
    void fboHandleNulledAfterContextDestroyed();
    void openGLPaintDevice();
};

struct SharedResourceTracker
{
    SharedResourceTracker()
    {
        reset();
    }

    void reset()
    {
        invalidateResourceCalls = 0;
        freeResourceCalls = 0;
        destructorCalls = 0;
    }

    int invalidateResourceCalls;
    int freeResourceCalls;
    int destructorCalls;
};

struct SharedResource : public QOpenGLSharedResource
{
    SharedResource(SharedResourceTracker *t)
        : QOpenGLSharedResource(QOpenGLContextGroup::currentContextGroup())
        , resource(1)
        , tracker(t)
    {
    }

    ~SharedResource()
    {
        tracker->destructorCalls++;
    }

    void invalidateResource()
    {
        resource = 0;
        tracker->invalidateResourceCalls++;
    }

    void freeResource(QOpenGLContext *context)
    {
        Q_ASSERT(context == QOpenGLContext::currentContext());
        resource = 0;
        tracker->freeResourceCalls++;
    }

    int resource;
    SharedResourceTracker *tracker;
};

void tst_QOpenGL::sharedResourceCleanup()
{
    QWindow window;
    window.setGeometry(0, 0, 10, 10);
    window.create();

    QOpenGLContext *ctx = new QOpenGLContext;
    ctx->create();
    ctx->makeCurrent(&window);

    SharedResourceTracker tracker;
    SharedResource *resource = new SharedResource(&tracker);
    resource->free();

    QCOMPARE(tracker.invalidateResourceCalls, 0);
    QCOMPARE(tracker.freeResourceCalls, 1);
    QCOMPARE(tracker.destructorCalls, 1);

    tracker.reset();

    resource = new SharedResource(&tracker);

    QOpenGLContext *ctx2 = new QOpenGLContext;
    ctx2->setShareContext(ctx);
    ctx2->create();
    delete ctx;

    resource->free();

    // no current context, freeResource() delayed
    QCOMPARE(tracker.invalidateResourceCalls, 0);
    QCOMPARE(tracker.freeResourceCalls, 0);
    QCOMPARE(tracker.destructorCalls, 0);

    ctx2->makeCurrent(&window);

    // freeResource() should now have been called
    QCOMPARE(tracker.invalidateResourceCalls, 0);
    QCOMPARE(tracker.freeResourceCalls, 1);
    QCOMPARE(tracker.destructorCalls, 1);

    tracker.reset();

    resource = new SharedResource(&tracker);

    // this should cause invalidateResource() to be called
    delete ctx2;

    QCOMPARE(tracker.invalidateResourceCalls, 1);
    QCOMPARE(tracker.freeResourceCalls, 0);
    QCOMPARE(tracker.destructorCalls, 0);

    // should have no effect other than destroying the resource,
    // as it has already been invalidated
    resource->free();

    QCOMPARE(tracker.invalidateResourceCalls, 1);
    QCOMPARE(tracker.freeResourceCalls, 0);
    QCOMPARE(tracker.destructorCalls, 1);
}

static bool fuzzyComparePixels(const QRgb testPixel, const QRgb refPixel, const char* file, int line, int x = -1, int y = -1)
{
    static int maxFuzz = 1;
    static bool maxFuzzSet = false;

    // On 16 bpp systems, we need to allow for more fuzz:
    if (!maxFuzzSet) {
        maxFuzzSet = true;
        if (QGuiApplication::primaryScreen()->depth() < 24)
            maxFuzz = 32;
    }

    int redFuzz = qAbs(qRed(testPixel) - qRed(refPixel));
    int greenFuzz = qAbs(qGreen(testPixel) - qGreen(refPixel));
    int blueFuzz = qAbs(qBlue(testPixel) - qBlue(refPixel));
    int alphaFuzz = qAbs(qAlpha(testPixel) - qAlpha(refPixel));

    if (refPixel != 0 && testPixel == 0) {
        QString msg;
        if (x >= 0) {
            msg = QString("Test pixel [%1, %2] is null (black) when it should be (%3,%4,%5,%6)")
                            .arg(x).arg(y)
                            .arg(qRed(refPixel)).arg(qGreen(refPixel)).arg(qBlue(refPixel)).arg(qAlpha(refPixel));
        } else {
            msg = QString("Test pixel is null (black) when it should be (%2,%3,%4,%5)")
                            .arg(qRed(refPixel)).arg(qGreen(refPixel)).arg(qBlue(refPixel)).arg(qAlpha(refPixel));
        }

        QTest::qFail(msg.toLatin1(), file, line);
        return false;
    }

    if (redFuzz > maxFuzz || greenFuzz > maxFuzz || blueFuzz > maxFuzz || alphaFuzz > maxFuzz) {
        QString msg;

        if (x >= 0)
            msg = QString("Pixel [%1,%2]: ").arg(x).arg(y);
        else
            msg = QString("Pixel ");

        msg += QString("Max fuzz (%1) exceeded: (%2,%3,%4,%5) vs (%6,%7,%8,%9)")
                      .arg(maxFuzz)
                      .arg(qRed(testPixel)).arg(qGreen(testPixel)).arg(qBlue(testPixel)).arg(qAlpha(testPixel))
                      .arg(qRed(refPixel)).arg(qGreen(refPixel)).arg(qBlue(refPixel)).arg(qAlpha(refPixel));
        QTest::qFail(msg.toLatin1(), file, line);
        return false;
    }
    return true;
}

static void fuzzyCompareImages(const QImage &testImage, const QImage &referenceImage, const char* file, int line)
{
    QCOMPARE(testImage.width(), referenceImage.width());
    QCOMPARE(testImage.height(), referenceImage.height());

    for (int y = 0; y < testImage.height(); y++) {
        for (int x = 0; x < testImage.width(); x++) {
            if (!fuzzyComparePixels(testImage.pixel(x, y), referenceImage.pixel(x, y), file, line, x, y)) {
                // Might as well save the images for easier debugging:
                referenceImage.save("referenceImage.png");
                testImage.save("testImage.png");
                return;
            }
        }
    }
}

#define QFUZZY_COMPARE_IMAGES(A,B) \
            fuzzyCompareImages(A, B, __FILE__, __LINE__)

#define QFUZZY_COMPARE_PIXELS(A,B) \
            fuzzyComparePixels(A, B, __FILE__, __LINE__)

void qt_opengl_draw_test_pattern(QPainter* painter, int width, int height)
{
    QPainterPath intersectingPath;
    intersectingPath.moveTo(0, 0);
    intersectingPath.lineTo(100, 0);
    intersectingPath.lineTo(0, 100);
    intersectingPath.lineTo(100, 100);
    intersectingPath.closeSubpath();

    QPainterPath trianglePath;
    trianglePath.moveTo(50, 0);
    trianglePath.lineTo(100, 100);
    trianglePath.lineTo(0, 100);
    trianglePath.closeSubpath();

    painter->setTransform(QTransform()); // reset xform
    painter->fillRect(-1, -1, width+2, height+2, Qt::red); // Background
    painter->translate(14, 14);
    painter->fillPath(intersectingPath, Qt::blue); // Test stencil buffer works
    painter->translate(128, 0);
    painter->setClipPath(trianglePath); // Test depth buffer works
    painter->setTransform(QTransform()); // reset xform ready for fill
    painter->fillRect(-1, -1, width+2, height+2, Qt::green);
}

void qt_opengl_check_test_pattern(const QImage& img)
{
    // As we're doing more than trivial painting, we can't just compare to
    // an image rendered with raster. Instead, we sample at well-defined
    // test-points:
    QFUZZY_COMPARE_PIXELS(img.pixel(39, 64), QColor(Qt::red).rgb());
    QFUZZY_COMPARE_PIXELS(img.pixel(89, 64), QColor(Qt::red).rgb());
    QFUZZY_COMPARE_PIXELS(img.pixel(64, 39), QColor(Qt::blue).rgb());
    QFUZZY_COMPARE_PIXELS(img.pixel(64, 89), QColor(Qt::blue).rgb());

    QFUZZY_COMPARE_PIXELS(img.pixel(167, 39), QColor(Qt::red).rgb());
    QFUZZY_COMPARE_PIXELS(img.pixel(217, 39), QColor(Qt::red).rgb());
    QFUZZY_COMPARE_PIXELS(img.pixel(192, 64), QColor(Qt::green).rgb());
}


void tst_QOpenGL::fboSimpleRendering()
{
    QWindow window;
    window.setGeometry(0, 0, 10, 10);
    window.create();
    QOpenGLContext ctx;
    ctx.create();

    ctx.makeCurrent(&window);

    if (!QOpenGLFramebufferObject::hasOpenGLFramebufferObjects())
        QSKIP("QOpenGLFramebufferObject not supported on this platform", SkipSingle);

    // No multisample with combined depth/stencil attachment:
    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);

    QOpenGLFramebufferObject *fbo = new QOpenGLFramebufferObject(200, 100, fboFormat);

    fbo->bind();

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFinish();

    QImage fb = fbo->toImage().convertToFormat(QImage::Format_RGB32);
    QImage reference(fb.size(), QImage::Format_RGB32);
    reference.fill(0xffff0000);

    QFUZZY_COMPARE_IMAGES(fb, reference);

    delete fbo;
}

// NOTE: This tests that CombinedDepthStencil attachment works by assuming the
//       GL2 engine is being used and is implemented the same way as it was when
//       this autotest was written. If this is not the case, there may be some
//       false-positives: I.e. The test passes when either the depth or stencil
//       buffer is actually missing. But that's probably ok anyway.
void tst_QOpenGL::fboRendering()
{
    QWindow window;
    window.setGeometry(0, 0, 10, 10);
    window.create();
    QOpenGLContext ctx;
    ctx.create();

    ctx.makeCurrent(&window);

    if (!QOpenGLFramebufferObject::hasOpenGLFramebufferObjects())
        QSKIP("QOpenGLFramebufferObject not supported on this platform", SkipSingle);

    // No multisample with combined depth/stencil attachment:
    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

    // Uncomplicate things by using NPOT:
    QOpenGLFramebufferObject fbo(256, 128, fboFormat);

    if (fbo.attachment() != QOpenGLFramebufferObject::CombinedDepthStencil)
        QSKIP("FBOs missing combined depth~stencil support", SkipSingle);

    fbo.bind();

    QPainter fboPainter;
    QOpenGLPaintDevice device(fbo.width(), fbo.height());
    bool painterBegun = fboPainter.begin(&device);
    QVERIFY(painterBegun);

    qt_opengl_draw_test_pattern(&fboPainter, fbo.width(), fbo.height());

    fboPainter.end();

    QImage fb = fbo.toImage().convertToFormat(QImage::Format_RGB32);

    qt_opengl_check_test_pattern(fb);
}

void tst_QOpenGL::fboHandleNulledAfterContextDestroyed()
{
    QWindow window;
    window.setGeometry(0, 0, 10, 10);
    window.create();

    QOpenGLFramebufferObject *fbo = 0;

    {
        QOpenGLContext ctx;
        ctx.create();

        ctx.makeCurrent(&window);

        if (!QOpenGLFramebufferObject::hasOpenGLFramebufferObjects()) {
            QSKIP("QOpenGLFramebufferObject not supported on this platform", SkipSingle);
        }

        fbo = new QOpenGLFramebufferObject(128, 128);

        QVERIFY(fbo->handle() != 0);
    }

    QCOMPARE(fbo->handle(), 0U);
}

void tst_QOpenGL::openGLPaintDevice()
{
    QWindow window;
    window.setGeometry(0, 0, 128, 128);
    window.create();

    QOpenGLContext ctx;
    ctx.create();

    ctx.makeCurrent(&window);

    QImage image(128, 128, QImage::Format_RGB32);
    QPainter p(&image);
    p.fillRect(0, 0, image.width() / 2, image.height() / 2, Qt::red);
    p.fillRect(image.width() / 2, 0, image.width() / 2, image.height() / 2, Qt::green);
    p.fillRect(image.width() / 2, image.height() / 2, image.width() / 2, image.height() / 2, Qt::blue);
    p.fillRect(0, image.height() / 2, image.width() / 2, image.height() / 2, Qt::white);
    p.end();

    QOpenGLFramebufferObject fbo(128, 128);
    fbo.bind();

    QOpenGLPaintDevice device(128, 128);
    p.begin(&device);
    p.fillRect(0, 0, image.width() / 2, image.height() / 2, Qt::red);
    p.fillRect(image.width() / 2, 0, image.width() / 2, image.height() / 2, Qt::green);
    p.fillRect(image.width() / 2, image.height() / 2, image.width() / 2, image.height() / 2, Qt::blue);
    p.fillRect(0, image.height() / 2, image.width() / 2, image.height() / 2, Qt::white);
    p.end();

    QCOMPARE(image, fbo.toImage().convertToFormat(QImage::Format_RGB32));

    p.begin(&device);
    p.fillRect(0, 0, image.width(), image.height(), Qt::black);
    p.drawImage(0, 0, image);
    p.end();

    QCOMPARE(image, fbo.toImage().convertToFormat(QImage::Format_RGB32));

    p.begin(&device);
    p.fillRect(0, 0, image.width(), image.height(), Qt::black);
    p.fillRect(0, 0, image.width(), image.height(), QBrush(image));
    p.end();

    QCOMPARE(image, fbo.toImage().convertToFormat(QImage::Format_RGB32));
}

QTEST_MAIN(tst_QOpenGL)
#include "tst_qopengl.moc"
