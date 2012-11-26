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
#include "opencvcamera.h"

OpenCVCamera::OpenCVCamera(int _camId):
    camId(_camId)
{
    isInitialized = initializeCamera();
}

OpenCVCamera::~OpenCVCamera()
{
    if(camera)
        cvReleaseCapture(&camera);
}

void OpenCVCamera::enableAutoShutter(bool autoShutter)
{
    //TODO::find a way to properly support it
    qDebug()<<"Not Supported with OPENCV interface YET!";
}

void OpenCVCamera::enableWhiteBalance(bool whiteBalance)
{
    //TODO::find a way to properly support it
    qDebug()<<"Not Supported with OPENCV interface YET!";
}

int OpenCVCamera::getCameraId()
{
    return camId;
}

QString OpenCVCamera::getCameraName()
{
    return cameraName;
}

int OpenCVCamera::getCameraType()
{
    return OPENCV;
}

bool OpenCVCamera::initializeCamera()
{
    if(!(camera  = cvCreateCameraCapture(camId)))
    {
        return false;
    }
    // The name of opencvCamears for now is defined by the Id
    cameraName = QString("OPENCV_").arg(camId);
    return true;
}
/*
  The grabbed images are supplied for various processing part:
    - Image display (Live view)
    - Image openGL (focused image)
    - Autofocus
    - Image acquisition
    - Encap Measurement

  An appox processing time per image is 75 msec, providing images faster
  than this time will cause the image processing threads to slow down
  and the whole UI to be less responsive.

  A sync timer is added below to force a sync of 80msec when grabbing images
  faster than 80msec.
  13 fps = 1000/13 approx 77 , so make it 80 msec
*/
void OpenCVCamera::run()
{
    frameTime.start();
    stopThread = false;
    int sycTime = 80;
    QTime timeSinceLastGrab;
    timeSinceLastGrab.restart();
    int elapsedMs;
    while(!stopThread)
    {
        if(!isInitialized)
        {
            // Camera not initialized yet
            msleep(10);
            continue;
        }
        emit frameReady(cvQueryFrame(camera));
        msleep(sycTime);
        elapsedMs = frameTime.restart();
        fps = qRound(1000/elapsedMs);
        if(recordVideo)
            cvWriteFrame(writer,cvQueryFrame(camera));
    }
    cvReleaseVideoWriter(&writer);
}

void OpenCVCamera::startRecording(QString videoFile)
{
    QMutexLocker locker(&mutex);
    if(writer)
    {
        cvReleaseVideoWriter(&writer);
    }
    if(grabbedFrame)
        writer = cvCreateVideoWriter(videoFile.toStdString().c_str(),CV_FOURCC_PROMPT,fps,cvSize(grabbedFrame->width,grabbedFrame->height),(grabbedFrame->nChannels==3)?1:0);
    if(!writer)
    {
        this->recordVideo   = false;
        emit warnUser(QString("Unable to creat a video file with the selected Codec!"),QMessageBox::Warning);
    }
    else
    {
        this->recordVideo   = true;
    }
    this->videoFileName = videoFile;
    qDebug()<<"Starting Video recording";
}

void OpenCVCamera::stopRecording()
{
    QMutexLocker locker(&mutex);
    qDebug()<<"Stopping Video recording";
    this->recordVideo   = false;
    if(writer)
    {
        cvReleaseVideoWriter(&writer);
    }
}

void OpenCVCamera::startGrabbing()
{
}

void OpenCVCamera::stopGrabbing()
{
}

void OpenCVCamera::stop()
{
    QMutexLocker lock(&mutex);
    stopThread = true;
}
