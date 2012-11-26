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
#ifndef AVTCAMERA_H
#define AVTCAMERA_H

#include <QThread>
#include <QMutexLocker>
#include <QTime>
#include <QMessageBox>
#include "dlogger.h"
#include "UniControl.h"
#include "UniTransform.h"
#include "cv.h"
#include "highgui.h"
#include "camera.h"

#define UINT8P_CAST(x) reinterpret_cast<UINT8_TYPE*>(x)
#define CALL_CHECKED_RETURN(Result,Expected,msgTxt)     \
        {                                               \
            if(Expected != Result)                      \
            {                                           \
                QString errorMsg;                       \
                errorMsg = msgTxt + " Reason:";         \
                char ar[256];                           \
                UCC_GetErrorInfo(Result,ar,256);        \
                errorMsg +=ar;                          \
                qDebug()<<errorMsg;                     \
                return false;                           \
            }                                           \
        }

/**
  IIDC predefined video modes to string conversion.
*/
inline QString fixed2String( UINT32_TYPE format, UINT32_TYPE mode)
{
    switch( format)
    {
    case 0:
        {
            switch(mode)
            {
            case 0: return ("160 X 120 YUV(4:4:4) Mode (24bit/pixel)");
            case 1: return ("320 X 240 YUV(4:2:2) Mode (16bit/pixel)");
            case 2: return ("640 X 480 YUV(4:1:1) Mode (12bit/pixel)");
            case 3: return ("640 X 480 YUV(4:2:2) Mode (16bit/pixel)");
            case 4: return ("640 X 480 RGB Mode (24bit/pixel)");
            case 5: return ("640 X 480 Y (Mono) Mode (8bit/pixel)");
            case 6: return ("640 X 480 Y (Mono16) Mode (16bit/pixel)");
            default:return ("Unknown");
            }
        }
    case 1:
        {
            switch(mode)
            {
            case 0: return ("800 X 600 YUV(4:2:2) Mode (16bit/pixel)");
            case 1: return ("800 X 600 RGB Mode (24bit/pixel)");
            case 2: return ("800 X 600 Y (Mono) Mode (8bit/pixel)");
            case 3: return ("1024 X 768 YUV(4:2:2) Mode (16bit/pixel)");
            case 4: return ("1024 X 768 RGB Mode (24bit/pixel)");
            case 5: return ("1024 X 768 Y (Mono) Mode (8bit/pixel)");
            case 6: return ("800 X 600 Y (Mono16) Mode (16bit/pixel)");
            case 7: return ("1024 X 768 Y (Mono16) Mode (16bit/pixel)");
            default:return ("Unknown");
            }
        }
    case 2:
        {
            switch(mode)
            {
            case 0: return ("1280 X 960 YUV(4:2:2) Mode (16bit/pixel)");
            case 1: return ("1280 X 960 RGB Mode (24bit/pixel)");
            case 2: return ("1280 X 960 Y (Mono) Mode (8bit/pixel)");
            case 3: return ("1600 X 1200 YUV(4:2:2) Mode (16bit/pixel)");
            case 4: return ("1600 X 1200 RGB Mode (24bit/pixel)");
            case 5: return ("1600 X 1200 Y (Mono) Mode (8bit/pixel)");
            case 6: return ("1280 X 960 Y (Mono16) Mode (16bit/pixel)");
            case 7: return ("1600X 1200 Y (Mono16) Mode (16bit/pixel)");
            default:return ("Unknown");
            }
        }
     default:return ("Unknown");
    }
}

inline QString colorCode2ColorSequence(UINT32_TYPE colorCode)
{
    QString channelSequence;
    switch(colorCode)
    {
        case E_CC_MONO8:        channelSequence = ("Mono8"); break;
        case E_CC_YUV411:       channelSequence = ("YUV411");break;
        case E_CC_YUV422:       channelSequence = ("YUV422");break;
        case E_CC_YUV444:       channelSequence = ("YUV44");break;
        case E_CC_RGB8:         channelSequence = ("RGB8");break;
        case E_CC_MONO16:       channelSequence = ("Mono16");break;
        case E_CC_RGB16:        channelSequence = ("RGB16");break;
        case E_CC_SMONO16:      channelSequence = ("SMono16");break;
        case E_CC_SRGB16:       channelSequence = ("SRGB16");break;
        case E_CC_RAW8:         channelSequence = ("Raw8");break;
        case E_CC_RAW16:        channelSequence = ("Raw16");break;
        case E_CC_MONOR:        channelSequence = ("MonoR");break;
        case E_CC_MONOG:        channelSequence = ("MonoG");break;
        case E_CC_MONOB:        channelSequence = ("MonoB");break;
        case E_CC_MONO12:       channelSequence = ("Mono12");break;
        case E_CC_RGB12:        channelSequence = ("RGB12");break;
        case E_CC_SMONO12:      channelSequence = ("SMono12");break;
        case E_CC_SRGB12:       channelSequence = ("SRGB12");break;
        case E_CC_RAW12:        channelSequence = ("Raw12");break;
        default:                channelSequence = ("Unknown");break;
    }
    return channelSequence;
}

/**
    IIDC frames per second to string conversion.
*/
static const QString   FPS_Strings[] =
{
    QString("1_875"),    //!< 1.875 FPS
    QString("3_75"),     //!< 3.75 FPS
    QString("7_5"),      //!< 7.5 FPS
    QString("15"),       //!< 15 FPS
    QString("30"),       //!< 30 FPS
    QString("60"),       //!< 60 FPS
    QString("120"),      //!< 120 FPS
    QString("240")

};

/**
    Enumeration of the supported IIDC video formats.
    Enumeration with UCC_GetSupportedFixedFormats will return a feature matrix (S_IIDCVideoInfoFormat)
    of all the video formats supported by the camera.
*/
inline bool enumFixedScanFormats(UINT32_TYPE CamId)
{
    S_IIDC_VIDEOINFO_FORMAT vf;
    UNI_RETURN_TYPE Result= UCC_GetSupportedFixedFormats(CamId,&vf);
    if( Result!=S_OK)
        return false;
    UINT32_TYPE format(0),mode(0),fps(0);
    while( format < 3)
    {
        mode=0;
        while( mode < 8)
        {
            fps=0;
            while( fps < 8)
            {
                if( vf.m_FormatInq[format].m_ModeInq[mode].m_FpsInq[fps] == TRUE)
                {
                    qDebug()<<" Fixed Scan Format:"<<format<<" Mode:"<<mode<<" ="<<fixed2String(format,mode)<<" "<<FPS_Strings[fps];
                }
                ++fps;
            }
            ++mode;
        }
        ++format;
    }
    return true;
}

/** Enumeration of the supported IIDC video formats.
    Enumerating the video formats with calls to UCC_PrepareFixedGrab.
    this will only enumerate video modes which are supported by the camera and the current iso speed.
*/
inline bool enumFixedFormats(UINT32_TYPE CamId)
{
    UINT32_TYPE format(0),mode(0),fps(0);
    UNI_RETURN_TYPE Result;
    while( format < 3)
    {
        mode=0;
        while( mode < 7)
        {
            fps=0;
            while( fps < 7)
            {
                Result = UCC_PrepareFixedGrab(CamId,format,mode,fps); // try to prepare grabbing
                if( Result==S_OK) // if ok then the mode might be posible
                {
                    qDebug()<<" Fixed Format:"<<format<<" Mode:"<<mode<<" ="<<fixed2String(format,mode)<<" "<<FPS_Strings[fps];
                }
                ++fps;
            }
            ++mode;
        }
        ++format;
    }
    return true;
}

/** Enumerating Video Presets.
    Enumerating through common video resolutions which are supported by the curent camera.
*/
inline bool enumPreFormats(UINT32_TYPE CamId)
{
    PRESETS_ENUMERATION         Presets;
    COLORCODE_ENUMERATION       cc_enum;
    UINT32_TYPE                 cnt,color_cnt,preset,w,h,color_code;
    UNI_RETURN_TYPE eRet = UCC_CreatePresetsEnum( CamId,&Presets,&cnt);     // Creation of the preset enumeration
    /* At the moment, preset enumeration is supported only by FireWire UniControl DLL. */
    if(eRet==S_OK)
    {
        for(UINT32_TYPE i=0; i < cnt;++i)                                   // walk through
        {
            UCC_EnumPresets( &preset,Presets,i);                            // Enum a Preset
            UCC_PresetToResolution( preset,&w,&h);                          // get AOI of the Preset
            UCC_CreateColorCodeEnum( CamId,&cc_enum,&color_cnt,preset);     // create color enum
            for( UINT32_TYPE j=0; j< color_cnt;++j)                         // walk through colors
            {
                UCC_EnumColorCode( &color_code,cc_enum,j);                  // enum a color code
                qDebug()<<"Width:"<<w<<" Height:"<<h<<" colorCode:"<<colorCode2ColorSequence(color_code);
            }
            UCC_DestroyColorCodeEnum(cc_enum);                              // destruction of color code enumerator
        }
        UCC_DestroyPresetsEnum(Presets);                                    // destruction of preset enumerator
    }
    return true;
}

class AVTCamera : public Camera
{
    Q_OBJECT
public:
    enum {
        ASYNCHRONOUS,
        SYNCHRONOUS
    };
    enum CapturingMechanism {
        POLLING,
        NOTIFICATION
    };
    AVTCamera(UINT32_TYPE camID);
    ~AVTCamera();
    void enableAutoShutter(bool autoShutter);
    void enableWhiteBalance(bool whiteBalance);
    int  getCameraId();
    QString getCameraName();
    int     getCameraType();
    bool initializeCamera();
    bool fastCameraInitialization();
    void run();
    void setGrabbingMode(int);
    void startGrabbing();
    void stopGrabbing();
    void stop();
public slots:
    void setShutterSpeed(int speed);
    virtual void startRecording(QString);
    virtual void stopRecording();
    signals:
        void frameReady(IplImage *);
        void shutterSpeedChanged(int);
        void warnUser(QString warning,int);
private:
    bool grabImage(UINT32_TYPE activeCamId);
    static VOID_TYPE NOTIFICATION_CALLING_CONVENTION onFrameReadyCallback ( S_UNI_CALLBACK_ARGUMENT arg );
    CvVideoWriter *writer;
    int fps;
    QMutex mutex;
    IplImage *grabbedFrame;
    int grabbingMode,getImageMechanism;
    int nChannels;
    bool stopThread,isColored;
    // Every UNI_API function will return this type
    UNI_RETURN_TYPE Result;
    UINT32_IN_TYPE shutterSpeed;
    UINT32_OUT_TYPE frameNumber;
    UINT32_TYPE camId;
    // frame ready events
    S_UNI_NOTIFICATION      frameReadyNotification;
    UNI_NOTIFICATION_HANDLE frameReadyHandle;
    // frame dropped events
    S_UNI_NOTIFICATION      frameDroppedNotification;
    UNI_NOTIFICATION_HANDLE frameDroppedHandle;
    QTime frameTime;
    bool newFrameArrived,recordVideo,isInitialized;
    QString videoFileName;
    QString cameraName;
};

#endif // AVTCAMERA_H
