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

#ifndef ENCAPDELAMINSPECTION_H
#define ENCAPDELAMINSPECTION_H

#include <QThread>
#include <QDir>
#include <QDateTime>
#include <QString>
#include <QMutex>
#include <QInputDialog>
#include <QImage>
#include <QTime>
#include <QReadWriteLock>
#include <QWriteLocker>
#ifndef _TTY_WIN_
    #include <Qt3Support>
#endif
#include "motorcontrol.h"
#include "cv.h"
#include "highgui.h"
#include "acdsettings.h"
#include <QKeyEvent>

class EncapDelamInspection : public QThread
{
    Q_OBJECT
public:
    EncapDelamInspection(QWidget *parent = 0,MotorControl *motorController=0);
    virtual ~EncapDelamInspection();
    void enableOnlineEncapAnalysis(bool);
    void captureImage();
    void run();
    void stop();
    bool isInspecting();
    void setHomePose(int pose);
    void setPrintHeadCode(QString code);
public slots:
    void isFocused(bool);
    void motorMovingChanged(int,bool,int);
    void recieveImage(IplImage *frameImage);
    void setInspection(bool);
    void shutterSpeedChanged(int);
    void keyPressed(QKeyEvent *e);
    void setSectionsPerDie(int sections);
    void setNextPositionIndex(int);
signals:
    void encapDelamInspectionEnded();
    void lastImageReached();
    void newFocus();
    void progressValue(int);
    void savingImages();
    void setShutterSpeed(int);
    void warnUser(QString,int);
    void newImagePositions(QVector<int>);
    void locationChanged(int imagePosition,int currentChip,int currentSection);
private:
    void saveImageBuffer();
    void releaseImageBuffer();
    QWidget * parent;
    CvCapture * camera;
    MotorControl * motorController;
    void getAppendString();
    IplImage *frameImage;
    QDir imageDir;
    QString   imageName;
    QDateTime dateTime,date;
    bool stopThread,inspectImages,imageFocused,newStart,motorMoving,gotNewFrame,firstStep;
    int motorPose,lastPose,numImages,currentChip,linearMotorPose,cameraMotorPose,homePose,shutterSpeed,actualShutterSpeed,currentPositionIndex,
        nextPositionIndex,sectionsPerDie,currentDieSection;
    QMutex mutex;
    QReadWriteLock readWriteLock;
    QString printHeadCode,relativePath;
    QVector <QString> namesBuffer;
    QVector <IplImage *> framesBuffer;
    QVector <int> imagePositions;
    QTime timer;
};

#endif /* ENCAPDELAMINSPECTION_H */
