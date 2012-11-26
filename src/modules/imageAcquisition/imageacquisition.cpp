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

#include "imageacquisition.h"

ImageAcquisition::ImageAcquisition(QWidget *_parent ,MotorControl *_motorController,EncapsulationCoverage *_encap):
        QThread(_parent),
        parent(_parent),
        motorController(_motorController),
        encapCoverage(_encap),
        frameImage(NULL),
        stopThread(false),
        acquireImages(false),
        imageFocused(false),
        newStart(true),
        motorMoving(true),
        firstStep(false),
        encapAnalysisEnabled(false),
        repeatAnalysis(false),
        motorPose(0),
        currentChip(0),
        homePose(0),
        printHeadCode("")
{
    relativePath =  ASG_PIT::settings().getImagesPath();
}

ImageAcquisition::~ImageAcquisition()
{

}

void ImageAcquisition::enableOnlineEncapAnalysis(bool enable)
{
    QMutexLocker lock(&mutex);
    encapAnalysisEnabled = enable;
}

void ImageAcquisition::isFocused(bool focusStatus)
{
    QMutexLocker lock(&mutex);
    imageFocused = focusStatus;
}

void ImageAcquisition::stop()
{
    QMutexLocker lock(&mutex);
    stopThread = true;
}

void ImageAcquisition::motorMovingChanged(int motorID,bool motorStatus,int motorPose)
{
    QMutexLocker lock(&mutex);
    if(motorID==LINEAR_MOTOR)
    {
        motorMoving = motorStatus;
        linearMotorPose = motorPose;
        if(linearMotorPose<0)
            linearMotorPose = 0;
        currentChip = (int)floor(linearMotorPose/float(ASG_PIT::settings().getChipStepSize()));
    }
    else
    {
        cameraMotorPose = motorPose;
    }
}

void ImageAcquisition::setAcquisition(bool acqStatus)
{
    QMutexLocker lock(&mutex);
    acquireImages = acqStatus;
    if(acqStatus==false)
    {
        saveImageBuffer();
        motorPose  = 0;
    }
    else
    {
        imageFocused = false;
        newStart = true;
    }
}

void ImageAcquisition::shutterSpeedChanged(int shutSpeed)
{
    QWriteLocker locker(&readWriteLock);
    actualShutterSpeed = shutSpeed;
}

bool ImageAcquisition::isAcquiring()
{
    return acquireImages;
}

void ImageAcquisition::run()
{
    int stepSize=1;
    int endPose = 0;
    while(!stopThread)
    {
        if(!acquireImages)
        {
            usleep(100000);
            continue;
        }
        if(newStart)
        {
            firstStep = true;
            timer.restart();
            newStart     = false;
            imageFocused = false;
            repeatAnalysis = false;
            motorPose = homePose;
            lastPose = 13234;
            motorMoving = true;
            stepSize  = qRound(ASG_PIT::settings().getImageStepSize() - ASG_PIT::settings().getImageStepSize()*ASG_PIT::settings().getOverLapPerc()/100.0);
            numImages = qRound(ASG_PIT::settings().getChipStepSize()*11/double(stepSize));
            endPose = (ASG_PIT::settings().getChipStepSize()*11 + stepSize) + homePose;
            qDebug()<<"StartPose:"<<homePose<<" endPose:"<<endPose<<" number of images 2 capture:"<<numImages;
            relativePath = ASG_PIT::settings().getImagesPath();
            namesBuffer.clear();
            if (!imageDir.exists(relativePath))
                imageDir.mkdir(relativePath);
            relativePath.append(QString("/%1_%2").arg(dateTime.currentDateTime().toString(QString("yyyyMMddThhmms"))).arg(printHeadCode));
            if(!imageDir.exists(relativePath))
            {
                if(!imageDir.mkdir(relativePath))
                {
                       qDebug() <<"Error Creating Folder:"<<qPrintable(relativePath)<<" try creating it manually";
                       acquireImages = false;
                       continue;
                }
            }
            ASG_PIT::settings().setAcqLastPath(relativePath);
        }

        while(motorMoving){usleep(10000);}

        if(!imageFocused)
        {
            usleep(10000);
            continue;
        }
        if(firstStep)
        {
            motorController->move(LINEAR_MOTOR,homePose);
            while(motorMoving){usleep(1000);}
            firstStep = false;
            imageFocused = false;
            continue;
        }
        if(lastPose !=motorPose)
        {
            lastPose = motorPose;
            captureImage();
            if(encapAnalysisEnabled && (framesBuffer.size()<=143))
            {
                try
                {
                    QTime time2Process;
                    time2Process.start();
                    encapCoverage->processImage(cvCloneImage(framesBuffer.last()),QString("%1/%2").arg(relativePath).arg(imageName),true);
                    qDebug()<<"Time 2 process:"<<time2Process.elapsed();
                    repeatAnalysis = false;
                }
                catch(ReferenceLineException &e)
                {
                    cvReleaseImage(&framesBuffer.last());
                    framesBuffer.pop_back();
                    namesBuffer.pop_back();
                    repeatAnalysis = true;
                    shutterSpeed = actualShutterSpeed - 500;
                    emit setShutterSpeed(shutterSpeed);
                    continue;
                }
            }
        }
        else if((encapAnalysisEnabled &&!repeatAnalysis) || !encapAnalysisEnabled)
        {
            continue;
        }
        else if(encapAnalysisEnabled && repeatAnalysis)// pose is the same and it's a repeat so analyse again
        {
            try
            {
                captureImage();
                encapCoverage->processImage(cvCloneImage(framesBuffer.last()),QString("%1/%2").arg(relativePath).arg(imageName),true);
                repeatAnalysis = false;
            }
            catch(ReferenceLineException &e)
            {
                shutterSpeed+=100;
                qDebug()<<"Shutter Speed changed to:"<<shutterSpeed;
                if(shutterSpeed>4095)
                    shutterSpeed=4095;
                emit setShutterSpeed(shutterSpeed);
                cvReleaseImage(&framesBuffer.last());
                framesBuffer.pop_back();
                namesBuffer.pop_back();
                repeatAnalysis = true;
                usleep(100000);
                continue;
            }
        }
        imageFocused = false;
        motorMoving = true;
        motorPose+=stepSize;
        //qDebug()<<"Current Image Number:"<<numImagesCaptured<<" Total:"<<numImages<<" Position:"<<motorPose;
        // Check if the next image is the last
        if( (motorPose+stepSize) >= endPose)
        {
            qDebug()<<"Focus the last image";
            emit lastImageReached();
        }

        motorController->move(LINEAR_MOTOR,motorPose);
        while(motorMoving){usleep(10000);}

        if(motorPose > endPose)
        {
            qDebug() <<"Acquisition took:"<<timer.elapsed()<<" msec";
            acquireImages = false;
            newStart = true;
            motorPose = 0;
            saveImageBuffer();
            if(encapAnalysisEnabled)
            {
                encapCoverage->finishProcessing();
                encapAnalysisEnabled = false;
            }
        }
    }
}

void ImageAcquisition::setHomePose(int pose)
{
    this->homePose = pose;
}

void ImageAcquisition::setPrintHeadCode(QString code)
{
    printHeadCode = code;
}

void ImageAcquisition::recieveImage(IplImage *_frameImage)
{
    QMutexLocker locker(&mutex);
    if(this->frameImage)
        cvReleaseImage(&frameImage);
    frameImage = cvCloneImage(_frameImage);
    imageFocused = true;
}

void ImageAcquisition::releaseImageBuffer()
{
    /*
    for(int i=0;i<framesBuffer.size();i++)
    {
        if(framesBuffer[i])
            cvReleaseImage(&framesBuffer[i]);
    }
    */
    framesBuffer.clear();
}

void ImageAcquisition::saveImageBuffer()
{
    emit savingImages();
    usleep(50000);
    QTime imageSaveTimer;
    imageSaveTimer.restart();
    for(int i=0;i<framesBuffer.size();i++)
    {
        cvSaveImage(qPrintable(namesBuffer[i]),framesBuffer[i]);
        emit progressValue(qRound(i*100/framesBuffer.size()));
        // immediately release to free up memory !
        if(framesBuffer[i])
            cvReleaseImage(&framesBuffer[i]);
    }
    releaseImageBuffer();
    if(frameImage)
        cvReleaseImage(&frameImage);
    emit acquisitionEnded(encapAnalysisEnabled);
    qDebug() <<"Saving Image buffer took :"<<imageSaveTimer.elapsed()<<"msec";
}

void ImageAcquisition::captureImage()
{
    char number[10];
    sprintf(number,"%03d",framesBuffer.size());
    framesBuffer.push_back(cvCloneImage(frameImage));
    imageName = QString("image_%1_%2_%3_Chip%4_%5_%6").arg(number).arg(dateTime.currentDateTime().toString(QString("yyyyMMdd_hhmms"))).arg(printHeadCode).arg(currentChip).arg(linearMotorPose).arg(cameraMotorPose);
    namesBuffer.push_back(QString("%1/%2.bmp").arg(relativePath).arg(imageName));
    emit progressValue(qRound(framesBuffer.size()*100/numImages));
}
