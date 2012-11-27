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
#ifndef QOPENCVWIDGET_H
#define QOPENCVWIDGET_H

#include <cv.h>
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QImage>
#include <QKeyEvent>
#include <QTime>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include "highgui.h"
#include "dlogger.h"

using namespace cv;

class QOpenCVWidget : public QWidget
{
    enum REF_LINE_SEL_STATE{
        POINT_ONE_SELECTED,
        POINT_TWO_SELECTED,
        NOTHING_SELECTED
    };
    Q_OBJECT
public:
    QOpenCVWidget(QWidget *parent = 0);
    ~QOpenCVWidget(void);
    void setRenderText(bool);
    void setEnableRefLineSelection(bool);
    void setEnableROISelection(bool);
public slots:
    void displayImage(Mat _mat);
    void displayImage(IplImage *image);
    void displayImageThenRelease(IplImage *);
    void recieveFrame(IplImage *);
    void resetView();
signals:
    void nozzelSelected(const QPoint &locationPressed,const QPoint &imageSize,int);
    void drawTemplateMask(const QPoint &locationPressed,const QPoint &imageSize);
    void roiChanged(QPoint start,QPoint end);
    void referenceLineSelected(QPoint start,QPoint end, bool isSelected);
private:
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent( QWheelEvent * event );
    void paint(QPainter *);
    QImage image2Draw;
    virtual void resizeEvent(QResizeEvent * s);
    int width,height;
    QPoint imageSize,centerPoint;
    IplImage *rgbImage,*image2Release;
    double zoomFactor,scale;
    bool roiSelectionStarted,roiSelected,renderText;
    QPoint roiStart,roiEnd;
    QPoint refLinePoint1,refLinePoint2;
    int refLineSelectionState;
    bool refLineSelected;
    bool enableRefLineSelection,enableROISelection;
    Mat receivedMat;
    CvMat* image2Draw_mat;
protected:
    void paintEvent(QPaintEvent *event);
};

#endif
