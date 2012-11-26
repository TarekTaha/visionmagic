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
#ifndef CAMERA_H
#define CAMERA_H

#include <QThread>
#include <QString>
#include "cv.h"
#include "highgui.h"
#include "acdexceptions.h"

class AVTCamera;
class OpenCVCamera;
class Camera : public QThread
{
    Q_OBJECT
public:
    enum CameraTypes{
        OPENCV_CAM,
        AVTFIRE,
        NONE
        };
    explicit Camera(QObject *parent = 0);
    virtual QString getCameraName()=0;
    virtual int     getCameraType()=0;
    virtual void enableAutoShutter(bool autoShutter)=0;
    virtual void enableWhiteBalance(bool whiteBalance)=0;
    virtual int  getCameraId()=0;
    virtual bool initializeCamera()=0;
    virtual void startGrabbing()=0;
    virtual void stopGrabbing()=0;
    virtual void stop()=0;
    static Camera* factory(const QString& type, int camId) throw(BadCameraCreation);
public slots:
    virtual void startRecording(QString)=0;
    virtual void stopRecording()=0;
};

#endif // CAMERA_H
