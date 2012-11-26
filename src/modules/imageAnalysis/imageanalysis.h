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
#ifndef IMAGEANALYSIS_H
#define IMAGEANALYSIS_H

#include <QThread>
#include <QDir>
#include <QVector>
#include <QMutex>
#include <QTextStream>
#include "dlogger.h"
#include "cv.h"
#include "highgui.h"
#include "imageanalysisalgorithms.h"
#include "acdsettings.h"
#include "messagelogger.h"
#include "mathfun.h"
#include "matchingnozzle.h"
#include "nhmap.h"

class ImageAnalysis : public QThread
{
    Q_OBJECT
public:
    enum{MANUAL,
         SEMI_AUTO,
         AUTO};
    enum{ONLINE,
         OFFLINE,
         IDLE};
    ImageAnalysis();
    ~ImageAnalysis();
    IplImage * analyseImage(const IplImage *_frameImage, bool reAnalyse);
    QVector <MatchingNozzle> applySecondFilter(QVector <MatchingNozzle> matchLocations);
    void createOnlineAnalysisFolder();
    void createProductionResponceDoc();
    void detectEdges(IplImage * const detectionImage);
    QString detectRepetitiveFailure();
    bool detectStartEdge(IplImage *_frameImage);
    bool detectEndEdge(IplImage *_frameImage);
    QVector <MatchingNozzle> detectEdge(IplImage *,QVector<IplImage *> templateList);
    double distance2Row(QPointF point,QVector<MatchingNozzle> row);
    void drawNozzels(QVector <MatchingNozzle> nozzelSet,bool numberThem);
    void drawNozzel(MatchingNozzle singleNozzel,CvScalar color,int index);
    void drawNozzel(MatchingNozzle singleNozzel,CvScalar color);
    void drawDeadNozzels(QVector <MatchingNozzle> nozzelSet);
    void drawMatchedNozzels(QVector <MatchingNozzle> nozzelSet);
    void drawNozzels(QVector <QVector<MatchingNozzle> > mappedNozzels);
    void drawNozzels(QVector <MatchingNozzle> nozzelSet,CvScalar color,bool numberThem);
    void drawRowLines(QVector <QVector<MatchingNozzle> > mappedNozzels);
    bool dropTriangleLocalized();
    bool edgesFound();
    bool endEdgeFound();
    void filterDeadNozzleNeighbours();
    void filterImage(IplImage *srcImage);
    void filterNozzlesAroundEdges();
    IplImage * getDisplayFrame();
    QPoint getEdgesTemplateSize();
    void getFileList();
    int getImagesSource();
    void getLineEdges(Line edgeLine, int imageH,QPointF &p1, QPointF &p2);
    void getLineEdges(QPointF &p1, QPointF &p2, int imageH);
    QPoint getNozzelTemplateSize();
    int getNumDeadNozzles();
    void insertIntoSortedMatches(MatchingNozzle matchL);
    bool isNozzelOnEdge(MatchingNozzle&);
    void keepRelevantChannelRows();
    void locateDeadNozzels();
    void localizeChipStart();
    void mouseClickedOnNozzle(const QPoint &loc, int button);
    void pushData2MesInbox();
    void redrawNozzels();
    void resetAnalysis();
    void resetDNMap();
    double rowAverageHeightPose(QVector<MatchingNozzle> row);
    void saveResult2File(QString filename, QTextStream &out);
    void saveTotal2File(QTextStream &out);
    void saveDNMap2File(QTextStream &out);
    void saveResult2DNMap();
    QVector <QVector<MatchingNozzle> > selectActiveChannels(QVector <QVector<MatchingNozzle> >);
    void setDir(QString dir);
    void setDNMapFileName(QString);
    void setDNMapGeneration(int);
    void setEdgeEndTemplateList(const QFileInfoList &);
    void setEdgeMatchingAccuracy(double);
    void setEdgeStartTemplateList(const QFileInfoList &);
    void setImagesSource(int imageSource);
    void setMatchingAccuracy(double);
    void setMode(int);
    void setNozzelTemplateList(const QFileInfoList &);
    void setPrintHeadCode(QString);
    void setShowClearNozzels(bool state);
    void setShowDeadNozzels(bool state);
    void setShowNumbers(bool state);
    void setType(int);
    void setupAnalysisFile(QString dirPath);
    void setupDNMapFile(QString dirPath);
    QVector <QVector<MatchingNozzle> > sortMatchesIntoRows(QVector <MatchingNozzle> matchLocations);
    bool startEdgeFound();
    void startNewAnalysis(int mode);
    QVector <MatchingNozzle> templateNozzleMatch(IplImage * const image2Match);
    void triggerNozzel(const QPoint &loc,int button);

    void run();
    void stop();
signals:
    void analysisFinished();
    void displayImage(IplImage *frameImage);
    void displayImageThenRelease(IplImage *frameImage);
    void finishedMatching();
    void processingImage(QString);
    void processingImageAnalysis();
    void sendOriginalDisplayImage(IplImage*);
    void stateProgress(QString state,int progress);
    void finishedProcessingImageAnalysis();
    void warnUser(QString,int);
    void viewDNMap(NozzleHealthMap*);
 public  slots:
        void nozzelSelected(const QPoint &locationPressed,int button);
        void setPaused(bool state);
        void setRepeatAnalysis(bool repeat);
        void setOnlineImage(IplImage *frameImage,int linearPose, int camPose);
        void setSecondFilter(bool);
        void setSecondMatchingAccuracy(double);
private:
        void clearImageContainers();
        void correlateImages(IplImage *,IplImage *,int overLap,bool mouseClicked);
        int getDistanceBetweenNozzels();
        void resetPhysicalDamageCount();
        double rowAverageMatch(QVector<MatchingNozzle> row);
        IplImage * loadImage(QString imageFileName);
        void prepareForThreadRestart();
        QMutex mutex;
        QFileInfoList listOfImages;
        QPoint imageSize,locationPressed,nozzelTempSize,edgesTempSize;;
        QFile analysisFile,dnMapFile;
        QTextStream dnMapOutStream,analysisOutStream,productionDocStream;
        QString productionResponceDocFileName,imagesPath,printHeadCode,onlineAnalysisPath,absolutePath;
        QDir parentDir;
        QVector <QVector<MatchingNozzle> > nozzelMap,correlationNozzleMap;
        QVector <MatchingNozzle> deadNozzels, allNozzels;
        Line startEdge, endEdge;
        bool stopThread,paused,nozzelPressed,displayContainerInitialized,repeatAnalysis,firstTime,newAnalysis,
             foundStartEdge,foundEndEdge,showDeadNozzels,showClearNozzels,showNumbers,prevImageContainsStartingEdge,mouseClicked,
             analysisFileHeaderPrinted,generateDNMap,dummyGapFilled[10],useSecondFilter;
        bool dnMap[5][14036],diePhysicalMap[11][10][640],testFailed;
        unsigned int diePhysicalMapCount[10][640];
        int numDeadNozzels,totalDeadNozzleNum,adaptBlockSize,gaussianConst,adjacentNozzelCounts[10],totalAdjacentNozzelCounts[10],
            imageSource,manualInputClicks,totalManualClicks,distanceBetweenNozzels,chipIndex,crossRowsReferences[10],
            buttonPressed, analysisType,analysisMode,linearMotorPose,cameraMotorPose,imageCounter,requestConfirmation,overlapPercentage;
        double matchingAccuracy,secondMatchingAccuracy,edgeMatchingAccuracy;
        IplImage *onlineImage,*disImage,*image,*temp,*oldOnlineImage,*displayFrame,*originalDisplay,*analysisImage,*tempFrameImage,
                 *oldImage,*correlationImage;
        QVector <IplImage *> nozzelTemplates,edgeStartTemplates,edgeEndTemplates;
        CvMemStorage* storage;
        CvFont font;
        CvScalar color;
        char str[100];
        NozzleHealthMap nhMap;
};

#endif // IMAGEANALYSIS_H
