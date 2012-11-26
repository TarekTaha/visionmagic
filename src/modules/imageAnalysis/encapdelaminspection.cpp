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

#include "encapdelaminspection.h"

EncapDelamInspection::EncapDelamInspection(QWidget *_parent ,MotorControl *_motorController):
        QThread(_parent),
        parent(_parent),
        motorController(_motorController),
        frameImage(NULL),
        stopThread(false),
        inspectImages(false),
        imageFocused(false),
        newStart(true),
        motorMoving(true),
        firstStep(false),
        motorPose(0),
        currentChip(0),
        homePose(0),
        printHeadCode("")
{
    relativePath =  ASG_PIT::settings().getImagesPath();
    sectionsPerDie = ASG_PIT::settings().getInspectionSectionsPerDie();
}

EncapDelamInspection::~EncapDelamInspection()
{

}

void EncapDelamInspection::isFocused(bool focusStatus)
{
    QMutexLocker lock(&mutex);
    imageFocused = focusStatus;
}

void EncapDelamInspection::keyPressed(QKeyEvent *e)
{
    qDebug()<<"Key Pressed";
    if(e->key() == Qt::Key_Space)
    {
        nextPositionIndex = currentPositionIndex+1;
        qDebug()<<"Current Position Index is:"<<currentPositionIndex;
        qDebug()<<"Next    Position Index is:"<<nextPositionIndex;
    }
    else if(e->key() == Qt::Key_A)
    {
        if(currentPositionIndex>0)
            nextPositionIndex = currentPositionIndex - 1;
    }
    else if(e->key() == Qt::Key_D)
    {
        nextPositionIndex = currentPositionIndex + 1;
    }
}

void EncapDelamInspection::stop()
{
    QMutexLocker lock(&mutex);
    stopThread = true;
}

void EncapDelamInspection::motorMovingChanged(int motorID,bool motorStatus,int motorPose)
{
    QMutexLocker lock(&mutex);
    if(motorID==LINEAR_MOTOR)
    {
        motorMoving = motorStatus;
        linearMotorPose = motorPose;
        if(linearMotorPose<0)
            linearMotorPose = 0;
        currentPositionIndex = (int)floor(linearMotorPose/float(ASG_PIT::settings().getImageStepSize()));
        nextPositionIndex = currentPositionIndex;
        currentChip = (int)floor(linearMotorPose/float(ASG_PIT::settings().getChipStepSize()));
        currentDieSection = (int)floor((linearMotorPose - ASG_PIT::settings().getChipStepSize()*currentChip)/float(ASG_PIT::settings().getChipStepSize()/float(sectionsPerDie)));
        emit locationChanged(currentPositionIndex,currentChip,currentDieSection);
    }
    else
    {
        cameraMotorPose = motorPose;
    }
}

void EncapDelamInspection::setInspection(bool inspectStatus)
{
    QMutexLocker lock(&mutex);
    inspectImages = inspectStatus;
    if(inspectImages==false)
    {
        saveImageBuffer();
        linearMotorPose  = 0;
    }
    else
    {
        imageFocused = false;
        newStart = true;
    }
}

void EncapDelamInspection::shutterSpeedChanged(int shutSpeed)
{
    QWriteLocker locker(&readWriteLock);
    actualShutterSpeed = shutSpeed;
}

bool EncapDelamInspection::isInspecting()
{
    return inspectImages;
}

void EncapDelamInspection::run()
{
    int stepSize=1;
    int endPose = 0;
    while(!stopThread)
    {
        if(!inspectImages)
        {
            usleep(100000);
            continue;
        }
        if(newStart)
        {
            timer.restart();
            newStart     = false;
            imageFocused = false;
            linearMotorPose = homePose;
            lastPose = 13234;
            motorMoving = true;
            currentPositionIndex = -1; // to indicate start
            nextPositionIndex = 0;
            stepSize  = qRound(ASG_PIT::settings().getImageStepSize() - ASG_PIT::settings().getImageStepSize()*ASG_PIT::settings().getOverLapPerc()/100.0);
            numImages = (int)ceil(ASG_PIT::settings().getChipStepSize()*11/double(stepSize)) + 1;
            endPose = (ASG_PIT::settings().getChipStepSize()*11 + stepSize) + homePose;
            qDebug()<<"StartPose:"<<homePose<<" endPose:"<<endPose<<" number of images 2 capture:"<<numImages;
            imagePositions.clear();
            imagePositions.resize(numImages);
            for(int i=0;i<numImages;i++)
            {
                imagePositions[i] = homePose + i*stepSize;
                qDebug()<<"Position["<<i<<"] is:"<<imagePositions[i];
            }
            emit newImagePositions(imagePositions);
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
                       inspectImages = false;
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
        if(currentPositionIndex != nextPositionIndex && nextPositionIndex<numImages)
        {
            motorController->move(LINEAR_MOTOR,imagePositions[nextPositionIndex]);
            currentPositionIndex = nextPositionIndex;
            while(motorMoving){usleep(1000);}
            imageFocused = false;
            emit locationChanged(currentPositionIndex,currentChip,currentDieSection);
            // Skip after moving to get a focused Images
            continue;
        }
        // if it's a new physical position then save keep an image for reference
        if(lastPose != linearMotorPose)
        {
            lastPose = linearMotorPose;
            captureImage();
        }
        else
        {
            continue;
        }
    }
}

void EncapDelamInspection::setHomePose(int pose)
{
    this->homePose = pose;
}

void EncapDelamInspection::setPrintHeadCode(QString code)
{
    printHeadCode = code;
}

void EncapDelamInspection::setSectionsPerDie(int numSections)
{
    this->sectionsPerDie = numSections;
}

void EncapDelamInspection::setNextPositionIndex(int nextPos)
{
    nextPositionIndex = nextPos;
}

void EncapDelamInspection::recieveImage(IplImage *_frameImage)
{
    QMutexLocker locker(&mutex);
    if(this->frameImage)
        cvReleaseImage(&frameImage);
    frameImage = cvCloneImage(_frameImage);
    imageFocused = true;
}

void EncapDelamInspection::releaseImageBuffer()
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

void EncapDelamInspection::saveImageBuffer()
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
    emit encapDelamInspectionEnded();
    qDebug() <<"Saving Image buffer took :"<<imageSaveTimer.elapsed()<<"msec";
}

void EncapDelamInspection::captureImage()
{
    char number[10];
    sprintf(number,"%03d",framesBuffer.size());
    framesBuffer.push_back(cvCloneImage(frameImage));
    imageName = QString("image_%1_%2_%3_Chip%4_%5_%6").arg(number).arg(dateTime.currentDateTime().toString(QString("yyyyMMdd_hhmms"))).arg(printHeadCode).arg(currentChip).arg(linearMotorPose).arg(cameraMotorPose);
    namesBuffer.push_back(QString("%1/%2.bmp").arg(relativePath).arg(imageName));
    emit progressValue(qRound(framesBuffer.size()*100/numImages));
}
