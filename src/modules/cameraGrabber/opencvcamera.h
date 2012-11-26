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
#ifndef OPENCVCAMERA_H
#define OPENCVCAMERA_H
#include <QThread>
#include <QMutexLocker>
#include <QTime>
#include <QMessageBox>
#include "cv.h"
#include "highgui.h"
#include "dlogger.h"
#include "camera.h"

class OpenCVCamera : public Camera
{
    Q_OBJECT
public:
    OpenCVCamera(int camId);
    ~OpenCVCamera();
    int     getCameraId();
    QString getCameraName();
    int     getCameraType();
    void enableAutoShutter(bool autoShutter);
    void enableWhiteBalance(bool whiteBalance);
    bool initializeCamera();
    void startGrabbing();
    void stopGrabbing();
    void stop();
    void run();
public slots:
    void startRecording(QString);
    void stopRecording();
    signals:
        void frameReady(IplImage *);
        void warnUser(QString warning,int);
private:
    CvVideoWriter *writer;
    CvCapture *camera;
    int fps,camId;
    QMutex mutex;
    IplImage *grabbedFrame;
    bool stopThread;
    QTime frameTime;
    bool recordVideo,isInitialized;
    QString videoFileName;
    QString cameraName;
};

#endif // OPENCVCAMERA_H
