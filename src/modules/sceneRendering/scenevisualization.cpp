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
#include "scenevisualization.h"

SceneVisualization::SceneVisualization(QWidget* parent):
        QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::Rgba | QGL::DirectRendering), parent),//QGL::SampleBuffers)DirectRendering
        zoomFactor(500),
        xOffset(10),
        yOffset(2),
        zOffset(0),
        yaw(0),
        pitch(0),
        fudgeFactor(3),
        frameRes(0.25),
        showGrids(true),
        firstTime(true),
        maskInitialized(false),
        gridInitialized(false),
        newFrame(false),
        infoBoxDrawn(false),
        frameImage(NULL),
        chipWidth(21200)
{
    zoomFactor/=1.1;
    clearColor = Qt::white;
    setFocusPolicy(Qt::StrongFocus);
    updateGL();
}

SceneVisualization::~SceneVisualization()
{
    qDebug()<< "SceneVisualization::~SceneVisualization()";
    makeCurrent();
    if(cameraFrameList)
        glDeleteLists(cameraFrameList,1);
    if(gridList)
        glDeleteLists(gridList,1);
    if(frameImage)
        cvReleaseImage(&frameImage);
}

QImage SceneVisualization::captureMap()
{
    return grabFrameBuffer();
}

void SceneVisualization::displayGrid()
{
    gridList = glGenLists(1);
    glNewList(gridList, GL_COMPILE);
        glPushMatrix();
            for(int i=-1000; i <=1000; i+=10)
            {
                glBegin(GL_LINES);
                if(i==0)
                {
                    glColor4f(0,0,0,0.5);
                }
                else
                {
                    glColor4f(0.8,0.8,0.8,0.5);
                }
                glVertex3f(-1000, i, 0);
                glVertex3f(1000, i, 0);
                glVertex3f(i,-1000, 0);
                glVertex3f(i, 1000, 0);
                glEnd();
            }
        glPopMatrix();
    glEndList();
    gridInitialized  = true;
}

void SceneVisualization::drawInfoBox()
{
    infoBoxList = glGenLists(1);
    glNewList(infoBoxList, GL_COMPILE);
    glPushMatrix();
        glColor4f(0.0f,0.0f,0.0f,0.5f);
        glLineWidth(2);
        glBegin(GL_LINE_STRIP);
            glVertex3f(-aspectRatio+0.2,0.8,0);
            glVertex3f(-aspectRatio+0.2,0.99,0);
            glVertex3f(aspectRatio-0.2,0.99,0);
            glVertex3f(aspectRatio-0.2,0.8,0);
            glVertex3f(-aspectRatio+0.2,0.8,0);
        glEnd();

        glLineWidth(1);

        glColor4f(0.3f,0.4f,0.6f,0.5f);
        glBegin(GL_QUADS);
            glVertex3f(-aspectRatio+0.2,0.8,0.5);
            glVertex3f(aspectRatio-0.2,0.8,0.5);
            glVertex3f(aspectRatio-0.2,0.99,0.5);
            glVertex3f(-aspectRatio+0.2,0.99,0.5);
        glEnd();
   glPopMatrix();

   glEndList();
   infoBoxDrawn  = true;
}

void SceneVisualization::focusInEvent(QFocusEvent *)
{
    makeCurrent();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    updateGL();
}

void SceneVisualization::focusOutEvent(QFocusEvent *)
{
    makeCurrent();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    updateGL();
}

QPoint SceneVisualization::getOGLPos(double x, double y)
{
    QPoint retval;
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    winX = x;
    winY = (float)viewport[3] - y;
    glReadPixels( (int)x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
    gluUnProject( winX, winY, winZ, modelMatrix, projMatrix, viewport, &posX, &posY, &posZ);
    position[0] = posX;
    position[1] = posY;
    retval.setX(int(imageSize.x()/2.0) + int(ceil(position[0])));
    retval.setY(int(imageSize.y()/2.0) - int(ceil(position[1])));
    qDebug()<<"Translated to x:"<<retval.x()<<" y:"<<retval.y()<<" z:"<<posZ;
    return retval;
}

void SceneVisualization::initializeGL()
{
    qDebug()<< "SceneVisualization::initializeGL()";

    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glFlush();

    glInitNames();
}

void SceneVisualization::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_C)
    {
        if(e->modifiers() && Qt::ShiftModifier)
        {

        }
        else
        {

        }
    }
    else if(e->key() == Qt::Key_W)
    {
        if(e->modifiers() && Qt::ShiftModifier)
        {
            emit moveMOUp();
        }
        else
        {
            yOffset += 0.01*zoomFactor;
//            qDebug()<<"Y offset is: "<<yOffset;
        }
    }
    else if(e->key() == Qt::Key_S)
    {
        if(e->modifiers() && Qt::ShiftModifier)
        {
            emit moveMODown();
        }
        else
        {
            yOffset -= 0.01*zoomFactor;
//            qDebug()<<"Y offset is: "<<yOffset;
        }
    }
    else if(e->key() == Qt::Key_A)
    {
        if(e->modifiers() && Qt::ShiftModifier)
        {
            emit moveMOLeft();
        }
        else
        {
            xOffset -= 0.01*zoomFactor;
//            qDebug()<<"X offset is: "<<xOffset;
        }
    }
    else if(e->key() == Qt::Key_D)
    {
        if(e->modifiers() && Qt::ShiftModifier)
        {
            emit moveMORight();
        }
        else
        {
            xOffset += 0.01*zoomFactor;
//            qDebug()<<"X offset is: "<<xOffset;
        }
    }
    else if(e->key() == Qt::Key_BracketLeft)
    {
        zoomFactor *= 1.1;
    }
    else if(e->key() == Qt::Key_BracketRight)
    {
        zoomFactor /= 1.1;
    }
    else if(e->key() == Qt::Key_Left)
    {
        if(e->modifiers() && Qt::ShiftModifier)
        {
            emit yawMOPos();
        }
        else
        {
            yaw += 5;
        }
    }
    else if(e->key() == Qt::Key_Right)
    {
        if(e->modifiers() && Qt::ShiftModifier)
        {
            emit yawMONeg();
        }
        else
        {
            yaw -= 5;
        }
    }
    else if(e->key() == Qt::Key_Up)
    {
        pitch += 5;
    }
    else if(e->key() == Qt::Key_Down)
    {
        pitch -= 5;
    }
    else if(e->key() == Qt::Key_R)
    {
        zoomFactor = 500;
        fudgeFactor = 3;
        xOffset= yOffset=zOffset=yaw=pitch=0;
    }
    else if(e->text() == "=")
    {
        fudgeFactor *=1.25;
    }
    else if(e->text()=="-")
    {
        fudgeFactor /=1.25;
    }
    else if(e->text() == "0")
    {
        fudgeFactor=3;
    }
    else
    {
        emit propagateKeyPressEvent(e);
    }
    qDebug()<<"Fudge factor set to:"<< fudgeFactor;
    updateGL();
}

void SceneVisualization::loadTexture(IplImage *cvimage)
{
    qDebug()<< "oldW:"<<cvimage->width << " oldH:"<<cvimage->height;
    newWidth =  (int) pow(2.0f, (int)ceil(log((float)cvimage->width) / log(2.f)));
    newHeight = (int) pow(2.0f, (int)ceil(log((float)cvimage->height) / log(2.f)));
    ratioW  = ((float) cvimage->width)/newWidth;
    ratioH  = ((float) cvimage->height)/newHeight;
    qDebug()<<"MW:"<< newWidth<<" MH:"<< newHeight<<" RatioW:"<< ratioW<<" RatioH:"<<ratioH;
    char * scaledData;
    scaledData = cvimage->imageData;
    QImage image;
    texId = bindTexture(image,GL_TEXTURE_2D);
    // Enable Texture Mapping
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cvimage->width, cvimage->height, 0, GL_RGBA, GL_BYTE, scaledData);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glDisable(GL_TEXTURE_2D);
//    delete[] scaledData;
}

QSize SceneVisualization::minimumSizeHint()
{
    return QSize(800,600);
}

void SceneVisualization::mouseDoubleClickEvent(QMouseEvent *me)
{
//    QPoint p(me->x(),me->y());
//    qDebug()<< "Mouse Double click x:"<< p.x() <<" y:" << p.y();
//    p = getOGLPos(p.x(),p.y());
//    emit nozzelSelected(p,me->button());
//    if(me->buttons()==Qt::RightButton)
//    {
//
//    }
}

void SceneVisualization::mouseMoveEvent ( QMouseEvent * me )
{
    QPoint p;
    p = getOGLPos(me->x(),me->y());
    emit drawTemplateMask(p);
    update();
}

void SceneVisualization::mousePressEvent(QMouseEvent *me)
{
    QPoint p;
    p = getOGLPos(me->x(),me->y());
    emit nozzelSelected(p,me->button());
    if(me->buttons()==Qt::RightButton)
    {
    }
}

void SceneVisualization::mouseReleaseEvent(QMouseEvent *)
{
}

void SceneVisualization::paintGL()
{
    qglClearColor(clearColor);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLoadIdentity();
    //    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    //    glDisable(GL_BLEND);
    //    glBlendFunc(GL_ONE, GL_ONE);
    //    glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA);
    //    glBlendFunc(GL_SRC_COLOR, GL_SRC_ALPHA);
    //    glBlendFunc(GL_DST_COLOR, GL_DST_ALPHA);
    //    glBlendFunc(GL_DST_COLOR, GL_ZERO);

    glEnable(GL_DEPTH_TEST);

    //    glDisable( GL_DEPTH_TEST );
    //    glDisable( GL_LIGHTING );
    //    glDisable(GL_DEPTH_TEST);
    //    glEnable(GL_POINT_SMOOTH);
    //    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    //    glEnable(GL_LINE_SMOOTH);
    //    glEnable(GL_POLYGON_SMOOTH);
    /*
    */
    glPushMatrix();
    glColor4f(0.0f,0.0f,0.0f,0.5f);
    glScalef(1/zoomFactor, 1/zoomFactor, 1/zoomFactor);
    glRotatef(pitch,1,0,0);
    glRotatef(yaw,0,0,1);
    glTranslatef(xOffset, yOffset, zOffset);

    glGetDoublev(GL_MODELVIEW_MATRIX,modelMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX,projMatrix);
    glGetIntegerv(GL_VIEWPORT,viewport);

    if(newFrame)
    {
        glDeleteLists(cameraFrameList,1);
        renderFrame();
        newFrame = false;
    }

    glCallList(cameraFrameList);

    glPopMatrix();
//    if(!infoBoxDrawn)
//        drawInfoBox();
//    glCallList(infoBoxList);
    glColor4f(1.0f,1.0f,0.0f,1.0f);
    QFont serifFont("Times", 10, QFont::Bold);
    renderText(10,20,QString("Shortcut Keys"),serifFont);
    renderText(10,40,QString("X: analyse Image - Space: skip Image - N: show nozzle number - C: show/hide clear nozzles - B: show/hide blocked nozzles - Arrows: rotate image"),serifFont);
    renderText(10,60,QString("Mouse Wheel: Zoom digital in/out - Mouse left click: toggle nozzle state - Mouse right click: remove misplaced nozzle"),serifFont);
}

void SceneVisualization::saveGLState()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}

void SceneVisualization::restoreGLState()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}

void SceneVisualization::renderFrameThenRelease(IplImage *cvimage)
{
    QMutexLocker locker(&mutex);
    if(!cvimage)
        return;
    frameImage = cvimage;
    imageSize.setX(frameImage->width);
    imageSize.setY(frameImage->height);
    newFrame = true;
    updateGL();
}

void SceneVisualization::renderFrame(IplImage *cvimage)
{
    QMutexLocker locker(&mutex);
    if(frameImage)
        cvReleaseImage(&frameImage);
    if(cvimage->nChannels==3)
    {
        frameImage = cvCloneImage(cvimage);
    }
    else
    {
        frameImage = cvCreateImage(cvGetSize(cvimage),cvimage->depth,3);
        cvCvtColor(cvimage,frameImage,CV_GRAY2RGB);
    }
    imageSize.setX(frameImage->width);
    imageSize.setY(frameImage->height);
    newFrame = true;
    updateGL();
}

void SceneVisualization::renderFrame()
{
    if(!frameImage)
        return;
    if(!frameImage->imageData)
        return;
    int width     = frameImage->width;
    int widthStep = frameImage->widthStep;
    int height    = frameImage->height;
    if(strcmp("BGR",frameImage->channelSeq) == 0)
    {
        cvCvtColor(frameImage,frameImage,CV_BGR2RGB);
    }
//    qDebug()<< "oldW:"<<width << " oldH:"<<height<<" Num channels:"<<frameImage->nChannels<<" widthStep:"<<widthStep;
    newWidth =  (int) pow(2.0f, (int)ceil(log((float)width) / log(2.f)));
    newHeight = (int) pow(2.0f, (int)ceil(log((float)height) / log(2.f)));
    ratioW  = ((float) width)/newWidth;
    ratioH  = ((float) height)/newHeight;
    int startX = (int)ceil((newWidth - width)/2.0f);
    int startY = (int)ceil((newHeight - height)/2.0f);
//    qDebug()<<"MW:"<< newWidth<<" MH:"<< newHeight<<" RatioW:"<< ratioW<<" RatioH:"<<ratioH;
    // getting the image back from image coordinate to orthogonal coordinate
    // this is equivalent to the freaking glScalef(1,-1,1) which doesn't
    // work for some fucking reason
    /* OpenCV image is stored with 3 byte color pixels (3 channels). We convert it to a 32 bit depth RGBA image  */
    unsigned char * buffer = (unsigned char*) calloc(newWidth*newHeight*4, sizeof(unsigned char));
    const uchar *iplImagePtr = (const uchar *)frameImage->imageData;
    int bufferX,bufferY;
    for(int i=0;i<width;i++)
    {
        for(int j=0;j<height;j++)
        {
            bufferX = i + startX;
            bufferY = j + startY;
            buffer[(bufferY*newWidth + bufferX)*4 + 0] = iplImagePtr[(height- j -1)*widthStep + (i)*3 + 0];
            buffer[(bufferY*newWidth + bufferX)*4 + 1] = iplImagePtr[(height- j -1)*widthStep + (i)*3 + 1];
            buffer[(bufferY*newWidth + bufferX)*4 + 2] = iplImagePtr[(height- j -1)*widthStep + (i)*3 + 2];
            buffer[(bufferY*newWidth + bufferX)*4 + 3] = 255;
        }
    }
    cameraFrameList = glGenLists(1);
    glNewList(cameraFrameList, GL_COMPILE);
    glDeleteTextures(1,&layerTexture);
    glGenTextures(1, &layerTexture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
//    glScalef(1,-1,1);
    glTranslatef(-(newWidth)/2.0f,-(newHeight)/2.0f,0);
    glBindTexture(GL_TEXTURE_2D, layerTexture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,buffer);
    glBindTexture(GL_TEXTURE_2D, layerTexture);
    glError()
    free(buffer);
    // we don't need the opencvImage anymore
    // it now exists as a opengl list/texture
    cvReleaseImage(&frameImage);
    //  The first value of glTexCoord2f is the X coordinate. 0.0f is the left side of the texture. 0.5f is the middle of the texture
    //  and 1.0f is the right side of the texture. The second value of glTexCoord2f is the Y coordinate. 0.0f is the bottom of the texture
    //  0.5f is the middle of the texture, and 1.0f is the top of the texture.

    QPointF buttomLeft,buttomRight,topRight,topLeft;
    buttomLeft.setX(0*newWidth);   buttomLeft.setY(0.0);
    buttomRight.setX(newWidth);    buttomRight.setY(0.0);
    topRight.setX(newWidth);       topRight.setY(newHeight);
    topLeft.setX(0*newWidth);      topLeft.setY(newHeight);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0,0.0);  glVertex2f(buttomLeft.x() ,buttomLeft.y());
        glTexCoord2f(1.0,0.0);  glVertex2f(buttomRight.x(),buttomRight.y());
        glTexCoord2f(1.0,1.0);  glVertex2f(topRight.x()   ,topRight.y());
        glTexCoord2f(0.0,1.0);  glVertex2f(topLeft.x()    ,topLeft.y());
    glEnd();
    glError()
    glPopMatrix();
    glEndList();
    glDisable(GL_TEXTURE_2D);
}

void SceneVisualization::resizeGL(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    aspectRatio = ((float) w)/((float) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(180, aspectRatio, 10,100);
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-aspectRatio, aspectRatio, -1, 1, -1, 1);
//    glOrtho(0, aspectRatio, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    updateGL();
}

void SceneVisualization::saveImage()
{
    bool ok;
    QString filename = QInputDialog::getText(this, "Image Capture","Enter a name for the Image:", QLineEdit::Normal,
                                             QString::null, &ok);
    const char * type = "PNG";
    if(ok && !filename.isEmpty())
    {
        QImage capturedMap = this->captureMap();
        capturedMap.save(filename,type,-1);
    }
}

void SceneVisualization::setShowGrids(int state)
{
    if(state==0)
    {
        showGrids = false;
    }
    else
    {
        showGrids = true;
    }
    update();
}

QSize SceneVisualization::sizeHint()
{
    return QSize(800,600);
}

void SceneVisualization::update()
{
    this->updateGL();
}

void SceneVisualization::wheelEvent( QWheelEvent * event )
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    if(numSteps > 0)
        zoomFactor /= 1.1;
    else
        zoomFactor *= 1.1;
    updateGL();
}
