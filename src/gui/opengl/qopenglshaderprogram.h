/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
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

#ifndef QOPENGLSHADERPROGRAM_H
#define QOPENGLSHADERPROGRAM_H

#ifndef QT_NO_OPENGL

#include <QtGui/qopengl.h>
#include <QtGui/qvector2d.h>
#include <QtGui/qvector3d.h>
#include <QtGui/qvector4d.h>
#include <QtGui/qmatrix4x4.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


class QOpenGLContext;
class QOpenGLShaderProgram;
class QOpenGLShaderPrivate;

class Q_GUI_EXPORT QOpenGLShader : public QObject
{
    Q_OBJECT
public:
    enum ShaderTypeBit
    {
        Vertex          = 0x0001,
        Fragment        = 0x0002,
        Geometry        = 0x0004
    };
    Q_DECLARE_FLAGS(ShaderType, ShaderTypeBit)

    explicit QOpenGLShader(QOpenGLShader::ShaderType type, QObject *parent = 0);
    virtual ~QOpenGLShader();

    QOpenGLShader::ShaderType shaderType() const;

    bool compileSourceCode(const char *source);
    bool compileSourceCode(const QByteArray& source);
    bool compileSourceCode(const QString& source);
    bool compileSourceFile(const QString& fileName);

    QByteArray sourceCode() const;

    bool isCompiled() const;
    QString log() const;

    GLuint shaderId() const;

    static bool hasOpenGLShaders(ShaderType type, QOpenGLContext *context = 0);

private:
    friend class QOpenGLShaderProgram;

    Q_DISABLE_COPY(QOpenGLShader)
    Q_DECLARE_PRIVATE(QOpenGLShader)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QOpenGLShader::ShaderType)


class QOpenGLShaderProgramPrivate;

#ifndef GL_EXT_geometry_shader4
#  define GL_LINES_ADJACENCY_EXT 0xA
#  define GL_LINE_STRIP_ADJACENCY_EXT 0xB
#  define GL_TRIANGLES_ADJACENCY_EXT 0xC
#  define GL_TRIANGLE_STRIP_ADJACENCY_EXT 0xD
#endif


class Q_GUI_EXPORT QOpenGLShaderProgram : public QObject
{
    Q_OBJECT
public:
    explicit QOpenGLShaderProgram(QObject *parent = 0);
    virtual ~QOpenGLShaderProgram();

    bool addShader(QOpenGLShader *shader);
    void removeShader(QOpenGLShader *shader);
    QList<QOpenGLShader *> shaders() const;

    bool addShaderFromSourceCode(QOpenGLShader::ShaderType type, const char *source);
    bool addShaderFromSourceCode(QOpenGLShader::ShaderType type, const QByteArray& source);
    bool addShaderFromSourceCode(QOpenGLShader::ShaderType type, const QString& source);
    bool addShaderFromSourceFile(QOpenGLShader::ShaderType type, const QString& fileName);

    void removeAllShaders();

    virtual bool link();
    bool isLinked() const;
    QString log() const;

    bool bind();
    void release();

    GLuint programId() const;

    int maxGeometryOutputVertices() const;

    void setGeometryOutputVertexCount(int count);
    int geometryOutputVertexCount() const;

    void setGeometryInputType(GLenum inputType);
    GLenum geometryInputType() const;

    void setGeometryOutputType(GLenum outputType);
    GLenum geometryOutputType() const;

    void bindAttributeLocation(const char *name, int location);
    void bindAttributeLocation(const QByteArray& name, int location);
    void bindAttributeLocation(const QString& name, int location);

    int attributeLocation(const char *name) const;
    int attributeLocation(const QByteArray& name) const;
    int attributeLocation(const QString& name) const;

    void setAttributeValue(int location, GLfloat value);
    void setAttributeValue(int location, GLfloat x, GLfloat y);
    void setAttributeValue(int location, GLfloat x, GLfloat y, GLfloat z);
    void setAttributeValue(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void setAttributeValue(int location, const QVector2D& value);
    void setAttributeValue(int location, const QVector3D& value);
    void setAttributeValue(int location, const QVector4D& value);
    void setAttributeValue(int location, const QColor& value);
    void setAttributeValue(int location, const GLfloat *values, int columns, int rows);

    void setAttributeValue(const char *name, GLfloat value);
    void setAttributeValue(const char *name, GLfloat x, GLfloat y);
    void setAttributeValue(const char *name, GLfloat x, GLfloat y, GLfloat z);
    void setAttributeValue(const char *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void setAttributeValue(const char *name, const QVector2D& value);
    void setAttributeValue(const char *name, const QVector3D& value);
    void setAttributeValue(const char *name, const QVector4D& value);
    void setAttributeValue(const char *name, const QColor& value);
    void setAttributeValue(const char *name, const GLfloat *values, int columns, int rows);

    void setAttributeArray
        (int location, const GLfloat *values, int tupleSize, int stride = 0);
    void setAttributeArray
        (int location, const QVector2D *values, int stride = 0);
    void setAttributeArray
        (int location, const QVector3D *values, int stride = 0);
    void setAttributeArray
        (int location, const QVector4D *values, int stride = 0);
    void setAttributeArray
        (int location, GLenum type, const void *values, int tupleSize, int stride = 0);
    void setAttributeArray
        (const char *name, const GLfloat *values, int tupleSize, int stride = 0);
    void setAttributeArray
        (const char *name, const QVector2D *values, int stride = 0);
    void setAttributeArray
        (const char *name, const QVector3D *values, int stride = 0);
    void setAttributeArray
        (const char *name, const QVector4D *values, int stride = 0);
    void setAttributeArray
        (const char *name, GLenum type, const void *values, int tupleSize, int stride = 0);

    void setAttributeBuffer
        (int location, GLenum type, int offset, int tupleSize, int stride = 0);
    void setAttributeBuffer
        (const char *name, GLenum type, int offset, int tupleSize, int stride = 0);

    void enableAttributeArray(int location);
    void enableAttributeArray(const char *name);
    void disableAttributeArray(int location);
    void disableAttributeArray(const char *name);

    int uniformLocation(const char *name) const;
    int uniformLocation(const QByteArray& name) const;
    int uniformLocation(const QString& name) const;

    void setUniformValue(int location, GLfloat value);
    void setUniformValue(int location, GLint value);
    void setUniformValue(int location, GLuint value);
    void setUniformValue(int location, GLfloat x, GLfloat y);
    void setUniformValue(int location, GLfloat x, GLfloat y, GLfloat z);
    void setUniformValue(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void setUniformValue(int location, const QVector2D& value);
    void setUniformValue(int location, const QVector3D& value);
    void setUniformValue(int location, const QVector4D& value);
    void setUniformValue(int location, const QColor& color);
    void setUniformValue(int location, const QPoint& point);
    void setUniformValue(int location, const QPointF& point);
    void setUniformValue(int location, const QSize& size);
    void setUniformValue(int location, const QSizeF& size);
    void setUniformValue(int location, const QMatrix2x2& value);
    void setUniformValue(int location, const QMatrix2x3& value);
    void setUniformValue(int location, const QMatrix2x4& value);
    void setUniformValue(int location, const QMatrix3x2& value);
    void setUniformValue(int location, const QMatrix3x3& value);
    void setUniformValue(int location, const QMatrix3x4& value);
    void setUniformValue(int location, const QMatrix4x2& value);
    void setUniformValue(int location, const QMatrix4x3& value);
    void setUniformValue(int location, const QMatrix4x4& value);
    void setUniformValue(int location, const GLfloat value[2][2]);
    void setUniformValue(int location, const GLfloat value[3][3]);
    void setUniformValue(int location, const GLfloat value[4][4]);
    void setUniformValue(int location, const QTransform& value);

    void setUniformValue(const char *name, GLfloat value);
    void setUniformValue(const char *name, GLint value);
    void setUniformValue(const char *name, GLuint value);
    void setUniformValue(const char *name, GLfloat x, GLfloat y);
    void setUniformValue(const char *name, GLfloat x, GLfloat y, GLfloat z);
    void setUniformValue(const char *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void setUniformValue(const char *name, const QVector2D& value);
    void setUniformValue(const char *name, const QVector3D& value);
    void setUniformValue(const char *name, const QVector4D& value);
    void setUniformValue(const char *name, const QColor& color);
    void setUniformValue(const char *name, const QPoint& point);
    void setUniformValue(const char *name, const QPointF& point);
    void setUniformValue(const char *name, const QSize& size);
    void setUniformValue(const char *name, const QSizeF& size);
    void setUniformValue(const char *name, const QMatrix2x2& value);
    void setUniformValue(const char *name, const QMatrix2x3& value);
    void setUniformValue(const char *name, const QMatrix2x4& value);
    void setUniformValue(const char *name, const QMatrix3x2& value);
    void setUniformValue(const char *name, const QMatrix3x3& value);
    void setUniformValue(const char *name, const QMatrix3x4& value);
    void setUniformValue(const char *name, const QMatrix4x2& value);
    void setUniformValue(const char *name, const QMatrix4x3& value);
    void setUniformValue(const char *name, const QMatrix4x4& value);
    void setUniformValue(const char *name, const GLfloat value[2][2]);
    void setUniformValue(const char *name, const GLfloat value[3][3]);
    void setUniformValue(const char *name, const GLfloat value[4][4]);
    void setUniformValue(const char *name, const QTransform& value);

    void setUniformValueArray(int location, const GLfloat *values, int count, int tupleSize);
    void setUniformValueArray(int location, const GLint *values, int count);
    void setUniformValueArray(int location, const GLuint *values, int count);
    void setUniformValueArray(int location, const QVector2D *values, int count);
    void setUniformValueArray(int location, const QVector3D *values, int count);
    void setUniformValueArray(int location, const QVector4D *values, int count);
    void setUniformValueArray(int location, const QMatrix2x2 *values, int count);
    void setUniformValueArray(int location, const QMatrix2x3 *values, int count);
    void setUniformValueArray(int location, const QMatrix2x4 *values, int count);
    void setUniformValueArray(int location, const QMatrix3x2 *values, int count);
    void setUniformValueArray(int location, const QMatrix3x3 *values, int count);
    void setUniformValueArray(int location, const QMatrix3x4 *values, int count);
    void setUniformValueArray(int location, const QMatrix4x2 *values, int count);
    void setUniformValueArray(int location, const QMatrix4x3 *values, int count);
    void setUniformValueArray(int location, const QMatrix4x4 *values, int count);

    void setUniformValueArray(const char *name, const GLfloat *values, int count, int tupleSize);
    void setUniformValueArray(const char *name, const GLint *values, int count);
    void setUniformValueArray(const char *name, const GLuint *values, int count);
    void setUniformValueArray(const char *name, const QVector2D *values, int count);
    void setUniformValueArray(const char *name, const QVector3D *values, int count);
    void setUniformValueArray(const char *name, const QVector4D *values, int count);
    void setUniformValueArray(const char *name, const QMatrix2x2 *values, int count);
    void setUniformValueArray(const char *name, const QMatrix2x3 *values, int count);
    void setUniformValueArray(const char *name, const QMatrix2x4 *values, int count);
    void setUniformValueArray(const char *name, const QMatrix3x2 *values, int count);
    void setUniformValueArray(const char *name, const QMatrix3x3 *values, int count);
    void setUniformValueArray(const char *name, const QMatrix3x4 *values, int count);
    void setUniformValueArray(const char *name, const QMatrix4x2 *values, int count);
    void setUniformValueArray(const char *name, const QMatrix4x3 *values, int count);
    void setUniformValueArray(const char *name, const QMatrix4x4 *values, int count);

    static bool hasOpenGLShaderPrograms(QOpenGLContext *context = 0);

private Q_SLOTS:
    void shaderDestroyed();

private:
    Q_DISABLE_COPY(QOpenGLShaderProgram)
    Q_DECLARE_PRIVATE(QOpenGLShaderProgram)

    bool init();
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QT_NO_OPENGL

#endif
