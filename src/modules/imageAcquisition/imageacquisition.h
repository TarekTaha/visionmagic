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

#ifndef IMAGEACQUISITION_H_
#define IMAGEACQUISITION_H_

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
#include "encapsulationcoverage.h"

class ImageAcquisition : public QThread
{
    Q_OBJECT
public:
    ImageAcquisition(QWidget *parent = 0,MotorControl *motorController=0,EncapsulationCoverage *encapCoverage=0);
    virtual ~ImageAcquisition();
    void enableOnlineEncapAnalysis(bool);
    void captureImage();
    void run();
    void stop();
    bool isAcquiring();
    void setHomePose(int pose);
    void setPrintHeadCode(QString code);
public slots:
    void isFocused(bool);
    void motorMovingChanged(int,bool,int);
    void recieveImage(IplImage *frameImage);
    void setAcquisition(bool);
    void shutterSpeedChanged(int);
signals:
    void acquisitionEnded(bool encapAnalysisEnabled);
    void lastImageReached();
    void newFocus();
    void progressValue(int);
    void savingImages();
    void setShutterSpeed(int);
    void warnUser(QString,int);
private:
    void saveImageBuffer();
    void releaseImageBuffer();
    QWidget * parent;
    CvCapture * camera;
    MotorControl * motorController;
    EncapsulationCoverage *encapCoverage;
    void getAppendString();
    IplImage *frameImage;
    QDir imageDir;
    QString   imageName;
    QDateTime dateTime,date;
    bool stopThread,acquireImages,imageFocused,newStart,motorMoving,gotNewFrame,firstStep,encapAnalysisEnabled,repeatAnalysis;
    int motorPose,lastPose,numImages,currentChip,linearMotorPose,cameraMotorPose,homePose,shutterSpeed,actualShutterSpeed;
    QMutex mutex;
    QReadWriteLock readWriteLock;
    QString printHeadCode,relativePath;
    QVector <QString> namesBuffer;
    QVector <IplImage *> framesBuffer;
    QTime timer;
};

#endif /* IMAGEACQUISITION_H_ */
