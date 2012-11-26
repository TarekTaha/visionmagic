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
#include "nozzlehealthviewer.h"

NozzleHealthViewer::NozzleHealthViewer(QGraphicsView  *graphView) :
        QGraphicsView(graphView),
        timerId(0),
        scene(NULL),
        nozzleHealthMaps(NULL),
        mouseIsPressed(false)
{
    scene = new QGraphicsScene();
    setScene(scene);
//    scene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
    setSceneRect(0, 0, 30*700, 11*800);
    //setSceneRect(0, 0, 400, 400);
    setRenderHint(QPainter::Antialiasing, false);
    setOptimizationFlags(DontAdjustForAntialiasing);
    setOptimizationFlags(DontSavePainterState);
    setOptimizationFlags(DontClipPainter);
//    setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

    // Don't use caching. It's SLOW !!!
    //setCacheMode(CacheBackground);
    setTransformationAnchor(AnchorUnderMouse);
    setResizeAnchor(AnchorUnderMouse);
    setInteractive(true);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
    connect(scene,SIGNAL(selectionChanged()),this,SLOT(selectionChanged()));
}

void NozzleHealthViewer::generateTNFPattern()
{
    QString currentDir  = QCoreApplication::applicationDirPath();
    qDebug()<<"Generating Borealis Pattern";
    QFile xmlPatternFile(QString("%1/tnfPattern.xml").arg(currentDir));
    if(!xmlPatternFile.open((QIODevice::WriteOnly)))
    {
        emit warnUser("Could not create XML Pattern File",QMessageBox::Warning);
        return;
    }
    QTextStream xmlStreamer(&xmlPatternFile);
    xmlStreamer << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
    xmlStreamer << "    <DARPrintTestPatternGenerator>\n";
    xmlStreamer << "    <Version>1.0</Version>\n";
    xmlStreamer << "    <ContoneScale>\n";
    xmlStreamer << "        <ContoneXScale>242</ContoneXScale>\n";
    xmlStreamer << "        <ContoneYScale>4</ContoneYScale>\n";
    xmlStreamer << "    </ContoneScale>\n";

    xmlStreamer << "    <FirstFiringNozzle>\n";
    xmlStreamer << "        <Chip>0</Chip>\n";
    xmlStreamer << "        <Row>0</Row>\n";
    xmlStreamer << "        <Nozzle>0</Nozzle>\n";
    xmlStreamer << "    </FirstFiringNozzle>\n";

    xmlStreamer << "    <LastFiringNozzle>\n";
    xmlStreamer << "        <Chip>10</Chip>\n";
    xmlStreamer << "        <Row>0</Row>\n";
    xmlStreamer << "        <Nozzle>620</Nozzle>\n";
    xmlStreamer << "    </LastFiringNozzle>\n";

    xmlStreamer << "    <NumberOfInks>5</NumberOfInks>\n";

    xmlStreamer << "    <Cross>\n";
    xmlStreamer << "        <Type>BitBox</Type>\n";
    xmlStreamer << "        <Position>-110</Position>\n";
    xmlStreamer << "        <Spacing>1280</Spacing>\n";
    xmlStreamer << "        <Offset>912</Offset>\n";
    xmlStreamer << "        <ZeroWidth>21</ZeroWidth>\n";
    xmlStreamer << "        <ZeroHeight>21</ZeroHeight>\n";
    xmlStreamer << "        <OneWidth>41</OneWidth>\n";
    xmlStreamer << "        <OneHeight>41</OneHeight>\n";
    xmlStreamer << "        <Height>96</Height>\n";
    xmlStreamer << "        <Width>96</Width>\n";
    xmlStreamer << "        <Ink>3</Ink>\n";
    xmlStreamer << "    </Cross>\n";

    xmlStreamer << "    <Cross>\n";
    xmlStreamer << "        <Type>BitBox</Type>\n";
    xmlStreamer << "        <Position>+120</Position>\n";
    xmlStreamer << "        <Spacing>1280</Spacing>\n";
    xmlStreamer << "        <Offset>400</Offset>\n";
    xmlStreamer << "        <ZeroWidth>21</ZeroWidth>\n";
    xmlStreamer << "        <ZeroHeight>21</ZeroHeight>\n";
    xmlStreamer << "        <OneWidth>41</OneWidth>\n";
    xmlStreamer << "        <OneHeight>41</OneHeight>\n";
    xmlStreamer << "        <Height>96</Height>\n";
    xmlStreamer << "        <Width>96</Width>\n";
    xmlStreamer << "        <Ink>3</Ink>\n";
    xmlStreamer << "    </Cross>\n";

    xmlStreamer << "    <RulerLocation>"<<currentDir.replace("/","\\")<<"\\tile_fiducials_labelled.soho.bl_8.tif</RulerLocation>\n";

    xmlStreamer << "    <SpitPattern>\n";
    xmlStreamer << "        <BlackSpace>480</BlackSpace>\n";
    xmlStreamer << "        <WedgeSpace>0</WedgeSpace>\n";
    xmlStreamer << "        <WedgeDensity>0.9</WedgeDensity>\n";
    xmlStreamer << "        <WhiteSpace>256</WhiteSpace>\n";
    xmlStreamer << "    </SpitPattern>\n";

    xmlStreamer << "    <CPTAnalysisGrid>\n";
    xmlStreamer << "        <PreWhiteSpace>120</PreWhiteSpace>\n";
    xmlStreamer << "        <SpitSpace>100</SpitSpace>\n";
    xmlStreamer << "        <PostSpitSpace>350</PostSpitSpace>\n";
    xmlStreamer << "        <LineLength>20</LineLength>\n";
    xmlStreamer << "        <LineSpace>10</LineSpace>\n";
    xmlStreamer << "        <LinePattern>00000000001111111111</LinePattern>\n";
    xmlStreamer << "        <ColourSpace>10</ColourSpace>\n";
    xmlStreamer << "        <RowSpace>20</RowSpace>\n";
    xmlStreamer << "        <PostWhiteSpace>350</PostWhiteSpace>\n";
    xmlStreamer << "        <ColourFiducials>Offset=64 Width=41 Height=41 HorzSpacing=128 VertSpace=81 Ink=4</ColourFiducials>\n";
    xmlStreamer << "        <ShowRuler>1</ShowRuler>\n";
    xmlStreamer << "        <IsTNFGrid>1</IsTNFGrid>\n";
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    for(int i=0;i<selectedItems.size();i++)
    {
        SingleNozzle *nozzle = (SingleNozzle*)selectedItems[i];
        xmlStreamer << "        <TargetedNozzle>Chip="<<nozzle->getChipID()<<" Row="<<nozzle->getRow()<<" Nozzle="<<nozzle->getColumn()<<"</TargetedNozzle>\n";
    }
    xmlStreamer << "        <RulerInk>3</RulerInk>\n";
    xmlStreamer << "    </CPTAnalysisGrid>\n";

    xmlStreamer << "</DARPrintTestPatternGenerator>";
    xmlStreamer.flush();
    xmlPatternFile.close();

    QString darApp      = currentDir.replace("/","\\") + "\\" + "dar_tpp.exe";
    QString tiff2BorApp = currentDir.replace("/","\\") + "\\" + "tiff2bor.exe";
    SHELLEXECUTEINFOW sei;
    memset(&sei, 0, sizeof(sei));
    sei.fMask  = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
    QString paramString = QString(" /p \"%1\\tnfPattern.xml\" /i \"%1\\tnfPattern.tif\" ").arg(currentDir.replace("/","\\"));
    sei.cbSize = sizeof(sei);
    sei.hwnd   = GetForegroundWindow();
    sei.lpVerb = L"open";
    sei.lpFile = reinterpret_cast<LPCWSTR>(darApp.utf16());
    sei.lpParameters = reinterpret_cast<LPCWSTR>(paramString.utf16());
    sei.nShow  = SW_SHOWNORMAL;
    BOOL bOK = ShellExecuteExW(&sei);
    if (bOK)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        WaitForSingleObject(sei.hProcess, 10000);
        CloseHandle(sei.hProcess);
        QApplication::restoreOverrideCursor();
    }
    else
    {
        emit warnUser("Error Running dar_tpp.exe",QMessageBox::Warning);
        return;
    }

    //Now run the tiff2BorApp
    paramString = QString(" --dithermatrix \"%1\\MJ_64_8_Jo05_1.0.dmb\" --reorgconf \"%1\\d4d-K_stencil.koi.inverted.reorg\" --a4 --scaling 242/1,4/1 --v7 --bilevel \"%1\\tnfPattern_bilevel.tif\" \"%1\\tnfPattern.tif\" ").arg(currentDir.replace("/","\\"));
    sei.lpFile = reinterpret_cast<LPCWSTR>(tiff2BorApp.utf16());
    sei.lpParameters = reinterpret_cast<LPCWSTR>(paramString.utf16());
    bOK = ShellExecuteExW(&sei);
    if (bOK)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        WaitForSingleObject(sei.hProcess, 10000);
        CloseHandle(sei.hProcess);
        QApplication::restoreOverrideCursor();
    }
    else
    {
        emit warnUser("Error Running tiff2bor.exe",QMessageBox::Warning);
        return;
    }
    emit warnUser(QString("Borealis File correctly created: %1\\tnfPattern.bor").arg(currentDir.replace("/","\\")),QMessageBox::Information);
}

void NozzleHealthViewer::keyPressEvent(QKeyEvent *event)
{
//    QMatrix matrix;
    switch( event->key() )
    {
        case Qt::Key_A:
//            qDebug()<<"Recieved A Key pressed";
//            matrix.translate(0,-1000);
//            setMatrix(matrix,true);
            break;
        case Qt::Key_D:
//            qDebug()<<"Recieved D Key pressed";
//            matrix.translate(0,1000);
//            setMatrix(matrix,true);
            break;
    }
}
//void NozzleHealthViewer::mousePressEvent(QMouseEvent *event)
//{
//    update();
//    mouseIsPressed = true;
//    mousePressedLocation = event->pos();
//    qDebug()<<"Mouse Clicked at x:"<<event->pos().x()<<" y:"<<event->pos().y();
//}
//
//void NozzleHealthViewer::mouseReleaseEvent(QMouseEvent *event)
//{
//    update();
//    mouseIsPressed = false;
//    qDebug()<<"Mouse Released at x:"<<event->pos().x()<<" y:"<<event->pos().y();
//}
//
//void NozzleHealthViewer::mouseMoveEvent(QMouseEvent *event)
//{
//    update();
//    if(mouseIsPressed)
//        translate(mousePressedLocation.x()-event->pos().x(),mousePressedLocation.y()-event->pos().y());
//    qDebug()<<"Mouse Moving at x:"<<event->pos().x()<<" y:"<<event->pos().y();
//}

void NozzleHealthViewer::mergeDNMaps()
{
    QStringList dNMapFiles = QFileDialog::getOpenFileNames(NULL, tr("Chose the DNMaps to merge"),
                                         ASG_PIT::settings().getLastDirPath(),
                                         tr("Images (*.csv)"));
    int n =0;
    n = dNMapFiles.size();
    for(int i=0;i<n;i++)
    {
        qDebug()<<"Selected File: ("<<dNMapFiles[i]<<")";
    }
    if(!dNMapFiles.isEmpty())
    {
        ASG_PIT::settings().setLastDirPath(QFileInfo(dNMapFiles[0]).absolutePath());
        nozzleHealthMaps  = new NozzleHealthMap[dNMapFiles.size()];
        mergedNozzleHealth.resetMap();
        if(n<=0)
            return;
        for(int i=0;i<n;i++)
        {
            qDebug()<<"Loading DNMap("<<i+1<<")";
            try
            {
                nozzleHealthMaps[i].loadMap(dNMapFiles[i]);
            }
            catch(MapFormatException &exp)
            {
                emit warnUser(exp.what(),QMessageBox::Warning);
                delete nozzleHealthMaps;
                return;
            }
        }
        for(int chip=0;chip<NozzleHealthMap::numOfChips;chip++)
        {
            for(int row=0;row<NozzleHealthMap::numOfRows;row++)
            {
                for(int nozzle=0;nozzle<NozzleHealthMap::numOfNozzlesPerRow;nozzle++)
                {
                    double averageHealth =0;
                    int goodCount =0;
                    for(int i=0;i<n;i++)
                    {
                        if(nozzleHealthMaps[i].getChip(chip).getNozzleHealth(row,nozzle))
                        {
                            goodCount++;
                        }
                    }
                    averageHealth = goodCount/double(n);
                    mergedNozzleHealth.setNozzleHealth(chip,row,nozzle,averageHealth);
                }
            }
        }
        delete nozzleHealthMaps;
       viewNozzleHealthMap(&mergedNozzleHealth);
    }
}

void NozzleHealthViewer::mergeIntoImage()
{
    QStringList dNMapFiles = QFileDialog::getOpenFileNames(NULL, tr("Chose the DNMaps to merge"),
                                         ASG_PIT::settings().getLastDirPath(),
                                         tr("Images (*.csv)"));
    if(dNMapFiles.size()==0)
        return;
    QStringList filters;
    QDir dir(QFileInfo(dNMapFiles[0]).absolutePath());
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::Name);
    filters << "*.csv";
    dir.setNameFilters(filters);
    QStringList dNMapFilesSorted;
    QFileInfoList listOfMaps = dir.entryInfoList();

    for(int i=0;i<listOfMaps.size();i++)
    {
        for(int j=0;j<dNMapFiles.size();j++)
        {
            if(listOfMaps[i].baseName() == QFileInfo(dNMapFiles[j]).baseName())
            {
                dNMapFilesSorted.push_back(dNMapFiles[j]);
                break;
            }
        }
    }

    int n= 0;
    n = dNMapFilesSorted.size();
    for(int i=0;i<n;i++)
    {
        qDebug()<<"Selected File: ("<<dNMapFilesSorted[i]<<")";
    }
    int displayStartX=0,displayStartY=0,gapBetweenColorChannels=10,topGap = 10, bottomGap=10;
    QDateTime dateTime;
    QString timeCode = dateTime.currentDateTime().toString(QString("yyyyMMddThhmms"));
    if(!dNMapFilesSorted.isEmpty())
    {
        ASG_PIT::settings().setLastDirPath(QFileInfo(dNMapFilesSorted[0]).absolutePath());
        nozzleHealthMaps  = new NozzleHealthMap[dNMapFilesSorted.size()];
        mergedNozzleHealth.resetMap();
        int height = dNMapFilesSorted.size()*5 + gapBetweenColorChannels*4 + topGap + bottomGap;
        IplImage *outImage = cvCreateImage(cvSize(14036,height),8,3);
        memset(outImage->imageData,255,outImage->width*outImage->height*outImage->nChannels*sizeof(uchar));
        NozzleOperations nozzleOps;
        CvFont font;
        cvInitFont(&font,CV_FONT_HERSHEY_PLAIN,0.4,0.4,0,1);
        for(int channel=0;channel<nozzleOps.numColorChannels;channel++)
        {
            for(int i=0;i<10;i++)
            {
                int yRef = 2 + channel*(gapBetweenColorChannels + dNMapFilesSorted.size());
                cvDrawLine(outImage,cvPoint(nozzleOps.firstChipRefs[channel] + i*1280,yRef),cvPoint(nozzleOps.firstChipRefs[channel] + i*1280,yRef + 5),CV_RGB(0,0,0),1);
                cvDrawLine(outImage,cvPoint(0,yRef+3),cvPoint(outImage->width-1,yRef+3),CV_RGB(0,0,0),1);
                //cvPutText(outImage,QString("Chip:%1").arg(i).toAscii(),cvPoint(nozzleOps.firstChipRefs[channel] + i*1280 +2,yRef),&font,CV_RGB(0,0,0));
            }
        }
        displayStartX = qRound((outImage->width - NozzleHealthMap::numOfNozzlesPerColorChannel)/2.0);
        displayStartY = topGap;
        if(n<=0)
            return;
        for(int i=0;i<n;i++)
        {
            qDebug()<<"Loading Map: "<<QFileInfo(dNMapFilesSorted[i]).baseName();
            try
            {
                nozzleHealthMaps[i].loadMap(dNMapFilesSorted[i]);
            }
            catch(MapFormatException &exp)
            {
                emit warnUser(exp.what(),QMessageBox::Warning);
                cvReleaseImage(&outImage);
                delete nozzleHealthMaps;
                return;
            }
        }
        CvScalar color[5];
        if(ASG_PIT::settings().getSPAPlumpingOder() == QString("CMYKK"))
        {
            color[0] = CV_RGB(0,255,255); color[1] = CV_RGB(255,0,255); color[2] = CV_RGB(255,255,0); color[3] = CV_RGB(0,0,0); color[4] = CV_RGB(0,0,0);
        }
        else // YKCKM
        {
            color[0] = CV_RGB(255,255,0); color[1] = CV_RGB(0,0,0); color[2] = CV_RGB(0,255,255); color[3] = CV_RGB(0,0,0); color[4] = CV_RGB(255,0,255);
        }
        for(int channel=0;channel<NozzleHealthMap::numColorChannels;channel++)
        {
            for(int i=0;i<n;i++)
            {
                for(int nozzle=0;nozzle<NozzleHealthMap::numOfNozzlesPerColorChannel;nozzle++)
                {
                    int chip,row,nozzleNum;
                    nozzleOps.abs2ChipLocal(nozzle,channel,chip,row,nozzleNum);
                    if(nozzleHealthMaps[i].getChip(chip).getNozzleHealth(row,nozzleNum))
                    {
                        outImage->imageData[displayStartY*outImage->widthStep + (displayStartX + nozzle)*outImage->nChannels + 0] = qRound(color[channel].val[0]);
                        outImage->imageData[displayStartY*outImage->widthStep + (displayStartX + nozzle)*outImage->nChannels + 1] = qRound(color[channel].val[1]);
                        outImage->imageData[displayStartY*outImage->widthStep + (displayStartX + nozzle)*outImage->nChannels + 2] = qRound(color[channel].val[2]);
                    }
                    else
                    {
                        outImage->imageData[displayStartY*outImage->widthStep + (displayStartX + nozzle)*outImage->nChannels + 0] = 128;
                        outImage->imageData[displayStartY*outImage->widthStep + (displayStartX + nozzle)*outImage->nChannels + 1] = 128;
                        outImage->imageData[displayStartY*outImage->widthStep + (displayStartX + nozzle)*outImage->nChannels + 2] = 128;
                    }
                }
                displayStartY++;
            }
            displayStartY+=gapBetweenColorChannels;
        }
        QString outPutImageName = QString("%1\\mergedMaps_%2.tif").arg(QFileInfo(dNMapFilesSorted[0]).absolutePath().replace("/","\\"),timeCode);
        cvSaveImage(outPutImageName.toAscii(),outImage);
        cvReleaseImage(&outImage);
        delete nozzleHealthMaps;
        emit warnUser(QString("Done merging data in Maps to image:%1").arg(outPutImageName),QMessageBox::Information);
    }
}

void NozzleHealthViewer::print()
{
#ifndef QT_NO_PRINTER
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QPainter painter(&printer);
        render(&painter);
    }
#endif
}

void NozzleHealthViewer::printTNFP()
{
    QString currentDir  = QCoreApplication::applicationDirPath();
    if(!QFileInfo(currentDir + "\\tnfPattern.bor").exists())
    {
        emit warnUser("tnfPattern.bor File not found, generate it first !",QMessageBox::Warning);
        return;
    }
    QString spoolFile = currentDir.replace("/","\\") + "\\" + "spoolFile.exe";
    SHELLEXECUTEINFOW sei;
    memset(&sei, 0, sizeof(sei));
    sei.fMask  = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
    QString paramString = QString(" \"%1\\tnfPattern.bor\" ").arg(currentDir.replace("/","\\"));
    sei.cbSize = sizeof(sei);
    sei.hwnd   = GetForegroundWindow();
    sei.lpVerb = L"open";
    sei.lpFile = reinterpret_cast<LPCWSTR>(spoolFile.utf16());
    sei.lpParameters = reinterpret_cast<LPCWSTR>(paramString.utf16());
    sei.nShow  = SW_SHOWNORMAL;
    BOOL bOK = ShellExecuteExW(&sei);
    if (bOK)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        WaitForSingleObject(sei.hProcess, 10000);
        CloseHandle(sei.hProcess);
        QApplication::restoreOverrideCursor();
    }
    else
    {
        emit warnUser("Error Running spoolFile.exe",QMessageBox::Warning);
        return;
    }
}

void NozzleHealthViewer::selectionChanged()
{
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    for(int i=0;i<selectedItems.size();i++)
    {
        SingleNozzle *nozzle = (SingleNozzle*)selectedItems[i];
        if(i==0)
            emit showStatusBarMsg(QString("Chip:%1 Row:%2  Nozzle:%3  AbsPose:%4  Color Channel:%5  Health:%6%").arg(nozzle->getChipID()).arg(nozzle->getRow()).arg(nozzle->getColumn()).arg(nozzle->getAbsPosition()).arg(nozzle->getColorChannel()).arg(nozzle->getStatus()*100));
    }
}

void NozzleHealthViewer::scaleView(qreal scaleFactor)
{
    qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.05 || factor > 150)
        return;
    QRectF rect = sceneRect();
    scale(scaleFactor, scaleFactor);
}

void NozzleHealthViewer::showInfo(QString msg)
{
    emit showStatusBarMsg(msg);
}

void NozzleHealthViewer::viewMap()
{
    QString fileName = QFileDialog::getOpenFileName(NULL, tr("Select a Map file (DNMap or NHMap)"),
                                         ASG_PIT::settings().getLastDirPath(),
                                         tr("Maps (*.csv)"));
    if(!fileName.isEmpty() && !fileName.isNull())
    {
        nhMap.resetMap();
        ASG_PIT::settings().setLastDirPath(QFileInfo(fileName).absolutePath());
        try
        {
            nhMap.loadMap(fileName);
        }
        catch(MapFormatException &exp)
        {
            emit warnUser(exp.what(),QMessageBox::Warning);
            return;
        }
        viewNozzleHealthMap(&nhMap);
    }
}

void NozzleHealthViewer::viewNozzleHealthMap(NozzleHealthMap *nhMap)
{
    scene->clear();
    for(int i=0;i<nhMap->getNumChips();i++)
    {
        ChipGraphicItem *chip = new ChipGraphicItem(this,nhMap->getChip(i));
        scene->addItem(chip);
    }
}

void NozzleHealthViewer::wheelEvent(QWheelEvent *event)
{
    scaleView(pow((double)2, event->delta() / 240.0));
}
