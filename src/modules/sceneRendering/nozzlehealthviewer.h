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
#ifndef NOZZLEHEALTHVIEWER_H
#define NOZZLEHEALTHVIEWER_H

#include "cv.h"
#include "highgui.h"
#include "nhmap.h"
#include <QGraphicsView>
#include <QWheelEvent>
#include <QGLWidget>
#include <QFileInfoList>
#include <QPrintDialog>
#include <QPrinter>
#include <QMessageBox>
#include <QDateTime>
#include "chipgraphicitem.h"
#include "dlogger.h"
#include "acdsettings.h"

class NozzleHealthViewer : public QGraphicsView
{
    Q_OBJECT
public:
    NozzleHealthViewer(QGraphicsView *);
    enum{
        CMYKK,
        YKCKM
    };
public  slots:
            void generateTNFPattern();
            void mergeDNMaps();
            void mergeIntoImage();
            void print();
            void printTNFP();
            void selectionChanged();
            void showInfo(QString msg);
            void viewMap();
            void viewNozzleHealthMap(NozzleHealthMap *);
        signals:
            void warnUser(QString,int);
            void showStatusBarMsg(QString);
protected:
    void keyPressEvent(QKeyEvent *event);
//    void timerEvent(QTimerEvent *event);
//    void mousePressEvent(QMouseEvent *event);
//    void mouseReleaseEvent(QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void scaleView(qreal scaleFactor);
private:
    int timerId;
    QGraphicsScene *scene;
    NozzleHealthMap nhMap, mergedNozzleHealth, *nozzleHealthMaps;
    bool mouseIsPressed;
    QPoint mousePressedLocation;
};

#endif // NOZZLEHEALTHVIEWER_H
