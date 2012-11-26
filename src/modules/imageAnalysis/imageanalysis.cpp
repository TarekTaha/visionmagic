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
#include "imageanalysis.h"

ImageAnalysis::ImageAnalysis():
        imagesPath("./"),
        stopThread(false),
        paused(false),
        nozzelPressed(false),
        displayContainerInitialized(false),
        repeatAnalysis(false),
        firstTime(true),
        analysisMode(IDLE),
        onlineImage(NULL),
        disImage(NULL),
        image(NULL),
        temp(NULL),
        oldOnlineImage(NULL),
        displayFrame(NULL),
        originalDisplay(NULL),
        analysisImage(NULL),
        tempFrameImage(NULL),
        oldImage(NULL),
        correlationImage(NULL),
        storage(NULL)
{
    crossRowsReferences[0] = 3;
    crossRowsReferences[1] = 5;
    crossRowsReferences[2] = 11;
    crossRowsReferences[3] = 13;
    crossRowsReferences[4] = 19;
    crossRowsReferences[5] = 21;
    crossRowsReferences[6] = 27;
    crossRowsReferences[7] = 29;
    crossRowsReferences[8] = 35;
    crossRowsReferences[9] = 37;
    chipIndex = -1;
    useSecondFilter = false;
    for(int i=0;i<10;i++)
        dummyGapFilled[i] =false;
    qDebug() <<"ImageAnalysis: Started";
}

ImageAnalysis::~ImageAnalysis()
{
    if(storage)
        cvClearMemStorage( storage );
    if(displayFrame)
        cvReleaseImage(&displayFrame);
    if(originalDisplay)
        cvReleaseImage(&originalDisplay);
    if(disImage)
        cvReleaseImage(&disImage);
    if(oldOnlineImage)
        cvReleaseImage(&oldOnlineImage);
    if(onlineImage)
        cvReleaseImage(&onlineImage);
    if(nozzelTemplates.size()!=0)
    {
        for(int i=0;i<nozzelTemplates.size();i++)
        {
            if(nozzelTemplates[i])
                cvReleaseImage(&nozzelTemplates[i]);
        }
    }
    if(edgeStartTemplates.size()!=0)
    {
        for(int i=0;i<edgeStartTemplates.size();i++)
        {
            if(edgeStartTemplates[i])
                cvReleaseImage(&edgeStartTemplates[i]);
        }
    }
    if(edgeEndTemplates.size()!=0)
    {
        for(int i=0;i<edgeEndTemplates.size();i++)
        {
            if(edgeEndTemplates[i])
                cvReleaseImage(&edgeEndTemplates[i]);
        }
    }
}

IplImage * ImageAnalysis::analyseImage(const IplImage *_frameImage, bool _repeatAnalysis)
{
    qDebug() <<"ImageAnalysisAlgorithms: Analysing new Image";
    cvSetErrMode(CV_ErrModeParent);
    repeatAnalysis = _repeatAnalysis;
    distanceBetweenNozzels = 0;
    overlapPercentage = ASG_PIT::settings().getOverLapPerc();
    if(!_frameImage)
    {
        qDebug()<<"Recieved an empty image for analysis, ";
        return NULL;
    }
    displayFrame  = cvCreateImage(cvGetSize(_frameImage), 8, 3 );
    analysisImage = cvCreateImage(cvGetSize(_frameImage), 8, 1 );
    if(_frameImage->nChannels==1)
    {
        qDebug()<<"Copying Frameimage (1 channel) to displayImage and Analysis Image ";
        cvCvtColor( _frameImage, displayFrame, CV_GRAY2BGR );
        cvCopy(_frameImage,analysisImage);
        qDebug()<<"Copying Done";
    }
    else
    {
        qDebug()<<"Copying Frameimage (3 channels) to displayImage and Analysis Image ";
        cvCvtColor( _frameImage, analysisImage, CV_BGR2GRAY );
        cvCopy(_frameImage,displayFrame);
        qDebug()<<"Copying Done";
    }
    if(originalDisplay)
    {
        qDebug()<<"Copying displayFrame into original Display";
        cvCopy(displayFrame,originalDisplay);
        qDebug()<<"Copying Done";
    }
    else
        originalDisplay = cvCloneImage(displayFrame);
    emit sendOriginalDisplayImage(originalDisplay);
    // Reset the number of manual input clicks
    manualInputClicks = 0;
    if( cvGetErrStatus() < 0 )
        qDebug()<<"Open CV is reporting some erros";
    /* This is a new Image so Save the result of last analysis
       for correlation
       */

    if(!repeatAnalysis && !firstTime)
    {
        // Store the information from the last analysis
        correlationNozzleMap.clear();
        correlationNozzleMap << nozzelMap;
        if(correlationImage)
        {
            qDebug()<<"Releasing correlationImage Image";
            cvReleaseImage(&correlationImage);
            qDebug()<<"CorrelationImage Released";
        }
        if(oldImage)
            correlationImage = cvCloneImage(oldImage);
    }
    else
    {
        prevImageContainsStartingEdge = false;
    }

    allNozzels.clear();
    nozzelMap.clear();
    //Step 1: Detect Edges it uses the analysis Image to detect Edges
    qDebug() <<"ImageAnalysisAlgorithms: Detecting Edge";
    detectEdges(analysisImage);
    //Step 2: Detect good nozzels based on the templates
    qDebug() <<"ImageAnalysisAlgorithms: Locating Good Nozzels";
    allNozzels = templateNozzleMatch(analysisImage);
    qDebug() << "Number of nozzles found:"<<allNozzels.size();
    //Step 3: Sort the nozzels into rows based on their height
    qDebug() <<"ImageAnalysisAlgorithms: Sorting Nozzels into Rows";
    nozzelMap  = sortMatchesIntoRows(allNozzels);
    nozzelMap  = selectActiveChannels(nozzelMap);
    // keep in allNozzels only those nozzles from selected channels
    keepRelevantChannelRows();
//    drawRowLines(nozzelMap);
//    drawNozzels(allNozzels,CV_RGB(0,255,0),true);
//    return displayFrame;
    // this should be called after the rows are sorted
    distanceBetweenNozzels = getDistanceBetweenNozzels();
    //Step 4: Locate Dead nozzels based on know geometric propeties of the printheads
    qDebug() <<"ImageAnalysisAlgorithms: Locating Dead Nozzels";
    locateDeadNozzels();

    //Step 5: Draw it
    qDebug() <<"ImageAnalysisAlgorithms: Displaying the nozzels";
    nozzelMap = sortMatchesIntoRows(allNozzels);

    if(foundStartEdge && generateDNMap)
        localizeChipStart();

    if(correlationImage && generateDNMap)
    {
        correlateImages(correlationImage,analysisImage,overlapPercentage,false);
    }

    //Step 6: apply the second filter if it's enabled (this should be after correlation)
    if(useSecondFilter)
    {
        filterDeadNozzleNeighbours();
    }
    drawNozzels(nozzelMap);
    drawRowLines(nozzelMap);
    // Store old image details in case we want to repeat
    // the analysis
    if(oldImage)
    {
        qDebug()<<"Releasing Old Image";
        cvReleaseImage(&oldImage);
        qDebug()<<"Old Image Released";
    }
    oldImage = cvCloneImage(analysisImage);
    /*
    if(foundStartEdge)
    {
        prevImageContainsStartingEdge = true;
        int maxIndex=0,dummyCount;
        // This is a way to make sure that we localize and
        // correlate the whole drop triangle.
        // This is not the best solution and should be thought
        // off carefully.
        for(int i=0;i<nozzelMap.size();i++)
        {
            maxIndex   = 0;
            dummyCount = 0;
            for(int j=nozzelMap[i].size()-1;j>=0;j--)
            {
                if(nozzelMap[i][j].getStatus() !=MatchingNozzle::Dummy)
                {
                    if (nozzelMap[i][j].getNozzleNum()>maxIndex)
                        maxIndex = nozzelMap[i][j].getNozzleNum();
                }
                else
                {
                    dummyCount++;
                }
            }
            if(maxIndex < crossRowsReferences[i] || dummyCount!=3)
            {
                dummyGapFilled[i] = false;
            }
            else
                dummyGapFilled[i] = true;
        }
    }
    else
    {
        prevImageContainsStartingEdge = false;
    }
    */
    saveResult2DNMap();
    qDebug() <<"Analysis Finished";
    if(this->analysisImage)
    {
        cvReleaseImage(&analysisImage);
    }
    emit finishedMatching();
    firstTime = false;
    return displayFrame;
}

QVector <MatchingNozzle> ImageAnalysis::applySecondFilter(QVector <MatchingNozzle> matchLocations)
{
    int tempW,tempH;
    QVector <MatchingNozzle> matchLocationsNew;
//    qDebug() <<"ImageAnalysisAlgorithms: Size of the old is:%d",matchLocations.size());
    for(int k=0; k<matchLocations.size();k++)
    {
        int j = qRound(matchLocations[k].location.x());
        int i = qRound(matchLocations[k].location.y());
        tempW = matchLocations[k].getTempW();
        tempH = matchLocations[k].getTempH();
        float maxValue=0;
        int maxValueIndex=0;
        MatchingNozzle bestLocationMatch;
        // elemetns in the same region, find the max value
        //Find the MAX local Value
        for(int m=0; m<matchLocations.size();m++)
        {
            int    x = qRound(matchLocations[m].location.x());
            int    y = qRound(matchLocations[m].location.y());
            float  v = matchLocations[m].getMatchingValue();
            if(j>=(x-tempW) && j<=(x+tempW) && i>=(y-tempH) && i<=(y+tempH))
            {
                if(v>maxValue)
                {
                    maxValue = v;
                    maxValueIndex = m;
                }
            }
        }
        // add the max Value to the new array
        matchLocationsNew.push_back(matchLocations[maxValueIndex]);
        // Remove all those Less than Max
        for(int m=0; m<matchLocations.size();m++)
        {
            int    x = qRound(matchLocations[m].location.x());
            int    y = qRound(matchLocations[m].location.y());
            float  v = matchLocations[m].getMatchingValue();
            if(j>=(x-tempW) && j<=(x+tempW) && i>=(y-tempH) && i<=(y+tempH))
            {
                if(v<maxValue)
                {
                    matchLocations.remove(m);
                    m--;
                }
            }
        }
    }
//
//    qDebug() <<"ImageAnalysisAlgorithms: Size of the new is:%d",matchLocationsNew.size());
    matchLocations.clear();
    matchLocations = matchLocationsNew;
    return matchLocations;
}

void ImageAnalysis::clearImageContainers()
{
    qDebug() << "Resetting image containers";
    QMutexLocker locker(&mutex);
    if(oldImage)
        cvReleaseImage(&oldImage);
    if(correlationImage)
        cvReleaseImage(&correlationImage);
    if(analysisImage)
        cvReleaseImage(&analysisImage);
//    if(displayFrame)
//        cvReleaseImage(&displayFrame);
    if(originalDisplay)
        cvReleaseImage(&originalDisplay);
    if(tempFrameImage)
        cvReleaseImage(&tempFrameImage);
    displayFrame = 0;
    originalDisplay = 0;
    analysisImage = 0;
    tempFrameImage = 0;
    oldImage = 0;
    correlationImage = 0;
    qDebug() << "Image Containers Reset";
}
/*!
  One of the major issues that I still have in localization/correlation is in the edge detection
  and proper row assignment. The reason is that because sometimes I don't get the whole drop triangle.
  Some of the rows will be missing and the row assignment will not be incorrect.
  A solution can be by forcing the user to manually select the edge (Lines representing the edge shape).
  then the correlation will be corretly fixd.
  */
void ImageAnalysis::correlateImages(IplImage *prevImage,IplImage *currentImage,int overLap,bool mouseClicked)
{
    qDebug() << "Correlating consequtive Images";
    if(!prevImage && currentImage)
        qDebug() << "Can't correlate empty image sections";
    if(overLap<5)
    {
        qDebug() << "At least 5% overlap is needed to match images";
        return;
    }
    int srcOverLapXSize   = (int)(prevImage->width*overLap/100.0);
    // This is to be sure that we can have exactly half sized ROIs for filtering
    if ((srcOverLapXSize%2)!=0)
    {
        srcOverLapXSize+=1;
    }
    int heightBufferSection = 10;
    int prevROIStartX    = prevImage->width - srcOverLapXSize;   int prevROIEndX    = prevImage->width;
    int prevROIStartY    = heightBufferSection;                  int prevROIEndY    = prevImage->height - 2*heightBufferSection;

    // This section of the prevImage will be used as a template to be matched in the currentImage
    qDebug() << "Getting Patch from prev Image";
    // The freaking rectangle is not an actual rectangle, it's the starting point edge (top-left) + (width,height) not another opposite point
    cvSetImageROI( prevImage, cvRect(prevROIStartX,prevROIStartY, prevROIEndX , prevROIEndY ));
    IplImage * prevMatchTempPatch = cvCreateImage( cvSize(srcOverLapXSize,prevImage->height-2*heightBufferSection),prevImage->depth,prevImage->nChannels);
    cvCopy(prevImage,prevMatchTempPatch);

    // take the first overLap + 2% of the second image as the ROI
    int destOverLapXSize   = (int)(prevImage->width*(overLap+1)/100.0);
    if((destOverLapXSize%2)!=0)
    {
        destOverLapXSize+=1;
    }
    cvSetImageROI( currentImage, cvRect(0, 0, destOverLapXSize, currentImage->height ));
    IplImage * destMatch = cvCreateImage( cvSize(destOverLapXSize,currentImage->height),currentImage->depth,currentImage->nChannels);
    cvCopy(currentImage,destMatch);
//    qDebug() << "Filtering Source";
//    IplImage* pyr = cvCreateImage( cvSize((int)(prevMatchTempPatch->width/2.0), (int)(prevMatchTempPatch->height/2.0)), 8, prevMatchTempPatch->nChannels );
//    // down-scale and upscale the image to filter out the noise
//    cvPyrDown(prevMatchTempPatch, pyr, 7 );
//    cvPyrUp(pyr, prevMatchTempPatch, 7 );
//    cvReleaseImage(&pyr);

//    qDebug() << "Filtering Destination";
//    pyr = cvCreateImage( cvSize((int)(destMatch->width/2.0), (int)(destMatch->height/2.0)), 8, destMatch->nChannels );
//    // down-scale and upscale the image to filter out the noise
//    cvPyrDown(destMatch, pyr, 7 );
//    cvPyrUp(pyr, destMatch, 7 );
//    cvReleaseImage(&pyr);

    // OpenCV says that size of the result must be the following:
    CvSize resultSize;
    resultSize.height= destMatch->height - prevMatchTempPatch->height + 1;
    resultSize.width = destMatch->width  - prevMatchTempPatch->width  + 1;
    //This will hold the result of the Match
    IplImage* result = cvCreateImage( resultSize,IPL_DEPTH_32F,1);
    /*!METHODS can be :  CV_TM_SQDIFF
                         CV_TM_SQDIFF_NORMED
                         CV_TM_CCORR
                         CV_TM_CCORR_NORMED
                         CV_TM_CCOEFF
                         CV_TM_CCOEFF_NORMED
                         */
    cvMatchTemplate( destMatch, prevMatchTempPatch, result, CV_TM_CCORR_NORMED);
    double minValue, maxValue;
    CvPoint minLoc, maxLoc;
    cvMinMaxLoc( result, &minValue, &maxValue, &minLoc, &maxLoc );
    cvRectangle( displayFrame,maxLoc,cvPoint(maxLoc.x + prevMatchTempPatch->width, maxLoc.y + prevMatchTempPatch->height),CV_RGB(0,0,255), 2, 0, 0 );
    cvSetImageROI(destMatch, cvRect(maxLoc.x,maxLoc.y,prevMatchTempPatch->width,prevMatchTempPatch->height));
    IplImage * matchedSection = cvCreateImage( cvSize(prevMatchTempPatch->width,prevMatchTempPatch->height),8,destMatch->nChannels);
    cvCopy(destMatch,matchedSection);

    //cvSaveImage("temp1.bmp",prevMatchTempPatch);
    //cvSaveImage("temp2.bmp",matchedSection);

    // Reset the ROI back to normal
    cvResetImageROI(prevImage);
    cvResetImageROI(currentImage);

    //! Phase two, find nozzels in old and new images and correlate them
    QVector <MatchingNozzle*> oldNozzels;
    QVector <MatchingNozzle*> newNozzels;
    // generate the nozzel list in the prev image in the matched patch
    for(int i=0;i<correlationNozzleMap.size();i++)
    {
        for(int j=0;j<correlationNozzleMap[i].size();j++)
        {
            int x = qRound(correlationNozzleMap[i][j].location.x());
            int y = qRound(correlationNozzleMap[i][j].location.y());
            if(x>=prevROIStartX && y>=prevROIStartY && y<=prevROIEndY)
            {
                oldNozzels.push_back(&correlationNozzleMap[i][j]);
            }
        }
    }
    // generate the nozzel list in the current image in the matched patch
    for(int i=0;i<nozzelMap.size();i++)
    {
        for(int j=0;j<nozzelMap[i].size();j++)
        {
            int x = qRound(nozzelMap[i][j].location.x());
            int y = qRound(nozzelMap[i][j].location.y());
            if(x>maxLoc.x && x<(maxLoc.x+prevMatchTempPatch->width) && y>maxLoc.y && y<(maxLoc.y+prevMatchTempPatch->height))
            {
                newNozzels.push_back(&nozzelMap[i][j]);
            }
        }
    }

    /*! now for each old nozzel find it in the new nozzel set
      and give it it's previous index.
    */
    for(int i=0;i<oldNozzels.size();i++)
    {
        int oldX = qRound(oldNozzels[i]->location.x());
        int oldY = qRound(oldNozzels[i]->location.y());
        for(int j=0;j<newNozzels.size();j++)
        {
            int newX  = qRound(newNozzels[j]->location.x());
            int newY  = qRound(newNozzels[j]->location.y());
            int tempW = newNozzels[j]->getTempW();
            int tempH = newNozzels[j]->getTempH();
            // the (newX/Y - maxLoc.x/y) transfers to the local patch corrdinate
            // the prevROIStartX/Y  transfers the patch in the old coordinate to the one in the old coordinate
            int newXTranslated = newX - maxLoc.x + prevROIStartX;
            int newYTranslated = newY - maxLoc.y + prevROIStartY;
            // there is always a pixel or 2 error in the matching, nothing is perfect
            int bufferX = int(tempW/2.0);
            int bufferY = int(tempH/2.0);
            int diffX = abs(newXTranslated - oldX);
            int diffY = abs(newYTranslated - oldY);
            if(diffX<bufferX && diffY<bufferY)
            {
                newNozzels[j]->setRow(oldNozzels[i]->getRow());
                newNozzels[j]->setNozzleNum(oldNozzels[i]->getNozzleNum());
                if(!mouseClicked)
                    newNozzels[j]->setStatus(oldNozzels[i]->getStatus());
                newNozzels[j]->setLocalized(true);
                cvInitFont(&font,CV_FONT_VECTOR0,0.5,0.5,0,2);
                sprintf(str,"%d",newNozzels[j]->getNozzleNum());
                cvPutText(displayFrame,str,cvPoint( newX+tempW/3, newY ),&font,CV_RGB(255,255,255));
                break;
            }
        }
    }
    // The nozzels on the right side (after the patch location)
    for(int i=0;i<nozzelMap.size();i++)
    {
//        bool rowDummyGapFixed = false;
        for(int j=1;j<nozzelMap[i].size();j++)
        {
            if(!nozzelMap[i][j].isLocalized() && nozzelMap[i][j-1].isLocalized())
            {
                if(nozzelMap[i][j-1].getStatus()==MatchingNozzle::Dummy &&nozzelMap[i][j-1].getNozzleNum()<3)
                {
                    nozzelMap[i][j].setStatus(MatchingNozzle::Dummy);
                    nozzelMap[i][j].setNozzleNum(nozzelMap[i][j-1].getNozzleNum() + 1);
                    nozzelMap[i][j].setLocalized(true);
                }
                // TODO:: check if we need to skip rows 11 and 12
                else if(nozzelMap[i][j-1].getStatus()==MatchingNozzle::Dummy && nozzelMap[i][j-1].getNozzleNum()==3)
                {
                    nozzelMap[i][j].setNozzleNum(crossRowsReferences[nozzelMap[i][j].getRow()+2]+1);
                    nozzelMap[i][j].setLocalized(true);
                }
                else
                {
                    nozzelMap[i][j].setNozzleNum(nozzelMap[i][j-1].getNozzleNum() + 1);
                    nozzelMap[i][j].setLocalized(true);
                }
                if(nozzelMap[i][j].getNozzleNum()>639)
                {
                    nozzelMap[i].remove(j);
                    j--;
                    continue;
                }
//                if(!dummyGapFilled[i] && nozzelMap[i][j].getNozzleNum>crossRowsReferences[nozzelMap[i][j].getRow()] && !rowDummyGapFixed)
//                {
//                    rowDummyGapFixed = true;
//                    nozzelMap[i][j]->getStatus() = MatchingNozzle::Dummy;
//                    dummyGapFilled[i] = true;
//                    nozzelMap[i][j].getRow() = nozzelMap[i][j].getRow() + 2;
//                }
                if((i==10 && nozzelMap[i][j].getNozzleNum()>crossRowsReferences[8]) || (i==11 && nozzelMap[i][j].getNozzleNum()>crossRowsReferences[9]))
                    nozzelMap[i][j].setStatus(MatchingNozzle::Dummy);
            }
        }
    }
    // The nozzles on the left side, (unmatached nozzles from prev)
    for(int i=0;i<nozzelMap.size();i++)
        for(int j = (nozzelMap[i].size()-1);j>=0;j--)
        {
            // a nozzel with no assigned location
            if((j+1)<(nozzelMap[i].size()))
            {
                if(!nozzelMap[i][j].isLocalized() && nozzelMap[i][j+1].isLocalized())
                {
                    if(nozzelMap[i][j+1].getStatus() == MatchingNozzle::Dummy && nozzelMap[i][j+1].getNozzleNum()!=1)
                    {
                        nozzelMap[i][j].setNozzleNum(nozzelMap[i][j+1].getNozzleNum() -1);
                        nozzelMap[i][j].setLocalized(true);
                        nozzelMap[i][j].setStatus(MatchingNozzle::Dummy);
                    }
                    else if(nozzelMap[i][j+1].getStatus() == MatchingNozzle::Dummy && nozzelMap[i][j+1].getNozzleNum()==1)
                    {
                        nozzelMap[i][j].setNozzleNum(crossRowsReferences[nozzelMap[i][j].getRow()]);
                        nozzelMap[i][j].setLocalized(true);
                    }
                    else
                    {
                        nozzelMap[i][j].setNozzleNum(nozzelMap[i][j+1].getNozzleNum() -1);
                        nozzelMap[i][j].setLocalized(true);
                    }
                }
            }
    }
    qDebug() <<"Images correlated";
}

void ImageAnalysis::createOnlineAnalysisFolder()
{
    QDateTime dateTime;
    QDir tempDir;
    onlineAnalysisPath = QString("%1/%2_%3_Online_Analysis").arg(ASG_PIT::settings().getImagesPath()).arg(dateTime.currentDateTime().toString(QString("yyyyMMddThhmms"))).arg(printHeadCode);
    if (!tempDir.exists(onlineAnalysisPath))
    {
        tempDir.mkdir(ASG_PIT::settings().getImagesPath());
        tempDir.mkdir(onlineAnalysisPath);
    }
}

void ImageAnalysis::createProductionResponceDoc()
{
    //\ProcessSegments\PR010027\EID-07730\2007_10\30\A00F75_DeadNozzleMap_20071030T142242.csv
    //ProductionResponse.@barcode.@datestring.@site_location.xml
    QFile productionResponceFile;
    QDateTime dateTime = dateTime.currentDateTime();
    QString timeCode = dateTime.toString(QString("yyyyMMddThhmms"));
    QDir destDir;
    destDir.setPath(QString("%1/%2").arg(ASG_PIT::settings().getBasePath()).arg("ProcessSegments/PR010027/"));
    QString equipmentID = ASG_PIT::settings().getEquipmentID();
    QString relativePath = equipmentID;
    qDebug()<<"Relative Path is:"<<relativePath;
    if(!destDir.exists(equipmentID))
    {
        if(destDir.mkdir(equipmentID))
        {
            qDebug()<<"Relative Path ["<<relativePath<<"] created";
            destDir.cd(relativePath);
        }
        else
        {
            qDebug()<<"Could not Create ["<<relativePath<<"]";
            emit warnUser(QString("Could not create [%1] Create it manuall then try again").arg(relativePath),QMessageBox::Warning);
            return;
        }
    }
    else
    {
        destDir.cd(relativePath);
    }
    QString year_month = dateTime.toString(QString("yyyy_MM"));
    relativePath.append(QString("/%1").arg(year_month));
    qDebug()<<"Relative Path is:"<<relativePath;
    if(!destDir.exists(year_month))
    {
        if(destDir.mkdir(year_month))
        {
            qDebug()<<"Relative Path ["<<relativePath<<"] created";
            destDir.cd(year_month);
        }
        else
        {
            qDebug()<<"Could not Create ["<<relativePath<<"]";
            emit warnUser(QString("Could not create [%1] Create it manuall then try again").arg(relativePath),QMessageBox::Warning);
            return;
        }
    }
    else
    {
        destDir.cd(year_month);
    }
    QString day = dateTime.toString(QString("dd"));
    relativePath.append(QString("/%1").arg(day));
    qDebug()<<"Relative Path is:"<<relativePath;
    if(!destDir.exists(day))
    {
        if(destDir.mkdir(day))
        {
            qDebug()<<"Relative Path ["<<relativePath<<"] created";
            destDir.cd(day);
        }
        else
        {
            qDebug()<<"Could not Create ["<<relativePath<<"]";
            emit warnUser(QString("Could not create [%1] Create it manuall then try again").arg(relativePath),QMessageBox::Warning);
            return;
        }
    }
    else
    {
        destDir.cd(day);
    }
    absolutePath = destDir.absolutePath();
    productionResponceDocFileName = QString("%1/ProductionResponse.%2.%3.%4.xml").arg(destDir.absolutePath()).arg(printHeadCode).arg(timeCode).arg(ASG_PIT::settings().getSiteLocation());
    productionResponceFile.setFileName(productionResponceDocFileName);
    if (!productionResponceFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Can not create Production Responce Document";
        emit warnUser("Can not create Production Responce Document!!!",QMessageBox::Critical);
        return;
    }
    QString dNMapfullPath = QString("\\ProcessSegments\\PR010027\\%1\\%2").arg(relativePath).arg(QFileInfo(dnMapFile.fileName()).fileName());
    dNMapfullPath.replace("/","\\");
    QString analysisFileFullPath = QString("\\ProcessSegments\\PR010027\\%1\\%2").arg(relativePath).arg(QFileInfo(analysisFile.fileName()).fileName());
    analysisFileFullPath.replace("/","\\");
    timeCode = dateTime.toString(QString("yyyy-MM-ddThh:mm:s.z"));
    productionDocStream.setDevice(&productionResponceFile);
    productionDocStream <<"<ProductionResponse xmlns=\"http://www.wbf.org/xml/b2mml-v0400\" xmlns:Extended=\"http://www.wbf.org/xml/b2mml-v0400-extensions\">\n";
    productionDocStream <<" <ID>0</ID>\n";
    productionDocStream <<"<SegmentResponse>\n";
    productionDocStream <<"    <ProcessSegmentID>"<<ASG_PIT::settings().getProcessSegmentID()<<"</ProcessSegmentID>\n";
    productionDocStream <<"    <ActualStartTime>"<<timeCode<<"</ActualStartTime>\n";
    productionDocStream <<"    <ActualEndTime>"<<timeCode<<"</ActualEndTime>\n";
    productionDocStream <<"    <ProductionData>\n";
    productionDocStream <<"      <ID>varFDCM_DeadNozzleMap</ID>\n";
    productionDocStream <<"      <Value>\n";
    productionDocStream <<"        <ValueString>"<<dNMapfullPath<<"</ValueString>\n";
    productionDocStream <<"        <DataType>uriReference</DataType>\n";
    productionDocStream <<"        <UnitOfMeasure>none</UnitOfMeasure>\n";
    productionDocStream <<"      </Value>\n";
    productionDocStream <<"    </ProductionData>\n";
    productionDocStream <<"    <ProductionData>\n";
    productionDocStream <<"      <ID>varFDCM_ImageAnalysisResultsFile</ID>\n";
    productionDocStream <<"      <Value>\n";
    productionDocStream <<"        <ValueString>"<<analysisFileFullPath<<"</ValueString>\n";
    productionDocStream <<"        <DataType>uriReference</DataType>\n";
    productionDocStream <<"        <UnitOfMeasure>none</UnitOfMeasure>\n";
    productionDocStream <<"      </Value>\n";
    productionDocStream <<"    </ProductionData>\n";
    productionDocStream <<"    <PersonnelActual>\n";
    productionDocStream <<"      <PersonID>"<<ASG_PIT::settings().getPersonID()<<"</PersonID>\n";
    productionDocStream <<"    </PersonnelActual>\n";
    productionDocStream <<"    <EquipmentActual>\n";
    productionDocStream <<"      <EquipmentID>"<<ASG_PIT::settings().getEquipmentID() <<"</EquipmentID>\n";
    productionDocStream <<"    </EquipmentActual>\n";
    productionDocStream <<"    <MaterialActual>\n";
    productionDocStream <<"      <MaterialClassID>assyPrinthead</MaterialClassID>\n";
    productionDocStream <<"      <MaterialLotID>"<<printHeadCode<<"</MaterialLotID>\n";
    productionDocStream <<"      <MaterialUse>Produced</MaterialUse>\n";
    productionDocStream <<"      <Quantity>\n";
    productionDocStream <<"        <QuantityString>1</QuantityString>\n";
    productionDocStream <<"        <DataType>float</DataType>\n";
    productionDocStream <<"        <UnitOfMeasure>each</UnitOfMeasure>\n";
    productionDocStream <<"      </Quantity>\n";
    productionDocStream <<"      <Extended:MaterialStatus>WIP</Extended:MaterialStatus>\n";
    productionDocStream <<"    </MaterialActual>\n";
    productionDocStream <<"    <Status>"<<(testFailed?("Scrapped"):("WIP"))<<"</Status>\n";
    productionDocStream <<"  </SegmentResponse>\n";
    productionDocStream <<"</ProductionResponse>\n";
    productionDocStream.flush();
    productionResponceFile.close();
}

void ImageAnalysis::detectEdges(IplImage * const detectionImage)
{
    QPointF p1, p2;
    QVector <MatchingNozzle> edgeMatchLocations;

    // Get the edge Start Match
    edgeMatchLocations = detectEdge(detectionImage,edgeStartTemplates);
    if(edgeMatchLocations.size()>0)
    {
        float maxValue=0;
        CvPoint maxLoc = cvPoint(0,0);
        int tempW, tempH,maxIndex=0;
        for(int i=0;i<edgeMatchLocations.size();i++)
        {
            if(edgeMatchLocations[i].getMatchingValue() > maxValue)
            {
                maxValue = edgeMatchLocations[i].getMatchingValue();
                maxLoc.x = qRound(edgeMatchLocations[i].location.x());
                maxLoc.y = qRound(edgeMatchLocations[i].location.y());
                maxIndex = i;
            }
            tempW = edgeMatchLocations[i].getTempW();
            tempH = edgeMatchLocations[i].getTempH();
            cvRectangle( displayFrame,cvPoint( qRound(edgeMatchLocations[i].location.x()), qRound(edgeMatchLocations[i].location.y()) ),
                                  cvPoint( qRound(edgeMatchLocations[i].location.x() + tempW), qRound(edgeMatchLocations[i].location.y() + tempH) ),
                                  CV_RGB(0,255,0), 2, 0, 0 );
        }
        tempW = edgeMatchLocations[maxIndex].getTempW();
        tempH = edgeMatchLocations[maxIndex].getTempH();
        p1.setX(maxLoc.x + tempW); p1.setY(maxLoc.y);
        p2.setX(maxLoc.x) ;        p2.setY(maxLoc.y + tempH);
        startEdge = Line(p1,p2);
        // This gets the edges of a line that passes through 2 points
        getLineEdges(p1,p2,detectionImage->height);
        CvPoint a,b; a.x = (int)p1.x(); a.y = (int)p1.y();
                     b.x = (int)p2.x(); b.y = (int)p2.y();
        cvLine( displayFrame, a ,
                b , CV_RGB(255,0,0), 3, CV_AA, 0 );
        foundStartEdge = true;
        if(!repeatAnalysis && !prevImageContainsStartingEdge)
        {
            chipIndex ++;
        }
    }
    else
    {
        foundStartEdge = false;
    }
    // Get the edge End Match
    edgeMatchLocations = detectEdge(detectionImage,edgeEndTemplates);
    if(edgeMatchLocations.size()>0)
    {
        float maxValue=0;
        CvPoint maxLoc = cvPoint(0,0);
        int tempW, tempH,maxIndex=0;
        for(int i=0;i<edgeMatchLocations.size();i++)
        {
            if(edgeMatchLocations[i].getMatchingValue() > maxValue)
            {
                maxValue = edgeMatchLocations[i].getMatchingValue();
                maxLoc.x = qRound(edgeMatchLocations[i].location.x());
                maxLoc.y = qRound(edgeMatchLocations[i].location.y());
                maxIndex = i;
            }
            tempW = edgeMatchLocations[i].getTempW();
            tempH = edgeMatchLocations[i].getTempH();
            cvRectangle( displayFrame,cvPoint( qRound(edgeMatchLocations[i].location.x()), qRound(edgeMatchLocations[i].location.y()) ),
                                  cvPoint( qRound(edgeMatchLocations[i].location.x() + tempW), qRound(edgeMatchLocations[i].location.y() + tempH) ),
                                  CV_RGB(0,255,0), 2, 0, 0 );

        }
        tempW = edgeMatchLocations[maxIndex].getTempW();
        tempH = edgeMatchLocations[maxIndex].getTempH();
        p1.setX(maxLoc.x + tempW); p1.setY(maxLoc.y);
        p2.setX(maxLoc.x) ;        p2.setY(maxLoc.y + tempH);
        endEdge = Line(p1,p2);
        // This gets the edges of a line that passes through 2 points
        getLineEdges(p1,p2,detectionImage->height);
        CvPoint a,b; a.x = (int)p1.x(); a.y = (int)p1.y();
                     b.x = (int)p2.x(); b.y = (int)p2.y();
        cvLine( displayFrame, a ,
                b , CV_RGB(255,0,0), 3, CV_AA, 0 );
        foundEndEdge = true;
    }
    else
    {
        foundEndEdge = false;
    }
    //TODO: this is a very nasty hack and MUST be fixed
    if(foundEndEdge && !foundStartEdge)
    {
        QPointF p = endEdge.getYaxisIntersectPoint();
        if(p.y()>(displayFrame->width -150))
            foundEndEdge = false;
    }
}

QVector <MatchingNozzle> ImageAnalysis::detectEdge(IplImage *detectionImage,QVector<IplImage *> templateList)
{
    QTime edgeDTimer;
    edgeDTimer.restart();
    QVector <MatchingNozzle> edgeMatchLocations;
    edgeMatchLocations.clear();
    qDebug()<<"Detecting Edge";
    for(int s=0;s<templateList.size();s++)
    {
        IplImage *srcImage       = templateList[s];
        if(!srcImage)
        {
            qDebug()<<"Error Loading edge Template at index:"<<s;
            return edgeMatchLocations;
        }
        qDebug() <<"ImageAnalysisAlgorithms: Template depth:="<<srcImage->depth<<"nChannels:=" <<srcImage->nChannels<<" Detection Image nchannels:="<<detectionImage->nChannels;
        int tempW = srcImage->width;
        int tempH = srcImage->height;
        IplImage* result = ImageAnalysisAlgorithms::templateMatch(detectionImage,srcImage,CV_TM_CCORR_NORMED);
        double threshold = edgeMatchingAccuracy;
        float value;
        /* loop the comparison result array */
        for( int i = 0 ; i < result->height ; i++ )
        {
            for( int j = 0 ; j < result->width ; j++ )
            {
                /* get the match value for the element */
                value = ((float*)(result->imageData + i*result->widthStep))[j];
                /* if value is greater than a threshold, then similar nozzel is found */
                if( value >= threshold )
                {
                    if(edgeMatchLocations.size()==0)
                    {
                        edgeMatchLocations.push_back(MatchingNozzle(QPoint( j, i ),tempW,tempH,value));
                        continue;
                    }
                    bool toBeInserted=false,newRegion=true;
                    for(int m=0; m<edgeMatchLocations.size();m++)
                    {
                        int    x = qRound(edgeMatchLocations[m].location.x());
                        int    y = qRound(edgeMatchLocations[m].location.y());
                        float  v = edgeMatchLocations[m].getMatchingValue();
                        if(j>=(x-tempW) && j<=(x+tempW) && i>=(y-tempH) && i<=(y+tempH))
                        {
                            newRegion = false;
                            if(value>v)
                            {
                                // replace the old value and match with the better one found in the sane area
                                toBeInserted = true;
                                edgeMatchLocations.remove(m);
                                m--;
                            }
                            else
                            {
                                // There is an element in this area with a higher value
                                // so no need to scan the rest as this entry doesn't
                                // qualify.
                                break;
                            }
                        }
                    }
                    if(newRegion || toBeInserted)
                    {
                        edgeMatchLocations.push_back(MatchingNozzle(QPoint( j, i ),tempW,tempH,value));
                    }
                }
            }
        }
        qDebug()<<"Finished Adding Edges";
        if(result)
            cvReleaseImage(&result);
        qDebug()<<"Result Image Relesed";
    }
    qDebug() <<"ImageAnalysisAlgorithms: Time for detecting an edge is:"<<edgeDTimer.elapsed();
    return edgeMatchLocations;
}

bool ImageAnalysis::detectEndEdge(IplImage *_frameImage)
{
    IplImage *tempFrameTest = cvCloneImage(_frameImage);
    IplImage *detectionImage;
    if(tempFrameTest->nChannels==3)
    {
        detectionImage = cvCreateImage( cvGetSize(tempFrameTest), 8, 1 );
        assert(detectionImage->imageData);
        cvCvtColor( tempFrameTest, detectionImage, CV_BGR2GRAY );
    }
    else
        detectionImage = tempFrameTest;
    this->detectEdges(detectionImage);

    if(tempFrameTest)
        cvReleaseImage(&tempFrameTest);
    if(detectionImage)
        cvReleaseImage(&detectionImage);
    return this->foundEndEdge;
}

QString ImageAnalysis::detectRepetitiveFailure()
{
    qDebug()<<"Detecting Repetitive physical Failure";
    testFailed = false;
    unsigned int maxAcceptedRepition = ASG_PIT::settings().getNumOfDieBlockageToFail();
    int clusterSize2Fail = ASG_PIT::settings().getNumOfClusterSizeToFail();
    qDebug()<<" Failure Repetition:"<<maxAcceptedRepition<<" cluster size:"<<clusterSize2Fail;
    resetPhysicalDamageCount();
    for(int i=0;i<11;i++)
    {
        for(int j=0;j<10;j++)
        {
            for(int k=0;k<640;k++)
            {
                if(!diePhysicalMap[i][j][k])
                {
                    diePhysicalMapCount[j][k]++;
                }
            }
        }
    }
    int clusterSize=0;
    int latestMax=0;
    /* Testing Files */
//    QFile testCount;
//    testCount.setFileName("testCount.csv");
//    if (!testCount.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        qDebug() << "Can not create testCount Responce Document";
//        return QString("");
//    }
//    QTextStream textOut;
//    textOut.setDevice(&testCount);
//    textOut<<"Row, Nozzle, Count\n";
//    for(int i=0;i<10;i++)
//    {
//        for(int j=0;j<640;j++)
//        {
//            textOut<<i<<","<<j<<","<<diePhysicalMapCount[i][j]<<"\n";
//        }
//    }
//    textOut.flush();
//    testCount.close();
//
//    testCount.setFileName("testDieCount.csv");
//    if (!testCount.open(QIODevice::WriteOnly | QIODevice::Text))
//    {
//        qDebug() << "Can not create testCount Responce Document";
//        return QString("");
//    }
//    textOut.setDevice(&testCount);
//    textOut<<"Chip, Row, Nozzle, Count\n";
//    for(int i=0;i<11;i++)
//    {
//        for(int j=0;j<10;j++)
//        {
//            for(int k=0;k<640;k++)
//            {
//                textOut<<i<<","<<j<<","<<k<<","<<diePhysicalMap[i][j][k]<<"\n";
//            }
//        }
//    }
//    textOut.flush();
//    testCount.close();

    /* End Testing Files */

    for(int i=0;i<10;i++)
    {
        for(int j=0;j<640;j++)
        {
            if(diePhysicalMapCount[i][j]>=maxAcceptedRepition)
            {
                clusterSize++;
                latestMax = diePhysicalMapCount[i][j];
            }
            else
            {
                if(clusterSize>=clusterSize2Fail)
                {
                    testFailed = true;
                    return QString(" A repetitive physical blockage was found and it repeats in %1 dies").arg(latestMax);
                }
                else
                    clusterSize =0;
            }
        }
    }
    return QString("No Repetitive physical blockage was found");
}

bool ImageAnalysis::detectStartEdge(IplImage *_frameImage)
{
    IplImage *tempFrameTest = cvCloneImage(_frameImage);
    IplImage *detectionImage;
    if(tempFrameTest->nChannels==3)
    {
        detectionImage = cvCreateImage( cvGetSize(tempFrameTest), 8, 1 );
        assert(detectionImage->imageData);
        cvCvtColor( tempFrameTest, detectionImage, CV_BGR2GRAY );
    }
    else
        detectionImage = tempFrameTest;
    this->detectEdges(detectionImage);

    if(tempFrameTest)
        cvReleaseImage(&tempFrameTest);
    if(detectionImage)
        cvReleaseImage(&detectionImage);
    return this->foundStartEdge;
}

double ImageAnalysis::distance2Row(QPointF p,QVector<MatchingNozzle> row)
{
    LineFit lineFit;
    Line linearRegression = lineFit.fitPoints(row);
    return linearRegression.distance2Point(p);
}

void ImageAnalysis::drawRowLines(QVector <QVector<MatchingNozzle> > mappedNozzels)
{
    QVector <QPointF> points;
    for(int row=0;row<mappedNozzels.size();row++)
    {
        points.clear();
        for(int i=0; i<mappedNozzels[row].size();i++)
        {
            QPointF p;
            p.setX(mappedNozzels[row][i].location.x());
            p.setY(mappedNozzels[row][i].location.y());
            points.push_back(p);
        }
        LineFit lineFit;
        Line linearRegression = lineFit.fitPoints(points);
        QPointF p1,p2;
        p1.setX(10);                        p1.setY(lineFit.getY(10));
        p2.setX(displayFrame->width -10);   p2.setY(lineFit.getY(displayFrame->width -10));
        CvPoint a,b; a.x = (int)p1.x(); a.y = (int)p1.y();
                     b.x = (int)p2.x(); b.y = (int)p2.y();
        cvLine( displayFrame, a , b , CV_RGB(255,255,0), 2, CV_AA, 0 );
    }
}

void ImageAnalysis::drawNozzels(QVector <MatchingNozzle> nozzelSet,bool numberThem)
{
    CvScalar color2 = CV_RGB( rand()&255, rand()&255, rand()&255 );
    for(int i=0;i<nozzelSet.size();i++)
    {
        if(nozzelSet[i].getStatus() == MatchingNozzle::Blocked && !showDeadNozzels)
            continue;
        if(nozzelSet[i].getStatus() == MatchingNozzle::Clear   && !showClearNozzels)
            continue;
        if(nozzelSet[i].getStatus() == MatchingNozzle::Dummy)
            continue;
        if(nozzelSet[i].getStatus() == MatchingNozzle::Blocked)
            color = CV_RGB(255,0,0);
//        else
//            color = color2;
        else
            color = CV_RGB(0,255,0);
        if(numberThem)
            drawNozzel(nozzelSet[i],color,nozzelSet[i].getNozzleNum());
        else
            drawNozzel(nozzelSet[i],color);
    }
}

void ImageAnalysis::drawNozzels(QVector <QVector<MatchingNozzle> > mappedNozzels)
{
    qDebug() << "Drawing Nozzels";
    for(int row=0;row<mappedNozzels.size();row++)
    {
        drawNozzels(mappedNozzels[row],true);
    }
    qDebug() << "Drawing Done";
}

void ImageAnalysis::drawNozzel(MatchingNozzle singleNozzel,CvScalar color)
{
    cvRectangle( displayFrame,cvPoint( qRound(singleNozzel.location.x()), qRound(singleNozzel.location.y()) ),cvPoint(qRound(singleNozzel.location.x() + singleNozzel.getTempW()), qRound(singleNozzel.location.y() + singleNozzel.getTempH())),color, 2, 0, 0 );
}

void ImageAnalysis::drawNozzel(MatchingNozzle singleNozzel,CvScalar color,int i)
{
    drawNozzel(singleNozzel,color);
    if(showNumbers)
    {
        if((i%5)!=0)
            return;
        cvInitFont(&font,CV_FONT_VECTOR0,0.5,0.5,0,2);
        sprintf(str,"%d_c%d_r%d",i,singleNozzel.getChip(),singleNozzel.getRow());
//        sprintf(str,"%d",i);
//        sprintf(str,"%d",(int)(singleNozzel.matchValue*100));
        cvPutText(displayFrame,str,cvPoint( qRound(singleNozzel.location.x()+singleNozzel.getTempW()/3.0), qRound(singleNozzel.location.y()) ),&font,CV_RGB(255,255,255));
    }
}

void ImageAnalysis::drawNozzels(QVector <MatchingNozzle> nozzelSet,CvScalar color,bool numberThem)
{
    for(int i=0;i<nozzelSet.size();i++)
    {
        if(numberThem)
            drawNozzel(nozzelSet[i],color,i);
        else
            drawNozzel(nozzelSet[i],color);
    }
}

void ImageAnalysis::drawDeadNozzels(QVector <MatchingNozzle> nozzelSet)
{
    drawNozzels(nozzelSet,CV_RGB(255,0,0),true);
}

void ImageAnalysis::drawMatchedNozzels(QVector <MatchingNozzle> nozzelSet)
{
    drawNozzels(nozzelSet,CV_RGB(0,255,0),false);
}

bool ImageAnalysis::dropTriangleLocalized()
{
//    for(int i=0;i<10;i++)
//    {
//        if(!dummyGapFilled[i])
//            return false;
//    }
    return true;
}
void ImageAnalysis::filterDeadNozzleNeighbours()
{
    int leftNeighbour,rightNeighbour;
    QMap<int,int> nozzles2Update;
    for(int i=0;i<nozzelMap.size();i++)
    {
        for(int j=0;j<nozzelMap[i].size();j++)
        {
            if(nozzelMap[i][j].getStatus()==MatchingNozzle::Blocked)
            {
                leftNeighbour = j-1;
                rightNeighbour = j+1;
                if(leftNeighbour>0)
                {
                    if(nozzelMap[i][leftNeighbour].getStatus()==MatchingNozzle::Clear && nozzelMap[i][leftNeighbour].getMatchingValue()<secondMatchingAccuracy && nozzelMap[i][leftNeighbour].getChip() == nozzelMap[i][j].getChip())
                    {
                        nozzles2Update.insert(i,leftNeighbour);
                    }
                }
                if(rightNeighbour<nozzelMap[i].size())
                {
                    if(nozzelMap[i][rightNeighbour].getStatus()==MatchingNozzle::Clear && nozzelMap[i][rightNeighbour].getMatchingValue()<secondMatchingAccuracy && nozzelMap[i][leftNeighbour].getChip() == nozzelMap[i][j].getChip() && nozzelMap[i][leftNeighbour].getNozzleNum()!=0)
                    {
                        nozzles2Update.insert(i,rightNeighbour);
                    }
                }
            }
        }
    }
    QMapIterator<int, int> i(nozzles2Update);
    while (i.hasNext())
    {
        i.next();
        nozzelMap[i.key()][i.value()].setStatus(MatchingNozzle::Blocked);
    }
}

void ImageAnalysis::filterNozzlesAroundEdges()
{
    QPointF startEdgeReference,endEdgeReference;
    int tempW, tempH,pixBuffer=10;
    for(int i=0;i<nozzelMap.size();i++)
    {
        for(int j=0;j<nozzelMap[i].size();j++)
        {
            tempW = nozzelMap[i][j].getTempW();
            tempH = nozzelMap[i][j].getTempH();
            switch(i)
            {
                case 0:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() + tempW/2);  startEdgeReference.setY(nozzelMap[i][j].location.y() + tempH/2);
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0);    endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
                case 1:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() - 2*tempW + pixBuffer);  startEdgeReference.setY(nozzelMap[i][j].location.y());
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0 - pixBuffer);    endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
                case 2:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() + pixBuffer);  startEdgeReference.setY(nozzelMap[i][j].location.y());
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0);      endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
                case 3:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() - pixBuffer);  startEdgeReference.setY(nozzelMap[i][j].location.y());
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0 - pixBuffer);      endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
                case 4:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() + pixBuffer);  startEdgeReference.setY(nozzelMap[i][j].location.y());
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0);      endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
                case 5:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() - tempW + pixBuffer);  startEdgeReference.setY(nozzelMap[i][j].location.y());
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0 - pixBuffer);      endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
                case 6:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() + pixBuffer);  startEdgeReference.setY(nozzelMap[i][j].location.y());
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0 );      endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
                case 7:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() - tempW + pixBuffer);  startEdgeReference.setY(nozzelMap[i][j].location.y());
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0 - pixBuffer);      endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
                case 8:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() - tempW/2.0 + pixBuffer );  startEdgeReference.setY(nozzelMap[i][j].location.y());
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0);      endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
                case 9:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() - tempW + pixBuffer);  startEdgeReference.setY(nozzelMap[i][j].location.y());
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0 - pixBuffer);      endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
                case 10:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() - pixBuffer);  startEdgeReference.setY(nozzelMap[i][j].location.y());
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0);      endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
                case 11:
                    startEdgeReference.setX(nozzelMap[i][j].location.x() - tempW + pixBuffer );  startEdgeReference.setY(nozzelMap[i][j].location.y());
                    endEdgeReference.setX(nozzelMap[i][j].location.x() + 3*tempW + tempW/2.0 - pixBuffer);      endEdgeReference.setY(nozzelMap[i][j].location.y());
                    break;
            }
            if(foundStartEdge && foundEndEdge)
            {
                if(startEdge.isPointLeftofLine(startEdgeReference) && endEdge.isPointRightofLine(endEdgeReference))
                {
                    nozzelMap[i].remove(j);
                    j--;
                    continue;
                }
            }
            else if(foundStartEdge)
            {
                if(startEdge.isPointLeftofLine(startEdgeReference))
                {
                    nozzelMap[i].remove(j);
                    j--;
                    continue;
                }
            }
            else if(foundEndEdge)
            {
                if(endEdge.isPointRightofLine(endEdgeReference))
                {
                    nozzelMap[i].remove(j);
                    j--;
                    continue;
                }
            }
        }
    }
}

bool ImageAnalysis::endEdgeFound()
{
    return foundEndEdge;
}

bool ImageAnalysis::edgesFound()
{
    return (foundStartEdge || foundEndEdge);
}

bool ImageAnalysis::startEdgeFound()
{
    return foundStartEdge;
}

IplImage * ImageAnalysis::getDisplayFrame()
{
    return cvCloneImage(displayFrame);
}

//! This gets the distance between nozzels independent of
// the template size
int ImageAnalysis::getDistanceBetweenNozzels()
{
    // TODO: this distance should be averaged over the whole set
    // later on
    float distance=0,diff;
    int counter=0,retDist=0;
    int x,nextX,tempW=0;
    for(int i=0;i<nozzelMap.size();i++)
    {
        for(int j=0;j<nozzelMap[i].size()-1;j++)
        {
            x     = qRound(nozzelMap[i][j].location.x());
            nextX = qRound(nozzelMap[i][j+1].location.x());
            tempW = nozzelMap[i][j+1].getTempW();
            diff = nextX-x;
            assert(diff>=0);
            if(diff<1.2*tempW)
            {
                distance+=diff;
                counter++;
            }
        }
    }
    retDist = int(distance/float(counter));
    if(counter!=0)
        return retDist;
    else
        qDebug()<< "Not enough close-distanced nozzles found to determine distance, dist:"<<retDist;
    return tempW;
}

QPoint ImageAnalysis::getEdgesTemplateSize()
{
    return this->edgesTempSize;
}

void ImageAnalysis::getFileList()
{
     QStringList filters;
     QDir dir(imagesPath);
     dir.setFilter(QDir::Files | QDir::NoSymLinks);
     if(getImagesSource()==ACD || getImagesSource()==THUNDERBIRD)
         dir.setSorting(QDir::Name);
     else
         dir.setSorting(QDir::Name);
     filters << "*.bmp" << "*.jpg" << "*.png";
     listOfImages.clear();
     dir.setNameFilters(filters);
     listOfImages = dir.entryInfoList();
     if(listOfImages.size()>0 && analysisMode==OFFLINE)
     {
         QString name = listOfImages[0].fileName();
         printHeadCode = name.section('_',4,4);
     }
     for(int i=0;i<listOfImages.size();i++)
     {
         QString name = listOfImages[i].fileName();
         if(name.contains("_analysed") || name.contains(".db") || name.contains("Thumbs.db"))
         {
             listOfImages.removeAt(i);
             if(i>0)
                i--;
         }
         qDebug()<<"Image File Added Found:"<<qPrintable(listOfImages[i].fileName());
     }
 }

int ImageAnalysis::getImagesSource()
{
    return this->imageSource;
}

void ImageAnalysis::getLineEdges(Line edgeLine, int imageH,QPointF &p1, QPointF &p2)
{
//    qDebug() <<"ImageAnalysisAlgorithms: P1 x:%d y:%d P2 x:%d y:%d",p1.x,p1.y,p2.x,p2.y);
//    qDebug() <<"ImageAnalysisAlgorithms: Slope:=%f constant:=%f",edgeLine.a,edgeLine.b);
    float x0 = -edgeLine.b/edgeLine.a; // y = ax + b; y=0 => ax=-b; => x = -b/a;
    // upper point
    p1.setX(int(x0)); p1.setY(0);
    // when y = max = height
    float x1 = (imageH - edgeLine.b)/edgeLine.a;
    p2.setX(int(x1)); p2.setY(imageH);
//    qDebug() <<"ImageAnalysisAlgorithms: P1 x:%d y:%d P2 x:%d y:%d",p1.x,p1.y,p2.x,p2.y);
}

void ImageAnalysis::getLineEdges(QPointF &p1, QPointF &p2, int imageH)
{
    Line edgeLine(p1,p2);
//    qDebug() <<"ImageAnalysisAlgorithms: P1 x:%d y:%d P2 x:%d y:%d",p1.x,p1.y,p2.x,p2.y);
//    qDebug() <<"ImageAnalysisAlgorithms: Slope:=%f constant:=%f",edgeLine.a,edgeLine.b);
    float x0 = -edgeLine.b/edgeLine.a; // y = ax + b; y=0 => ax=-b; => x = -b/a;
    // upper point
    p1.setX(int(x0)); p1.setY(0);
    // when y = max = height
    float x1 = (imageH - edgeLine.b)/edgeLine.a;
    p2.setX(int(x1)); p2.setY(imageH);
//    qDebug() <<"ImageAnalysisAlgorithms: P1 x:%d y:%d P2 x:%d y:%d",p1.x,p1.y,p2.x,p2.y);
}

QPoint ImageAnalysis::getNozzelTemplateSize()
{
    return this->nozzelTempSize;
}

int ImageAnalysis::getNumDeadNozzles()
{
    qDebug()<<"The total num of blocked nozzles is:"<<totalDeadNozzleNum;
    return totalDeadNozzleNum;
}

bool matchPoseCompareX(const MatchingNozzle &s1, const MatchingNozzle &s2)
{
    return s1.location.x() < s2.location.x();
}

bool matchPoseCompareY(const QVector<MatchingNozzle> &s1, const QVector<MatchingNozzle> &s2)
{
    return s1[0].location.y() < s2[0].location.y();
}

bool matchPoseCompareMatchValue(const QVector<MatchingNozzle> &s1, const QVector<MatchingNozzle> &s2)
{
    return s1[0].matchValue < s2[0].matchValue;
}

void ImageAnalysis::insertIntoSortedMatches(MatchingNozzle matchL)
{
    for(int i=0;i<nozzelMap.size();i++)
    {
        // find which row it belongs to
//        if(fabs(rowAverageHeightPose(nozzelMap[i]) - matchL.location.y)<=(matchL.tempH/2.0))
        QPointF p; p.setX(matchL.location.x()); p.setY(matchL.location.y());
        if(distance2Row(p,nozzelMap[i])<=(matchL.getTempH()/2.0))
        {
            // insert it into the row
            matchL.setRow(i);
            nozzelMap[i].push_back(matchL);
            // resort the row
            qStableSort(nozzelMap[i].begin(),nozzelMap[i].end(),matchPoseCompareX);
            // localize the recently added Nozzle
            for(int j=0;j<(nozzelMap[i].size()-1);j++)
            {
                if(!nozzelMap[i][j].isLocalized())
                {
                    nozzelMap[i][j].setNozzleNum( nozzelMap[i][j+1].getNozzleNum() -1);
                    nozzelMap[i][j].setRow(i);
                    nozzelMap[i][j].setLocalized(true);
                    break;
                }
            }
            // if the added nozzel is at the end
            if(!nozzelMap[i][nozzelMap[i].size()-1].isLocalized() && nozzelMap[i].size()>1)
            {
                nozzelMap[i][nozzelMap[i].size()-1].setNozzleNum(nozzelMap[i][nozzelMap[i].size()-2].getNozzleNum() + 1);
                nozzelMap[i][nozzelMap[i].size()-1].setRow(i);
                nozzelMap[i][nozzelMap[i].size()-1].setLocalized(true);
            }
            return;
        }
    }
}

bool ImageAnalysis::isNozzelOnEdge(MatchingNozzle &nozzel)
{
    QPointF endEdgeReference,startEdgeReference;
    endEdgeReference.setX(nozzel.location.x() + nozzel.getTempW());            endEdgeReference.setY(nozzel.location.y() + nozzel.getTempH());
    startEdgeReference.setX(nozzel.location.x() + int (nozzel.getTempW()));    startEdgeReference.setY(nozzel.location.y() + int (nozzel.getTempH()/2.0));
//    startEdgeReference.setX(nozzel.location.x);  startEdgeReference.setY(nozzel.location.y);
    if(foundEndEdge && foundStartEdge)
    {
        if(startEdge.isPointRightofLine(startEdgeReference))
        {
            nozzel.setChip(chipIndex);
            return false;
        }
        else if(endEdge.isPointLeftofLine(endEdgeReference))
        {
            nozzel.setChip(chipIndex-1);
            return false;
        }
        else
        {
            nozzel.setStatus(MatchingNozzle::Dummy);
            return true;
        }
    }
    else if(foundEndEdge)
    {
        if(endEdge.isPointLeftofLine(endEdgeReference))
        {
            nozzel.setChip(chipIndex);
            return false;
        }
        else
            return true;
    }
    else if(foundStartEdge)
    {
        if(startEdge.isPointRightofLine(startEdgeReference))
        {
            nozzel.setChip(chipIndex);
            return false;
        }
        else
            return true;
    }
    else if(!foundStartEdge && !foundEndEdge)
    {
        nozzel.setChip(chipIndex);
        return false;
    }
    return true;
}

void ImageAnalysis::keepRelevantChannelRows()
{
    qDebug() <<"ImageAnalysisAlgorithms: allNozzel Array size was:" <<allNozzels.size();
    allNozzels.clear();
    for(int i=0;i<nozzelMap.size();i++)
    {
        for(int j=0;j<nozzelMap[i].size();j++)
        {
            allNozzels.push_back(nozzelMap[i][j]);
        }
    }
    qDebug() <<"ImageAnalysisAlgorithms: allNozzel Array size is now:"<< allNozzels.size();
}

IplImage * ImageAnalysis::loadImage(QString imageFileName)
{
    #ifdef _TTY_WIN_
        imageFileName.replace(QString("/"), QString("\\"));
    #endif
    IplImage * tempImage=0;
    qDebug() << "Loading offline Image:"<<qPrintable(imageFileName);
    tempImage = cvLoadImage(qPrintable(imageFileName),-1);
    if(tempImage==NULL)
        qDebug()<<"Error Loading the Image";
    return tempImage;
}

/*! This is the initia  */
void ImageAnalysis::localizeChipStart()
{
    qDebug() <<"ImageAnalysisAlgorithms: Localizing Nozzels";
    if(nozzelMap.size()<=10)
    {
        qDebug() <<"Row Count is less than 10, rows="<<nozzelMap.size()<<"Something wrong went in localizing the chip start: not enough rows were found!!!";
    }
    int rowReferences[10]={4,6,0,0,0,0,0,0,0,0};
    int rightSideIndexing;
    int numMiddleDummyNozzels = 3;
//    int pixBuffer = 10;
    filterNozzlesAroundEdges();
    for(int i=0;i<nozzelMap.size();i++)
    {
        rightSideIndexing = 0;
        for(int j=0;j<nozzelMap[i].size();j++)
        {
            // force those nozzels left of end edge to be correlated with previous location
            if((nozzelMap[i][j].getChip()==(chipIndex-1) || prevImageContainsStartingEdge ) &&!mouseClicked)
            {
                if(i>9)
                {
                    // The nozzle is in an irrelavent Row;
                    nozzelMap[i][j].setStatus(MatchingNozzle::Dummy);
                }
                else
                {
                    nozzelMap[i][j].setRow(i);
                }
            }
            // new chip so restart localization
            else if(nozzelMap[i][j].getChip()==chipIndex)
            {
                int row=0,column=0;
                QPointF startEdgeReference;
//                int tempW = nozzelMap[i][j].getTempW();
//                int tempH = nozzelMap[i][j].getTempH();
                if (i==0)
                {
                    row = 0;
                    column = rowReferences[row]++;
                }
                else if(i==1)
                {
                    row = 1;
                    column = rowReferences[row]++;
                }
                else
                {
                    int chipRow = i-2;
                    if(rightSideIndexing<=crossRowsReferences[chipRow])
                    {
                        row = chipRow;
                        column = rightSideIndexing;
                        if(nozzelMap[i][j].getStatus() == MatchingNozzle::Dummy)
                            nozzelMap[i][j].setStatus(MatchingNozzle::Clear);
                    }
                    else if(rightSideIndexing>crossRowsReferences[chipRow] && rightSideIndexing<=(crossRowsReferences[chipRow]+numMiddleDummyNozzels))
                    {
                        // the middle useless section
                        nozzelMap[i][j].setStatus(MatchingNozzle::Dummy);
                        column = rightSideIndexing -crossRowsReferences[chipRow];
                    }
                    else if (rightSideIndexing>(crossRowsReferences[chipRow]+numMiddleDummyNozzels))
                    {
                        // irrelevant dummy nozzels
                        if(i==10 || i==11)
                        {
                            nozzelMap[i][j].setStatus(MatchingNozzle::Dummy);
                            column = rightSideIndexing - crossRowsReferences[chipRow];
                        }
                        else
                        {
                            row = i;
                            column = rightSideIndexing - (crossRowsReferences[chipRow]+numMiddleDummyNozzels) + crossRowsReferences[row];
                            if(nozzelMap[i][j].getStatus() == MatchingNozzle::Dummy)
                                nozzelMap[i][j].setStatus(MatchingNozzle::Clear);
                        }
                    }
                }
                nozzelMap[i][j].setLocalized(true);
                nozzelMap[i][j].setNozzleNum(column);
                nozzelMap[i][j].setRow(row);
                rightSideIndexing ++;
            }
        }
    }
    qDebug() <<"ImageAnalysisAlgorithms: Nozzels Localized";
}

void ImageAnalysis::locateDeadNozzels()
{
    qDebug() <<"ImageAnalysisAlgorithms: Locating dead Nozzles ";
    if(nozzelTemplates.size()==0)
    {
        return;
    }
    deadNozzels.clear();
    numDeadNozzels=0;
    int deadNozzleMatchingValue=1,numDeadNozzleAdded=0;
    for(int i=0;i<nozzelMap.size();i++)
    {
        emit stateProgress("Locating Dead Nozzels",int(((i+1)*100)/nozzelMap.size()));
        for(int j=0;j<nozzelMap[i].size();j++)
        {
            //TODO: double check if this case (false positive happens very often)
            if (nozzelMap[i].size() ==1)
                continue;
            int x = qRound(nozzelMap[i][j].location.x());
//            int y = nozzelMap[i][j].location.y();
            int tempW = nozzelMap[i][j].getTempW();
            int tempH = nozzelMap[i][j].getTempH();
//            assert(tempH>=nozzelTempSize.y());
//            assert(tempW>=nozzelTempSize.x());
            // first good nozzel in a row, check for a possibility of blocked nozzels at the begining
            int edgeBuffer = 10;
            if(j==0 && x> (distanceBetweenNozzels+edgeBuffer))
            {
                numDeadNozzels = (int)floor(x/(float)distanceBetweenNozzels);
//                assert(numDeadNozzels>=1);
                for(int d =1;d<=numDeadNozzels;d++)
                {
                    int deadNozzelX = x - d*distanceBetweenNozzels;
                    LineFit lineFit;
                    lineFit.fitPoints(nozzelMap[i]);
                    int deadNozzelY = (int)lineFit.getY(deadNozzelX);
                    MatchingNozzle nozzelNew = MatchingNozzle(QPoint(deadNozzelX,deadNozzelY),tempW,tempH,MatchingNozzle::Blocked,deadNozzleMatchingValue);
                    if(!isNozzelOnEdge(nozzelNew))
                    {
                        allNozzels.push_back(nozzelNew);
                        numDeadNozzleAdded++;
                    }
                }
            }
            // Last good nozzel in a row, check for a possibility of blocked nozzels at the end
            if( j==(nozzelMap[i].size()-1) && (displayFrame->width - (x + tempW))>(distanceBetweenNozzels+edgeBuffer))
            {
                numDeadNozzels = (int)floor((displayFrame->width - (x + tempW))/(float)distanceBetweenNozzels);
                assert(numDeadNozzels>=1);
                for(int d =1;d<=numDeadNozzels;d++)
                {
                    int deadNozzelX = x + d*distanceBetweenNozzels;
//                    int deadNozzelY = y;
                    LineFit lineFit;
                    lineFit.fitPoints(nozzelMap[i]);
                    int deadNozzelY = (int)lineFit.getY(deadNozzelX);
                    MatchingNozzle nozzelNew = MatchingNozzle(QPoint(deadNozzelX,deadNozzelY),tempW,tempH,MatchingNozzle::Blocked,deadNozzleMatchingValue);
                    if(!isNozzelOnEdge(nozzelNew))
                    {
                        allNozzels.push_back(nozzelNew);
                        numDeadNozzleAdded++;
                    }
                }
            }
            // Look for an in-between dead nozzel.
            if((j+1)<nozzelMap[i].size())
            {
                int xNext = qRound(nozzelMap[i][j+1].location.x());
                assert(xNext>=x);
                int errorFlex=0;
                int gap = (xNext-(x+tempW)+errorFlex);
                if(gap>=tempW)
                {
                    numDeadNozzels = (int)floor(gap/(float)distanceBetweenNozzels);
                    // a bit smaller than a temp width
                    if(numDeadNozzels==0)
                        numDeadNozzels =1;
//                    qDebug()<<"Distance between Nozzles:"<< distanceBetweenNozzels <<" Gap is:"<<gap <<"Num dead nozzles:"<<numDeadNozzels;
                    assert(numDeadNozzels>=1);
                    for(int d =1;d<=numDeadNozzels;d++)
                    {
                        int deadNozzelX = x + d*distanceBetweenNozzels;
//                        int deadNozzelY = y;
                        LineFit lineFit;
                        lineFit.fitPoints(nozzelMap[i]);
                        int deadNozzelY = (int)lineFit.getY(deadNozzelX);
                        // Check if the nozzel is not at the edge !!!
                        MatchingNozzle nozzelNew = MatchingNozzle(QPoint(deadNozzelX,deadNozzelY),tempW,tempH,MatchingNozzle::Blocked,deadNozzleMatchingValue);
                        if(!isNozzelOnEdge(nozzelNew))
                        {
                            allNozzels.push_back(nozzelNew);
                            numDeadNozzleAdded++;
                        }
                    }
                }
            }
        }
    }
    qDebug() <<"ImageAnalysisAlgorithms: ["<<numDeadNozzleAdded<<"] Dead Nozzles Located";
}

void ImageAnalysis::nozzelSelected(const QPoint &locationPressed, int button)
{
    this->locationPressed = locationPressed;
    this->buttonPressed = button;
    nozzelPressed = true;
}

void ImageAnalysis::mouseClickedOnNozzle(const QPoint &loc, int button)
{
    qDebug() <<"ImageAnalysisAlgorithms: MatchingNozzle Selected";
    manualInputClicks ++;
    if(displayFrame)
    {
        int translatedX = loc.x();
        int translatedY = loc.y();
        if(translatedX<=displayFrame->width &&translatedY<=displayFrame->height && translatedX>=0 && translatedY>=0)
        {
            triggerNozzel(QPoint(translatedX,translatedY),button);
        }
    }
}
void ImageAnalysis::prepareForThreadRestart()
{
    // for reinitializing the displayimage once we restart the thread
    displayContainerInitialized = false;
    stopThread = false;
    paused = false;
    nozzelPressed = false;
    repeatAnalysis= false;
    firstTime = true;
    newAnalysis = true;
    analysisMode = IDLE;
}

void ImageAnalysis::pushData2MesInbox()
{
    qDebug()<<"Pushing production Responce file into Inbox";
    createProductionResponceDoc();
    bool productionResponceCopied = QFile::copy(productionResponceDocFileName,QString("%1/%2").arg(ASG_PIT::settings().getInboxLocation()).arg(QFileInfo(productionResponceDocFileName).fileName()));
    bool dNMapFileCopied = QFile::copy(dnMapFile.fileName(),QString("%1/%2").arg(absolutePath).arg(QFileInfo(dnMapFile.fileName()).fileName()));
    bool imageAnalysisFileCopied = QFile::copy(analysisFile.fileName(),QString("%1/%2").arg(absolutePath).arg(QFileInfo(analysisFile.fileName()).fileName()));
    if(dNMapFileCopied && productionResponceCopied && imageAnalysisFileCopied)
    {
        qDebug()<<"Files successfully copied";
        emit warnUser(QString("File:%1 successfully copied to %2 and file %3 copied to %4 and %5 to %6").arg(dnMapFile.fileName()).arg(QFileInfo(dnMapFile.fileName()).fileName()).arg(productionResponceDocFileName).arg(QString("%1/%2").arg(ASG_PIT::settings().getInboxLocation()).arg(QFileInfo(productionResponceDocFileName).fileName())).arg(analysisFile.fileName()).arg(QFileInfo(analysisFile.fileName()).fileName()),QMessageBox::Information);
    }
    else
    {
        QString warningMsg;
        warningMsg.append("Could not copy ");
        if(!dNMapFileCopied)
            warningMsg.append(QString(", file:%1 to file:%2").arg(dnMapFile.fileName()).arg(QString("%1/%2").arg(absolutePath).arg(QFileInfo(dnMapFile.fileName()).fileName())));
        if(!productionResponceCopied)
            warningMsg.append(QString(", file:%1 to file:%2").arg(productionResponceDocFileName).arg(QString("%1/%2").arg(ASG_PIT::settings().getInboxLocation()).arg(QFileInfo(productionResponceDocFileName).fileName())));
        if(!imageAnalysisFileCopied)
            warningMsg.append(QString(", file:%1 to file:%2").arg(analysisFile.fileName()).arg(QString("%1/%2").arg(absolutePath).arg(QFileInfo(analysisFile.fileName()).fileName())));
        emit warnUser(warningMsg,QMessageBox::Warning);
        qDebug() << warningMsg;
    }
    qDebug()<<"Production Responce file pushed into Inbox";
}

void ImageAnalysis::redrawNozzels()
{
    //Clearing the display Image for redrawing
    cvCopy(originalDisplay,displayFrame);
    drawNozzels(nozzelMap);
}

void ImageAnalysis::resetAnalysis()
{
    qDebug() << "Analysis Reset";
    allNozzels.clear();
    deadNozzels.clear();
    nozzelMap.clear();
    foundStartEdge  = false;
    foundEndEdge    = false;
    prevImageContainsStartingEdge = false;
    mouseClicked = false;
    analysisFileHeaderPrinted = false;
    firstTime = true;
    chipIndex       = -1;
    clearImageContainers();
    for(int i=0;i<10;i++)
        totalAdjacentNozzelCounts[i] =0;
    totalManualClicks = 0;
    resetDNMap();
}

void ImageAnalysis::resetDNMap()
{
    for(int i=0;i<5;i++)
        for(int j=0;j<NozzleOperations::numOfNozzlesPerColorChannel;j++)
            dnMap[i][j]= true;
    for(int i=0;i<11;i++)
        for(int j=0;j<10;j++)
            for(int k=0;k<640;k++)
                diePhysicalMap[i][j][k] = true;
    resetPhysicalDamageCount();
}

void ImageAnalysis::resetPhysicalDamageCount()
{
    for(int i=0;i<10;i++)
        for(int j=0;j<640;j++)
            diePhysicalMapCount[i][j]=0;
}

double ImageAnalysis::rowAverageHeightPose(QVector<MatchingNozzle> row)
{
    double sum=0;
    for(int i=0; i<row.size();i++)
    {
        // STUPID !!! this mistake frustrated me and I will leave it as a lesson !!!
        // (I didn't accout for the change of reference point while sorting rows==FRUSTRATING)
//        sum+=(row[i].location.y() + row[i].getTempH()/2.0);
        sum+=(row[i].location.y());
    }
    return sum/double(row.size());
}

double ImageAnalysis::rowAverageMatch(QVector<MatchingNozzle> row)
{
    double sum=0, count=0;
    for(int i=0; i<row.size();i++)
    {
        if(row[i].getStatus()==MatchingNozzle::Clear)
        {
            sum+=row[i].getMatchingValue();
            count++;
        }
    }
    if(count==0)
        return 0;
    return sum/count;

}

void ImageAnalysis::run()
{
    QString dirPath;
    requestConfirmation = false;
    char fileName[600];
    stopThread = false;
    newAnalysis = true;
    int imageCounter=0;
    cvSetErrMode(CV_ErrModeParent);
    resetAnalysis();
    while(1)
    {
        // Just in case it changes
        generateDNMap = ASG_PIT::settings().getGenerateDNMap();
        if(analysisMode==IDLE)
        {
            usleep(100000);
            continue;
        }
        if(nozzelPressed)
        {
            nozzelPressed = false;
            mouseClickedOnNozzle(locationPressed,buttonPressed);
            emit displayImageThenRelease(cvCloneImage(disImage));
        }
        if(repeatAnalysis && !firstTime && !paused)
        {
            if(disImage)
                cvReleaseImage(&disImage);
            emit processingImageAnalysis();
            if(analysisMode==OFFLINE)
            {
                disImage = analyseImage(image,repeatAnalysis);
            }
            else
            {
                disImage = analyseImage(onlineImage,repeatAnalysis);
            }
            if(analysisType==SEMI_AUTO)
            {
                if(getNumDeadNozzles()!=0 || edgesFound())
                {
                    requestConfirmation = true;
                    qDebug()<<"Changing Analysis Type to Manual";
                }
                else
                {
                    qDebug()<<"Changing Analysis Type to AUTO";
                    if(!newAnalysis && !(nozzelTemplates.size()==0))
                        requestConfirmation = false;
                }
            }
            if(!disImage)
            {
                emit warnUser("Something went wrong and the Analysis returned an empty image, inform developers",QMessageBox::Warning);
                continue;
            }
            emit displayImageThenRelease(cvCloneImage(disImage));
            emit finishedProcessingImageAnalysis();
            paused = true;
            continue;
        }
        // only wait for un-pausing key "SpaceBar" if
        // we are analysing Manually
        if((analysisType == MANUAL && paused) || (analysisType == SEMI_AUTO && requestConfirmation))
        {
            if(stopThread)
            {
                qDebug()<<"Thread Stopped";
                break;
            }
            usleep(20000);
            continue;
        }
        if(!firstTime)
        {
            if(analysisMode==OFFLINE)
            {
                QFileInfo fileInfo = listOfImages.at(imageCounter);
                sprintf(fileName,"%s",qPrintable(fileInfo.fileName()));
                // Save the statistics of the analyis
                saveResult2File(fileName,analysisOutStream);
                saveResult2DNMap();
                // Save the previously analysed and manually corrected Image
                sprintf(fileName,"%s/%s_analysed.jpg",qPrintable(fileInfo.absolutePath()),qPrintable(fileInfo.baseName()));
                cvSaveImage(fileName,disImage);
                // Release the Image only when it's Saved and we moved to the next Image
                if(disImage)
                    cvReleaseImage(&disImage);
                imageCounter++;
                if(imageCounter > (listOfImages.size()-1))
                {
                    stopThread=true;
                }
            }
            else
            {
                qDebug()<<"Saving the online Image";
                if(onlineImage)
                {
                    sprintf(fileName,"%s/%03d_%d_%d_original.bmp",qPrintable(onlineAnalysisPath),imageCounter,linearMotorPose,cameraMotorPose);
                    if(oldOnlineImage)
                        cvSaveImage(fileName,oldOnlineImage);
                    QFileInfo fileInfo; fileInfo.setFile(fileName);
                    // Save the statistics of the analyis
                    saveResult2File(qPrintable(fileInfo.fileName()),analysisOutStream);
                    saveResult2DNMap();
                    // Save the previously analysed and manually corrected Image
                    sprintf(fileName,"%s/%03d_%d_%d_analysed.bmp",qPrintable(onlineAnalysisPath),imageCounter,linearMotorPose,cameraMotorPose);
                    imageCounter++;
                    if(disImage)
                        cvSaveImage(fileName,disImage);
                    if(disImage)
                        cvReleaseImage(&disImage);
                }
                else
                    emit warnUser("Online Image is Empty",QMessageBox::Warning);
            }
            newAnalysis = false;
            if(stopThread)
            {
                qDebug()<<"Thread Stopped";
                usleep(200000);
                break;
            }
        }
        if(analysisMode==OFFLINE)
        {
            QFileInfo fileInfo = listOfImages.at(imageCounter);
            emit processingImage(QString("Image:%1 [%2/%3]").arg(fileInfo.fileName()).arg(imageCounter).arg(listOfImages.size()));
            if(image)
                cvReleaseImage(&image);
            qDebug()<<"Loading Image:"<<fileInfo.absoluteFilePath();
            image = loadImage(fileInfo.absoluteFilePath());
            if(!image)
            {
                emit warnUser(QString("Could not Load image:%1").arg(fileInfo.absoluteFilePath()),QMessageBox::Warning);
                break;
            }
            if(image->nChannels==1)
            {
                temp = cvCreateImage( cvGetSize(image), 8, 3 );
                cvCvtColor( image, temp, CV_GRAY2BGR );
                cvReleaseImage(&image);
                image = temp;
            }
            emit displayImageThenRelease(cvCloneImage(image));
            emit processingImageAnalysis();
            disImage = analyseImage(image,false);
            emit finishedProcessingImageAnalysis();
            if(analysisType==SEMI_AUTO)
            {
                qDebug()<<"Num Dead Nozzles:"<<getNumDeadNozzles();
                if(getNumDeadNozzles()!=0 || edgesFound())
                {
                    requestConfirmation = true;
                    qDebug()<<"Changing Analysis Type to Manual";
                }
                else
                {
                    qDebug()<<"Changing Analysis Type to AUTO";
                    if(!newAnalysis && !(nozzelTemplates.size()==0))
                        requestConfirmation = false;
                    else
                        requestConfirmation = true;
                }
            }
            if(!disImage)
            {
                emit warnUser("Something went wrong and the Analysis returned an empty image, inform developers",QMessageBox::Warning);
                continue;
            }
        }
        else
        {
            if(onlineImage)
            {
                qDebug()<<" Analysing new online Image";
                emit displayImageThenRelease(cvCloneImage(onlineImage));
                emit processingImageAnalysis();
                disImage = analyseImage(onlineImage,false);
                emit finishedProcessingImageAnalysis();
                if(!disImage)
                {
                    emit warnUser("Something went wrong and the Analysis returned an empty image, inform developers",QMessageBox::Warning);
                    continue;
                }
                if(oldOnlineImage)
                    cvReleaseImage(&oldOnlineImage);
                oldOnlineImage = cvCloneImage(onlineImage);
            }
            else
                emit warnUser("Online Image is Empty",QMessageBox::Warning);
        }
        if(newAnalysis)
        {
            if(!startEdgeFound())
            {
                analysisType = MANUAL;
                emit finishedProcessingImageAnalysis();
                emit warnUser("You need to specify a starting edge first !! Add an appropriate edge start template then press X",QMessageBox::Warning);
                emit displayImageThenRelease(cvCloneImage(disImage));
                paused = true;
                firstTime = true;
                continue;
            }
            else
                firstTime = false;
        }
        emit displayImageThenRelease(cvCloneImage(disImage));
//        nhMap.setDNMap(dnMap);
//        emit viewDNMap(&nhMap);
        firstTime = false;
        paused = true;
        qDebug() <<"Finished with this image";
        if( cvGetErrStatus() < 0 )
            qDebug()<<"Open CV is reporting some erros";
    }
    prepareForThreadRestart();
    if(disImage)
        cvReleaseImage(&disImage);
    if(oldOnlineImage)
        cvReleaseImage(&oldOnlineImage);
    saveTotal2File(analysisOutStream);
    analysisOutStream.flush();
    QString userAltert("");
    if(generateDNMap)
    {
        saveDNMap2File(dnMapOutStream);
        dnMapOutStream.flush();
        dnMapFile.close();
        userAltert.append(detectRepetitiveFailure());
        nhMap.setDNMap(dnMap);
        emit viewDNMap(&nhMap);
    }

    analysisFile.close();
    emit analysisFinished();
    if(imageCounter>=listOfImages.size() && userAltert==QString(""))
        emit warnUser(QString("Finised Analysing all images in the specified folder."),QMessageBox::Information);
    else if(imageCounter>=listOfImages.size() && !stopThread)
        emit warnUser(QString("Finised Analysing all images in the specified folder. %1").arg(userAltert),QMessageBox::Warning);
    else
        emit warnUser(QString("Analysis Stopped. %1").arg(userAltert),QMessageBox::Information);
}

void ImageAnalysis::saveDNMap2File(QTextStream &spaDNMapFileOut)
{
    spaDNMapFileOut<<"Chip,Row,Nozzle,Good,Y offset um, X Offset um,Abs Nozzle,Paper pix,Inkpix,Abs COG Y mm,AbsCOG X mm\n";
    for(int i=0;i<5;i++)
    {
        for(int j=0;j<NozzleOperations::numOfNozzlesPerColorChannel;j++)
        {
            int actualRow=i*2+(((j%2)==0)?0:1);
            int chip = (int)(floor((j+1)/1280.0));
            spaDNMapFileOut<<chip<<","<<actualRow<<",0,"<<dnMap[i][j]<<",0,0,"<<j<<",0,0,0,0\n";
        }
    }
}

void ImageAnalysis::saveResult2DNMap()
{
    qDebug()<<"Saving 2 DNMap";
    int absGlobalPosition,chip,row,column,colorChannel;
    bool good;
    totalDeadNozzleNum=0;
    for(int i=0;i<nozzelMap.size();i++)
    {
        for(int j=0;j<nozzelMap[i].size();j++)
        {
            chip = nozzelMap[i][j].getChip();
            row = nozzelMap[i][j].getRow();
            column = nozzelMap[i][j].getNozzleNum();
            good = (nozzelMap[i][j].getStatus() == MatchingNozzle::Blocked)?false:true;
            if(inRange(chip,0,10) && inRange(row,0,9) && inRange(column,0,639))
            {
                if(!good)
                {
                    //qDebug()<<"Chip:"<<chip<<" Row:"<<row<<" Column:"<<column<<" Status:"<<good;
                    diePhysicalMap[chip][row][column] = false;
                }
                fflush(stdout);
            }
            else
            {
                //qDebug()<<"Outside Expected range >>> Chip:"<<chip<<" row:"<<row<<" column:"<<column<<" status:"<<good;
                continue;
            }
            // an irrelevant nozzel
            if(nozzelMap[i][j].getStatus() == MatchingNozzle::Dummy)
                continue;
            if(!good)
                totalDeadNozzleNum++;
            NozzleOperations nozOp;
            QPoint globalLoc = nozOp.localLocation2Abs(chip,row,column);
            absGlobalPosition = globalLoc.x();
            colorChannel = globalLoc.y();
            // These are the non-ejecting nozzles at the begining and end of the printhead
            if(absGlobalPosition==-1 || colorChannel==-1)
            {
                //qDebug()<<"Non-Firing nozzle, ignoring it in DNMap generation, chip:"<<chip<<" row:"<<row<<" column:"<<column;
                continue;
            }
            if( colorChannel>4 || absGlobalPosition > NozzleOperations::numOfNozzlesPerColorChannel)
            {
                //qDebug()<<" Something went wrong colorChannel is:"<<colorChannel <<"absPosition is:" <<absGlobalPosition;
            }
            else
                dnMap[colorChannel][absGlobalPosition] = good;
        }
    }
}

void ImageAnalysis::saveResult2File(QString imageName,QTextStream &out)
{
    if(!analysisFileHeaderPrinted)
    {
        out<<"File Name, Single Blocked MatchingNozzle, Double, Triple, 4, 5, 6, 7, 8, 9, 10, Manual Input\n";
        analysisFileHeaderPrinted = true;
    }
    int adjacentBlockedNozzlesCounter = 0;;
    for(int i=0;i<10;i++)
        adjacentNozzelCounts[i] =0;
    for(int i=0;i<nozzelMap.size();i++)
        for(int j=0;j<nozzelMap[i].size();j++)
        {
            if(nozzelMap[i][j].getStatus() == MatchingNozzle::Blocked)
                adjacentBlockedNozzlesCounter++;
            else if(adjacentBlockedNozzlesCounter!=0)
            {
                adjacentNozzelCounts[adjacentBlockedNozzlesCounter-1]++;
                adjacentBlockedNozzlesCounter = 0;
            }
        }
    out<<imageName<<",";
    for(int i=0;i<10;i++)
    {
        totalAdjacentNozzelCounts[i]+=adjacentNozzelCounts[i];
        out << adjacentNozzelCounts[i]<<", ";
    }
    totalManualClicks+= manualInputClicks;
    out<<manualInputClicks<< "\n";
    out.flush();
}

void ImageAnalysis::saveTotal2File(QTextStream &out)
{
    out<<"Total,";
    for(int i=0;i<10;i++)
    {
        out << totalAdjacentNozzelCounts[i]<<", ";
    }
    out<<totalManualClicks<< "\n";
    out.flush();
}

QVector <QVector<MatchingNozzle> > ImageAnalysis::selectActiveChannels(QVector <QVector<MatchingNozzle> > sortedNozzelMap)
{
    qDebug() <<"Selecting Active channels ";
    //get the 10 rows with the best matches
    QVector <QVector<MatchingNozzle> > activeChannels;
    QMap<int, int>::const_iterator iter;
    QVector <double> averageMatcheValuePerRow;
    QMap <int,int> rowPairs;
    int pos =0,minSizeOfRow;
    /*
    if(newAnalysis)//applies for only the first Image of a set
        minSizeOfRow = 15;
    else
        minSizeOfRow = 30;
    */
    minSizeOfRow = 5;
    if(foundEndEdge && !foundStartEdge)
    {
        minSizeOfRow = 1;
    }
    // Pair rows to form channels
    for(int i=0;i<sortedNozzelMap.size();i++)
    {
        if(sortedNozzelMap[i].size()<minSizeOfRow)
            continue;
        for(int j=0;j<sortedNozzelMap.size();j++)
        {
            if(sortedNozzelMap[j].size()<minSizeOfRow)
                continue;
            if(i==j)
                continue;
            double diff = fabs(rowAverageHeightPose(sortedNozzelMap[i]) - rowAverageHeightPose(sortedNozzelMap[j]));
            if(diff<=1.8*sortedNozzelMap[i][0].getTempH())
            {
                qDebug()<<"Diff is:"<<diff<<" Height is:"<<sortedNozzelMap[i][0].getTempH();
                if((rowPairs.contains(i) && rowPairs.value(i)==j) || (rowPairs.contains(j) && rowPairs.value(j)==i))
                    continue;
                QPoint p1,p2,start,end;
                p1.setX(qRound(sortedNozzelMap[i][0].location.x())); p1.setY(qRound(sortedNozzelMap[i][0].location.y()));
                p2.setX(qRound(sortedNozzelMap[j][sortedNozzelMap[j].size()-1].location.x())); p2.setY(qRound(sortedNozzelMap[j][sortedNozzelMap[j].size()-1].location.y()));
                int tempH = sortedNozzelMap[i][0].getTempH();
                int tempW = sortedNozzelMap[i][0].getTempW();
                if(j>i)
                {
                    rowPairs.insert(i,j);
                    start = p1;
                    end = p2;
                }
                else
                {
                    start = p2;
                    end = p1;
                    rowPairs.insert(j,i);
                }
                double average = (rowAverageMatch(sortedNozzelMap[i])+rowAverageMatch(sortedNozzelMap[j]))/2.0;
                averageMatcheValuePerRow.push_back(average);
                //                    color = CV_RGB( rand()&255, rand()&255, rand()&255 );
                color = CV_RGB(0,0,255);
                cvRectangle( displayFrame,cvPoint(start.x(),start.y()),cvPoint(end.x()+tempW,end.y()+tempH),color, 2, 0, 0 );
                cvInitFont(&font,CV_FONT_VECTOR0,0.8,0.8,0,2);
                sprintf(str,"pos:%d_av:%f",pos++,average);
                cvPutText(displayFrame,str,cvPoint(start.x()+10,start.y()),&font,CV_RGB(255,0,255));
            }
        }
    }
    // Get only the 5/6 relevant channels - 10/12 rows
    int channelsToKeep;
    if(foundEndEdge || foundStartEdge || !dropTriangleLocalized())
    {
        channelsToKeep = 6;
    }
    else
        channelsToKeep = 5;

    qDebug() << "Getting the["<<channelsToKeep<<"] Channels from the total of:"<<rowPairs.size();
    // Remove the extra channel if needed
    int maxStartIndex=0;
    if(rowPairs.size()>channelsToKeep)
    {
        qDebug()<<"Yes I am filtering the :"<<rowPairs.size()<<" rows";
        double maxSum=0,aSum=0;
        for(int i=0;i<(rowPairs.size()-channelsToKeep+1);i++)
        {
            aSum=0;
            for(int j=i;j<(channelsToKeep+i);j++)
            {
                aSum+= averageMatcheValuePerRow[j];
            }
            if(aSum>maxSum)
            {
                maxSum   = aSum;
                maxStartIndex= i;
            }
        }
    }
    iter = rowPairs.constBegin();
    // The will remain sorted after insertion as they are
    // insterted into the pair sorted
    int index=0;
    while (iter != rowPairs.constEnd())
    {
        if(index>=maxStartIndex && index<(maxStartIndex+channelsToKeep))
        {
            int a= iter.key() ;
            int b= iter.value();
            activeChannels.push_back(sortedNozzelMap[a]);
            activeChannels.push_back(sortedNozzelMap[b]);
        }
        iter++;
        index++;
    }
    if(activeChannels.size() < 2*channelsToKeep)
    {
        emit warnUser(QString("Number of nozzle channels found is: %1 which is less than expected %2. Add more templates and re-analyse image").arg(activeChannels.size()).arg(2*channelsToKeep),QMessageBox::Warning);
        QMutexLocker locker(&mutex);
        repeatAnalysis = true;
    }
    else
    {
        QMutexLocker locker(&mutex);
        repeatAnalysis = false;
    }
    qDebug() <<"Active channels Selected Num:"<<nozzelMap.size();
    return activeChannels;
}

void ImageAnalysis::setDir(QString dir)
{
    this->imagesPath = dir;
}

void ImageAnalysis::setDNMapGeneration(int state)
{
    QMutexLocker locker(&mutex);
    this->generateDNMap = state;
}

void ImageAnalysis::setImagesSource(int _imageSource)
{
    this->imageSource = _imageSource;
    switch(imageSource)
    {
        case ACD:
            qDebug() <<"ImageAnalysisAlgorithms: Source is ACD Large Temps";
//            this->nozzelTempSize.setX(30);
//            this->nozzelTempSize.setY(60);
//            this->edgesTempSize.setX(65);
//            this->edgesTempSize.setY(65);
            this->nozzelTempSize.setX(20);
            this->nozzelTempSize.setY(40);
            this->edgesTempSize.setX(50);
            this->edgesTempSize.setY(50);
            break;
        case PIT_SMALL_TEMPS:
            qDebug() <<"ImageAnalysisAlgorithms: Source is ACD Small Temps";
//            this->nozzelTempSize.setX(30);
//            this->nozzelTempSize.setY(35);
//            this->edgesTempSize.setX(65);
//            this->edgesTempSize.setY(65);
            this->nozzelTempSize.setX(20);
            this->nozzelTempSize.setY(30);
            this->edgesTempSize.setX(50);
            this->edgesTempSize.setY(50);
            break;
        case THUNDERBIRD:
            qDebug() <<"ImageAnalysisAlgorithms: Source is THUNDERBIRD";
            this->nozzelTempSize.setX(26);
            this->nozzelTempSize.setY(50);
            this->edgesTempSize.setX(60);
            this->edgesTempSize.setY(60);
            break;
        default:
            qDebug() <<"ImageAnalysisAlgorithms: Unknown Image Source";
    }
}

void ImageAnalysis::setMode(int anaMode)
{
    QMutexLocker locker(&mutex);
    this->analysisMode = anaMode;
}

void ImageAnalysis::setOnlineImage(IplImage *onlineImg,int linearPose, int cameraPose)
{
    qDebug()<<"Recieving a new Online Image. W:"<<onlineImg->width<<" H:"<<onlineImg->height<<" Channels:"<<onlineImg->nChannels;
    QMutexLocker locker(&mutex);
    IplImage *tempImage = NULL;
    if(onlineImage)
    {
        tempImage = onlineImage;
        onlineImage= cvCloneImage(onlineImg);
        cvReleaseImage(&tempImage);
    }
    this->linearMotorPose = linearPose;
    this->cameraMotorPose = cameraPose;
    this->repeatAnalysis = false;
    qDebug()<<"Online Image recieved successfully";
}

void ImageAnalysis::setPaused(bool state)
{
    QMutexLocker locker(&mutex);
    this->paused = state;
    this->requestConfirmation = false;
}

void ImageAnalysis::setPrintHeadCode(QString printHCode)
{
    QMutexLocker locker(&mutex);
    this->printHeadCode = printHCode;
}

void ImageAnalysis::setRepeatAnalysis(bool repeat)
{
    QMutexLocker locker(&mutex);
    repeatAnalysis = repeat;
    paused = false;
}

void ImageAnalysis::setType(int type)
{
    QMutexLocker locker(&mutex);
    this->analysisType = type;
}

void ImageAnalysis::setupAnalysisFile(QString dirPath)
{
    QDateTime dateTime = dateTime.currentDateTime();
    QString timeCode = dateTime.toString(QString("yyyyMMddThhmms"));
    analysisFile.setFileName(QString("%1/%2_%3_%4.csv").arg(dirPath).arg(printHeadCode).arg(ASG_PIT::settings().getAnalysisFileName()).arg(timeCode));
    if (!analysisFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Can not create Analysis File";
        emit warnUser("Can not create Analysis File, try to create the file Manually, close and program using the file or change the fileName in the options",QMessageBox::Warning);
    }
    analysisOutStream.setDevice(&analysisFile);
}

void ImageAnalysis::setupDNMapFile(QString dirPath)
{
    QDateTime dateTime = dateTime.currentDateTime();
    QString timeCode = dateTime.toString(QString("yyyyMMddThhmms"));
    dnMapFile.setFileName(QString("%1/%2_%3_%4.csv").arg(dirPath).arg(printHeadCode).arg(ASG_PIT::settings().getDNMapFileName()).arg(timeCode));
    if (!dnMapFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to create/open DNMap file";
        emit warnUser("Can not create DNMap File, try to create the file Manually, close and program using the file or change the fileName in the options",QMessageBox::Warning);
    }
    dnMapOutStream.setDevice(&dnMapFile);
}

void ImageAnalysis::startNewAnalysis(int mode)
{
    QMutexLocker lock(&mutex);
    this->newAnalysis = true;
    this->stopThread = false;
    this->firstTime = true;
    generateDNMap = ASG_PIT::settings().getGenerateDNMap();
    if(mode==OFFLINE)
    {
        getFileList();
        if(listOfImages.size()==0)
        {
            emit warnUser("No images in the selected folder, please select a different folder",QMessageBox::Warning);
            emit analysisFinished();
            newAnalysis = false;
            analysisMode=IDLE;
            return;
        }
        setupAnalysisFile(imagesPath);
        if(generateDNMap)
        {
            setupDNMapFile(imagesPath);
        }
    }
    else
    {
        createOnlineAnalysisFolder();
        setupAnalysisFile(onlineAnalysisPath);
        if(generateDNMap)
        {
            setupDNMapFile(onlineAnalysisPath);
        }
    }
    this->imageCounter=0;
    this->analysisMode = mode;
}

void ImageAnalysis::setMatchingAccuracy(double accuracy)
{
    QMutexLocker locker(&mutex);
    this->matchingAccuracy = accuracy;
}

void ImageAnalysis::setNozzelTemplateList(const QFileInfoList &tempList)
{
    QMutexLocker locker(&mutex);
    if(nozzelTemplates.size()!=0)
    {
        for(int i=0;i<nozzelTemplates.size();i++)
        {
            if(nozzelTemplates[i])
                cvReleaseImage(&nozzelTemplates[i]);
        }
    }
    nozzelTemplates.clear();
    for(int i=0;i<tempList.size();i++)
    {
        IplImage * originalImage = cvLoadImage(qPrintable(tempList[i].absoluteFilePath()),CV_LOAD_IMAGE_GRAYSCALE);
        nozzelTemplates.push_back(originalImage);
        IplImage * flippedImage = cvCloneImage(originalImage);
        cvFlip(originalImage,flippedImage,0);
        nozzelTemplates.push_back(flippedImage);
        LOG(Logger::Info,"Template Image Loaded:"<<qPrintable(tempList[i].absoluteFilePath()))
    }
}

void ImageAnalysis::setEdgeMatchingAccuracy(double value)
{
    QMutexLocker locker(&mutex);
    edgeMatchingAccuracy = value;
}

void ImageAnalysis::setEdgeStartTemplateList(const QFileInfoList &tempList)
{
    QMutexLocker locker(&mutex);
    if(edgeStartTemplates.size()!=0)
    {
        for(int i=0;i<edgeStartTemplates.size();i++)
        {
            if(edgeStartTemplates[i])
                cvReleaseImage(&edgeStartTemplates[i]);
        }
    }
    edgeStartTemplates.clear();
    for(int i=0;i<tempList.size();i++)
    {
        edgeStartTemplates.push_back(cvLoadImage(qPrintable(tempList[i].absoluteFilePath()),CV_LOAD_IMAGE_GRAYSCALE));
    }
}

void ImageAnalysis::setEdgeEndTemplateList(const QFileInfoList &tempList)
{
    QMutexLocker locker(&mutex);
    if(edgeEndTemplates.size()!=0)
    {
        for(int i=0;i<edgeEndTemplates.size();i++)
        {
            if(edgeEndTemplates[i])
                cvReleaseImage(&edgeEndTemplates[i]);
        }
    }
    edgeEndTemplates.clear();
    for(int i=0;i<tempList.size();i++)
    {
        edgeEndTemplates.push_back(cvLoadImage(qPrintable(tempList[i].absoluteFilePath()),CV_LOAD_IMAGE_GRAYSCALE));
    }
}

void ImageAnalysis::setSecondFilter(bool state)
{
    QMutexLocker locker(&mutex);
    if(state)
        qDebug()<<"Second Filter Enabled";
    else
        qDebug()<<"Second Filter Disabled";
    this->useSecondFilter = state;
}

void ImageAnalysis::setSecondMatchingAccuracy(double accuracy)
{
    QMutexLocker locker(&mutex);
    this->secondMatchingAccuracy = accuracy;
}

void ImageAnalysis::setShowClearNozzels(bool state)
{
    QMutexLocker locker(&mutex);
    this->showClearNozzels = state;
}

void ImageAnalysis::setShowNumbers(bool state)
{
    QMutexLocker locker(&mutex);
    this->showNumbers = state;
}

void ImageAnalysis::setShowDeadNozzels(bool state)
{
    QMutexLocker locker(&mutex);
    this->showDeadNozzels = state;
}

QVector <QVector<MatchingNozzle> > ImageAnalysis::sortMatchesIntoRows(QVector <MatchingNozzle> matchLocations)
{
    QVector <QVector<MatchingNozzle> > sortedNozzelMap;
    sortedNozzelMap.clear();
    QVector<MatchingNozzle> tempMatches;
    tempMatches << matchLocations;
    qDebug() <<"Sorting Into rows the ("<<tempMatches.size()<<") nozzles";
    for(int i=0;i<tempMatches.size();i++)
    {
        QVector<MatchingNozzle> row;
        row.clear();
        row.push_back(tempMatches[i]);
        for(int j=i+1;j<tempMatches.size();j++)
        {
            float diff;
            QPointF p;
            p.setX(tempMatches[j].location.x());
            p.setY(tempMatches[j].location.y());
            if(row.size()>5)
            {
                diff = distance2Row(p,row);
            }
            else
                 diff = fabs(tempMatches[j].location.y() - rowAverageHeightPose(row));
            assert(diff>=0);
            if(diff<=(tempMatches[j].getTempH()/2.0))
            {
                row.push_back(tempMatches[j]);
                tempMatches.remove(j);
                j--;
            }
        }
        sortedNozzelMap.push_back(row);
    }
    for(int i=0;i<sortedNozzelMap.size();i++)
    {
        qStableSort(sortedNozzelMap[i].begin(),sortedNozzelMap[i].end(),matchPoseCompareX);
    }
    qStableSort(sortedNozzelMap.begin(),sortedNozzelMap.end(),matchPoseCompareY);
    // record the sorted information into the nozzel structure
    for(int i=0;i<sortedNozzelMap.size();i++)
    {
        for(int j=0;j<sortedNozzelMap[i].size();j++)
        {
            sortedNozzelMap[i][j].setRow(i);
            sortedNozzelMap[i][j].setNozzleNum(-1); // not localized yet !!!
        }
    }
    qDebug() <<"ImageAnalysisAlgorithms: Matches sorted into Rows: "<<sortedNozzelMap.size();
    return sortedNozzelMap;
}

void ImageAnalysis::stop()
{
    qDebug()<<"Stopping Analysis";
    QMutexLocker lock(&mutex);
    stopThread = true;
}

QVector <MatchingNozzle> ImageAnalysis::templateNozzleMatch(IplImage * const image2Match)
{
    int tempW,tempH;
    QVector <MatchingNozzle> matchLocations;
    matchLocations.clear();
    emit stateProgress("Matching Templates",0);
    qDebug()<<"Matching Nozzle with ("<<nozzelTemplates.size()<<") templates";
    for(int k=0;k<nozzelTemplates.size();k++)
    {
        IplImage* srcImage = nozzelTemplates[k];
        if(!srcImage)
        {
            qDebug()<<"Error Loading Template Image with Index:"<<k;
            return matchLocations;
        }
        tempW = srcImage->width;
        tempH = srcImage->height;
        assert(srcImage);
        IplImage *result = ImageAnalysisAlgorithms::templateMatch(image2Match, srcImage);
        float threshold = matchingAccuracy;
        float value;
        qDebug() << "Filtering the Matches with Matching accuracy:"<<threshold;
        /* loop the comparison result array */
        // TODO: Optimize this, its the bottle neck in the matching algo !!!
        for( int i = 0 ; i < result->height ; i++ )
        {
            for( int j = 0 ; j < result->width ; j++ )
            {
                /* get the match value for the element */
                value = ((float*)(result->imageData + i*result->widthStep))[j];
                /* if value is greater than a threshold, then similar nozzel is found */
                if( value >= threshold )
                {
                    if(matchLocations.size()==0)
                    {
                        MatchingNozzle nozzelNew = MatchingNozzle(QPoint( j, i ),tempW,tempH,MatchingNozzle::Clear,value);
                        if(!isNozzelOnEdge(nozzelNew))
                            matchLocations.push_back(nozzelNew);
                        continue;
                    }
                    bool toBeInserted=false,newRegion=true;
                    for(int m=0; m<matchLocations.size();m++)
                    {
                        int    x = qRound(matchLocations[m].location.x());
                        int    y = qRound(matchLocations[m].location.y());
                        float  v = matchLocations[m].getMatchingValue();
                        if(j>=(x-tempW) && j<=(x+tempW) && i>=(y-tempH) && i<=(y+tempH))
                        {
                            newRegion = false;
                            if(value>v)
                            {
                                // replace the old value and match with the better one found in the same area
                                toBeInserted = true;
                                matchLocations.remove(m);
                                m--;
                            }
                            else
                            {
                                // There is an element in this area with a higher value
                                // so no need to scan the rest as this entry doesn't
                                // qualify.
                                //                                    toBeInserted = false;
                                break;
                            }
                        }
                    }
                    if(newRegion || toBeInserted)
                    {
                        MatchingNozzle nozzelNew = MatchingNozzle(QPoint( j, i ),tempW,tempH,MatchingNozzle::Clear,value);
                        if(!isNozzelOnEdge(nozzelNew))
                            matchLocations.push_back(nozzelNew);
                    }
                }
            }
        }
        qDebug()<<"Finished adding nozzles";
        cvReleaseImage(&result);
        qDebug()<<"Released Results Image";
        emit stateProgress("Matching Templates",int(((k+1)*100)/nozzelTemplates.size()));
    }
    qDebug() << "Applying Second Filter on the ("<<matchLocations.size()<<") matches.";
    matchLocations = applySecondFilter(matchLocations);
    qDebug() << "Filterning resulted in ("<<matchLocations.size()<<") matches";
    qDebug() << "Matching Done";
    return matchLocations;
}

void ImageAnalysis::triggerNozzel(const QPoint &loc,int button)
{
    qDebug() <<"ImageAnalysisAlgorithms: Triggering MatchingNozzle";
    mouseClicked  = true;
    // Change from unblocked to blocked
    for(int i=0;i<nozzelMap.size();i++)
    {
        for(int j=0;j<nozzelMap[i].size();j++)
        {
            QPoint match(qRound(nozzelMap[i][j].location.x()),qRound(nozzelMap[i][j].location.y()));
            int tempH = nozzelMap[i][j].getTempH();
            if(loc.x()>=match.x() && loc.x()<=(match.x()+distanceBetweenNozzels) && loc.y()>=match.y() && loc.y()<=(match.y()+tempH) )
            {
                if(button==Qt::RightButton)
                {
                    nozzelMap[i].remove(j);
                }
                else if(button==Qt::LeftButton)
                {
                    if(nozzelMap[i][j].getStatus()==MatchingNozzle::Blocked)
                    {
                        nozzelMap[i][j].setStatus(MatchingNozzle::Clear);
                    }
                    else if(nozzelMap[i][j].getStatus()==MatchingNozzle::Clear)
                    {
                        nozzelMap[i][j].setStatus(MatchingNozzle::Blocked);
                    }
                }
                if(foundStartEdge)
                    localizeChipStart();
                if(correlationImage && oldImage)
                    correlateImages(correlationImage,oldImage,overlapPercentage,true);
                if(useSecondFilter)
                {
                    filterDeadNozzleNeighbours();
                }
                redrawNozzels();
                mouseClicked = false;
                return;
            }
        }
    }

    // Right mouse button should not be used to add nozzles
    if(button==Qt::RightButton)
        return;
    // not in the list so see if we can add it
    for(int i=0;i<nozzelMap.size();i++)
    {
        if(!(distance2Row(loc,nozzelMap[i])<(nozzelMap[i][0].getTempH()/2.0)))
            continue;
        for(int k=0;k<nozzelMap[i].size();k++)
        {
            int x =qRound(nozzelMap[i][k].location.x());
            int y =qRound(nozzelMap[i][k].location.y());
            int tempW = nozzelMap[i][k].getTempW();
            int tempH = nozzelMap[i][k].getTempH();
            // is the selected nozzle is in this row and next to this nozzle
//            if(loc.y()>=y && loc.y()<=(y+tempH))
            {
                // find the right neighbour
                if(loc.x()<x)
                {
                    //inserting at the beginning
                    if(k==0)
                    {
                        // do we actually have space for it ?
                        if(x>=distanceBetweenNozzels)
                        {
                            int startingX = x - ((int)floor((x-loc.x())/float(distanceBetweenNozzels))+1)*distanceBetweenNozzels;
                            int startingY = y;
                            MatchingNozzle nozzelNew = MatchingNozzle(QPoint(startingX,startingY),0,tempW,tempH,MatchingNozzle::Clear);
                            // Instead of using the isNozzelOnEdge to check for location on edge
                            // it's used here to fill the chip location information
                            // necessary for localization
                            isNozzelOnEdge(nozzelNew);
                            {
                                insertIntoSortedMatches(nozzelNew);
                                if(foundStartEdge)
                                    localizeChipStart();
                                if(correlationImage && oldImage)
                                    correlateImages(correlationImage,oldImage,overlapPercentage,true);
                                if(useSecondFilter)
                                {
                                    filterDeadNozzleNeighbours();
                                }
                                redrawNozzels();
                                mouseClicked = false;
                                return;
                            }
                        }
                    }
                    else
                    {
                        int startingX = x - ((int)floor((x-loc.x())/float(distanceBetweenNozzels))+1)*distanceBetweenNozzels;
                        int startingY = y;
                        MatchingNozzle nozzelNew = MatchingNozzle(QPoint(startingX,startingY),0,tempW,tempH,MatchingNozzle::Clear);
                        // Instead of using the isNozzelOnEdge to check for location on edge
                        // it's used here to fill the chip location information
                        // necessary for localization
                        isNozzelOnEdge(nozzelNew);
                        {
                            insertIntoSortedMatches(nozzelNew);
                            if(foundStartEdge)
                                localizeChipStart();
                            if(correlationImage && oldImage)
                                correlateImages(correlationImage,oldImage,overlapPercentage,true);
                            if(useSecondFilter)
                            {
                                filterDeadNozzleNeighbours();
                            }
                            redrawNozzels();
                            mouseClicked = false;
                            return;
                        }
                    }
                    return;
                }
                else if (k==(nozzelMap[i].size()-1))
                {
                    //insert at the end
                    // do we actually have space for it ?
                    if((displayFrame->width-x)>=distanceBetweenNozzels)
                    {
                        int startingX = x + ((int)floor((loc.x()-x)/float(distanceBetweenNozzels)))*distanceBetweenNozzels;
                        int startingY = y;
                        MatchingNozzle nozzelNew = MatchingNozzle(QPoint(startingX,startingY),0,tempW,tempH,MatchingNozzle::Clear);
                        // Instead of using the isNozzelOnEdge to check for location on edge
                        // it's used here to fill the chip location information
                        // necessary for localization
                        isNozzelOnEdge(nozzelNew);
                        {
                            insertIntoSortedMatches(nozzelNew);
                            if(foundStartEdge)
                                localizeChipStart();
                            if(correlationImage && oldImage)
                                correlateImages(correlationImage,oldImage,overlapPercentage,true);
                            if(useSecondFilter)
                            {
                                filterDeadNozzleNeighbours();
                            }
                            redrawNozzels();
                            mouseClicked = false;
                            return;
                        }
                    }
                }
            }
        }
    }
    mouseClicked = false;
}


