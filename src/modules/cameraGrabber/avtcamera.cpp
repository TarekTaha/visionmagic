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
#include "avtcamera.h"

AVTCamera::AVTCamera(UINT32_TYPE _camId):
        writer(false),
        grabbedFrame(NULL),
        grabbingMode(ASYNCHRONOUS),
        getImageMechanism(POLLING),//POLLING, NOTIFICATION
        camId(_camId),
        newFrameArrived(false),
        recordVideo(false),
        isInitialized(false)
{

}

AVTCamera::~AVTCamera()
{
    stopGrabbing();
}

void AVTCamera::enableAutoShutter(bool autoShutter)
{
    if(autoShutter)
    {
        UCC_SetFeatureStatus(camId,E_FEAT_SHUTTER,E_FEATSTATE_AUTO);
    }
    else
    {
        UCC_SetFeatureStatus(camId,E_FEAT_SHUTTER,E_FEATSTATE_ON);
    }
}

void AVTCamera::enableWhiteBalance(bool whiteBalance)
{
    if(whiteBalance && isColored)
    {
        UCC_Whitebalance_Set(camId,E_FEATSTATE_AUTO,0,0 );
    }
    else
    {
        UCC_Whitebalance_Set(camId,E_FEATSTATE_OFF,0,0 );
    }
}

bool AVTCamera::fastCameraInitialization()
{
    UINT32_TYPE maxSpeed;
    if(UCC_OpenCamera(camId)!=S_OK)
    {

        qDebug()<<"UCC_OpenCamera() failed, grabbing stopped.";
        return false;
    }
    UINT32_TYPE imageDepth;
    UINT32_TYPE maxHorizontalResolution = 0,maxVerticalResolution = 0, isColoredCamera=0, nMode=0;
    char model[128];
    UCC_GetDataBitRateMax  ( camId,&maxSpeed);
    // This gets the max height, max width
    UCC_GetCameraCapabilities( camId, &maxHorizontalResolution, &maxVerticalResolution, &isColoredCamera, &imageDepth );
    if(isColoredCamera)
    {
        isColored = true;
    }
    else
        isColored = false;
    cameraName = model;

    UCC_SetFeatureStatus(camId,E_FEAT_SHUTTER,E_FEATSTATE_ON);

    nMode = 0;
    UINT32_TYPE nColorFormat;
    UINT32_TYPE busLoad,bc(12);
    if(!isColored)
    {
        nColorFormat = E_CC_MONO8;
        nChannels = 1;
    }
    else
    {
        nColorFormat = E_CC_RGB8;
        nChannels = 3;
    }
    busLoad = 1000*maxSpeed;
    // use the max width and height setting obtained above as the desired wxh
    Result = UCC_PrepareFreeGrab( camId, &nMode, &nColorFormat, &maxHorizontalResolution, &maxVerticalResolution, &bc, NULL, NULL, &busLoad);
    if(Result!=S_OK)
    {
        qDebug()<<"UCC_PrepareFreeGrab failed, program execution stopped.";
        return false;
    }
    if(isColored)
    {
        UCC_Whitebalance_Set(camId,E_FEATSTATE_AUTO,0,0 );
        UCC_SetDataBitRate(camId,E_DATABITRATE_F800);
    }
    else
    {
        UCC_SetDataBitRate(camId,E_DATABITRATE_F400);
    }
    qDebug()<<"True Bus Load:"<<busLoad;

    // Sharpness
    UINT32_TYPE sharpness_min(0),sharpness_max(0),sharpness_val(0);
    UCC_GetFeatureMin   ( camId, E_FEAT_SHARPNESS , &sharpness_min);
    UCC_GetFeatureMax   ( camId, E_FEAT_SHARPNESS , &sharpness_max);
    UCC_GetFeatureValue ( camId, E_FEAT_SHARPNESS , &sharpness_val);
    qDebug()<<"Sharpness min is:"<<sharpness_min<<" max:"<<sharpness_max<<" current value:"<<sharpness_val;
    UCC_SetFeatureStatus(camId,E_FEAT_SHARPNESS,E_FEATSTATE_ON);
    UCC_SetFeatureValue(camId,E_FEAT_SHARPNESS,sharpness_max);

    grabbedFrame = cvCreateImage(cvSize(maxHorizontalResolution,maxVerticalResolution),8, nChannels);

    if(grabbingMode == ASYNCHRONOUS)
    {
        // Setup Notification struct for frame ready event
        if(getImageMechanism == NOTIFICATION)
        {
            frameReadyNotification.m_CamId                                     = UNI_ALL_CAMERAS;
            frameReadyNotification.m_NotificationType                          = E_UNI_NOTIFICATION_CALLBACK;
            frameReadyNotification.m_NotificationCallback.m_Arg.m_Parameter    = this;
            frameReadyNotification.m_NotificationCallback.m_Callback           = onFrameReadyCallback;
            CALL_CHECKED_RETURN(UCC_RegisterFrameReadyNotification(&frameReadyHandle,&frameReadyNotification),S_OK,QString("Could not Register Notification"))
        }
        if(UCC_GrabStart(camId,1000)!= S_OK)
        {
            qDebug()<<"UCC_GrabStart() failed, program execution stopped.";
            return false;
        }
    }
    qDebug()<<"Finised Setup";
    return true;
}

int AVTCamera::getCameraId()
{
    return camId;
}

QString AVTCamera::getCameraName()
{
    if(cameraName == QString(""))
    {
        if(UCC_OpenCamera(camId)!=S_OK)
        {
            qDebug()<<"UCC_OpenCamera() failed, grabbing stopped.";
        }
        char model[128];
        UINT32_TYPE msize(128);
        UCC_GetCameraInfoString( camId,E_CAMINFO_MODEL,model,&msize);
        cameraName = model;
        UCC_CloseCamera(camId);
    }
    return cameraName;
}

int AVTCamera::getCameraType()
{
    return AVTFIRE;
}

bool AVTCamera::grabImage(UINT32_TYPE camId)
{
    QTime innerGrab;
    bool result =true;
    // Synchronous uses Grab Methods
    try
    {
        if( NULL == grabbedFrame)
            return false;
        innerGrab.restart();
        if(isColored)
            UCC_GrabBitmap24Image( camId, UINT8P_CAST(grabbedFrame->imageData),100 );
        else
            UCC_GrabBitmap8Image( camId, UINT8P_CAST(grabbedFrame->imageData), 100);
        qDebug()<<"Inner Grabbing took:"<<innerGrab.elapsed();
    }
    catch(const std::bad_alloc &e)
    {
        qDebug()<<e.what();
        result=false;
    }
    catch(...)
    {
        result = false;
    }
    return true;
}

VOID_TYPE NOTIFICATION_CALLING_CONVENTION AVTCamera::onFrameReadyCallback ( S_UNI_CALLBACK_ARGUMENT arg )
{
    AVTCamera   *This = 0;
    This = reinterpret_cast<AVTCamera*>(arg.m_Parameter);
    // Synchronous uses Get Methods
    try
    {
        if( This->grabbedFrame == NULL)
            return;
        //UCC_GetBitmapImage(This->camId, UINT8P_CAST(This->grabbedFrame->imageData),0);
        if(This->isColored)
            This->Result = UCC_GetBitmap24ImageEx(This->camId, UINT8P_CAST(This->grabbedFrame->imageData),This->frameNumber,0 );
        else
            This->Result = UCC_GetBitmap8ImageEx(This->camId, UINT8P_CAST(This->grabbedFrame->imageData),This->frameNumber,0);
        This->mutex.lock();
        This->newFrameArrived = true;
        This->mutex.unlock();
    }
    catch(const std::bad_alloc &e)
    {
        //This->warnUser(QString("Exception allocate vector. %1").arg(e.what()),QMessageBox::Warning);
        qDebug()<<e.what();
    }
    catch(...)
    {
        //This->warnUser("Exception unknown",QMessageBox::Warning);
    }
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
void AVTCamera::run()
{
    frameTime.start();
    stopThread = false;
    UINT32_IN_TYPE newShutterSpeed;
    int sycTime = 80;
    QTime timeSinceLastGrab;
    QTime time2Grab;
    timeSinceLastGrab.restart();
    newFrameArrived = false;
    int elapsedMs;
    while(!stopThread)
    {
        if(!isInitialized)
        {
            // Camera not initialized yet
            msleep(10);
            continue;
        }
        if(grabbingMode == SYNCHRONOUS)
        {
            if(timeSinceLastGrab.elapsed()>=sycTime)
            {
                timeSinceLastGrab.restart();
                //time2Grab.restart();
                grabImage(camId);
                //qDebug()<<"Grabbing Took:"<<time2Grab.elapsed();
                if(recordVideo)
                    cvWriteFrame(writer,grabbedFrame);
                emit frameReady(grabbedFrame);
                elapsedMs = frameTime.restart();
                if(elapsedMs!=0)
                    fps = qRound(1000/elapsedMs);
            }
            UCC_GetFeatureValue (camId,E_FEAT_SHUTTER,&newShutterSpeed);
            if(newShutterSpeed!=shutterSpeed)
            {
                shutterSpeed = newShutterSpeed;
                emit shutterSpeedChanged(shutterSpeed);
            }
        }
        else if((grabbingMode == ASYNCHRONOUS && newFrameArrived) || (grabbingMode == ASYNCHRONOUS && getImageMechanism==POLLING))
        {
            //The statement below takes approx 10 msec
            UCC_GetFeatureValue (camId,E_FEAT_SHUTTER,&newShutterSpeed);
            if(newShutterSpeed!=shutterSpeed)
            {
                shutterSpeed = newShutterSpeed;
                emit shutterSpeedChanged(shutterSpeed);
            }
            if(getImageMechanism==POLLING)
            {
                if(isColored)
                    Result = UCC_GetBitmap24ImageEx(camId, UINT8P_CAST(grabbedFrame->imageData),frameNumber,100 );
                else
                    Result = UCC_GetBitmap8ImageEx(camId, UINT8P_CAST(grabbedFrame->imageData),frameNumber,100);
                if(Result!=S_OK)
                {
                    continue;
                }
                msleep(2);
            }
            elapsedMs = frameTime.restart();
            if(elapsedMs!=0)
                fps = qRound(1000/elapsedMs);
            if(recordVideo)
                cvWriteFrame(writer,grabbedFrame);
            emit frameReady(grabbedFrame);
            mutex.lock();
            newFrameArrived = false;
            mutex.unlock();
            //qDebug()<<"Elapsed Time:"<<elapsedMs<<" fps:"<<fps<<" frameNumber:"<<frameNumber;
        }
        else
        {
            msleep(1);
        }
    }
    cvReleaseVideoWriter(&writer);
}

void AVTCamera::setGrabbingMode(int grabMode)
{
    this->grabbingMode = grabMode;
}

void AVTCamera::startRecording(QString videoFile)
{
    QMutexLocker locker(&mutex);
    if(writer)
    {
        cvReleaseVideoWriter(&writer);
    }
    if(grabbedFrame)
        writer = cvCreateVideoWriter(videoFile.toStdString().c_str(),CV_FOURCC_PROMPT,fps,cvSize(grabbedFrame->width,grabbedFrame->height),isColored);
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

void AVTCamera::stopRecording()
{
    QMutexLocker locker(&mutex);
    qDebug()<<"Stopping Video recording";
    this->recordVideo   = false;
    if(writer)
    {
        cvReleaseVideoWriter(&writer);
    }
}

void AVTCamera::setShutterSpeed(int shutterSpeed)
{
    UCC_SetFeatureValue(camId,E_FEAT_SHUTTER,shutterSpeed);
    /*
    UINT32_TYPE shutter_min(0),shutter_max(0),shutter_val(0);
    UCC_GetFeatureMin   ( camId, E_FEAT_SHUTTER, &shutter_min);
    UCC_GetFeatureMax   ( camId, E_FEAT_SHUTTER, &shutter_max);
    UCC_GetFeatureValue ( camId, E_FEAT_SHUTTER, &shutter_val);
    */
}

bool AVTCamera::initializeCamera()
{
    UINT32_TYPE maxSpeed,freeBandwidth;
    if(UCC_OpenCamera(camId)==S_OK)
    {
        Result  = UCC_GetDataBitRateMax(camId,&maxSpeed);
        Result &= UCC_GetFreeBandwidth(camId,&freeBandwidth);
        if(Result == S_OK)
            qDebug()<<" Camera with id:"<<camId<<" has max BitRate speed:"<<maxSpeed<<" free bandwith:"<<freeBandwidth;
    }
    else
    {
        qDebug()<<"UCC_OpenCamera() failed, grabbing stopped.";
        return false;
    }
    enumFixedScanFormats(camId);
    UINT32_TYPE imageWidth, imageHeight, colorCoding, imageDepth,dummy1,dummy2;;
    UINT32_TYPE bitsPerPixel,isInterlaced,bayerPattern,format=0,sfps=0;
    UINT32_TYPE maxHorizontalResolution = 0,maxVerticalResolution = 0, isColoredCamera=0, nMode=0;
    char vendor[128],model[128],busLoadText[128],isColoredImageText[128];
    UINT32_TYPE vsize(128),msize(128),isColoredImageSize(128);
    UCC_GetCameraInfoString( camId,E_CAMINFO_VENDOR,vendor,&vsize);
    UCC_GetCameraInfoString( camId,E_CAMINFO_MODEL,model,&msize);
    UCC_GetCameraInfoString( camId,E_CAMINFO_BUS_LOAD ,busLoadText,&isColoredImageSize);
    UCC_GetCameraInfoString( camId,E_CAMINFO_COLORCAMERA ,isColoredImageText, &isColoredImageSize);
    UCC_GetDataBitRateMax  ( camId,&maxSpeed);
    //UCC_GetCameraInfo(camId,E_CAMINFO_COLORCAMERA,&isColoredCamera);
    // This gets the max height, max width
    UCC_GetCameraCapabilities( camId, &maxHorizontalResolution, &maxVerticalResolution, &isColoredCamera, &imageDepth );
    UCC_GetCurrentFixedFormat( camId,&dummy1,&nMode,&dummy2);
    qDebug()<<"Current Fixed Mode is:"<<nMode;
    if(isColoredCamera)
    {
        isColored = true;
    }
    else
        isColored = false;
    cameraName = model;
    qDebug()<<"Camera Vendor: "<<vendor<<" Model: "<<model<<" Bus Load:"<<busLoadText<<" is Colored Camera:"<<isColoredImageText<<"MaxWidth: "<<maxHorizontalResolution<<" MaxHeight: "<<maxVerticalResolution<<" Max Speed:"<<maxSpeed;

    UINT32_TYPE pos(0),mode,color_code,x_max,y_max;
    while( static_cast<UINT32_TYPE>(-1)  != pos)
    {
        if( UCC_EnumFreeModes(camId,&pos,&mode,&color_code,&x_max,&y_max) == S_OK)
        {
            qDebug()<<"F7Mode:"<<mode<<" Seq:"<<colorCode2ColorSequence(color_code)<<" width:"<<x_max<<" height:"<<y_max;
        }
    }

    // Get information about the camera's current image format
    Result  = UCC_GetCurrentImageFormat   ( camId, &imageWidth, &imageHeight, &colorCoding, &imageDepth );
    Result &= UCC_GetCameraInfo           ( camId,E_CAMINFO_INTERLACED   , &isInterlaced);
    Result &= UCC_GetCameraInfo           ( camId,E_CAMINFO_BAYERPATTERN , &bayerPattern);
    Result &= UIT_GetBitsPerPixel         ( colorCoding, &bitsPerPixel);
    if(Result!=S_OK)
    {
        qDebug()<<"Failed to get required information from camera, program execution stopped.";
        return false;
    }

    QString channelSequence;
    channelSequence = colorCode2ColorSequence(colorCoding);

    qDebug()<<"Current settings W:"<<imageWidth<<" H:"<<imageHeight<<" colorCode:"<<colorCoding<<" channel sequence:"<<channelSequence<<" depth:"<<imageDepth<<" isInterlaced:"<<isInterlaced<<" bayerPattern:"<<bayerPattern<<" format:"<<format<<" mode:"<<mode<<" fps:"<<sfps<<" bitsperpix:"<<bitsPerPixel;

    UCC_SetFeatureStatus(camId,E_FEAT_SHUTTER,E_FEATSTATE_ON);

    nMode = 0;
    UINT32_TYPE nColorFormat;
    UINT32_TYPE busLoad,bc(12);
    int nChannels;
    if(!isColored)
    {
        nColorFormat = E_CC_MONO8;
        nChannels = 1;
    }
    else
    {
        nColorFormat = E_CC_RGB8;
        nChannels = 3;
    }
    qDebug()<<"Num Channels:"<<nChannels;
    busLoad = 1000*maxSpeed;
    // use the max width and height setting obtained above as the desired wxh
    Result = UCC_PrepareFreeGrab( camId, &nMode, &nColorFormat, &maxHorizontalResolution, &maxVerticalResolution, &bc, NULL, NULL, &busLoad);
    if(Result!=S_OK)
    {
        qDebug()<<"UCC_PrepareFreeGrab failed, program execution stopped.";
        return false;
    }
    if(isColored)
    {
        UCC_Whitebalance_Set(camId,E_FEATSTATE_AUTO,0,0 );
        UCC_SetDataBitRate(camId,E_DATABITRATE_F800);
    }
    else
    {
        UCC_SetDataBitRate(camId,E_DATABITRATE_F400);
    }
    qDebug()<<"True Bus Load:"<<busLoad;

    grabbedFrame = cvCreateImage(cvSize(maxHorizontalResolution,maxVerticalResolution),8, nChannels);
    Result = UCC_GetCurrentImageFormat(  camId, &imageWidth, &imageHeight, &colorCoding, &imageDepth );
    if(S_OK != Result)
    {
        qDebug()<<"UCC_GetCurrentImageFormat() failed, program execution stopped.";
        return false;
    }
    channelSequence = colorCode2ColorSequence(colorCoding);
    qDebug()<<"Changed the settings to W:"<<imageWidth<<" H:"<<imageHeight<<" colorCode:"<<colorCoding<<" channel sequence:"<<channelSequence<<" depth:"<<imageDepth<<" isInterlaced:"<<isInterlaced<<" bayerPattern:"<<bayerPattern<<" format:"<<format<<" mode:"<<mode<<" fps:"<<sfps<<" bitsperpix:"<<bitsPerPixel;

    // Sharpness
    UINT32_TYPE sharpness_min(0),sharpness_max(0),sharpness_val(0);
    UCC_GetFeatureMin   ( camId, E_FEAT_SHARPNESS , &sharpness_min);
    UCC_GetFeatureMax   ( camId, E_FEAT_SHARPNESS , &sharpness_max);
    UCC_GetFeatureValue ( camId, E_FEAT_SHARPNESS , &sharpness_val);
    qDebug()<<"Sharpness min is:"<<sharpness_min<<" max:"<<sharpness_max<<" current value:"<<sharpness_val;

    if(grabbingMode == ASYNCHRONOUS)
    {
        // Setup Notification struct for frame ready event
        if(getImageMechanism == NOTIFICATION)
        {
            frameReadyNotification.m_CamId                                     = UNI_ALL_CAMERAS;
            frameReadyNotification.m_NotificationType                          = E_UNI_NOTIFICATION_CALLBACK;
            frameReadyNotification.m_NotificationCallback.m_Arg.m_Parameter    = this;
            frameReadyNotification.m_NotificationCallback.m_Callback           = onFrameReadyCallback;
            CALL_CHECKED_RETURN(UCC_RegisterFrameReadyNotification(&frameReadyHandle,&frameReadyNotification),S_OK,QString("Could not Register Notification"))
        }
        if(UCC_GrabStart(camId,1000)!= S_OK)
        {
            qDebug()<<"UCC_GrabStart() failed, program execution stopped.";
            return false;
        }
    }
    qDebug()<<"Finised Setup";
    return true;
}

void AVTCamera::startGrabbing()
{
    //isInitialized = initializeCamera();
    isInitialized = fastCameraInitialization();
}

void AVTCamera::stopGrabbing()
{
    if(grabbingMode==ASYNCHRONOUS)
    {
        if(getImageMechanism==NOTIFICATION)
            UCC_UnRegisterNotification(frameReadyHandle);
        // Stop and cancel any pending images
        UCC_GrabStop(camId,1);
    }
    UCC_CloseCamera(camId);
    isInitialized = false;
}

void AVTCamera::stop()
{
    QMutexLocker lock(&mutex);
    stopThread = true;
}
