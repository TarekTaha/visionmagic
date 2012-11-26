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
#include "QOpenCVWidget.h"

// Constructor
QOpenCVWidget::QOpenCVWidget(QWidget *parent) :
        QWidget(parent),
        width(100),
        height(100),
        zoomFactor(0.7),
        roiSelectionStarted(false),
        roiSelected(false),
        renderText(true),
        refLineSelectionState(NOTHING_SELECTED),
        refLineSelected(false),
        enableRefLineSelection(false),
        enableROISelection(false),
        image2Draw_mat(NULL)
{
    resetView();
}

QOpenCVWidget::~QOpenCVWidget(void)
{
}

void QOpenCVWidget::keyPressEvent( QKeyEvent *e )
{
    switch( e->key() )
    {
    case Qt::Key_Up:
        break;
    case Qt::Key_Down:
        break;
    case Qt::Key_Right:
        break;
    case Qt::Key_Left:
        break;
    case Qt::Key_Space:
        break;
    }
}

void QOpenCVWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    int w,h;

    if(!image2Draw_mat)
        return;
    w = image2Draw_mat->cols;
    h= image2Draw_mat->rows;

    int offsetX = qRound((width/2.0)  - (w/2.0)*zoomFactor);
    int offsetY = qRound((height/2.0)  - (h/2.0)*zoomFactor);
    if(event->button() == Qt::LeftButton && enableROISelection)
    {
        roiSelectionStarted = true;
        roiSelected = false;
        roiStart = p;
        roiStart.setX((roiStart.x() - offsetX)/zoomFactor);
        roiStart.setY((roiStart.y() - offsetY)/zoomFactor);
        setMouseTracking(true);
    }
    else if(event->button() == Qt::RightButton && enableRefLineSelection)
    {
        if(refLineSelectionState == NOTHING_SELECTED)
        {
            refLinePoint1 = p;
            refLinePoint1.setX((refLinePoint1.x() - offsetX)/zoomFactor);
            refLinePoint1.setY((refLinePoint1.y() - offsetY)/zoomFactor);
            refLineSelectionState = POINT_ONE_SELECTED;
        }
        else if(refLineSelectionState == POINT_ONE_SELECTED)
        {
            refLinePoint2 = p;
            refLinePoint2.setX((refLinePoint2.x() - offsetX)/zoomFactor);
            refLinePoint2.setY((refLinePoint2.y() - offsetY)/zoomFactor);
            refLineSelectionState = POINT_TWO_SELECTED;
            refLineSelected  = true;
            emit referenceLineSelected(refLinePoint1,refLinePoint2,refLineSelected);
        }
        else if(refLineSelectionState == POINT_TWO_SELECTED)
        {
            refLineSelectionState = NOTHING_SELECTED;
            refLineSelected  = false;
            emit referenceLineSelected(refLinePoint1,refLinePoint2,refLineSelected);
        }
        update();
    }
}

void QOpenCVWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int w,h;
    if(!image2Draw_mat)
        return;
    w = image2Draw_mat->cols;
    h= image2Draw_mat->rows;

    QPoint p = event->pos();
    if(event->button() == Qt::LeftButton && enableROISelection)
    {
        int offsetX = qRound((width/2.0)  - (w/2.0)*zoomFactor);
        int offsetY = qRound((height/2.0)  - (h/2.0)*zoomFactor);
        QPoint pTransf((p.x() - offsetX)/zoomFactor,(p.y() - offsetY)/zoomFactor);
        if(roiSelectionStarted && !(roiStart.x()==pTransf.x() && roiStart.y()==pTransf.y()))
        {
            roiEnd = pTransf;
            roiSelected = true;
            QPoint temp;
            if(roiStart.x()>roiEnd.x() && roiStart.y()>roiEnd.y())
            {
                temp     = roiStart;
                roiStart = roiEnd;
                roiEnd   = temp;
            }
            emit roiChanged(roiStart,roiEnd);
        }
        else
            roiSelected = false;
        roiSelectionStarted = false;
        setMouseTracking(false);
    }
    if(imageSize.x()==0 && imageSize.y()==0)
        return;
    // 9 Pixels is tha margin of the image
    p.setX(p.x()-9);
    emit nozzelSelected(p,imageSize,event->button());
}

void QOpenCVWidget::mouseMoveEvent(QMouseEvent *event)
{
    int w,h;
    if(!image2Draw_mat)
        return;
    w = image2Draw_mat->cols;
    h= image2Draw_mat->rows;

    QPoint p = event->pos();
    if(roiSelectionStarted && !(roiStart.x()==p.y() || roiStart.y()==p.y()))
    {
        roiEnd = p;
        roiSelected = true;
        int offsetX = qRound((width/2.0)  - (w/2.0)*zoomFactor);
        int offsetY = qRound((height/2.0)  - (h/2.0)*zoomFactor);
        roiEnd.setX((roiEnd.x() - offsetX)/zoomFactor);
        roiEnd.setY((roiEnd.y() - offsetY)/zoomFactor);
    }
    update();
    // 9 Pixels is tha margin of the image
    p.setX(p.x()-9);
    emit drawTemplateMask(p,imageSize);
}

void QOpenCVWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setViewport(0,0,width,height);
    painter.save();
    QPointF translatePose;

    if(image2Draw_mat)
    {
        Mat temp(image2Draw_mat);
        if(roiSelected)
        {
            rectangle(temp,Point(roiStart.x(),roiStart.y()),Point(roiEnd.x(),roiEnd.y()),Scalar(255,0,0),2);
        }
        if(refLineSelectionState == POINT_ONE_SELECTED)
        {
            circle(temp,Point(qRound(refLinePoint1.x()),qRound(refLinePoint1.y())),2,CV_RGB(0,0,255),2);
        }
        else if(refLineSelectionState == POINT_TWO_SELECTED)
        {
            circle(temp,Point(qRound(refLinePoint1.x()),qRound(refLinePoint1.y())),2,CV_RGB(0,0,255),2);
            circle(temp,Point(qRound(refLinePoint2.x()),qRound(refLinePoint2.y())),2,CV_RGB(0,0,255),2);
            line(temp,Point(refLinePoint1.x(),refLinePoint1.y()),Point(refLinePoint2.x(),refLinePoint2.y()),CV_RGB(0,255,0),2);
        }
        translatePose.setX((width/2.0)  - (image2Draw_mat->cols/2.0)*zoomFactor);
        translatePose.setY((height/2.0) - (image2Draw_mat->rows/2.0)*zoomFactor);
        painter.translate(translatePose);
        painter.scale(zoomFactor,zoomFactor);
        painter.drawImage(0, 0, image2Draw);
    }
    else
    {
        QPixmap tempPixMap = QPixmap::fromImage(QImage(":/SBR_logo"));
        painter.translate(width/2.0 - tempPixMap.width()/2.0,height/2.0 - tempPixMap.height()/2.0);
        painter.drawPixmap(0, 0, tempPixMap);
    }
    painter.restore();
    // draw the overlayed text using QPainter
    if(renderText)
    {
        painter.setPen(QColor(197, 197, 197, 157));
        painter.setBrush(QColor(197, 197, 197, 127));
        painter.drawRect(QRect(0, 0, painter.window().width(), 50));
        painter.setPen(Qt::black);
        painter.setBrush(Qt::NoBrush);
        const QString str1(tr("Mouse | Wheel: zoom in/out"));
        const QString str2(tr("Shortcut Keys | A: move Left - D: move Right - W: zoom Up - S: zoom Down - Home: first die - End: last die"));
        QFontMetrics fm(painter.font());
        painter.drawText(painter.window().width()/2 - fm.width(str1)/2, 20, str1);
        painter.drawText(painter.window().width()/2 - fm.width(str2)/2, 20 + fm.lineSpacing(), str2);
    }
}

void QOpenCVWidget::displayImage(Mat _mat)
{
    CvMat c_img = _mat;
    int origin=0;
    if( CV_IS_IMAGE_HDR(&c_img ))
        origin = ((IplImage*)&c_img)->origin;

    if(!image2Draw_mat)
    {
        image2Draw_mat = cvCreateMat( c_img.rows, c_img.cols, CV_8UC3 );
    }
    else if (!CV_ARE_SIZES_EQ(image2Draw_mat,&c_img))
    {
        cvReleaseMat(&image2Draw_mat);
        image2Draw_mat = cvCreateMat( c_img.rows, c_img.cols, CV_8UC3 );
        updateGeometry();
    }
    image2Draw = QImage((const unsigned char *) image2Draw_mat->data.ptr,image2Draw_mat->cols,image2Draw_mat->rows, image2Draw_mat->step,QImage::Format_RGB888);
    //image2Draw = image2Draw.rgbSwapped();
    cvConvertImage(&c_img,image2Draw_mat,(origin != 0 ? CV_CVTIMG_FLIP : 0) + CV_CVTIMG_SWAP_RB );
    update();
}

void QOpenCVWidget::displayImageThenRelease(IplImage *cvimage)
{
    if(cvimage)
    {
        recieveFrame(cvimage);
        cvReleaseImage(&cvimage);
    }
    else
    {
        qDebug() << "QOpenCVWidget:: Null Image recieved";
    }
}

void QOpenCVWidget::displayImage(IplImage *cvimage)
{
    if (!cvimage)
    {
        return;
    }
    if(rgbImage)
    {
        image2Release = rgbImage;
    }
    if(cvimage->nChannels==3)
    {
        rgbImage = cvCloneImage(cvimage);
    }
    else
    {
        rgbImage = cvCreateImage(cvGetSize(cvimage),cvimage->depth,3);
        cvCvtColor(cvimage,rgbImage,CV_GRAY2RGB);
    }
    if(!rgbImage)
        return;
    centerPoint.setX(int(rgbImage->width/2.0F));
    centerPoint.setY(int(rgbImage->height/2.0f));
    if(rgbImage->nChannels==3)
    {
        if(strcmp("BGR",rgbImage->channelSeq) == 0)
        {
            cvCvtColor(rgbImage,rgbImage,CV_BGR2RGB);
        }
        update();
    }
    if(image2Release)
        cvReleaseImage(&image2Release);
}

void QOpenCVWidget::resetView()
{
    update();
}

void QOpenCVWidget::resizeEvent(QResizeEvent * s)
{
    width = s->size().width();
    height= s->size().height();
    update();
}

void QOpenCVWidget::setEnableRefLineSelection(bool state)
{
    enableRefLineSelection = state;
}

void QOpenCVWidget::setEnableROISelection(bool state)
{
    enableROISelection = state;
}

void QOpenCVWidget::setRenderText(bool status)
{
    renderText = status;
}

void QOpenCVWidget::wheelEvent( QWheelEvent * event )
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    if(numSteps > 0)
        zoomFactor /= 1.1;
    else
        zoomFactor *= 1.1;
    if(image2Draw_mat)
    {
        imageSize.setX(image2Draw_mat->cols*zoomFactor);
        imageSize.setY(image2Draw_mat->rows*zoomFactor);
    }
}

void QOpenCVWidget::recieveFrame(IplImage *newFrame)
{
    receivedMat = cvarrToMat(newFrame,true);
    displayImage(receivedMat);
}


