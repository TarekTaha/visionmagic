/**************************************************************************
*                         Vision Magic                                    *
*   Copyright (C) 2012 by:                                                *
*      Tarek Taha  <tarek@tarektaha.com>                                  *
*                                                                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not,If not, see                           *
*    <http://www.gnu.org/licenses/>.                                      *
***************************************************************************/
#ifndef SCENEVISUALIZATION_H
#define SCENEVISUALIZATION_H

#include <QtGui>
#include <QtOpenGL>
#include <math.h>
#include <QGLWidget>
#include <QGLFormat>
#include <QTimer>
#include <QWheelEvent>
#include <QInputDialog>
#include <qgl.h>
#include <GL/gl.h>
#include <GL/GLU.h>
#include <cmath>
#include <cv.h>
#include <highgui.h>
#include "dlogger.h"

#define glError()\
                {\
                        GLenum  gl_error = glGetError();\
                        for (; (gl_error); gl_error = glGetError())\
                        {\
                                qDebug()<<"("<<gluErrorString(gl_error)<<") : GLError caught";\
                        }\
                }

class SceneVisualization : public QGLWidget
{
Q_OBJECT
public:
    SceneVisualization(QWidget* parent = 0);
    virtual ~SceneVisualization();
    void setShowGrids(int state);
    QPoint getOGLPos(double x, double y);
    void loadTexture(IplImage *cvimage);
    void saveImage();
    QImage captureMap();
signals:
    void moveMOLeft();
    void moveMORight();
    void moveMOUp();
    void moveMODown();
    void yawMOPos();
    void yawMONeg();
    void nozzelSelected(const QPoint &locationPressed,int);
    void drawTemplateMask(const QPoint &locationPressed);
    void propagateKeyPressEvent(QKeyEvent *e);
public slots:
    void renderFrame(IplImage *cvimage);
    void renderFrameThenRelease(IplImage *cvimage);
    void renderFrame();
private:
    void initializeGL();
    void update();
    QSize sizeHint();
    QSize minimumSizeHint();
    void displayGrid();
    void drawInfoBox();
    void paintGL();
    void resizeGL(int w, int h);
    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *me);
    void mouseDoubleClickEvent(QMouseEvent *me);
    void mouseMoveEvent ( QMouseEvent * me );
    void wheelEvent( QWheelEvent * event );
    void mouseReleaseEvent(QMouseEvent *me);
    void focusInEvent(QFocusEvent *fe);
    void focusOutEvent(QFocusEvent *fe);
    int screenHeight,screenWidth,count;
    void restoreGLState();
    void saveGLState();
    double zoomFactor,xOffset, yOffset, zOffset,yaw, pitch,aspectRatio,fudgeFactor,frameRes;
    bool showGrids,firstTime,maskInitialized,gridInitialized,newFrame,infoBoxDrawn;
    QColor clearColor;
    GLdouble modelMatrix[16];
    double position[3];
    int viewport[4],mapList;
    GLdouble projMatrix[16];
    GLuint texId,layerTexture;;
    double RGB_COLOR[10][3];
    float ratioW, ratioH;
    int newWidth,newHeight;
    IplImage *frameImage;
    GLuint maskTextureList,gridList,cameraFrameList,infoBoxList;
    float chipWidth;
    QMutex mutex;
    QPoint imageSize;
};

#endif // SCENEVISUALIZATION_H
