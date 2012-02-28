/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qopengl_p.h"

#include "qopenglcontext.h"
#include "qopenglfunctions.h"

QT_BEGIN_NAMESPACE

QOpenGLExtensionMatcher::QOpenGLExtensionMatcher(const char *str)
{
    init(str);
}

typedef const GLubyte * (QOPENGLF_APIENTRYP qt_glGetStringi)(GLenum, GLuint);

#ifndef GL_NUM_EXTENSIONS
#define GL_NUM_EXTENSIONS 0x821D
#endif

QOpenGLExtensionMatcher::QOpenGLExtensionMatcher()
{
    const char *extensionStr = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));

    if (extensionStr) {
        init(extensionStr);
    } else {
        // clear error state
        while (glGetError()) {}

        QOpenGLContext *ctx = QOpenGLContext::currentContext();
        if (ctx) {
            qt_glGetStringi glGetStringi = (qt_glGetStringi)ctx->getProcAddress("glGetStringi");

            if (!glGetStringi)
                return;

            GLint numExtensions;
            glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

            for (int i = 0; i < numExtensions; ++i) {
                const char *str = reinterpret_cast<const char *>(glGetStringi(GL_EXTENSIONS, i));

                m_offsets << m_extensions.size();

                while (*str != 0)
                    m_extensions.append(*str++);
                m_extensions.append(' ');
            }
        }
    }
}

void QOpenGLExtensionMatcher::init(const char *str)
{
    m_extensions = str;

    // make sure extension string ends with a space
    if (!m_extensions.endsWith(' '))
        m_extensions.append(' ');

    int index = 0;
    int next = 0;
    while ((next = m_extensions.indexOf(' ', index)) >= 0) {
        m_offsets << index;
        index = next + 1;
    }
}


QT_END_NAMESPACE
