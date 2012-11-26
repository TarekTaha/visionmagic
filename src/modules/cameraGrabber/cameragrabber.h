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
#ifndef CAMERAGRABBER_H
#define CAMERAGRABBER_H

#include <QObject>
#include <QThread>
#include <QMutexLocker>
#include <QTime>
#include "UniControl.h"
#include "UniTransform.h"
#include "cv.h"
#include "highgui.h"
#include "dlogger.h"
#include "camera.h"
#include "avtcamera.h"
#include "acdsettings.h"

enum ConnectionDirection {
    TO_CAMERA_SIGNAL,
    TO_CAMERA_SLOT
};
/*!
  Definition of connections that will allow an object
  to connect to a specific camera.
  The camera grabber is responsible for re-connecting the signals
  when the active camera changes
  */
typedef struct _SettingsConnection
{
    _SettingsConnection(){}
    _SettingsConnection(const QObject *l,const char *sig,const char *slo,int t,int id):listener(l),signal(sig),slot(slo),type(t),camId(id){}
    const QObject *listener;
    const char * signal;
    const char * slot;
    int type;
    int camId;
} SettingsConnection;

class CameraGrabber : public QThread
{
    Q_OBJECT
public:
    enum Cameras {
        ACTIVE_CAM = -1,
        AUX_CAM_1 = -2,
        AUX_CAM_2 = -3
    };
    CameraGrabber();
    ~CameraGrabber();
    // When the camId = -1 we connect to the main activeCamera
    void addSettingsConnection(const QObject *listener,const char *, const char *,int type, int camId = ACTIVE_CAM);
    void addFrameReceiver(const QObject *listener,int camId = ACTIVE_CAM);
    void enableAutoShutter(bool autoShutter);
    void enableWhiteBalance(bool whiteBalance);
    bool findOpenCVCameras();
    bool findAVTCameras();
    Camera * getActiveCamera();
    bool isCameraAvailable();
    void run();
    void setActiveCamera(QString);
    void setActiveCamera(int);
    void setupGrabbingSource();
    void stop();
public slots:
    void setShutterSpeed(int speed);
    void startRecording(QString);
    void stopRecording();
    signals:
        void camerasFound(QVector<QString>);
        void camerasFound(QHash<QString,int>);
        void warnUser(QString warning,int);
private:
    void updateConnections();
    void updateSettingsConnections();
    QVector<Camera*>  availableCameras;
    QVector<SettingsConnection> settingsConnections;
    QMutex mutex;
    bool stopThread;
    // Every UNI_API function will return this type
    UNI_RETURN_TYPE Result;
    // create an array for up to 5 camera ids
    UINT32_TYPE camIds[5], nCameras,activeCamId;
    QHash<const QObject *,int> cameraFrameReceivers;
    QVector<QString>  cameraNames;
    QHash<QString,int> cameraName2ID;
    bool activeCameraChanged;
    int sameCameraDiscriminator;
};

#endif // CAMERAGRABBER_H
