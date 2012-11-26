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
#include "cameragrabber.h"
#include <QMessageBox>
CameraGrabber::CameraGrabber():
    activeCameraChanged(false),
    sameCameraDiscriminator(0)
{
}

CameraGrabber::~CameraGrabber()
{
    for(int i=0;i<availableCameras.size();i++)
    {
        while(availableCameras[i]->isRunning())
        {
            availableCameras[i]->stop();
            msleep(10);
        }
        delete availableCameras[i];
    }
}

void CameraGrabber::addSettingsConnection(const QObject *listener,const char * signal, const char *slot,int type, int _camId)
{
    settingsConnections.push_back(SettingsConnection(listener,signal,slot,type,_camId));
}

void CameraGrabber::addFrameReceiver(const QObject *listener,int _camId)
{
    cameraFrameReceivers[listener] = _camId;
}

void CameraGrabber::enableAutoShutter(bool autoShutter)
{
    Camera *actvC = getActiveCamera();
    if(actvC)
        actvC->enableAutoShutter(autoShutter);
}

void CameraGrabber::enableWhiteBalance(bool whiteBalance)
{
    Camera *actvC = getActiveCamera();
    if(actvC)
        actvC->enableWhiteBalance(whiteBalance);
}

bool CameraGrabber::findAVTCameras()
{
    // Each call to this method clears previously found cameras
    nCameras = 0;
    availableCameras.clear();

    // Initialize the API: this is only done here, the AVTCameras
    // assume that this API has been initialised correctly
    if(FAILED(UCC_Init()))
    {
        qDebug()<<"UCC_init() failed, program execution stopped.";
        return false;
    }
    nCameras = 5;
    Result = UCC_GetCameras( &nCameras, camIds );
    if( (Result != UNI_RESULT_MORE_DATA) && (Result!=S_OK) )
    {
        qDebug()<<"UCC_GetCameras() failed, grabbing stopped.";
        return false;
    }
    qDebug()<<"AVT Grabbing ("<< (long)nCameras <<") cameras found";
    for(uint32_t  i=0;i<nCameras;i++)
    {
        qDebug()<<"Camera :"<<i<<" has Id:"<<camIds[i];
        Camera *avtCamera = Camera::factory("AVT",camIds[i]);
        avtCamera->start();
        availableCameras.push_back(avtCamera);
        QString cameraReferenceName;
        if(cameraName2ID.contains(avtCamera->getCameraName()))
            cameraReferenceName = QString("%1-%2").arg(avtCamera->getCameraName()).arg(sameCameraDiscriminator++);
        else
            cameraReferenceName = avtCamera->getCameraName();
        cameraName2ID[cameraReferenceName] = camIds[i];
        cameraNames.push_back(cameraReferenceName);
    }
    return (nCameras>0);
}

Camera * CameraGrabber::getActiveCamera()
{
    for(uint32_t i=0;i<uint32_t(availableCameras.size());i++)
    {
        if(uint32_t(availableCameras[i]->getCameraId()) == activeCamId)
            return availableCameras[i];
    }
    return NULL;
}

bool CameraGrabber::findOpenCVCameras()
{
    // For now, we only look for an opencv supported camera on index 0
    // TODO::find a way to search for all cameras
    CvCapture *camera  =   cvCreateCameraCapture(0);
    if(camera)
    {
        cameraName2ID[QString("OPENCV_I%1").arg(0)] = 0;
        cameraNames.push_back(QString("OPENCV_I%1").arg(0));
        qDebug()<<"OpenCV camera found at index 0";
        cvReleaseCapture(&camera);
        Camera *opencvCamera = Camera::factory("OPENCV_CAM",0);
        availableCameras.push_back(opencvCamera);
        return true;
    }
    else
        return false;
}

bool CameraGrabber::isCameraAvailable()
{
    return (availableCameras.size()>0);
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
void CameraGrabber::run()
{
    stopThread = false;
    int numSettingsConnections = 0;
    int frameReceivers = 0;

    while(!stopThread)
    {
        if( frameReceivers!= cameraFrameReceivers.size())
        {
            updateConnections();
            frameReceivers = cameraFrameReceivers.size();
            activeCameraChanged = false;
        }
        if( settingsConnections.size()!= numSettingsConnections)
        {
            updateSettingsConnections();
            numSettingsConnections = settingsConnections.size();
            activeCameraChanged = false;
        }
        // Update all connections when active camera changes
        if(activeCameraChanged)
        {
            updateConnections();
            frameReceivers = cameraFrameReceivers.size();
            updateSettingsConnections();
            numSettingsConnections = settingsConnections.size();
            activeCameraChanged = false;
        }
        msleep(100);
    }
}

void CameraGrabber::startRecording(QString videoFile)
{

}

void CameraGrabber::stopRecording()
{

}

void CameraGrabber::setShutterSpeed(int shutterSpeed)
{
    //TODO::forward this call to the activeCam
    Camera *actvC = getActiveCamera();
    if(actvC)
    {
        AVTCamera * avtCam = (AVTCamera *) actvC;
        avtCam->setShutterSpeed(shutterSpeed);
    }
}

void CameraGrabber::setActiveCamera(int camId)
{
    if(activeCamId == uint32_t(camId))
        return;
    // Stop the currently active camera
    for(int j=0;j<availableCameras.size();j++)
    {
        if(uint32_t(availableCameras[j]->getCameraId()) == activeCamId)
        {
            availableCameras[j]->stopGrabbing();
        }
    }
    activeCamId = camId;
    // Start the new active camera
    for(int j=0;j<availableCameras.size();j++)
    {
        //Re-connect the updated ones only
        if(uint32_t(availableCameras[j]->getCameraId()) == activeCamId)
        {
            availableCameras[j]->startGrabbing();
        }
    }
    activeCameraChanged = true;
}

void CameraGrabber::setActiveCamera(QString cameraSelected)
{
    if(cameraName2ID.contains(cameraSelected))
    {
        // Stop the currently active camera
        for(int j=0;j<availableCameras.size();j++)
        {
            if(uint32_t(availableCameras[j]->getCameraId()) == activeCamId)
            {
                availableCameras[j]->stopGrabbing();
            }
        }
        activeCamId = cameraName2ID[cameraSelected];
        // Start the new active camera
        for(int j=0;j<availableCameras.size();j++)
        {
            //Re-connect the updated ones only
            if(uint32_t(availableCameras[j]->getCameraId()) == activeCamId)
            {
                availableCameras[j]->startGrabbing();
            }
        }
        activeCameraChanged = true;
    }
    else
    {
        qDebug()<<"Selected Camera can't be found";
    }
}

void CameraGrabber::setupGrabbingSource()
{
    bool opencvCameraFound = false;
    if(!findAVTCameras() && !(opencvCameraFound = findOpenCVCameras()))
    {
        qDebug()<<"No Camera stream source found";
    }
    int mainCamera = Camera::NONE;
    //OpenCV has higher priority than AVT
    if(opencvCameraFound)
    {
        mainCamera  = Camera::OPENCV_CAM;
        activeCamId = 0;
    }
    else if(availableCameras.size()>0)
    {
        mainCamera  = Camera::AVTFIRE;
        // The first camera found will be the active
        activeCamId = camIds[0];
    }
    emit camerasFound(cameraNames);
    emit camerasFound(cameraName2ID);
}

void CameraGrabber::stop()
{
    QMutexLocker lock(&mutex);
    stopThread = true;
}

void CameraGrabber::updateConnections()
{
    qDebug()<<"Updating Connections";
    QHashIterator<const QObject *,int> i(cameraFrameReceivers);
    while (i.hasNext())
    {
        i.next();
        for(int j=0;j<availableCameras.size();j++)
        {
            disconnect(availableCameras[j],0,i.key(),0);
        }
    }
    i.toFront();
    while (i.hasNext())
    {
        i.next();
        for(int j=0;j<availableCameras.size();j++)
        {
            //Re-connect the updated ones only
            if(availableCameras[j]->getCameraId() == i.value() ||( (uint32_t(availableCameras[j]->getCameraId()) == activeCamId) && (i.value()==ACTIVE_CAM)))
            {
                connect(availableCameras[j],SIGNAL(frameReady(IplImage *)),i.key(),SLOT(recieveFrame(IplImage *)));
            }
        }
    }
    qDebug()<<"Connections Updated";
}

void CameraGrabber::updateSettingsConnections()
{
    qDebug()<<"Disconnecting previous signals";
    for(int i=0;i<settingsConnections.size();i++)
    {
        for(int j=0;j<availableCameras.size();j++)
        {
            if(settingsConnections[i].type == TO_CAMERA_SIGNAL)
                disconnect(availableCameras[j],settingsConnections[i].signal,settingsConnections[i].listener,settingsConnections[i].slot);
            else
                disconnect(settingsConnections[i].listener,settingsConnections[i].signal,availableCameras[j],settingsConnections[i].slot);
        }
    }
    qDebug()<<"Reconnecting appropriate signals";
    for(int i=0;i<settingsConnections.size();i++)
    {
        for(int j=0;j<availableCameras.size();j++)
        {
            //Re-connect the updated ones only
            if(availableCameras[j]->getCameraId() == settingsConnections[i].camId ||( (uint32_t(availableCameras[j]->getCameraId()) == activeCamId) && (settingsConnections[i].camId==ACTIVE_CAM)))
            {
                qDebug()<<"Connect signal:"<<settingsConnections[i].signal<<" to slot:"<<settingsConnections[i].slot;
                if(settingsConnections[i].type  == TO_CAMERA_SIGNAL)
                    connect(availableCameras[j],settingsConnections[i].signal,settingsConnections[i].listener,settingsConnections[i].slot);
                else
                    connect(settingsConnections[i].listener,settingsConnections[i].signal,availableCameras[j],settingsConnections[i].slot);
            }
        }
    }
}
