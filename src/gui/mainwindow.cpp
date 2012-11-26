#include "mainwindow.h"
#include "treemodel.h"
#include "qsqldatabase.h"
#include "qsqldriver.h"
#include "qsqlquery.h"
#include "qsqlerror.h"
#include "qsqlrecord.h"

MainWindow::MainWindow(SplashScreen *splashScreen,QWidget *parent)
    : QMainWindow(parent),
    imageViewer(NULL),
    settingsDialog(NULL),
    comPortDetection(NULL),
    comPortDetectionThread(NULL),
    ledLightingController(NULL),
    dnMapGeneratorDialog(NULL),
    autoFocus(NULL),
    motorControl(NULL),
    imageAcquisition(NULL),
    imageAnalysis(NULL),
//    nhaWrapper(NULL),
    cameraGrabber(NULL),
    frame(NULL),
    displayFrame(NULL),
    offlineImage(NULL),
    nozzleHealthViewer(NULL),
    encapsulationCoverage(NULL),
    matchValue(NULL),
    addNozzelTemplateBtn(NULL),
    addEdgeStartTemplateBtn(NULL),
    addEdgeEndTemplateBtn(NULL),
    clearNozzelTempListBtn(NULL),
    clearEdgeStartTempListBtn(NULL),
    clearEdgeEndTempListBtn(NULL),
    mesPushDataBtn(NULL),
    manualAnalysis(NULL),
    semiAutoAnalysis(NULL),
    autoAnalysis(NULL),
    acdImagesSource(NULL),
    acdImagesSourceSmall(NULL),
    zebraFishImagesSource(NULL),
    novoImagesSource(NULL),
    thunderBirdImageSource(NULL),
    showDeadNozzels(NULL),
    showClearNozzels(NULL),
    showNumbers(NULL),
    generateDNMapChkBox(NULL),
    analysisSetting(NULL),
    analysisTypeGrp(NULL),
    analysisDisplay(NULL),
    matchPercentageGrp(NULL),
    nozzelTempGrp(NULL),
    imagesSourceGrp(NULL),
    displayNozzelsGrp(NULL),
    mesGrp(NULL),
    templatesTab(NULL),
    linearMotorPose(0),
    homePose(0),
    cameraMotorPose(0),
    currentChip(0),
    sceneRendering(NULL),
    msgBox(NULL),
    logger(NULL),
    AVTInitialized(false),
    analysingOffline(false),
    threadsInitialized(false),
    analysingOnline(false),
    addingTemplate(false),
    repeatAnalysis(false),
    analysingImage(true),
    scriptEngineStarted(false),
    testLoopStarted(false),
    timedAcqusition(false),
    alreadyGotHeadCode(false),
    continousSweeping(false),
    calibratingIntensity(false),
    focusWindowSet(false),
    steppingMotion(false),
    keysDisabled(false)
{
    graph2Plot = ANGLE_VS_TIME;
    qRegisterMetaType<QVector<QPointF> >("QVector<QPointF>");
    qRegisterMetaType<QVector<int> >("QVector<int>");
    nozzelTempPath = QString("%1/nozzelTemplates").arg(QApplication::applicationDirPath());
    edgeStartPath  = QString("%1/edgeStartTemplates").arg(QApplication::applicationDirPath());
    edgeEndPath    = QString("%1/edgeEndTemplates").arg(QApplication::applicationDirPath());
    settingsDialog = new SettingsDialog();
    dnMapGeneratorDialog = new DNMapGeneratorDialog(this);
    logger = new MessageLogger();
    qDebug()<<"MainWindow: Started";
    ui.setupUi(this);
    // Camera Initialization
    useOpenCVGrabber = true;
    distanceBetweenChips = ASG_PIT::settings().getChipStepSize();
    splashScreen->showMessage("Loading Components", Qt::AlignRight, Qt::white);
    qDebug()<<"ACD launched from:"<<QDir::currentPath()<<" application is installed in:"<<QApplication::applicationDirPath();


    cameraGrabber = new CameraGrabber();
    cameraGrabber->addFrameReceiver(ui.cvWidget);
    cameraGrabber->addFrameReceiver(ui.encapDelamDisplay);
    cameraGrabber->addFrameReceiver(this);
    connect(cameraGrabber,SIGNAL(camerasFound(QVector<QString>)),this,SLOT(camerasFound(QVector<QString>)));
    cameraGrabber->addSettingsConnection(this,SIGNAL(shutterSpeedChanged(int)),SLOT(on_shutterSpeedChanged(int)),TO_CAMERA_SIGNAL);
    cameraGrabber->addSettingsConnection(this,SIGNAL(startRecording(QString)),SLOT(startRecording(QString)),TO_CAMERA_SLOT);
    cameraGrabber->addSettingsConnection(this,SIGNAL(stopRecording()),SLOT(stopRecording()),TO_CAMERA_SLOT);
    ui.autoSkip2Next->setChecked(ASG_PIT::settings().getAutoSkip2Next());
    ui.nextStep->setEnabled(!ASG_PIT::settings().getAutoSkip2Next());
    // hide the NHA Settings
    settingsDialog->ui.pageList->item(7)->setHidden(true);
    // ui.tabWidget->removeTab(5); delamination Tab
    //ui.tabWidget->removeTab(4);
    //ui.tabWidget->removeTab(3);
    //ui.tabWidget->removeTab(2);
    // Scene OpenGL rendering
    sceneRendering = new SceneVisualization(this);
    connect(sceneRendering,SIGNAL(nozzelSelected(const QPoint&,int)),SLOT(nozzelSelected(const QPoint&,int)));
    connect(sceneRendering,SIGNAL(drawTemplateMask(const QPoint&)),SLOT(drawTemplateMask(const QPoint&)));
    connect(sceneRendering,SIGNAL(propagateKeyPressEvent(QKeyEvent*)),this,SLOT(keyPressEvent(QKeyEvent*)));
    setupOfflineAnalysisTab();
    setupImageAnalysis();
    //Encap
    encapsulationCoverage = new EncapsulationCoverage;
    connect(encapsulationCoverage,SIGNAL(warnUser(QString,int)),SLOT(warnUser(QString,int)));
    connect(encapsulationCoverage,SIGNAL(displayImageThenRelease(IplImage*)),ui.encapsulationDisplay,SLOT(putImageThenRelease(IplImage*)));
    connect(encapsulationCoverage,SIGNAL(processingImage(QString,int)),SLOT(processingImage(QString,int)));
    ui.cvWidget->setEnableROISelection(true);
    loadProgrammedList();
    ui.actionOnline_Analysis->setVisible(true);
    ui.actionOnline_Analysis->setEnabled(true);
    cameraGrabber->setupGrabbingSource();
    cameraGrabber->start();
    QStringList labels;
    setupNHAViewerTab();
    //Tree View
    /*
    QStringList headers;
    headers << tr("Die") << tr("Section");
    TreeModel *model = new TreeModel(headers);
    ui.view->setModel(model);
    for (int column = 0; column < model->columnCount(); ++column)
        ui.view->resizeColumnToContents(column);
    */

//    nhaWrapper = new NhaWrapper();
//    connect(nhaWrapper,SIGNAL(displayImage(IplImage*)),ui.imageDisplay,SLOT(putImageThenRelease(IplImage*)));
//    connect(nhaWrapper,SIGNAL(resetView()),ui.imageDisplay,SLOT(resetView()));
//    connect(nhaWrapper,SIGNAL(stateProgress(QString,int)),SLOT(showProgress(QString,int)));
//    connect(nhaWrapper,SIGNAL(warnUser(QString,int)),SLOT(warnUser(QString,int)));
//    connect(nhaWrapper,SIGNAL(spaAnalysisFinised()),SLOT(spaFinished()));
//    connect(nhaWrapper,SIGNAL(viewHealthMap(NozzleHealthMap *)),nozzleHealthViewer,SLOT(viewNozzleHealthMap(NozzleHealthMap *)));
    // Hide all settings other than the SPA
//    settingsDialog->ui.pageList->item(0)->setHidden(true);
//    settingsDialog->ui.pageList->item(1)->setHidden(true);
//    settingsDialog->ui.pageList->item(2)->setHidden(true);
//    settingsDialog->ui.pageList->item(3)->setHidden(true);
//    settingsDialog->ui.pageList->item(4)->setHidden(true);
//    settingsDialog->ui.pageList->item(5)->setHidden(true);
//    settingsDialog->ui.pageList->item(6)->setHidden(true);
    if(ASG_PIT::settings().getSPAChartVersion()==0.4)
        ui.chartVersion4Radio->setChecked(true);
    else
        ui.chartVersion6Radio->setChecked(true);
#ifdef LOCK_SPA_PARAMETERS
    settingsDialog->ui.pageList->item(7)->setHidden(true);
    ui.menuTools->setVisible(false);
    ui.menuTools->clear();
    delete ui.menuTools;
    ui.groupBox_3->setVisible(false);
    //ui.groupBox_8->setVisible(false);
    ui.generateTNFP->setVisible(false);
    ui.backDoorPrint->setVisible(false);
    //ui.mergeDNMaps->setVisible(false);
    //ui.displayDNMap->setVisible(false);
    ui.tabWidget_3->removeTab(1);
    // in gui mode saving debug messages is disabled
    ASG_PIT::settings().setSPASaveDebugImage(false);
#endif
    settingsDialog->setWindowTitle(tr("NHA Settings"));
    ui.imagesTableList->setSelectionBehavior(QAbstractItemView::SelectRows);
    labels.clear();
    labels << tr("File Name") << tr("Size");
    ui.imagesTableList->setHorizontalHeaderLabels(labels);
    ui.imagesTableList->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    ui.imagesTableList->verticalHeader()->hide();
    ui.imagesTableList->setShowGrid(false);
    connect(ui.imagesTableList,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),this,SLOT(showImageDetails(QTableWidgetItem*)));
    connect(ui.delamTable,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),this,SLOT(showDetails(QTableWidgetItem*)));

    connect(ui.removeSelectedTableItems, SIGNAL(released()), this, SLOT(clearTableSelection()));
    connect(ui.traverseDirTree,SIGNAL(released()),this,SLOT(extractSPAImagesFromDirTree()));
    connect(settingsDialog->ui_spa.toleranceToFiducialSize,SIGNAL(sliderReleased()),SLOT(changeSPASettings()));
    connect(settingsDialog->ui_spa.toleranceToFiducialSquares,SIGNAL(sliderReleased()),SLOT(changeSPASettings()));
    connect(settingsDialog->ui_spa.toleranceToLinePatternSize,SIGNAL(sliderReleased()),SLOT(changeSPASettings()));
    connect(settingsDialog->ui_spa.toleranceToOffset,SIGNAL(sliderReleased()),SLOT(changeSPASettings()));

    labels.clear();
    labels<<tr("Die")<<tr("Section")<<tr("Ch")<<tr("Row")<<tr("RD")<<tr("Img");
    ui.delamTable->setHorizontalHeaderLabels(labels);
    ui.delamTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.delamTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    ui.delamTable->verticalHeader()->hide();
    populateSPAGui();

    QStyle *arthurStyle = new ArthurStyle();
    QList<QWidget *> widgets = qFindChildren<QWidget *>(ui.centralwidget);
    foreach (QWidget *w, widgets)
        w->setStyle(arthurStyle);
    splashScreen->showMessage("Initialising GUI", Qt::AlignRight, Qt::white);
    connect(ui.actionTimed_Acquisition,SIGNAL(triggered()),this,SLOT(startTimedAquisition()));
    connect(ui.actionRun_Test_Script,SIGNAL(triggered()),this,SLOT(runTestScript()));
    connect(ui.actionRefocus,SIGNAL(triggered()),SLOT(refocus()));
    connect(ui.actionLoading_Position,SIGNAL(triggered()),SLOT(gotoLoadingPosition()));
    connect(ui.actionLeft,SIGNAL(triggered()),SLOT(goLeft()));
    connect(ui.actionRight,SIGNAL(triggered()),SLOT(goRight()));
    connect(ui.actionNext_Chip,SIGNAL(triggered()),SLOT(gotoNextChip()));
    connect(ui.actionPrevious_Chip,SIGNAL(triggered()),SLOT(gotoPrevChip()));
    connect(ui.actionSave_Image,SIGNAL(triggered()),SLOT(saveCurrentImage()));
    connect(ui.tabWidget,SIGNAL(currentChanged(int)),SLOT(changeFocusedTab(int)));
    connect(ui.actionOnline_Analysis,SIGNAL(triggered()),SLOT(onlineAnalysisChanged()));
    connect(ui.actionAbout, SIGNAL(triggered()), SLOT(about()));
    connect(ui.actionManual, SIGNAL(triggered()), SLOT(helpManual()));
    connect(ui.actionComPort, SIGNAL(triggered()), SLOT(detectComPortsDialog()));
    connect(ui.actionShow_log,SIGNAL(triggered()),SLOT(showLog()));
    connect(ui.printHeadCode, SIGNAL(textChanged(const QString&)), SLOT(printHeadCodeChanged(const QString&)));
    connect(ui.actionOptions, SIGNAL(triggered()), SLOT(showSettingsDialog()));
    connect(ui.actionProbabilistic_DNMaps, SIGNAL(triggered()), SLOT(showProbabilisticDNMap()));
    connect(ui.actionOffline_Analysis, SIGNAL(triggered()), SLOT(startOfflineAnalysis()));
    connect(settingsDialog, SIGNAL(connect2MotorOnPort(QString)),this,SLOT(initiateMotors(QString)));
    connect(ui.actionAuto_Focus, SIGNAL(triggered()), SLOT(changeFocusStatus()));
    connect(ui.actionFine_Focus,SIGNAL(triggered()),SLOT(changeFineFocusStatus()));
    connect(ui.actionEnd, SIGNAL(triggered()), SLOT(gotoEnd()));
    connect(ui.actionHome, SIGNAL(triggered()), SLOT(gotoHome()));
    connect(ui.actionAcquisition, SIGNAL(triggered()), SLOT(triggerAcquisition()));
    connect(ui.actionRun_Script,SIGNAL(triggered()),this,SLOT(runScript()));
    ui.processingImageLabel->setVisible(false);
    ui.progressBar->setVisible(false);
    ui.acqProgress->setVisible(false);
    ui.printHeadCode->setVisible(false);
    ui.printHeadName->setVisible(false);
    scriptEngine.globalObject().setProperty("ACD", scriptEngine.newQObject(this));
    splashScreen->showMessage("Loading Plotting Components ...", Qt::AlignRight, Qt::white);
    QSqlDatabase dB = QSqlDatabase::addDatabase("QMYSQL");
    defaultDB = new QSqlDatabase(dB);
    //defaultDB->addDatabase("QMYSQL");
    defaultDB->setDatabaseName( "fa" );
    defaultDB->setUserName( "root" );
    defaultDB->setPassword( "tarektatam" );
    defaultDB->setHostName( "asg-pc");
    qDebug() << QSqlDatabase::drivers();
    if( defaultDB->open() )
    {
        qDebug()<< "Sucessfully Connected \n";
    }
    else
        qDebug()<< "Connection Failed \n";

}

MainWindow::~MainWindow()
{
    qDebug() << "Clearing Resources and Stopping Threads";
    if(imageAcquisition)
    {
        while(imageAcquisition->isRunning())
        {
            imageAcquisition->stop();
            // wait for a max of 200ms for the thread to finish
            imageAcquisition->wait(200);
        }
        delete imageAcquisition;
        qDebug() << "Image Acquisition resources cleared";
    }
    if(encapDelamInspection)
    {
        while(encapDelamInspection->isRunning())
        {
            encapDelamInspection->stop();
            // wait for a max of 200ms for the thread to finish
            encapDelamInspection->wait(200);
        }
        delete encapDelamInspection;
        qDebug() << "Image Acquisition resources cleared";
    }
    if(autoFocus)
    {
        while(autoFocus->isRunning())
        {
            autoFocus->stop();
            // wait for a max of 200ms for the thread to finish
            autoFocus->wait(200);
        }
        delete autoFocus;
        qDebug() << "AutoFocus resources cleared";
    }

    if(motorControl)
    {
        while(motorControl->isRunning())
        {
            motorControl->stop();
            // wait for a max of 100ms for the thread to finish
            motorControl->wait(500);
        }
        delete motorControl;
        qDebug() << "motorControl resources cleared";
    }
    if(imageAnalysis)
    {
        while(imageAnalysis->isRunning())
        {
            imageAnalysis->stop();
            // wait for a max of 5000ms for the thread to finish
            imageAnalysis->wait(5000);
        }
        delete imageAnalysis;
        qDebug() << "ImageAnalysis resources cleared";
    }
    if(sceneRendering)
        delete sceneRendering;
//    if(nhaWrapper)
//    {
//        while(nhaWrapper->isRunning())
//        {
//            nhaWrapper->stop();
//            // wait for a max of 1000ms for the thread to finish
//            nhaWrapper->wait(5000);
//        }
//        delete nhaWrapper;
//        qDebug() << "SpiderAnalysis resources cleared";
//    }
    if(encapsulationCoverage)
    {
        while(encapsulationCoverage->isRunning())
        {
            encapsulationCoverage->stop();
            encapsulationCoverage->wait(5000);
        }
        delete encapsulationCoverage;
        qDebug() << "Encapsulation Coverage resources cleared";
    }
    if(cameraGrabber)
    {
        while(cameraGrabber->isRunning())
        {
            cameraGrabber->stop();
            cameraGrabber->wait(5000);
        }
        delete cameraGrabber;
        qDebug() << "cameraGrabber resources cleared";
    }
    if(comPortDetection)
    {
        delete comPortDetection;
        qDebug() << "ComPortDetection resources cleared";
    }
    if(comPortDetectionThread)
    {
        delete comPortDetectionThread;
        qDebug() << "ComPortDetectionThread resources cleared";
    }
    if(ledLightingController)
    {
        delete ledLightingController;
        qDebug() << "ledLightingController resources cleared";
    }
    if(displayFrame)
        cvReleaseImage(&displayFrame);
    if(msgBox)
    {
        delete msgBox;
    }
    // clear the plotting data
    for(int i=0;i<plotDataPointers.size();i++)
        free(plotDataPointers[i]);
    if(imageViewer)
        delete imageViewer;
    if(defaultDB->open())
    {
        defaultDB->close();
        delete defaultDB;
    }
    qDebug() << "Clearing Resources and Stopping Threads:: DONE";
 }

void MainWindow::about()
{
//    #ifdef PIT_ONLY
    QString aboutInfo = QString("<b>ACD</b> %1  Built on %2 at %3<br/>Autonomous Contamination Detection,").arg(APP_VERSION_LONG,QLatin1String(__DATE__),QLatin1String(__TIME__))
                        .append(" is a tool developed to facilitate print head inspection and fault ")
                        .append("detection. Developed by: <b>SBR-ASG</b>. For help contact <b>Tarek Taha</b> or <b>David Worboys</b>.");
        QMessageBox::about(this, tr("About ACD"),aboutInfo);
//    #else
//    const QString aboutInfo = tr(
//            "<h3> Nozzle Health Analyser (NHA) %1</h3>"
//            "<br/>"
//            "Built on %2 at %3<br />"
//            "<br/>"
//            "%4"
//            "<br/>"
//            "<br/>"
//            "Copyright %5 %6. All rights reserved.<br/>"
//            "<br/>").arg(NHA_VERSION_LONG,QLatin1String(__DATE__),QLatin1String(__TIME__),QString("A tool to facilitate Nozzle Health inspection."),QLatin1String(YEAR),(QLatin1String(AUTHOR)));
//        QMessageBox::about(this, tr("About NHA"),aboutInfo);
//    #endif
}

void MainWindow::addEdgeEndTemplateBtnClicked()
{
    if(!isTemplateSouceAvailable())
        return;
    templateType  = EDGE_END;
    addingTemplate = true;
    addNozzelTemplateBtn->setEnabled(false);
    addEdgeStartTemplateBtn->setEnabled(false);
    addEdgeEndTemplateBtn->setEnabled(false);
    sceneRendering->setMouseTracking(true);
    sceneRendering->setCursor(Qt::CrossCursor);
}

void MainWindow::addFiles2TableView(QStringList files)
{
    QStringList labels;
    labels << tr("File Name") << tr("Size");
    ui.imagesTableList->setHorizontalHeaderLabels(labels);
    for(int i=0;i<files.size();i++)
    {
        qint64 size = QFileInfo(files[i]).size();
        QTableWidgetItem *fileNameItem = new QTableWidgetItem(files[i]);
        fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *sizeItem = new QTableWidgetItem(tr("%1 MB").arg(double((size + 1023) / (1024*1024))));
        sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sizeItem->setFlags(sizeItem->flags() ^ Qt::ItemIsEditable);

        ui.imagesTableList->insertRow(i);
        ui.imagesTableList->setItem(i, 0, fileNameItem);
        ui.imagesTableList->setItem(i, 1, sizeItem);
    }
}

void MainWindow::addEdgeStartTemplateBtnClicked()
{
    if(!isTemplateSouceAvailable())
        return;
    templateType  = EDGE_START;
    addingTemplate = true;
    addNozzelTemplateBtn->setEnabled(false);
    addEdgeStartTemplateBtn->setEnabled(false);
    addEdgeEndTemplateBtn->setEnabled(false);
    sceneRendering->setMouseTracking(true);
    sceneRendering->setCursor(Qt::CrossCursor);
}


void MainWindow::addNozzelTemplateBtnClicked()
{
    if(!isTemplateSouceAvailable())
        return;
    templateType  = NOZZEL;
    addingTemplate = true;
    addNozzelTemplateBtn->setEnabled(false);
    addEdgeStartTemplateBtn->setEnabled(false);
    addEdgeEndTemplateBtn->setEnabled(false);
    sceneRendering->setMouseTracking(true);
    sceneRendering->setCursor(Qt::CrossCursor);
}

void MainWindow::acquisitionEnded(bool encapEnabled)
{
    ui.actionAcquisition->setText("Image Acquisition");
    hideProgress();
    if(encapEnabled)
    {
        ui.encapOnlineAnalysis->setText("Start Online Analysis");
        ui.tabWidget->widget(1)->setEnabled(true);
        ui.groupBox_16->setEnabled(true);
        ui.groupBox_13->setEnabled(true);
        ui.groupBox_20->setEnabled(true);

        ui.printHeadCode->setEnabled(true);
        ui.printHeadName->setVisible(true);
        ui.printHeadCode->setVisible(true);
        imageAcquisition->setAcquisition(false);
        ui.progressBar->setVisible(false);
        ui.acqProgress->setVisible(false);
        ui.tabWidget->widget(1)->setEnabled(true);
        ui.groupBox_16->setEnabled(true);
        ui.groupBox_14->setEnabled(true);
        ui.groupBox_13->setEnabled(true);
        ui.groupBox_20->setEnabled(true);
        QList<QWidget *> widgets = qFindChildren<QWidget *>(ui.toolBar);
        foreach (QWidget *w, widgets)
            w->setEnabled(true);
        ui.actionOnline_Analysis->setEnabled(false);
    }
    if(testLoopStarted) // here we are in the test loop so keep everyting disabled
    {
        analyseLastAcquiredImageSet();
        ui.progressBar->setVisible(false);
        ui.acqProgress->setVisible(false);
        ui.printHeadCode->setEnabled(true);
        ui.printHeadCode->setVisible(false);
        ui.printHeadName->setVisible(false);
    }
    else // here it was a single image aquisition procedure so re-enable the toolbar
    {
        ui.printHeadCode->setEnabled(true);
        ui.printHeadName->setVisible(true);
        ui.printHeadCode->setVisible(true);
        ui.progressBar->setVisible(false);
        ui.acqProgress->setVisible(false);
        ui.tabWidget->widget(1)->setEnabled(true);
        ui.groupBox_16->setEnabled(true);
        ui.groupBox_14->setEnabled(true);
        ui.groupBox_13->setEnabled(true);
        ui.groupBox_20->setEnabled(true);
        QList<QWidget *> widgets = qFindChildren<QWidget *>(ui.toolBar);
        foreach (QWidget *w, widgets)
            w->setEnabled(true);
        ui.actionOnline_Analysis->setEnabled(false);
    }
}

void MainWindow::analysisFinished()
{
    ui.actionOffline_Analysis->setText("Offline Analysis");
    if(cameraSource == Camera::NONE)
    {
        disableCameraRelatedOptions();
    }
    else
    {
        QList<QWidget *> widgets1 = qFindChildren<QWidget *>(ui.toolBar);
        foreach (QWidget *w, widgets1)
            w->setEnabled(true);
    }
    QList<QWidget *> widgets2 = qFindChildren<QWidget *>(ui.imageAnalysisTab);
    foreach (QWidget *w, widgets2)
        w->setEnabled(true);
    ui.processingImageLabel->setVisible(false);
    analysingOffline = false;
    mesPushDataBtn->setEnabled(true);
    ui.actionOnline_Analysis->setEnabled(false);
    if(testLoopStarted)
    {
        testLoopStarted = false;
        gotoLoadingPosition();
    }
}

void MainWindow::analyseLastAcquiredImageSet()
{
    startOfflineAnalysis(ASG_PIT::settings().getAcqLastPath());
}

void MainWindow::analyseSPA()
{
//    if(nhaWrapper && ui.imagesTableList->rowCount()!=0)
//    {
//        changeSPASettings();
//        QStringList patternList;
//        for(int i=0;i<ui.imagesTableList->rowCount();i++)
//        {
//            patternList<< ui.imagesTableList->item(i,0)->text();
//            qDebug()<<ui.imagesTableList->item(i,0)->text();
//        }
//        nhaWrapper->setSpaImages2Analyse(patternList);
//        nhaWrapper->start();
//        QList<QWidget *> widgets = qFindChildren<QWidget *>(ui.spaTab);
//        foreach (QWidget *w, widgets)
//            w->setEnabled(false);
//    }
//    else
//    {
//        warnUser("Select scanned Patterns first then try again",QMessageBox::Warning);
//    }
    QString nhaApp = QString("\"C:\\Program Files\\SBR\\PIM\\Nozzle Health Analyser\\NHA.exe\"");
    SHELLEXECUTEINFOW sei;
    memset(&sei, 0, sizeof(sei));
    sei.fMask  = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
    for(int i=0;i<ui.imagesTableList->rowCount();i++)
    {
        ui.imagesTableList->item(i,0)->text();
        QString paramString = QString(" --dnmap \"%1\" --dir \"%2\"").arg(ui.imagesTableList->item(i,0)->text()).arg(QFileInfo(ui.imagesTableList->item(i,0)->text()).absolutePath()).replace("/","\\");
        qDebug() << "Param string: " << nhaApp + paramString;
        sei.cbSize = sizeof(sei);
        sei.hwnd   = GetForegroundWindow();
        sei.lpVerb = L"open";
        sei.lpFile = reinterpret_cast<LPCWSTR>(nhaApp.utf16());
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
            warnUser("Error Running",QMessageBox::Warning);
            return;
        }
    }
}

void MainWindow::camerasFound(QVector<QString> cameraNames)
{
    if(cameraNames.size()>0)
    {
        if(cameraNames[0].contains("*OPENCV*"))
            this->cameraSource = Camera::OPENCV_CAM;
        else
            this->cameraSource = Camera::AVTFIRE;
    }
    if(cameraNames.size() == 0)
     {
         warnUser(QString("Camera Not Connected, Image Acquisition is off"),QMessageBox::Warning);
         disableCameraRelatedOptions();
         ui.tabWidget->removeTab(0);
         ui.actionRecordVideo->setEnabled(false);
     }
     else
     {
         if(cameraSource==Camera::OPENCV_CAM)
         {
             qDebug()<<"OpenCV Camera found";
             ui.groupBox_21->setVisible(false);
         }
         else if(cameraSource==Camera::AVTFIRE)
         {
             qDebug()<<"AVT Camera found";
             cameraGrabber->addSettingsConnection(this,SIGNAL(shutterSpeedChanged(int)),SLOT(on_shutterSpeedChanged(int)),TO_CAMERA_SIGNAL);
         }
         startComDetection();
         ui.actionRecordVideo->setEnabled(true);
     }
    ui.availableCamerasCB->clear();
    for(int i=0;i<cameraNames.size();i++)
    {
        ui.availableCamerasCB->addItem(cameraNames[i]);
    }
}

void MainWindow::chartVersionChanged()
{
    if(ui.chartVersion4Radio->isChecked())
        ASG_PIT::settings().setSPAChartVersion(0.4);
    else
        ASG_PIT::settings().setSPAChartVersion(0.6);
    qDebug()<<"Chart Version changed to:"<<ASG_PIT::settings().getSPAChartVersion();
}

void MainWindow::changeFineFocusStatus()
{
    if(autoFocus)
    {
        autoFocus->setFineFocusStatus(ui.actionFine_Focus->isChecked());
    }
    this->setFocus();
}

void MainWindow::changeImagesSource(int source)
{
    switch (source)
    {
        case ACD:
            acdImagesSource->setChecked(true);
            break;
        case PIT_SMALL_TEMPS:
            acdImagesSourceSmall->setChecked(true);
            break;
        case THUNDERBIRD:
            thunderBirdImageSource->setChecked(true);
            break;
        default:
            qDebug() << "Unknow Image Source";
    }
    if(imageAnalysis)
        imageAnalysis->setImagesSource(source);
    ASG_PIT::settings().setImagesSource(source);
}

void MainWindow::changeImagesSource()
{
    if(analysingOffline || analysingOnline)
    {
        warnUser("You can not change the souce while analysing, stop analysis first!!!",QMessageBox::Warning);
        // return the previous source
        changeImagesSource(ASG_PIT::settings().getImagesSource());
    }
    else
    {
        if(acdImagesSource->isChecked())
        {
            imagesSource = ACD;
        }
        else if(thunderBirdImageSource->isChecked())
        {
            imagesSource = THUNDERBIRD;
        }
        else if(acdImagesSourceSmall->isChecked())
        {
            imagesSource = PIT_SMALL_TEMPS;
        }
        if(imageAnalysis)
            imageAnalysis->setImagesSource(imagesSource);
        ASG_PIT::settings().setImagesSource(imagesSource);
    }
    this->setFocus();
}

void MainWindow::changeFocusedTab(int)
{
//    qDebug("Ya I captured the tab change");
//    (ui.tabWidget->currentWidget())->setFocus();
}

void MainWindow::changeFocusStatus()
{
    if(autoFocus)
    {
        autoFocus->setFocusStatus(ui.actionAuto_Focus->isChecked());
    }
    this->setFocus();
}

void MainWindow::changeNozzelDisplay()
{
    if(showDeadNozzels->isChecked())
        imageAnalysis->setShowDeadNozzels(true);
    else
        imageAnalysis->setShowDeadNozzels(false);
    if(showClearNozzels->isChecked())
        imageAnalysis->setShowClearNozzels(true);
    else
        imageAnalysis->setShowClearNozzels(false);
    if(showNumbers->isChecked())
        imageAnalysis->setShowNumbers(true);
    else
        imageAnalysis->setShowNumbers(false);
    if(analysingOffline || analysingOnline)
    {
        imageAnalysis->redrawNozzels();
        sceneRendering->renderFrameThenRelease(imageAnalysis->getDisplayFrame());
    }
    this->setFocus();
}

void MainWindow::changeSPASettings()
{
    ASG_PIT::settings().setSPASaveDebugImage(ui.saveDebugImgChkBox->isChecked());
}

void MainWindow::clearTableSelection()
{
    QList<QTableWidgetItem*> selectedFiles = ui.imagesTableList->selectedItems();
    for(int i=(selectedFiles.size()-1);i>=0;i--)
        ui.imagesTableList->removeRow(selectedFiles[i]->row());
}

void MainWindow::clearEdgeEndTempList()
{
    copyOldTemplates(edgeEndPath);
    edgeEndTempModel->reloadPieces();
    imageAnalysis->setEdgeEndTemplateList(edgeEndTempModel->getTemplatesList());
}

void MainWindow::clearEdgeStartTempList()
{
    copyOldTemplates(edgeStartPath);
    edgeStartTempModel->reloadPieces();
    imageAnalysis->setEdgeStartTemplateList(edgeStartTempModel->getTemplatesList());
}

void MainWindow::clearNozzelTempList()
{
    copyOldTemplates(nozzelTempPath);
    nozzelTempModel->reloadPieces();
    imageAnalysis->setNozzelTemplateList(nozzelTempModel->getTemplatesList());
}

void MainWindow::closeDialog()
{
//    if(fileSelector)
//    {
//        fileSelector->close();
//        delete fileSelector;
//    }
}

void MainWindow::comDetectionStatus(bool motorsPortFound,bool ledLightingPortFound)
{
    QString warningMsg;
    warningMsg.append("Couldn't Find: ");
    if(!motorsPortFound)
    {
        warningMsg.append("<b>Motors</b> COM port ");
        setEnableMotorRelatedOptions(false);
    }
    if(!ledLightingPortFound)
    {
        if(!motorsPortFound)
            warningMsg.append("and <b>LedLighting</b> COM port.");
        else
            warningMsg.append("<b>LedLighting</b> COM port.");
    }
    warningMsg.append("Close any program using the COM ports and try again or go to (Tools->Settings) and try to manually select the com Ports ");
    if(!motorsPortFound || !ledLightingPortFound)
        warnUser(warningMsg,QMessageBox::Warning);
}

//TODO::check if this really works, it didn't create the archive dir in Linux
void MainWindow::copyOldTemplates(QString source)
{
    QDir dir(source);
    QFileInfoList listOfFiles;
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    listOfFiles = dir.entryInfoList();
    QString templateArchivePath(QString("%1/%2_archive").arg(source).arg(dir.dirName()));
    if(!dir.exists(templateArchivePath))
    {
        qDebug()<<"Created:"<<templateArchivePath;
        dir.mkdir(templateArchivePath);
    }
    for(int i=0;i<listOfFiles.size();i++)
    {
        QFile::copy(listOfFiles[i].fileName(),QString("%1/%2").arg(templateArchivePath).arg(listOfFiles[i].fileName()));
        qDebug()<<"File:"<<listOfFiles[i].fileName()<<" destination:"<<QString("%1/%2").arg(templateArchivePath).arg(listOfFiles[i].fileName());
        dir.remove(listOfFiles[i].fileName());
    }
}

void MainWindow::detectComPortsDialog()
{
    if(comPortDetection)
    {
        comPortDetection->done(1);
        delete comPortDetection;
    }
    comPortDetection = new ComPortDetection(this);
    comPortDetection->show();
    comPortDetection->raise();
    comPortDetection->activateWindow();
}

void MainWindow::detectTriangles()
{
}

void MainWindow::disableAllWidgetsBut(QList<QWidget *> toBeEnabled)
{
    QList<QWidget *> widgets = qFindChildren<QWidget *>(ui.toolBar);
    foreach (QWidget *w, widgets)
    {
            w->setEnabled(false);
    }
    QList<QTabWidget *> qTabWidgets = qFindChildren<QTabWidget *>(ui.centralwidget);
    foreach (QTabWidget *w, qTabWidgets)
    {
        QList<QWidget *> widgets = qFindChildren<QWidget *>(w);
        foreach (QWidget *cw, widgets)
        {
            cw->setEnabled(false);
        }
    }

    foreach (QWidget *w, toBeEnabled)
    {
        w->setEnabled(true);
    }
}

void MainWindow::disableCameraRelatedOptions()
{
    ui.actionAcquisition->setEnabled(false);
    ui.actionHome->setEnabled(false);
    ui.actionEnd->setEnabled(false);
    ui.actionOnline_Analysis->setEnabled(false);
    ui.actionJog_Left->setEnabled(false);
    ui.actionJog_Right->setEnabled(false);
    ui.actionLoading_Position->setEnabled(false);
    ui.actionTimed_Acquisition->setEnabled(false);
    ui.actionReset_Home->setEnabled(false);
    ui.actionRefocus->setEnabled(false);
    ui.actionLeft->setEnabled(false);
    ui.actionRight->setEnabled(false);
    ui.actionNext_Chip->setEnabled(false);
    ui.actionPrevious_Chip->setEnabled(false);
    ui.actionSave_Image->setEnabled(false);
}

void MainWindow::dnmapGenerationChanged(int state)
{
    this->generateDNMap = state;
    if(!state)
    {
        showNumbers->setChecked(false);
        showNumbers->setEnabled(false);
        imageAnalysis->setShowNumbers(false);
    }
    else
    {
        showNumbers->setEnabled(true);
    }
    if(generateDNMapChkBox->isChecked())
    {
        ASG_PIT::settings().setGenerateDNMap(true);
        if(imageAnalysis)
            imageAnalysis->setDNMapGeneration(true);
    }
    else
    {
        ASG_PIT::settings().setGenerateDNMap(false);
        if(imageAnalysis)
            imageAnalysis->setDNMapGeneration(false);
    }
}

void MainWindow::drawTemplateMask(const QPoint &l)
{
    if(addingTemplate)
    {
        if(templateType == NOZZEL)
        {
            tempSize = imageAnalysis->getNozzelTemplateSize();
        }
        else
        {
            tempSize = imageAnalysis->getEdgesTemplateSize();
        }
        qDebug()<<"TempSize W:"<<tempSize.x()<<" H:"<<tempSize.y();
        if(analysingOffline)
        {
            IplImage * tempImage = cvCloneImage(offlineImage);
            cvRectangle( tempImage,cvPoint( l.x(), l.y() ),cvPoint(l.x() + tempSize.x(), l.y() + tempSize.y() ),CV_RGB(0,255,0), 2, 0, 0 );
            sceneRendering->renderFrameThenRelease(tempImage);
        }
        else
        {
            IplImage * tempImage = cvCloneImage(displayFrame);
            cvRectangle( tempImage,cvPoint( l.x(), l.y() ),cvPoint(l.x() + tempSize.x(), l.y() + tempSize.y() ),CV_RGB(0,255,0), 2, 0, 0 );
            sceneRendering->renderFrameThenRelease(tempImage);
        }
     }
}

void MainWindow::edgeMatchingValueChanged(int value)
{
    edgeTempMatchingValue->setText(QString("%1").arg(value));
    imageAnalysis->setEdgeMatchingAccuracy(value/100.0);
    ASG_PIT::settings().setEdgeMatchingAccuracy(value);
}

void MainWindow::extractContours()
{
//    if(nhaWrapper)
//    {
//        changeSPASettings();
//        nhaWrapper->start();
//    }
}

void MainWindow::extractSPAImagesFromDirTree()
{
    QString rootDir = QFileDialog::getExistingDirectory(this, "Chose the root folder where search should start",
                                                        ASG_PIT::settings().getLastDirPath(),
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks
                                                        | QFileDialog::DontUseNativeDialog);
    bool ok;
    QString regExp = QInputDialog::getText(this, tr("Regular Expression Filtering"),
                                         tr("Regular Expression:"), QLineEdit::Normal,
                                         "*.tif", &ok);
    if (ok && !regExp.isEmpty() && !rootDir.isEmpty())
    {
        QRegExp rx(regExp);
        rx.setPatternSyntax(QRegExp::Wildcard);
        rx.setCaseSensitivity(Qt::CaseInsensitive);
        QStringList totalFileList = traverseDirTree(rootDir,rx);
        qDebug()<<"Total Number of Images Found:"<<totalFileList.size();
        addFiles2TableView(totalFileList);
    }
}

void MainWindow::finishedAddingTemplate()
{
    addNozzelTemplateBtn->setEnabled(true);
    addEdgeStartTemplateBtn->setEnabled(true);
    addEdgeEndTemplateBtn->setEnabled(true);
    sceneRendering->setMouseTracking(false);
    getFreshDisplayFrame();
    if(analysingOffline)
        sceneRendering->renderFrameThenRelease(cvCloneImage(offlineImage));
    else
        sceneRendering->renderFrameThenRelease(cvCloneImage(displayFrame));
    sceneRendering->unsetCursor();
}

void MainWindow::finishedProcessingImageAnalysis()
{
    qDebug()<<"Finished Processing This image";
    analysingImage = false;
    QList<QWidget *> widgets2 = qFindChildren<QWidget *>(ui.imageAnalysisTab);
    foreach (QWidget *w, widgets2)
        w->setEnabled(true);
    analysisDisplay->setEnabled(true);
    matchPercentageGrp->setEnabled(true);
    templatesTab->setEnabled(true);
    imagesSourceGrp->setEnabled(true);
    displayNozzelsGrp->setEnabled(true);
    if(!analysingOnline)
        analysisTypeGrp->setEnabled(true);
}

void MainWindow::finishedTraversingProgrammedSet()
{
    ui.traverseSteps->setText("Traverse");
    ui.programmedPos->setEnabled(true);
    ui.addProgPose->setEnabled(true);
    ui.saveProgrammedList->setEnabled(true);
    ui.fillCurrentPose->setEnabled(true);
    ui.clearProgrammedList->setEnabled(true);
    ui.delayBetweenSteps->setEnabled(true);
    ui.label_9->setEnabled(true);
    ui.label_10->setEnabled(true);
    ui.actionAuto_Focus->setChecked(true);
    if(autoFocus)
        autoFocus->setFocusStatus(true);
}

void MainWindow::getFreshDisplayFrame()
{
    if(!frame)
        return;
    if(!displayFrame)
    {
        displayFrame = cvCreateImage( cvGetSize(frame), 8, 3 );
    }
    if(frame->nChannels == 1)
    {
        cvCvtColor( frame, displayFrame, CV_GRAY2RGB );
    }
    else
    {
        cvCvtColor( frame, displayFrame, CV_BGR2RGB );
    }
}

void MainWindow::getOfflineImage(IplImage *_offlineImage)
{
    if(this->offlineImage)
    {
        qDebug()<<"Releasing offlineImage";
        cvReleaseImage(&offlineImage);
        qDebug()<<"OfflineImage Released";
    }
    offlineImage = cvCloneImage(_offlineImage);
    this->setFocus();
}

void MainWindow::generateBorealisTNFPattern()
{
    //system("python \"\"\"c:\\Documents and Settings\\ttaha\\Desktop\"\\stripe.py -c media_length=20 -c nozzle_position=10 -c colour_plane='0' -o test.bor");
    //system("\"c:\\Documents and Settings\\ttaha\\Desktop\"\\doit.bat");
    qDebug()<<"Generating Borealis Pattern";
//    QString program = "python";
//    QStringList arguments;
    //         arguments << "\"\"\"c:\\Documents and Settings\\ttaha\\Desktop\\stripe.py\"\"\" -c media_length=20 -c nozzle_position=10 -c colour_plane='0' -o test.bor";
    //         arguments << "c:\\stripe.py -c media_length=20 -c nozzle_position=10 -c colour_plane='0' -o test.bor";
//    arguments <<">> test";
//    QProcess myProcess(this) ;//= new QProcess(this);
//    myProcess->setWorkingDirectory("c:");
//    myProcess->start(program, arguments);
//    myProcess->start("dir >> test");
//    myProcess->setStandardOutputFile("c:\\test.txt");
//    myProcess.startDetached("python C:\\stripe.py -c media_length=20 -c nozzle_position=10 -c colour_plane=\'0\' -o test.bor");
//    qDebug()<<myProcess->readAllStandardOutput();
//    qDebug()<<myProcess->readAllStandardError();
//    myProcess->waitForFinished();
//    system("python c:\\stripe.py -c media_length=20 -c nozzle_position=10 -c colour_plane=\'0\' -o test.bor");
//    delete myProcess;
    QString currentDir  = QCoreApplication::applicationDirPath();
    QString darApp      = currentDir + "/" + "dar_tpp.exe";
    QString tiff2BorApp = currentDir + "/" + "tiff2bor.exe";
//    QString app = QString("python");
    SHELLEXECUTEINFOW sei;
    memset(&sei, 0, sizeof(sei));
    sei.fMask  = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT;
//    QString paramString = QString("c:\\stripe.py -c media_length=20 -c nozzle_position=10 -c colour_plane=\'0\' -o test.bor");
    QString paramString = QString(" /p \"C:\\Documents and Settings\\ttaha\\Desktop\\SBR ISG\\01-ASG_PIT\\bin\\newSpider007_a4.xml\" /i newSpider007_a4.tif");//.arg("newSpider007_a4");
    qDebug() << "Param string: " << darApp + paramString;
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
        warnUser("Error Running",QMessageBox::Warning);
        return;
    }
}

bool MainWindow::getPrintHeadCode()
{
     bool ok;
     if(timedAcqusition && alreadyGotHeadCode)
         return true;
     alreadyGotHeadCode = true;
     QString text = QInputDialog::getText(this, tr("PrintHead Code"),
                                          tr("Enter PrintHead Code, align Head wait for the autofocus to finish then press OK"), QLineEdit::Normal,
                                          ui.printHeadCode->text(), &ok);
     if (ok && !text.isEmpty())
     {
         if(imageAcquisition)
             imageAcquisition->setPrintHeadCode(text);
         if(imageAnalysis)
             imageAnalysis->setPrintHeadCode(text);
         if(encapDelamInspection)
             encapDelamInspection->setPrintHeadCode(text);
         ui.printHeadCode->setVisible(true);
         ui.printHeadName->setVisible(true);
         ui.printHeadCode->setText(text);
         return true;
     }
     return false;
}

void MainWindow::gotoEnd()
{
    if(motorControl)
    {
        linearMotorPose = 223100;
        motorControl->move(LINEAR_MOTOR,linearMotorPose);
        refocus();
    }
}

void MainWindow::gotoFocusPose()
{
    if(motorControl)
    {
        linearMotorPose = 1400;
        imageAcquisition->motorMovingChanged(LINEAR_MOTOR,true,linearMotorPose);
        encapDelamInspection->motorMovingChanged(LINEAR_MOTOR,true,linearMotorPose);
        autoFocus->motorMovingChanged(LINEAR_MOTOR,true,linearMotorPose);
        motorControl->move(LINEAR_MOTOR,linearMotorPose);
        refocus();
    }
}

void MainWindow::gotoHome()
{
    if(motorControl)
    {
        linearMotorPose = 0;
        motorControl->move(LINEAR_MOTOR,homePose);
        refocus();
    }
}

void MainWindow::goLeft()
{
    if(motorControl)
    {
        linearMotorPose-=qRound(ASG_PIT::settings().getImageStepSize() - ASG_PIT::settings().getImageStepSize()*ASG_PIT::settings().getOverLapPerc()/100.0f);
        //linearMotorPose-=1250;
        motorControl->move(LINEAR_MOTOR,linearMotorPose);
    }
}

void MainWindow::gotoLoadingPosition()
{
    if(motorControl)
    {
        linearMotorPose = 290000;
        motorControl->move(LINEAR_MOTOR,linearMotorPose);
    }
}

void MainWindow::gotoNextChip()
{
    if(motorControl)
    {
        if((currentChip++)>10)
            currentChip = 10;
        linearMotorPose=currentChip*ASG_PIT::settings().getChipStepSize();
        autoFocus->reInitializeFocusWindow();
        motorControl->move(LINEAR_MOTOR,linearMotorPose);
    }
}

void MainWindow::gotoPrevChip()
{
    if(motorControl)
    {
        if((currentChip--)<0)
            currentChip = 0;
        linearMotorPose=currentChip*ASG_PIT::settings().getChipStepSize();
        autoFocus->reInitializeFocusWindow();
        motorControl->move(LINEAR_MOTOR,linearMotorPose);
    }
}

void MainWindow::goRight()
{
    if(motorControl)
    {
        linearMotorPose+=qRound(ASG_PIT::settings().getImageStepSize() - ASG_PIT::settings().getImageStepSize()*ASG_PIT::settings().getOverLapPerc()/100.0f);
        //linearMotorPose+=1250;//qRound(ASG_PIT::settings().getImageStepSize() - ASG_PIT::settings().getImageStepSize()*ASG_PIT::settings().getOverLapPerc()/100.0f);
        motorControl->move(LINEAR_MOTOR,linearMotorPose);
    }
}

void MainWindow::helpManual()
{
    #ifdef PIT_ONLY
        QWebView *view = new QWebView();
        QString url = ASG_PIT::settings().getCurrentWorkingDirectory().append("/manual/index.html");
        view->load(QUrl(url));
        view->show();
    #else
        QWebView *view = new QWebView();
        QString url = ASG_PIT::settings().getCurrentWorkingDirectory().append("/manual/index.html");
        view->load(QUrl(url));
        view->show();
        view->setWindowTitle("NHA Manual");
    #endif
}

void MainWindow::hideProgress()
{
    ui.progressBar->setVisible(false);
    ui.acqProgress->setVisible(false);
}

void MainWindow::initiateLedLighting(QString _port)
{
    qDebug()<<"Initiating LedLighting";
    if(ledLightingController)
        delete ledLightingController;
    ledLightingController = new LedLightingController;
    ledLightingController->initializePort((char *)qPrintable(_port));
    ledLightingController->setLightingCurrent(ASG_PIT::settings().getLedLightingDefaultCurrent());
    connect(settingsDialog, SIGNAL(ledLightingCurrentValueChanged()), SLOT(ledLightingCurrentValueChanged()));
    ui.ledLightIntensitySlider->setValue(ASG_PIT::settings().getLedLightingDefaultCurrent());
}

void MainWindow::initiateMotors(QString _port)
{
    qDebug()<<"Initiating Motors";
    if(motorControl)
    {
        while(motorControl->isRunning())
        {
            motorControl->stop();
            // wait for a max of 100ms for the thread to finish
            motorControl->wait(100);
        }
        motorControl->setPort(_port);
        if(motorControl->initialize())
        {
            motorControl->start(QThread::HighestPriority);
        }
    }
    else
    {
        motorControl = new MotorControl(_port);
    }
    if(motorControl)
    {
        connect(motorControl,SIGNAL(motorsInitialized(bool)),this,SLOT(motorsInitialized(bool)));
        motorControl->start(QThread::HighestPriority);
    }
}

void MainWindow::isFocused(bool _focused)
{
    this->focused = _focused;
    if(!_focused)
    {
        ui.actionJog_Left->setEnabled(false);
        ui.actionJog_Right->setEnabled(false);
        ui.traverseSteps->setEnabled(false);
        ui.groupBox_17->setEnabled(false);
        ui.groupBox_18->setEnabled(false);
        ui.groupBox_19->setEnabled(false);
        return;
    }
    ui.actionJog_Left->setEnabled(true);
    ui.actionJog_Right->setEnabled(true);
    ui.traverseSteps->setEnabled(true);
    ui.groupBox_17->setEnabled(true);
    ui.groupBox_18->setEnabled(true);
    ui.groupBox_19->setEnabled(true);
    {
        // Image Acquisition image supply part
        /*
            At this stage the autoFocus is not quering images
            from the camera, so we can take an image without having to worry about blocking
        */
        usleep(100000);
        if(imageAcquisition && frame)
            imageAcquisition->recieveImage(frame);
        if(encapDelamInspection && frame)
            encapDelamInspection->recieveImage(frame);
    }
    getFreshDisplayFrame();
    if(sceneRendering && frame)
        sceneRendering->renderFrame(frame);
    this->statusBar()->showMessage(QString("Linear Pose:%1  Camear Pose:%2 Current Chip:%3 Image Focused").arg(linearMotorPose).arg(cameraMotorPose).arg(currentChip));
    if(encapsulationCoverage && calibratingIntensity)
    {
        if(frame)
        {
            try
            {
                calibtationStep++;
                ui.calibStepLabel->setText(QString("Calibration step %1/%2").arg(calibtationStep).arg(totalCalibrationSteps));
                encapsulationCoverage->processImage(cvCloneImage(frame));
                if(calibtationStep>=totalCalibrationSteps)
                {
                    calibratingIntensity = false;
                    ui.calibrateLightIntensity->setChecked(false);
                    on_calibrateLightIntensity_released();
                }
                else
                {
                    if(ASG_PIT::settings().getAutoSkip2Next())
                        gotoNextChip();
                }
            }
            catch(EncapException &encap)
            {
                warnUser(QString("%1. Adjust the light intensity and re-analyse.").arg(encap.what()),QMessageBox::Warning);
                ui.reAnalyse->setEnabled(true);
            }
        }
    }
    else if(!analysingOffline && analysingOnline)
    {
        if(imageAnalysis && displayFrame)
        {
            imageAnalysis->setOnlineImage(displayFrame,linearMotorPose,cameraMotorPose);
            imageAnalysis->setMode(ImageAnalysis::ONLINE);
            imageAnalysis->setPaused(false);
            if(!imageAnalysis->isRunning())
                imageAnalysis->start();
        }
        else
        {
            warnUser("ImageAnalysis is not initialized or displayFrame is empty, this is a bug, report Developers",QMessageBox::Critical);
        }
        sceneRendering->renderFrameThenRelease(cvCloneImage(displayFrame));
    }
}

bool MainWindow::isTemplateSouceAvailable()
{
    if(analysingOnline && !displayFrame)
    {
        warnUser("No display Image to take a template from!!!",QMessageBox::Warning);
        return false;
    }
    else if(analysingOffline && !offlineImage)
    {
        warnUser("No Image loaded to take a template from!!!",QMessageBox::Warning);
        return false;
    }
    else if(!analysingOffline && !analysingOnline)
    {
        // we are not analysing anything
        warnUser("Start Analysis First before selecting a template",QMessageBox::Warning);
        return false;
    }
    return true;
}

void MainWindow::keyPressEvent( QKeyEvent *e )
{
    if(encapDelamInspection->isInspecting())
    {
        encapDelamInspection->keyPressed(e);
        return;
    }
    if(keysDisabled)
        return;
    switch( e->key() )
    {
    case Qt::Key_A:
        goLeft();
        break;
    case Qt::Key_B:
        if(!analysingImage)
        {
            showDeadNozzels->setChecked(!showDeadNozzels->isChecked());
            changeNozzelDisplay();
        }
        break;
    case Qt::Key_C:
        if(!analysingImage)
        {
            showClearNozzels->setChecked(!showClearNozzels->isChecked());
            changeNozzelDisplay();
        }
        break;
    case Qt::Key_D:
        goRight();
        break;
    case Qt::Key_K:
        detectTriangles();
        break;
    case Qt::Key_N:
        if(showNumbers->isEnabled() && ! analysingImage)
        {
            showNumbers->setChecked(!showNumbers->isChecked());
            changeNozzelDisplay();
        }
        break;
    case Qt::Key_R:
        refocus();
        break;
    case Qt::Key_W:
        if(motorControl)
        {
            cameraMotorPose+=50;
            motorControl->move(CAMERA_MOTOR,cameraMotorPose);
            if(autoFocus)
                autoFocus->shiftFocusWindow(50);
        }
        break;
    case Qt::Key_S:
        if(motorControl)
        {
            cameraMotorPose-=50;
            motorControl->move(CAMERA_MOTOR,cameraMotorPose);
            if(autoFocus)
                autoFocus->shiftFocusWindow(-50);
        }
        break;
    case Qt::Key_Home:
        gotoHome();
        break;
    case Qt::Key_End:
        gotoEnd();
        break;
    case Qt::Key_Space:
            if(analysingOnline)
            {
                linearMotorPose+=1600;
                // After the motor moves it will trigger a autofocuse then isFocused event
                // which will in turn send an image to the image analysis
                // and unpause the inner loop
                motorControl->move(LINEAR_MOTOR,linearMotorPose);
            }
            else if (analysingOffline)
            {
                emit setAnalysisPause(false);
            }
            break;
    case Qt::Key_Up:
        gotoNextChip();
        break;
    case Qt::Key_Down:
        gotoPrevChip();
        break;
    case Qt::Key_Escape:
        if(addingTemplate)
        {
            finishedAddingTemplate();
            addingTemplate = false;
        }
        break;
    case Qt::Key_X:
        if(!analysingImage)
        {
            triggerAnalysis();
        }
        break;
    }
    this->setFocus();
}

void MainWindow::ledLightingCurrentValueChanged()
{
    if(ledLightingController)
        ledLightingController->setLightingCurrent(ASG_PIT::settings().getLedLightingDefaultCurrent());
}

void MainWindow::loadProgrammedList()
{
    QStringList labels;
    labels << tr("Name") << tr("Position");
    ui.programmedPositionView->setHorizontalHeaderLabels(labels);
    ui.programmedPositionView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.programmedPositionView->verticalHeader()->hide();

    QString fileName = QString("%1/programmedPositions.csv").arg(QApplication::applicationDirPath());
    QFile programmedSetFile;
    QTextStream inStream;
    programmedSetFile.setFileName(fileName);
    if (!programmedSetFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    inStream.setDevice(&programmedSetFile);
    while (!inStream.atEnd())
    {
        QString position = inStream.readLine();
        int rowNum = ui.programmedPositionView->rowCount();
        QTableWidgetItem *positionNameItem = new QTableWidgetItem(QString("Position %1").arg(rowNum));
        positionNameItem->setFlags(positionNameItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *positionItem = new QTableWidgetItem(tr("%1").arg(position.toInt()));
        positionItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        positionItem->setFlags(positionItem->flags() | Qt::ItemIsEditable);

        ui.programmedPositionView->insertRow(rowNum);
        ui.programmedPositionView->setItem(rowNum, 0, positionNameItem);
        ui.programmedPositionView->setItem(rowNum, 1, positionItem);
    }
    programmedSetFile.close();
    qDebug() << "Loaded Programmed List file:"<<fileName;
}

void MainWindow::locationChanged(int _currentImagePosition,int chip, int chipSection)
{
    ui.currentChipLCD->display(chip);
    ui.currentSectionLDC->display(chipSection);
    currentDie = chip;
    currentDieSection = chipSection;
    currentImagePosition = _currentImagePosition;
    /*
    if(currentImagePosition>=0 && currentImagePosition <ui.delamTable->rowCount() && currentImagePosition!=-1)
    {
        ui.delamTable->selectRow(currentImagePosition);
        ui.delamTable->item(currentImagePosition,2)->setText(QString("%1").arg(chip));
        ui.delamTable->item(currentImagePosition,3)->setText(QString("%1").arg(chipSection));
    }
    */
}

void MainWindow::matchingValueChanged(int value)
{
    matchValue->setText(QString("%1").arg(value));
    imageAnalysis->setMatchingAccuracy(value/100.0);
    ASG_PIT::settings().setMatchingAccuracy(value);
    secondMatchingSlider->setMinimum(value);
}

void MainWindow::mesBottonPressed()
{
    qDebug()<<"Push Data to MES pressed";
    if(imageAnalysis)
    {
        imageAnalysis->pushData2MesInbox();
        mesPushDataBtn->setEnabled(false);
    }
}

void MainWindow::motorsInitialized(bool motorsConnected)
{
    if(motorsConnected && !threadsInitialized)
    {
        qDebug()<<"Initialized Correctly";
        // Image Acquisition Thread
        imageAcquisition = new ImageAcquisition(this,motorControl,encapsulationCoverage);
        encapDelamInspection = new EncapDelamInspection(this,motorControl);
        // AutoFocus Thread
        autoFocus = new AutoFocus(motorControl);
        connect(imageAcquisition,SIGNAL(progressValue(int)),ui.progressBar,SLOT(setValue(int)));
        connect(encapDelamInspection,SIGNAL(progressValue(int)),ui.progressBar,SLOT(setValue(int)));
        connect(autoFocus,SIGNAL(isFocused(bool)),SLOT(isFocused(bool)));
        cameraGrabber->addFrameReceiver(autoFocus);
        cameraGrabber->addSettingsConnection(imageAcquisition,SIGNAL(shutterSpeedChanged(int)),SLOT(on_shutterSpeedChanged(int)),TO_CAMERA_SIGNAL);
        connect(cameraGrabber,SIGNAL(warnUser(QString,int)),this,SLOT(warnUser(QString,int)));
        connect(imageAcquisition,SIGNAL(acquisitionEnded(bool)),SLOT(acquisitionEnded(bool)));
        connect(imageAcquisition,SIGNAL(savingImages()),SLOT(savingImages()));
        connect(imageAcquisition,SIGNAL(lastImageReached()),autoFocus,SLOT(lastImageReached()));
        connect(imageAcquisition,SIGNAL(setShutterSpeed(int)),cameraGrabber,SLOT(setShutterSpeed(int)));

        connect(encapDelamInspection,SIGNAL(acquisitionEnded(bool)),SLOT(acquisitionEnded(bool)));
        connect(encapDelamInspection,SIGNAL(savingImages()),SLOT(savingImages()));
        connect(encapDelamInspection,SIGNAL(lastImageReached()),autoFocus,SLOT(lastImageReached()));
        connect(encapDelamInspection,SIGNAL(newImagePositions(QVector<int>)),this,SLOT(newImagePositions(QVector<int>)));
        connect(encapDelamInspection,SIGNAL(locationChanged(int,int,int)),this,SLOT(locationChanged(int,int,int)));

        connect(motorControl,SIGNAL(updateMotorPose(int,int)),SLOT(updateMotorPose(int,int)));
        connect(motorControl,SIGNAL(motorMoving(int,bool,int)),SLOT(motorMovingChanged(int,bool,int)));
        connect(motorControl,SIGNAL(motorMoving(int,bool,int)),autoFocus,SLOT(motorMovingChanged(int,bool,int)),Qt::DirectConnection);
        connect(motorControl,SIGNAL(motorMoving(int,bool,int)),imageAcquisition,SLOT(motorMovingChanged(int,bool,int)));
        connect(motorControl,SIGNAL(motorMoving(int,bool,int)),encapDelamInspection,SLOT(motorMovingChanged(int,bool,int)));
        connect(motorControl,SIGNAL(warnUser(QString,int)),SLOT(warnUser(QString,int)));
        connect(motorControl,SIGNAL(finishedTraversingProgrammedSet()),this,SLOT(finishedTraversingProgrammedSet()));
        // Make it scriptable
        scriptEngine.globalObject().setProperty("Acquisition", scriptEngine.newQObject(imageAcquisition));
        autoFocus->start(QThread::HighestPriority);
        imageAcquisition->start(QThread::HighestPriority);
        encapDelamInspection->start(QThread::HighestPriority);
        threadsInitialized = true;
    }
}

void MainWindow::motorMovingChanged(int motorId,bool state,int)
{
    QMutexLocker lock(&mutex);
    motorMoving = state;
}

void MainWindow::newImagePositions(QVector<int> _imagePositions)
{
    QStringList labels;
    labels<<tr("Die")<<tr("Section")<<tr("Ch")<<tr("Row")<<tr("RD")<<tr("Img");
    ui.delamTable->setHorizontalHeaderLabels(labels);
    ui.delamTable->setEnabled(true);
    imagePositions = _imagePositions;
    int sectionsPerDie = ASG_PIT::settings().getInspectionSectionsPerDie();
    /*
    for(int i=0;i<imagePositions.size();i++)
    {
        QTableWidgetItem *phPosition = new QTableWidgetItem(QString("%1").arg(i));
        phPosition->setFlags(phPosition->flags() ^ Qt::ItemIsEditable);
        phPosition->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QComboBox* combo = new QComboBox();
        combo->addItem("No-Delamination");
        combo->addItem("Delamination");
        combo->setCurrentIndex(0);

        int chip = (int)floor(imagePositions[i]/float(ASG_PIT::settings().getChipStepSize()));
        int chipSection = (int)floor(imagePositions[i] - ASG_PIT::settings().getChipStepSize()*chip)/float(ASG_PIT::settings().getChipStepSize()/float(sectionsPerDie));

        QTableWidgetItem *dieNumber = new QTableWidgetItem(QString("%1").arg(chip));
        dieNumber->setFlags(dieNumber->flags() ^ Qt::ItemIsEditable);
        dieNumber->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QTableWidgetItem *dieSectionNumber = new QTableWidgetItem(QString("%1").arg(chipSection));
        dieSectionNumber->setFlags(dieSectionNumber->flags() ^ Qt::ItemIsEditable);
        dieSectionNumber->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);


        ui.delamTable->insertRow(i);
        ui.delamTable->setItem(i, 0, phPosition);
        ui.delamTable->setCellWidget(i,1,combo);
        ui.delamTable->setItem(i, 2, dieNumber);
        ui.delamTable->setItem(i, 3, dieSectionNumber);
    }
    */
}

void MainWindow::nozzelSelected(const QPoint &l,int button)
{
    if(addingTemplate)
    {
        finishedAddingTemplate();
        //cancel the addition
        if(button == Qt::RightButton)
        {
            addingTemplate = false;
            return;
        }
        addingTemplate = false;
        IplImage * selectedTemp,*test;
        if(analysingOffline)
        {
            test=cvCloneImage(offlineImage);
        }
        else
        {
            getFreshDisplayFrame();
            test=cvCloneImage(displayFrame);
        }
        selectedTemp = cvCreateImage( cvSize(tempSize.x(),tempSize.y()),8,test->nChannels);
        if(l.x()<0 ||l.y()<0 || (l.x()+tempSize.x()) > (test->width-1) || (l.y()+tempSize.y()) > (test->height-1) )
        {
            warnUser(QString("The location you clicked on crosses the image boundaries, try again."),QMessageBox::Warning);
        }
        else
        {
            cvSetImageROI( test, cvRect( l.x(), l.y(),tempSize.x(),tempSize.y()));
            cvCopy(test,selectedTemp);
            QString relativePath;
            QDateTime dateTime;
            QDir tempDir;
            switch(templateType)
            {
                case NOZZEL:
                        relativePath = nozzelTempPath;
                        if (!tempDir.exists(relativePath))
                            tempDir.mkdir(relativePath);
                        cvSaveImage(qPrintable(QString("%1/nozTemp%2.png").arg(nozzelTempPath).arg(dateTime.currentDateTime().toString(QString("yyyyMMdd_hhmms")))),selectedTemp);
                        nozzelTempModel->reloadPieces();
                        imageAnalysis->setNozzelTemplateList(nozzelTempModel->getTemplatesList());
                    break;
                case EDGE_START:
                        relativePath = edgeStartPath;
                        if (!tempDir.exists(relativePath))
                            tempDir.mkdir(relativePath);
                        cvSaveImage(qPrintable(QString("%1/edgeStartTemp%2.png").arg(edgeStartPath).arg(dateTime.currentDateTime().toString(QString("yyyyMMdd_hhmms")))),selectedTemp);
                        edgeStartTempModel->reloadPieces();
                        imageAnalysis->setEdgeStartTemplateList(edgeStartTempModel->getTemplatesList());
                    break;
                case EDGE_END:
                        relativePath = edgeEndPath;
                        if (!tempDir.exists(relativePath))
                            tempDir.mkdir(relativePath);
                        cvSaveImage(qPrintable(QString("%1/edgeEndTemp%2.png").arg(edgeEndPath).arg(dateTime.currentDateTime().toString(QString("yyyyMMdd_hhmms")))),selectedTemp);
                        edgeEndTempModel->reloadPieces();
                        imageAnalysis->setEdgeEndTemplateList(edgeEndTempModel->getTemplatesList());
                    break;
            }
        }
        cvReleaseImage(&test);
        cvReleaseImage(&selectedTemp);
    }
    else if(analysingOffline)
    {
        emit propagateNozzelSelected(l,button);
    }
    else if(analysingOnline)
    {
        emit propagateNozzelSelected(l,button);
        sceneRendering->renderFrameThenRelease(cvCloneImage(displayFrame));
    }
}

void MainWindow::onlineAnalysisChanged()
{
    analysingOnline = ui.actionOnline_Analysis->isChecked();
    this->setFocus();
    if(analysingOffline)
    {
        warnUser(QString("Offline Analysis is currently on, Stop it first before switching to online analysis!!!"),QMessageBox::Warning);
        ui.actionOnline_Analysis->setChecked(false);
        return;
    }
    if(analysingOnline)
    {
        if(getPrintHeadCode())
        {
            ui.printHeadCode->setEnabled(false);
            while(imageAnalysis->isRunning())
            {
                imageAnalysis->stop();
                usleep(100000);
            }
            if(imageAnalysis)
                imageAnalysis->startNewAnalysis(ImageAnalysis::ONLINE);

            if(focused)
                isFocused(true);
            manualAnalysis->setChecked(true);
            analysisTypeGrp->setEnabled(false);
        }
        else
        {
            ui.actionOnline_Analysis->setChecked(false);
            analysisTypeGrp->setEnabled(true);
            analysingOnline = false;
            warnUser("Online Analysis Aborted",QMessageBox::Information);
        }
    }
    else
        analysisTypeGrp->setEnabled(true);
}

void MainWindow::populateSPAGui()
{
    ui.saveDebugImgChkBox->setChecked(ASG_PIT::settings().getSPASaveDebugImage());
    settingsDialog->ui_spa.toleranceToFiducialSize->setValue(ASG_PIT::settings().getSPATolerance2FiducialSize());
    settingsDialog->ui_spa.toleranceToFiducialSquares->setValue(ASG_PIT::settings().getSPATolerance2SquareShape());
    settingsDialog->ui_spa.toleranceToLinePatternSize->setValue(ASG_PIT::settings().getSPATolerance2LinePatternSize());
    settingsDialog->ui_spa.toleranceToOffset->setValue(ASG_PIT::settings().getSPATolerance2Offset());
    int x = ui.plumpingCombo->findText(ASG_PIT::settings().getSPAPlumpingOder());
    ui.plumpingCombo->setCurrentIndex(x);
}

void MainWindow::processEncapsulationImages()
{
    qDebug()<<"Sending processing click";
    if(ui.processEncapsulationImages->text()==QString("Process Images"))
    {
        if(ui.encapsulationImagesSource->text()==QString(""))
        {
            warnUser("Select image source path first",QMessageBox::Warning);
            return;
        }
        if(encapsulationCoverage)
        {
            while(encapsulationCoverage->isRunning())
            {
                encapsulationCoverage->stop();
                // wait for a max of 200ms for the thread to finish
                encapsulationCoverage->wait(200);
            }
            ui.processEncapsulationImages->setText("Stop Processing");
            ui.toolBar->setEnabled(false);
            ui.tabWidget->widget(1)->setEnabled(false);
            ui.groupBox_14->setEnabled(false);
            ui.groupBox_16->setEnabled(false);
            ui.groupBox_20->setEnabled(false);
            encapsulationCoverage->setImageSrcPath(ASG_PIT::settings().getEncapsulationImgSrc());
            encapsulationCoverage->start();
        }
    }
    else
    {
        ui.processEncapsulationImages->setText("Process Images");
        ui.toolBar->setEnabled(true);
        ui.tabWidget->widget(1)->setEnabled(true);
        ui.groupBox_14->setEnabled(true);
        ui.groupBox_16->setEnabled(true);
        ui.groupBox_20->setEnabled(true);
        encapsulationCoverage->stop();
    }
}

void MainWindow::printHeadCodeChanged(const QString& text)
{
    if(imageAcquisition)
        imageAcquisition->setPrintHeadCode(text);
    if(encapDelamInspection)
        encapDelamInspection->setPrintHeadCode(text);
}

void MainWindow::processingImage(QString msg)
{
    ui.processingImageLabel->setVisible(true);
    ui.processingImageLabel->setText(msg);
}

void MainWindow::processingImage(QString msg,int progress)
{
    ui.processingImageLabel->setVisible(true);
    ui.processingImageLabel->setText(msg);
    if(progress==-1)
    {
        hideProgress();
        ui.processingImageLabel->setVisible(false);
        ui.processEncapsulationImages->setText("Process Images");
        return;
    }
    ui.acqProgress->setText("Processing Encapsulation Images:");
    ui.progressBar->setValue(progress);
    ui.progressBar->setVisible(true);
    ui.acqProgress->setVisible(true);
}

void MainWindow::processingImageAnalysis()
{
    analysingImage = true;
    analysisDisplay->setEnabled(false);
    matchPercentageGrp->setEnabled(false);
    templatesTab->setEnabled(false);
    imagesSourceGrp->setEnabled(false);
    displayNozzelsGrp->setEnabled(false);
    if(!analysingOnline)
        analysisTypeGrp->setEnabled(true);
}

void MainWindow::recieveFrame(IplImage *newFrame)
{
    this->frame = newFrame;
    if(frame && !focusWindowSet)
    {
        settingsDialog->ui_focus.focusWindowH->setMaximum(frame->height-200);
        settingsDialog->ui_focus.focusWindowW->setMaximum(frame->width-200);
        settingsDialog->ui_focus.focusWindowX->setMaximum(frame->width-200);
        settingsDialog->ui_focus.focusWindowY->setMaximum(frame->height-200);
        focusWindowSet = true;
    }
}

void MainWindow::refocus()
{
    if(motorControl)
    {
        if(autoFocus)
        {
            autoFocus->reInitializeFocusWindow();
            autoFocus->startNewFocus();
        }
    }
}

void MainWindow::runScript()
{

    QString fileName = QFileDialog::getOpenFileName(this, tr("Chose script"),
                                                    ASG_PIT::settings().getLastDirPath(),
                                                    tr("Scripts (*.js)"));
    if(fileName == QString(""))
        return;
    QFile scriptFile(fileName);
    scriptFile.open(QIODevice::ReadOnly);
    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();

    QScriptValue result = scriptEngine.evaluate(contents, fileName);
    if (scriptEngine.hasUncaughtException())
    {
        int lineNo = scriptEngine.uncaughtExceptionLineNumber();
        qWarning() << "line" << lineNo << ":" << result.toString();
        QMessageBox::critical(0, "Script",
                              QString::fromLatin1("%0:%1: %2")
                              .arg(fileName)
                              .arg(result.property("lineNumber").toInt32())
                              .arg(result.toString()));
        return;
    }
    qDebug()<<"script ended";
    return;
}

void MainWindow::runTestScript()
{
    if(!testLoopStarted)
    {
        testLoopStarted  = true;
        triggerAcquisition();
    }
}

void MainWindow::runScriptEngine()
{
    /*
    qDebug()<<"script started";
    engine.globalObject().setProperty("ACD", engine.newQObject(this));
    engine.globalObject().setProperty("Acquisition", engine.newQObject(imageAcquisition));
    engine.globalObject().setProperty("ImageAnalysis", engine.newQObject(imageAnalysis));
    QString fileName;
    fileName = QString("%1/pdgTestScript.js").arg(QApplication::applicationDirPath());
    QFile scriptFile(fileName);
    scriptFile.open(QIODevice::ReadOnly);
    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();

    QScriptValue result = engine.evaluate(contents, fileName);
    if (engine.hasUncaughtException())
    {
        int lineNo = engine.uncaughtExceptionLineNumber();
        qWarning() << "line" << lineNo << ":" << result.toString();
        QMessageBox::critical(0, "Pdg Test Script",
                              QString::fromLatin1("%0:%1: %2")
                              .arg(fileName)
                              .arg(result.property("lineNumber").toInt32())
                              .arg(result.toString()));
        return;
    }
    qDebug()<<"script ended";
    return;
    */
}

void MainWindow::saveCurrentImage()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image Into"),
                                                    ASG_PIT::settings().getLastDirPath(),
                                                    tr("Images (*.png *.bmp *.jpg)"));
    if(this->frame && !fileName.isEmpty())
        cvSaveImage(fileName.toAscii(),frame);
}

void MainWindow::savingImages()
{
    ui.acqProgress->setText("Saving Images:");
}

void MainWindow::scanPattern()
{
    /*
    ScannerInterface scanner;
    QString scannedImageName;
    QStringList items;
    items << tr("Manual Single Page") << tr("ADF (Automatic Feeder)");
    bool autoFeeder= false;
    bool ok;
    QString item = QInputDialog::getItem(this, tr("Select paper source"),
                                         tr("Source:"), items, 0, false, &ok);
    if (ok && item == QString("ADF (Automatic Feeder)"))
    {
        autoFeeder = true;
    }
    else if (ok && item == QString("Manual Single Page"))
    {
        autoFeeder = false;
    }
    else
        return;
    warnUser("Automated Scanning and Cropping are still experimental and have been tested on a limited set of Scanners !!!",QMessageBox::Information);
    QString dir = QFileDialog::getExistingDirectory(this, tr("Destination Directory"),
                                                        ASG_PIT::settings().getLastDirPath(),
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks
                                                        | QFileDialog::DontUseNativeDialog);
    if(!dir.isEmpty())
        ASG_PIT::settings().setLastDirPath(dir);
    else
        return;
    int i=0;
    if(autoFeeder)
    {
        if(!scanner.getHasADF())
        {
            warnUser("Scanner Does not have an ADF.",QMessageBox::Warning);
            return;
        }
        bool processedPages = false;
        while(scanner.getHasPaperLoaded())
        {
            qDebug()<<"Paper found in ADF Tray";
            processedPages = true;
            if(scanner.getNextPageFromADF())
            {
                qDebug()<<"Paper ("<<i+1<<") loaded and is now being Scanned";
                scannedImageName = QString("%1\\scannerTemp.tif").arg(QApplication::applicationDirPath());
                if(scanner.getScanFromFlatBed(scannedImageName,ASG_PIT::settings().getSPALowResScanning(),ScannerInterface::RGB))
                {
                    QRectF roi = getRegionOfInterest(scannedImageName);
                    qDebug()<<"Scanning region of interest in High resolution";
                    QStringList scanneImage;
                    scanneImage <<QString("%1\\imag_cropped%2.tif").arg(dir).arg(i);
                    if(scanner.getRegionFromFlatBed(scanneImage[0],ASG_PIT::settings().getSPAHighResScanning(),roi))
                        addFiles2TableView(scanneImage);
                }
                else
                {
                    qDebug()<<"Error while trying to scan Pattern.";
                    warnUser("Error while trying to scan Pattern.",QMessageBox::Warning);
                    break;
                }
                i++;
            }
        }
        if(processedPages)
        {
            scanner.clearPageFromADF();
            warnUser("Finished Scanning all Pages in the ADF.",QMessageBox::Information);
        }
    }
    else
    {
        scannedImageName = QString("%1/scannerTemp.png").arg(QApplication::applicationDirPath());
        scanner.getScanFromFlatBed(scannedImageName,ASG_PIT::settings().getSPALowResScanning(),ScannerInterface::RGB);
        QRectF roi = getRegionOfInterest(scannedImageName);
        QStringList scanneImage;
        scanneImage <<QString("%1/%2.tif").arg(dir).arg(ASG_PIT::settings().getSPAScannedPrefix());
        if(scanner.getRegionFromFlatBed(scanneImage[0],ASG_PIT::settings().getSPAHighResScanning(),roi))
        {
            addFiles2TableView(scanneImage);
            warnUser("Finished Cropping the image.",QMessageBox::Information);
        }
    }
    */
}

void MainWindow::secondMatchLevelToggled(bool state)
{
    imageAnalysis->setSecondFilter(state);
    ASG_PIT::settings().setEnableSecondMatchLevel(state);
}

void MainWindow::secondMatchingSliderValueChanged(int value)
{
    secondMatchingValue->setText(QString("%1").arg(value));
    imageAnalysis->setSecondMatchingAccuracy(value/100.0);
    ASG_PIT::settings().setSecondMatchingAccuracy(value);
}

QRectF MainWindow::getRegionOfInterest(QString scannedImageName)
{
    qDebug()<<"Cropping Pattern from paper";
    QFileInfo fileInfo(scannedImageName);
    QString baseFileName = fileInfo.baseName();
    QString baseDir = fileInfo.absolutePath();
    double minValue, maxValue;
    CvPoint minLoc, maxLoc;

    IplImage *srcImage   = cvLoadImage(qPrintable(scannedImageName.replace("/","\\")),CV_LOAD_IMAGE_GRAYSCALE);
    IplImage *rgbDisplay = cvCreateImage( cvGetSize(srcImage), 8, 3 );
    cvCvtColor( srcImage, rgbDisplay, CV_GRAY2RGB );

    cvMinMaxLoc(srcImage, &minValue, &maxValue, &minLoc, &maxLoc );
    cvThreshold(srcImage,srcImage , maxValue*0.85 ,255, CV_THRESH_BINARY_INV);
    QString saveTo = QString("%1/filtered.png").arg(baseDir).replace("/","\\");
    cvSaveImage(saveTo.toAscii(),srcImage);
    CvMemStorage *storage = cvCreateMemStorage(0);
    CvSeq* contours = 0, *contTemp=0;
    cvFindContours( srcImage, storage, &contours, sizeof(CvContour),CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cvPoint(0,0) );

    /*!
        refer to : http://serdis.dis.ulpgc.es/~itis-fia/FIA/doc/Moments/OpenCv/OpenCV_Moments.pdf
        for more information.
     */
//    cvDrawContours( rgbDisplay, contours, CV_RGB(0,0,255), CV_RGB(0,0,255), 1, 1, 8 );
    double m10,m01,m00;
    contTemp = contours;
    CvMoments moment;
    CvBox2D boundingBox;
    QVector <CvBox2D> alignmentFidSet;
    double eccentricity;
    //int xc,yc;
    float w,h;
    float minDim = 10, maxDim = 16;
    QPointF topLeftFid,buttomRightFid,topRightFid,buttomLeftFid;
    for(; contTemp!=0; contTemp = contTemp->h_next)
    {
        if(contTemp->total <6 )
            continue;
        boundingBox = cvFitEllipse2(contTemp);
        w = boundingBox.size.width;
        h = boundingBox.size.height;
        eccentricity = ellipseEccentricity(qRound(w),qRound(h));
        //roundness(perimeter,area);
        if(!inRange(eccentricity,0.0,0.5) || !inRange(w,minDim,maxDim) || !inRange(h,minDim,maxDim))
            continue;
        cvContourMoments(contTemp,&moment);
        m00 = cvGetSpatialMoment(&moment,0,0);
        m01 = cvGetSpatialMoment(&moment,0,1);
        m10 = cvGetSpatialMoment(&moment,1,0);
        int xc = qRound(m10/m00);
        int yc = qRound(m01/m00);
//        cvDrawContours(rgbDisplay, contTemp, CV_RGB(0,0,255), CV_RGB(0,0,255), 1, CV_FILLED, 8 );
        cvRectangle(rgbDisplay,cvPoint(qRound(xc - w/2.0),qRound(yc - h/2.0)),cvPoint(qRound(xc + w/2.0),qRound(yc + h/2.0)),CV_RGB(0,255,0),1);
        alignmentFidSet.push_back(boundingBox);
    }

    int startX,startY;
    qDebug()<<"Fiducials found:"<<alignmentFidSet.size();
    if(alignmentFidSet.size()!=4)
    {
        warnUser(QString("Expected 4 Fiducails but found %1").arg(alignmentFidSet.size()),QMessageBox::Warning);
        return QRectF(0,0,0,0);
    }
    QVector <QPointF> fidsFound;
    for(int i=0;i<alignmentFidSet.size();i++)
    {
        startX = qRound(alignmentFidSet[i].center.x);
        startY = qRound(alignmentFidSet[i].center.y);
        if(srcImage->imageData[srcImage->widthStep*startY + startX] == 0)
        {
            cvCircle(rgbDisplay,cvPoint(startX,startY),3,CV_RGB(255,0,0),-1);
            topLeftFid = QPointF(startX,startY);
        }
        else
        {
            cvCircle(rgbDisplay,cvPoint(startX,startY),1,CV_RGB(0,0,255),-1);
            fidsFound.push_back(QPointF(startX,startY));
        }
        //qDebug()<<"Value is:"<<(int)(srcImage->imageData[srcImage->widthStep*startY + startX]);
    }
    double dist0 = eucDistance(topLeftFid,fidsFound[0]);
    double dist1 = eucDistance(topLeftFid,fidsFound[1]);
    double dist2 = eucDistance(topLeftFid,fidsFound[2]);
    if(dist0>dist1 && dist0>dist2)
    {
        buttomRightFid = fidsFound[0];
        fidsFound.remove(0);
    }
    else if(dist1>dist0 && dist1>dist2)
    {
        buttomRightFid = fidsFound[1];
        fidsFound.remove(1);
    }
    else
    {
        buttomRightFid = fidsFound[2];
        fidsFound.remove(2);
    }
    Line line = Line(topLeftFid,buttomRightFid);
    double rotationAngle = RTOD(atan2((buttomRightFid.y()-topLeftFid.y()),(buttomRightFid.x()-topLeftFid.x())));
    qDebug()<<"Angle is:"<<-rotationAngle;
    QMatrix m;
    m.rotate(-rotationAngle);
    m.translate(-topLeftFid.x(),-topLeftFid.y());
    QPointF rotatedFid1 = m.map(fidsFound[0]);
    qDebug()<<"Before Rotation X:"<<fidsFound[0].x()<<" Y:"<<fidsFound[0].y()<<"After Rotation X:"<<rotatedFid1.x()<<" Y:"<<rotatedFid1.y();
    QPointF rotatedFid2 = m.map(fidsFound[1]);
    qDebug()<<"Before Rotation X:"<<fidsFound[1].x()<<" Y:"<<fidsFound[1].y()<<"After Rotation X:"<<rotatedFid2.x()<<" Y:"<<rotatedFid2.y();
    if(rotatedFid1.x()>0 && rotatedFid1.y()<0)
    {
        topRightFid   = fidsFound[0];
        buttomLeftFid = fidsFound[1];
    }
    else
    {
        topRightFid   = fidsFound[1];
        buttomLeftFid = fidsFound[0];
    }

    cvLine(rgbDisplay,cvPoint(qRound(topLeftFid.x()),qRound(topLeftFid.y())),cvPoint(qRound(topRightFid.x()),qRound(topRightFid.y())),CV_RGB(0,255,0),1);
    cvLine(rgbDisplay,cvPoint(qRound(buttomLeftFid.x()),qRound(buttomLeftFid.y())),cvPoint(qRound(buttomRightFid.x()),qRound(buttomRightFid.y())),CV_RGB(0,255,0),1);

    double pixelsPerInch = eucDistance(topLeftFid,topRightFid)/6.88976378;
    qDebug()<<"Pixels per Inch:"<<pixelsPerInch;
    rotationAngle = (atan2((topRightFid.y()-topLeftFid.y()),(topRightFid.x()-topLeftFid.x())));
    qDebug()<<"Pattern Rotation Angle is:"<<rotationAngle;
    qDebug()<<"Top Left:"<<topLeftFid<<" Top Right:"<<topRightFid;
    // The location of the pattern of interest in the fiducial coordinate
    QPointF patternTopLeft      = QPointF(-0.78740, 2.79527559);
    QPointF patternTopRight     = QPointF(7.48031 , 2.79527559);
    QPointF patternButtomRight  = QPointF(7.48031 , 5.27559055);
    QPointF patternButtomLeft   = QPointF(-0.78740, 5.27559055);
    double cosa = cos(rotationAngle);
    double sina = sin(rotationAngle);
    QPointF translatedTopLeft     = QPointF(cosa*patternTopLeft.x()*pixelsPerInch -sina*patternTopLeft.y()*pixelsPerInch + topLeftFid.x(),sina*patternTopLeft.x()*pixelsPerInch + cosa*patternTopLeft.y()*pixelsPerInch + topLeftFid.y());
    QPointF translatedTopRight    = QPointF(cosa*patternTopRight.x()*pixelsPerInch -sina*patternTopRight.y()*pixelsPerInch + topLeftFid.x(),sina*patternTopRight.x()*pixelsPerInch + cosa*patternTopRight.y()*pixelsPerInch + topLeftFid.y());
    QPointF translatedButtomRight = QPointF(cosa*patternButtomRight.x()*pixelsPerInch -sina*patternButtomRight.y()*pixelsPerInch + topLeftFid.x(),sina*patternButtomRight.x()*pixelsPerInch + cosa*patternButtomRight.y()*pixelsPerInch + topLeftFid.y());
    QPointF translatedButtomLeft  = QPointF(cosa*patternButtomLeft.x()*pixelsPerInch -sina*patternButtomLeft.y()*pixelsPerInch + topLeftFid.x(),sina*patternButtomLeft.x()*pixelsPerInch + cosa*patternButtomLeft.y()*pixelsPerInch + topLeftFid.y());

    qDebug()<<"Translated Pattern Top left x:"<<translatedTopLeft.x()<<" y:"<<translatedTopLeft.y();
    qDebug()<<"Translated Pattern Buttom Right x:"<<translatedButtomRight.x()<<" y:"<<translatedButtomRight.y();
    // Necessary boundary check, otherwise the scanner wont scan
    translatedTopLeft.setX(limit2Boundaries(translatedTopLeft.x(),0.0,double(srcImage->width)));
    translatedTopLeft.setY(limit2Boundaries(translatedTopLeft.y(),0.0,double(srcImage->height)));

    translatedTopRight.setX(limit2Boundaries(translatedTopRight.x(),0.0,double(srcImage->width)));
    translatedTopRight.setY(limit2Boundaries(translatedTopRight.y(),0.0,double(srcImage->height)));

    translatedButtomRight.setX(limit2Boundaries(translatedButtomRight.x(),0.0,double(srcImage->width)));
    translatedButtomRight.setY(limit2Boundaries(translatedButtomRight.y(),0.0,double(srcImage->height)));

    translatedButtomLeft.setX(limit2Boundaries(translatedButtomLeft.x(),0.0,double(srcImage->width)));
    translatedButtomLeft.setY(limit2Boundaries(translatedButtomLeft.y(),0.0,double(srcImage->height)));

    cvLine(rgbDisplay,cvPoint(qRound(translatedTopLeft.x()),qRound(translatedTopLeft.y())),cvPoint(qRound(translatedTopRight.x()),qRound(translatedTopRight.y())),CV_RGB(0,0,255),1);
    cvLine(rgbDisplay,cvPoint(qRound(translatedTopRight.x()),qRound(translatedTopRight.y())),cvPoint(qRound(translatedButtomRight.x()),qRound(translatedButtomRight.y())),CV_RGB(0,0,255),1);
    cvLine(rgbDisplay,cvPoint(qRound(translatedButtomRight.x()),qRound(translatedButtomRight.y())),cvPoint(qRound(translatedButtomLeft.x()),qRound(translatedButtomLeft.y())),CV_RGB(0,0,255),1);
    cvLine(rgbDisplay,cvPoint(qRound(translatedButtomLeft.x()),qRound(translatedButtomLeft.y())),cvPoint(qRound(translatedTopLeft.x()),qRound(translatedTopLeft.y())),CV_RGB(0,0,255),1);

    cvCircle(rgbDisplay,cvPoint(qRound(translatedTopLeft.x()),qRound(translatedTopLeft.y())),4,CV_RGB(255,0,0),-1);
    cvCircle(rgbDisplay,cvPoint(qRound(translatedButtomRight.x()),qRound(translatedButtomRight.y())),4,CV_RGB(0,0,255),-1);

    //ui.imageDisplay->recieveFrame(rgbDisplay);
    cvSaveImage("foundAt.png",rgbDisplay);
    cvReleaseImage(&srcImage);
    cvReleaseImage(&rgbDisplay);
    cvClearSeq(contours);
    cvReleaseMemStorage( &storage );
    translatedTopLeft.setX(translatedTopLeft.x()/pixelsPerInch);translatedTopLeft.setY(translatedTopLeft.y()/pixelsPerInch);
    translatedButtomRight.setX(translatedButtomRight.x()/pixelsPerInch);translatedButtomRight.setY(translatedButtomRight.y()/pixelsPerInch);
    return QRectF(translatedTopLeft,translatedButtomRight);
}

void MainWindow::selectSPAImage()
{

}

void MainWindow::setEnableMotorRelatedOptions(bool enable)
{
    if(!enable)
    {
        ui.toolBar->removeAction(ui.actionAcquisition);
        ui.toolBar->removeAction(ui.actionLeft);
        ui.toolBar->removeAction(ui.actionRight);
        ui.toolBar->removeAction(ui.actionEnd);
        ui.toolBar->removeAction(ui.actionAuto_Focus);
        ui.toolBar->removeAction(ui.actionJog_Left);
        ui.toolBar->removeAction(ui.actionJog_Right);
        ui.toolBar->removeAction(ui.actionPrevious_Chip);
        ui.toolBar->removeAction(ui.actionNext_Chip);
        ui.toolBar->removeAction(ui.actionLoading_Position);
        ui.toolBar->removeAction(ui.actionRefocus);
        ui.toolBar->removeAction(ui.actionRun_Test_Script);
        ui.toolBar->removeAction(ui.actionRun_Script);
        ui.toolBar->removeAction(ui.actionReset_Home);
        ui.toolBar->removeAction(ui.actionTimed_Acquisition);
        ui.toolBar->removeAction(ui.actionHome);
    }
    else
    {
        ui.toolBar->addAction(ui.actionAcquisition);
        ui.toolBar->addAction(ui.actionLeft);
        ui.toolBar->addAction(ui.actionRight);
        ui.toolBar->addAction(ui.actionEnd);
        ui.toolBar->addAction(ui.actionAuto_Focus);
        ui.toolBar->addAction(ui.actionJog_Left);
        ui.toolBar->addAction(ui.actionJog_Right);
        ui.toolBar->addAction(ui.actionPrevious_Chip);
        ui.toolBar->addAction(ui.actionNext_Chip);
        ui.toolBar->addAction(ui.actionLoading_Position);
        ui.toolBar->addAction(ui.actionRefocus);
        ui.toolBar->addAction(ui.actionRun_Test_Script);
        ui.toolBar->addAction(ui.actionReset_Home);
        ui.toolBar->addAction(ui.actionTimed_Acquisition);
        ui.toolBar->addAction(ui.actionHome);
    }
}

void MainWindow::setEncapsulationImagesSource()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Chose Directory with the ACD Captured Images",
                                                    ASG_PIT::settings().getEncapsulationImgSrc(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks
                                                    | QFileDialog::DontUseNativeDialog);
    if(dir == "")
    {
        warnUser(QString("No folder selected."),QMessageBox::Information);
        return;
    }
    ASG_PIT::settings().setEncapsulationImgSrc(dir);
    ui.encapsulationImagesSource->setText(dir);
}

void MainWindow::setPlumpingOder(QString newOrder)
{
    ASG_PIT::settings().setSPAPlumpingOder(newOrder);
}

void MainWindow::setupImageAnalysis()
{
    // Image Analysis Thread
    imageAnalysis = new ImageAnalysis();
    imageAnalysis->setShowNumbers(false);
    imageAnalysis->setShowClearNozzels(showClearNozzels->isChecked());
    imageAnalysis->setShowDeadNozzels(showDeadNozzels->isChecked());
    imageAnalysis->setShowNumbers(showNumbers->isChecked());
    imageAnalysis->setNozzelTemplateList(nozzelTempModel->getTemplatesList());
    imageAnalysis->setEdgeStartTemplateList(edgeStartTempModel->getTemplatesList());
    imageAnalysis->setEdgeEndTemplateList(edgeEndTempModel->getTemplatesList());
    imageAnalysis->setMatchingAccuracy(ASG_PIT::settings().getMatchingAccuracy()/100.0);
    imageAnalysis->setEdgeMatchingAccuracy(ASG_PIT::settings().getEdgeMatchingAccuracy()/100.0);
    imageAnalysis->setSecondMatchingAccuracy(ASG_PIT::settings().getSecondMatchingAccuracy()/100.0);
    // Make it scriptable
    scriptEngine.globalObject().setProperty("ImageAnalysis", scriptEngine.newQObject(imageAnalysis));
    changeImagesSource(ASG_PIT::settings().getImagesSource());
    switch (ASG_PIT::settings().getAnalysisType())
    {
        case ImageAnalysis::MANUAL:
            manualAnalysis->setChecked(true);
            analysisType = ImageAnalysis::MANUAL;
            break;
        case ImageAnalysis::SEMI_AUTO:
            semiAutoAnalysis->setChecked(true);
            analysisType = ImageAnalysis::SEMI_AUTO;
            break;
        case ImageAnalysis::AUTO:
            autoAnalysis->setChecked(true);
            analysisType = ImageAnalysis::AUTO;
            break;
    }
    imageAnalysis->setType(analysisType);
    connect(imageAnalysis,SIGNAL(displayImageThenRelease(IplImage*)),sceneRendering,SLOT(renderFrameThenRelease(IplImage*)));
    connect(imageAnalysis,SIGNAL(warnUser(QString,int)),SLOT(warnUser(QString,int)));
    connect(imageAnalysis,SIGNAL(analysisFinished()),SLOT(analysisFinished()));
    connect(imageAnalysis,SIGNAL(processingImage(QString)),SLOT(processingImage(QString)));
    connect(imageAnalysis,SIGNAL(warnUser(QString,int)),SLOT(warnUser(QString,int)));
    connect(imageAnalysis,SIGNAL(stateProgress(QString,int)),SLOT(showProgress(QString,int)));
    connect(imageAnalysis,SIGNAL(finishedMatching()),SLOT(hideProgress()));
    connect(imageAnalysis,SIGNAL(sendOriginalDisplayImage(IplImage*)),SLOT(getOfflineImage(IplImage*)));
    connect(imageAnalysis,SIGNAL(processingImageAnalysis()),this,SLOT(processingImageAnalysis()));
    connect(imageAnalysis,SIGNAL(finishedProcessingImageAnalysis()),this,SLOT(finishedProcessingImageAnalysis()));
    connect(imageAnalysis,SIGNAL(viewDNMap(NozzleHealthMap*)),nozzleHealthViewer,SLOT(viewNozzleHealthMap(NozzleHealthMap*)));
    connect(this,SIGNAL(propagateNozzelSelected(const QPoint&,int)),imageAnalysis,SLOT(nozzelSelected(const QPoint&,int)));
    connect(this,SIGNAL(setAnalysisPause(bool)),imageAnalysis,SLOT(setPaused(bool)));
    connect(enableSecondLevelMatch,SIGNAL(toggled(bool)),SLOT(secondMatchLevelToggled(bool)));
}

void MainWindow::setupNHAViewerTab()
{
    nozzleHealthViewer = new NozzleHealthViewer(ui.nozzleHealthViewer);
    connect(ui.displayDNMap,SIGNAL(released()),nozzleHealthViewer,SLOT(viewMap()));
    connect(ui.printView,SIGNAL(released()),nozzleHealthViewer,SLOT(print()));
    connect(ui.backDoorPrint,SIGNAL(released()),nozzleHealthViewer,SLOT(printTNFP()));
    connect(nozzleHealthViewer,SIGNAL(warnUser(QString,int)),this,SLOT(warnUser(QString,int)));
    connect(nozzleHealthViewer,SIGNAL(showStatusBarMsg(QString)),this,SLOT(showStatusBarMsg(QString)));
    connect(ui.mergeDNMaps,SIGNAL(released()),nozzleHealthViewer,SLOT(mergeDNMaps()));
    connect(ui.mergeIntoImage,SIGNAL(released()),nozzleHealthViewer,SLOT(mergeIntoImage()));
    connect(ui.generateTNFP,SIGNAL(released()),nozzleHealthViewer,SLOT(generateTNFPattern()));
}

void MainWindow::setupOfflineAnalysisTab()
{
    // offline analysis widget
    QHBoxLayout * hLayout = new QHBoxLayout;

    analysisDisplay = new QGroupBox(tr("Last Focused Image"));
    QHBoxLayout * displayLayout = new QHBoxLayout(analysisDisplay);
    displayLayout->addWidget(sceneRendering);
    analysisDisplay->setMinimumSize(QSize(1045, 750));
    analysisDisplay->setLayout(displayLayout);

    analysisSetting = new QGroupBox(tr("Analysis Settings"));
    analysisSetting->setFixedWidth(260);
    QVBoxLayout * analysisLayout = new QVBoxLayout;

    //matching percentage box
    QGridLayout *gridLayout;
    matchPercentageGrp = new QGroupBox(analysisSetting);
    matchPercentageGrp->setAttribute(Qt::WA_ContentsPropagated);
    matchPercentageGrp->setTitle("Matching Accuracy");
    gridLayout = new QGridLayout(matchPercentageGrp);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(11);

    QLabel *firstMatch, *secondMatch, *edgeTempMatch;
    secondMatchingValue     = new  QLabel;
    matchValue              = new  QLabel;
    edgeTempMatchingValue   = new QLabel;

    firstMatch      = new QLabel("Nozzle");
    secondMatch     = new QLabel("Filter");
    edgeTempMatch   = new QLabel("Edge");

    matchingSlider         = new QSlider(Qt::Horizontal, matchPercentageGrp);
    secondMatchingSlider   = new QSlider(Qt::Horizontal, matchPercentageGrp);
    edgeTempMatchSlider    = new QSlider(Qt::Horizontal, matchPercentageGrp);

    matchingSlider->setRange(75,100);
    matchingSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    secondMatchingSlider->setRange(ASG_PIT::settings().getMatchingAccuracy(),100);
    secondMatchingSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    secondMatchingSlider->setValue(ASG_PIT::settings().getSecondMatchingAccuracy());
    secondMatchingValue->setText(QString("%1").arg(ASG_PIT::settings().getSecondMatchingAccuracy()));

    edgeTempMatchSlider->setRange(85,100);
    edgeTempMatchSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    edgeTempMatchSlider->setValue(ASG_PIT::settings().getEdgeMatchingAccuracy());

    matchValue->setText(QString("%1").arg(ASG_PIT::settings().getMatchingAccuracy()));
    enableSecondLevelMatch = new QCheckBox("Enable");
    enableSecondLevelMatch->setChecked(ASG_PIT::settings().getSecondMatchLevelEnabled());
    edgeTempMatchingValue->setText(QString("%1").arg(ASG_PIT::settings().getEdgeMatchingAccuracy()));

    gridLayout->addWidget(firstMatch,1,1,1,1);
    gridLayout->addWidget(matchingSlider,1,2,1,3);
    gridLayout->addWidget(matchValue,1,5,1,1,Qt::AlignCenter);

    gridLayout->addWidget(secondMatch,2,1,1,1);
    gridLayout->addWidget(secondMatchingSlider,2,2,1,2);
    gridLayout->addWidget(secondMatchingValue,2,4,1,1,Qt::AlignCenter);
    gridLayout->addWidget(enableSecondLevelMatch,2,5,1,1);

    gridLayout->addWidget(edgeTempMatch,3,1,1,1);
    gridLayout->addWidget(edgeTempMatchSlider,3,2,1,3);
    gridLayout->addWidget(edgeTempMatchingValue,3,5,1,1,Qt::AlignCenter);

    templatesTab = new QTabWidget(analysisSetting);
    templatesTab->setTabPosition(QTabWidget::East);
    QWidget *nozzelTempTab = new QWidget();
    QWidget *edgeStartTempTab = new QWidget();
    QWidget *edgeEndTempTab = new QWidget();
    templatesTab->addTab(nozzelTempTab,QString("Nozzel Temps"));
    templatesTab->addTab(edgeStartTempTab,QString("EdgeStart Temps"));
    templatesTab->addTab(edgeEndTempTab,QString("EdgeEnd Temps"));

    // Nozzel Templates box
    nozzelTempGrp = new QGroupBox(analysisSetting);
    nozzelTempGrp->setAttribute(Qt::WA_ContentsPropagated);
    nozzelTempGrp->setTitle("Nozzel Templates");
    QListView *nozzelTempList = new QListView(nozzelTempGrp);
    nozzelTempList->setDragEnabled(true);
    nozzelTempList->setViewMode(QListView::IconMode);
    nozzelTempList->setIconSize(QSize(25, 45));
    nozzelTempList->setGridSize(QSize(35, 55));
    nozzelTempList->setSpacing(10);
    nozzelTempList->setMovement(QListView::Snap);
    nozzelTempList->setAcceptDrops(true);
    nozzelTempList->setDropIndicatorShown(true);

    addNozzelTemplateBtn = new QPushButton(tr("Add Nozzel Temp"));
    clearNozzelTempListBtn = new QPushButton(tr("Clear list"));
    nozzelTempModel = new TemplateModel(this);
    nozzelTempList->setModel(nozzelTempModel);
    nozzelTempModel->addPieces(nozzelTempPath);

    QVBoxLayout *nozzelTempGroupLayout = new QVBoxLayout(nozzelTempGrp);
    nozzelTempGroupLayout->addWidget(nozzelTempList,Qt::AlignCenter);
    nozzelTempGroupLayout->addWidget(addNozzelTemplateBtn,Qt::AlignCenter);
    nozzelTempGroupLayout->addWidget(clearNozzelTempListBtn,Qt::AlignCenter);

    // Edge Start Templates box
    QGroupBox *edgeStartGrp = new QGroupBox(analysisSetting);
    edgeStartGrp->setAttribute(Qt::WA_ContentsPropagated);
    edgeStartGrp->setTitle("Edge Start Templates");
    QListView *edgeStartTempList = new QListView(edgeStartGrp);
    edgeStartTempList->setDragEnabled(true);
    edgeStartTempList->setViewMode(QListView::IconMode);
    edgeStartTempList->setIconSize(QSize(25, 45));
    edgeStartTempList->setGridSize(QSize(35, 55));
    edgeStartTempList->setSpacing(10);
    edgeStartTempList->setMovement(QListView::Snap);
    edgeStartTempList->setAcceptDrops(true);
    edgeStartTempList->setDropIndicatorShown(true);

    addEdgeStartTemplateBtn   = new QPushButton(tr("Add edge Start Temp"));
    clearEdgeStartTempListBtn = new QPushButton(tr("Clear list"));
    edgeStartTempModel = new TemplateModel(this);
    edgeStartTempList->setModel(edgeStartTempModel);
    edgeStartTempModel->addPieces(edgeStartPath);

    QVBoxLayout *edgeStartTempGroupLayout = new QVBoxLayout(edgeStartGrp);
    edgeStartTempGroupLayout->addWidget(edgeStartTempList,Qt::AlignCenter);
    edgeStartTempGroupLayout->addWidget(addEdgeStartTemplateBtn,Qt::AlignCenter);
    edgeStartTempGroupLayout->addWidget(clearEdgeStartTempListBtn,Qt::AlignCenter);

    // Edge End Templates box
    QGroupBox *edgeEndGrp = new QGroupBox(analysisSetting);
    edgeEndGrp->setAttribute(Qt::WA_ContentsPropagated);
    edgeEndGrp->setTitle("Edge End Templates");
    QListView *edgeEndTempList = new QListView(edgeEndGrp);
    edgeEndTempList->setDragEnabled(true);
    edgeEndTempList->setViewMode(QListView::IconMode);
    edgeEndTempList->setIconSize(QSize(25, 45));
    edgeEndTempList->setGridSize(QSize(35, 55));
    edgeEndTempList->setSpacing(10);
    edgeEndTempList->setMovement(QListView::Snap);
    edgeEndTempList->setAcceptDrops(true);
    edgeEndTempList->setDropIndicatorShown(true);

    addEdgeEndTemplateBtn   = new QPushButton(tr("Add edge End Temp"));
    clearEdgeEndTempListBtn = new QPushButton(tr("Clear list"));
    edgeEndTempModel = new TemplateModel(this);
    edgeEndTempList->setModel(edgeEndTempModel);
    edgeEndTempModel->addPieces(edgeEndPath);

    QVBoxLayout *edgeEndTempGroupLayout = new QVBoxLayout(edgeEndGrp);
    edgeEndTempGroupLayout->addWidget(edgeEndTempList,Qt::AlignCenter);
    edgeEndTempGroupLayout->addWidget(addEdgeEndTemplateBtn,Qt::AlignCenter);
    edgeEndTempGroupLayout->addWidget(clearEdgeEndTempListBtn,Qt::AlignCenter);

    // Analysis Type Box
    analysisTypeGrp = new QGroupBox(analysisSetting);
    analysisTypeGrp->setAttribute(Qt::WA_ContentsPropagated);
    analysisTypeGrp->setTitle("Interaction Method");
    manualAnalysis    = new QRadioButton(tr("Manual"));
    semiAutoAnalysis  = new QRadioButton(tr("Semi-Auto"));
    autoAnalysis      = new QRadioButton(tr("Continious-Auto"));
    autoAnalysis->setEnabled(true);

    QVBoxLayout *analysisTypeLayout = new QVBoxLayout(analysisTypeGrp);
    analysisTypeLayout->addWidget(manualAnalysis);
    analysisTypeLayout->addWidget(semiAutoAnalysis);
    analysisTypeLayout->addWidget(autoAnalysis);

    // Images Source Radio Box
    imagesSourceGrp = new QGroupBox(analysisSetting);
    imagesSourceGrp->setAttribute(Qt::WA_ContentsPropagated);
    imagesSourceGrp->setTitle("Images Source");
    acdImagesSource         = new QRadioButton(tr("ACD (big Templates)"));
    acdImagesSourceSmall    = new QRadioButton(tr("ACD (small Templates)"));
    thunderBirdImageSource  = new QRadioButton(tr("Thunderbird"));
    generateDNMapChkBox     = new QCheckBox(tr("Generate DNMap"));
    generateDNMapChkBox->setChecked(false);

    QVBoxLayout *imagesSourceLayout = new QVBoxLayout(imagesSourceGrp);
    imagesSourceLayout->addWidget(acdImagesSource);
    imagesSourceLayout->addWidget(acdImagesSourceSmall);
    imagesSourceLayout->addWidget(thunderBirdImageSource);
    imagesSourceLayout->addWidget(generateDNMapChkBox);

    // Display Dead and Clear Nozzels checkboxes
    displayNozzelsGrp = new QGroupBox(analysisSetting);
    displayNozzelsGrp->setAttribute(Qt::WA_ContentsPropagated);
    displayNozzelsGrp->setTitle("Display");
    showDeadNozzels         = new QCheckBox(tr("Dead Nozzels"));
    showClearNozzels        = new QCheckBox(tr("Clear Nozzels"));
    showNumbers             = new QCheckBox(tr("Show Numbers"));
    showDeadNozzels->setChecked(true);
    showClearNozzels->setChecked(true);

    // This is to disable number showing when DNMap generation is switched off
    showNumbers->setChecked(false);
    showNumbers->setEnabled(false);

    QVBoxLayout *displayNozzelLayout = new QVBoxLayout(displayNozzelsGrp);
    displayNozzelLayout->addWidget(showDeadNozzels);
    displayNozzelLayout->addWidget(showClearNozzels);
    displayNozzelLayout->addWidget(showNumbers);

    // MES Box
    mesGrp = new QGroupBox(analysisSetting);
    mesGrp->setAttribute(Qt::WA_ContentsPropagated);
    mesGrp->setTitle("MES Options");
    mesPushDataBtn = new QPushButton(tr("Push data to MES"));
    mesPushDataBtn->setEnabled(false);

    QVBoxLayout *mesLayout = new QVBoxLayout(mesGrp);
    mesLayout->addWidget(mesPushDataBtn);


    QVBoxLayout *nozzleTemplatesTabLayout = new QVBoxLayout(nozzelTempTab);
    nozzleTemplatesTabLayout->addWidget(nozzelTempGrp);
    nozzelTempTab->setLayout(nozzleTemplatesTabLayout);

    QVBoxLayout *edgeStartTemplatesTabLayout = new QVBoxLayout(edgeStartTempTab);
    edgeStartTemplatesTabLayout->addWidget(edgeStartGrp);
    edgeStartTempTab->setLayout(edgeStartTemplatesTabLayout);

    QVBoxLayout *edgeEndTemplatesTabLayout = new QVBoxLayout(edgeEndTempTab);
    edgeEndTemplatesTabLayout->addWidget(edgeEndGrp);
    edgeEndTempTab->setLayout(edgeEndTemplatesTabLayout);

    // Putting them all together
    analysisLayout->addWidget(matchPercentageGrp);
    analysisLayout->addWidget(templatesTab);
    analysisLayout->addWidget(analysisTypeGrp);
    analysisLayout->addWidget(imagesSourceGrp);
    analysisLayout->addWidget(displayNozzelsGrp);
    analysisLayout->addWidget(mesGrp);
    analysisLayout->addStretch(1);
    analysisSetting->setLayout(analysisLayout);

    hLayout->addWidget(analysisDisplay);
    hLayout->addWidget(analysisSetting);

    ui.imageAnalysisTab->setLayout(hLayout);
    generateDNMapChkBox->setChecked(ASG_PIT::settings().getGenerateDNMap());
    matchingSlider->setValue(ASG_PIT::settings().getMatchingAccuracy());

    connect(matchingSlider,SIGNAL(sliderMoved(int)),SLOT(matchingValueChanged(int)));
    connect(matchingSlider,SIGNAL(sliderReleased()),SLOT(triggerAnalysis()));
    connect(edgeTempMatchSlider,SIGNAL(sliderMoved(int)),SLOT(edgeMatchingValueChanged(int)));
    connect(edgeTempMatchSlider,SIGNAL(sliderReleased()),SLOT(triggerAnalysis()));
    connect(secondMatchingSlider,SIGNAL(valueChanged(int)),SLOT(secondMatchingSliderValueChanged(int)));
    connect(manualAnalysis,SIGNAL(released()),SLOT(triggerAnalysisType()));
    connect(autoAnalysis,SIGNAL(released()),SLOT(triggerAnalysisType()));
    connect(semiAutoAnalysis,SIGNAL(released()),SLOT(triggerAnalysisType()));
    connect(acdImagesSource,SIGNAL(released()),SLOT(changeImagesSource()));
    connect(acdImagesSourceSmall,SIGNAL(released()),SLOT(changeImagesSource()));
    connect(thunderBirdImageSource,SIGNAL(released()),SLOT(changeImagesSource()));
    connect(generateDNMapChkBox,SIGNAL(stateChanged(int)),SLOT(dnmapGenerationChanged(int)));
    connect(clearNozzelTempListBtn,SIGNAL(released()),SLOT(clearNozzelTempList()));
    connect(clearEdgeEndTempListBtn,SIGNAL(released()),SLOT(clearEdgeEndTempList()));
    connect(clearEdgeStartTempListBtn,SIGNAL(released()),SLOT(clearEdgeStartTempList()));
    connect(showDeadNozzels,SIGNAL(released()),SLOT(changeNozzelDisplay()));
    connect(showClearNozzels,SIGNAL(released()),SLOT(changeNozzelDisplay()));
    connect(showNumbers,SIGNAL(released()),SLOT(changeNozzelDisplay()));
    connect(addNozzelTemplateBtn,SIGNAL(clicked()),SLOT(addNozzelTemplateBtnClicked()));
    connect(addEdgeStartTemplateBtn,SIGNAL(clicked()),SLOT(addEdgeStartTemplateBtnClicked()));
    connect(addEdgeEndTemplateBtn,SIGNAL(clicked()),SLOT(addEdgeEndTemplateBtnClicked()));
    connect(mesPushDataBtn,SIGNAL(clicked()),SLOT(mesBottonPressed()));
}

void MainWindow::showDetails(QTableWidgetItem *item)
{
    if(!imageViewer)
        imageViewer = new ImageViewer();
    QImage image;
    image = item->data(1).value<QImage>();
    //qVariantSetValue(item->data(2),image);
    //img->setData(1,qVariantFromValue(imageFrame))
    imageViewer->open(image);
    imageViewer->normalSize();
    imageViewer->resize(QImage(":/SBR_logo").width(),QImage(":/SBR_logo").height());
    imageViewer->fitToWindow();
    imageViewer->show();
}

void MainWindow::showImageDetails(QTableWidgetItem *item)
{
    warnUser(item->text(),QMessageBox::Information);
}

void MainWindow::showLog()
{
    if(!logger->isVisible())
    {
        logger->show();
        logger->loadLogs();
    }
}

void MainWindow::showProbabilisticDNMap()
{
    if(!dnMapGeneratorDialog->isVisible())
    {
        dnMapGeneratorDialog->show();
    }
}

void MainWindow::showProgress(QString label, int progress)
{
    if(progress==-1)
    {
        ui.progressBar->setValue(0);
        hideProgress();
        return;
    }
    ui.acqProgress->setText(label);
    ui.progressBar->setValue(progress);
    ui.progressBar->setVisible(true);
    ui.acqProgress->setVisible(true);
}

void MainWindow::showSettingsDialog()
{
    if(settingsDialog)
        if(!settingsDialog->isVisible())
        {
            #ifdef PIT_ONLY
                settingsDialog->exec(0);
            #else
                settingsDialog->exec(7);
            #endif
        }
}

void MainWindow::showStatusBarMsg(QString msg)
{
    ui.statusbar->setVisible(true);
    ui.statusbar->showMessage(msg);
}

void MainWindow::spaFinished()
{
    QList<QWidget *> widgets = qFindChildren<QWidget *>(ui.spaTab);
    foreach (QWidget *w, widgets)
        w->setEnabled(true);
    ui.tabWidget->setCurrentIndex(1);
}

void MainWindow::specifySPAResultsSuffix(QString suffix)
{
    ASG_PIT::settings().setSPAContoursSuffix(suffix);
}

void MainWindow::specifyDNMapSuffix(QString suffix)
{
    ASG_PIT::settings().setSPADNMapSuffix(suffix);
}

void MainWindow::startComDetection()
{
    qDebug()<<"Com detection Started";
    if(comPortDetectionThread)
    {
        while(comPortDetectionThread->isRunning())
        {
            comPortDetectionThread->stop();
            usleep(100000);
        }
        delete comPortDetectionThread;
    }
    comPortDetectionThread = new ComPortDetectionThread();
    connect(comPortDetectionThread,SIGNAL(motorFound(QString)),this,SLOT(initiateMotors(QString)));
    connect(comPortDetectionThread,SIGNAL(ledLightingFound(QString)),this,SLOT(initiateLedLighting(QString)));
    // Reports the detection status for both of the ports
    connect(comPortDetectionThread,SIGNAL(comDetectionStatus(bool,bool)),this,SLOT(comDetectionStatus(bool,bool)));
    comPortDetectionThread->start();
    this->statusBar()->setVisible(true);
    this->statusBar()->showMessage(QString("Searching COM ports for the linear stage and led light controller ..."));
}

void MainWindow::startOfflineAnalysis()
{
    if(analysingOnline)
    {
        warnUser("Stop online analysis first before initiating an offline analysis!!!",QMessageBox::Warning);
        return;
    }
    // Send the stop signal
    if(analysingOffline)
    {
        // send a stop request
        imageAnalysis->stop();
        while(imageAnalysis->isRunning())
        {
            // wait for a max of 200ms for the thread to finish
            imageAnalysis->wait(200);
        }
        return;
    }
    QString dir = QFileDialog::getExistingDirectory(this, "Chose Directory with the Images",
                                                    ASG_PIT::settings().getLastDirPath(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks
                                                    | QFileDialog::DontUseNativeDialog);
    if(dir == "")
    {
        warnUser(QString("No folder selected, Offline analysis is Aborted"),QMessageBox::Information);
        return;
    }
    ui.tabWidget->setCurrentIndex(1);
    startOfflineAnalysis(dir);
}

void MainWindow::startTimedAquisition()
{
    bool ok;
    if(ui.actionTimed_Acquisition->text()==QString("Timed Acquisition"))
    {
        timedAcqusition = true;
        alreadyGotHeadCode = false;
        int i = QInputDialog::getDouble(this, tr("Enter Timer Value"),
                                     tr("Minutes:"), 6.0, 6.0, 1440, 1, &ok);
        if (ok)
        {
            connect(&acquisitionTimer,SIGNAL(timeout()),this,SLOT(triggerAcquisition()));
            acquisitionTimer.start(i*60*1000);
            triggerAcquisition();
            ui.actionTimed_Acquisition->setText("Stop Timer");
        }
    }
    else
    {
        timedAcqusition = false;
        alreadyGotHeadCode = false;
        ui.actionTimed_Acquisition->setText("Timed Acquisition");
        acquisitionTimer.stop();
        disconnect(&acquisitionTimer,SIGNAL(timeout()),this,SLOT(triggerAcquisition()));
    }
}

void MainWindow::startOfflineAnalysis(QString dir)
{
    QList<QWidget *> widgets = qFindChildren<QWidget *>(ui.toolBar);
    foreach (QWidget *w, widgets)
        w->setEnabled(false);
    QList<QWidget *> widgets2 = qFindChildren<QWidget *>(ui.imageAnalysisTab);
    foreach (QWidget *w, widgets2)
        w->setEnabled(false);
    mesPushDataBtn->setEnabled(false);
    ui.actionOffline_Analysis->setEnabled(true);
    ASG_PIT::settings().setLastDirPath(QFileInfo(dir).path());
    if (!imageAnalysis)
    {
        imageAnalysis = new ImageAnalysis();
        if(sceneRendering)
            connect(imageAnalysis,SIGNAL(displayImageThenRelease(IplImage*)),sceneRendering,SLOT(renderFrameThenRelease(IplImage*)));
        connect(this,SIGNAL(propagateNozzelSelected(const QPoint&,int)),imageAnalysis,SLOT(nozzelSelected(const QPoint&,int)));
        connect(imageAnalysis,SIGNAL(analysisFinished()),SLOT(analysisFinished()));
        connect(this,SIGNAL(setAnalysisPause(bool)),imageAnalysis,SLOT(setPaused(bool)));
        connect(imageAnalysis,SIGNAL(sendOfflineImage(IplImage *)),SLOT(getOfflineImage(IplImage *)));
    }
    while(imageAnalysis->isRunning())
    {
        imageAnalysis->stop();
        // wait for a max of 200ms for the thread to finish
        imageAnalysis->wait(200);
    }
    imageAnalysis->setDir(dir);
    imageAnalysis->setMode(ImageAnalysis::OFFLINE);
    imageAnalysis->setType(analysisType);
    imageAnalysis->startNewAnalysis(ImageAnalysis::OFFLINE);
    ui.tabWidget->setCurrentIndex(1);
    ui.actionOffline_Analysis->setText("Stop Analysis");
    imageAnalysis->start();
    this->setFocus();
    analysingOffline = true;
    analysingOnline = false;
    ui.actionOnline_Analysis->setChecked(false);
    ui.actionOnline_Analysis->setEnabled(false);
}

/*!
  Dynamic programming method to traverse directory structure and
  extract files with names matching a pre-defined regular expression
  */
QStringList MainWindow::traverseDirTree(QString rootDir,QRegExp rx)
{
    qDebug()<<"Traversing Dir:"<<rootDir;
    QRegExpValidator v(rx,0);
    //int pos = 0;
    QFileInfoList listOfImages;
    QStringList totalFileList;
    QDir dir(rootDir);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    listOfImages.clear();
    listOfImages = dir.entryInfoList();
    for(int i=0;i<listOfImages.size();i++)
    {
        QString name = listOfImages[i].fileName();
        if(listOfImages[i].isDir())
        {
            totalFileList << traverseDirTree(listOfImages[i].absoluteFilePath(),rx);
        }
//        if(v.validate(name,pos) == QValidator::Acceptable)
        if(rx.exactMatch(name))
        {
            totalFileList<<listOfImages[i].absoluteFilePath();
//            qDebug()<<"Adding: "<<qPrintable(listOfImages[i].fileName());
        }
    }
    return totalFileList;
}

bool MainWindow::triggerAcquisition()
{
    bool retVal = false;
    if(imageAcquisition->isAcquiring())
    {
        ui.actionAcquisition->setText("Image Acquisition");
        ui.printHeadCode->setEnabled(true);
        ui.printHeadName->setVisible(true);
        ui.printHeadCode->setVisible(true);
        imageAcquisition->setAcquisition(false);
        ui.progressBar->setVisible(false);
        ui.acqProgress->setVisible(false);
        ui.tabWidget->widget(1)->setEnabled(true);
        ui.groupBox_16->setEnabled(true);
        ui.groupBox_14->setEnabled(true);
        ui.groupBox_13->setEnabled(true);
        ui.groupBox_20->setEnabled(true);
        if(cameraSource == Camera::NONE)
        {
            disableCameraRelatedOptions();
        }
        else
        {
            QList<QWidget *> widgets = qFindChildren<QWidget *>(ui.toolBar);
            foreach (QWidget *w, widgets)
                w->setEnabled(true);
        }
        ui.actionOnline_Analysis->setEnabled(false);
    }
    else
    {
        if(getPrintHeadCode())
        {
            ui.tabWidget->setCurrentIndex(0);
            QList<QWidget *> widgets2 = qFindChildren<QWidget *>(ui.spaTab);
            foreach (QWidget *w, widgets2)
                w->setEnabled(false);
            QList<QWidget *> widgets1 = qFindChildren<QWidget *>(ui.toolBar);
            foreach (QWidget *w, widgets1)
                w->setEnabled(false);
            ui.progressBar->setVisible(true);
            ui.acqProgress->setVisible(true);
            ui.printHeadCode->setEnabled(false);
            ui.printHeadName->setVisible(true);
            ui.printHeadCode->setVisible(true);
            ui.tabWidget->widget(1)->setEnabled(false);
            ui.groupBox_16->setEnabled(false);
            ui.groupBox_14->setEnabled(false);
            ui.groupBox_13->setEnabled(false);
            ui.groupBox_20->setEnabled(false);
            ui.acqProgress->setText("Acquisition Progress:");
            ui.actionAcquisition->setText("Stop Acquisition");
            gotoFocusPose();
            imageAcquisition->setAcquisition(true);
            ui.actionAcquisition->setEnabled(true);
            ui.actionTimed_Acquisition->setEnabled(true);
            retVal = true;
        }
        else
        {
            warnUser("Invalid printhead code, try again.",QMessageBox::Information);
            retVal = false;
        }
    }
    return retVal;
}

void MainWindow::triggerAnalysis()
{
    repeatAnalysis = true;
    imageAnalysis->setRepeatAnalysis(repeatAnalysis);
    repeatAnalysis = false;
    qDebug()<<"Triggering Analysis";
}

void MainWindow::triggerAnalysisType()
{
    if(manualAnalysis->isChecked())
    {
        analysisType = ImageAnalysis::MANUAL;
    }
    else if(autoAnalysis->isChecked())
    {
        analysisType = ImageAnalysis::AUTO;
    }
    else if(semiAutoAnalysis->isCheckable())
    {
        analysisType = ImageAnalysis::SEMI_AUTO;
    }
    if(imageAnalysis)
        imageAnalysis->setType(analysisType);
    ASG_PIT::settings().setAnalysisType(analysisType);
    this->setFocus();
}

void MainWindow::updateMotorPose(int motorId,int motorPose)
{
    if(motorId==LINEAR_MOTOR)
    {
        linearMotorPose = motorPose;
        if(linearMotorPose<0)
            currentChip = 0;
        else
            currentChip = (int)floor((linearMotorPose-homePose)/float(distanceBetweenChips));
        if(continousSweeping)
        {
            if(sweepingStartPose<sweepingEndPose)
            {
                if(linearMotorPose>sweepingEndPose)
                {
                    if(motorControl)
                        motorControl->stopJogging(LINEAR_MOTOR);
                    continousSweeping = false;
                    ui.continiousSweepBtn->setText("Start Sweep");
                }
            }
            else
            {
                if(linearMotorPose<sweepingEndPose)
                {
                    if(motorControl)
                        motorControl->stopJogging(LINEAR_MOTOR);
                    continousSweeping = false;
                    ui.continiousSweepBtn->setText("Start Sweep");
                }
            }
        }
        else if (steppingMotion)
        {
            if( (linearMotorPose>=ui.programmedPositionView->item(stepIndex,1)->text().toInt() && traversingDirection == LEFT ) ||
                (linearMotorPose<=ui.programmedPositionView->item(stepIndex,1)->text().toInt() && traversingDirection == RIGHT ))
            {
                if(motorControl)
                {
                    motorControl->stopJogging(LINEAR_MOTOR);
                    motorControl->sleepMotor(LINEAR_MOTOR,ui.delayBetweenSteps->value()*1000);
                    stepIndex++;
                    if(stepIndex<ui.programmedPositionView->rowCount())
                    {
                        if(linearMotorPose>ui.programmedPositionView->item(stepIndex,1)->text().toInt())
                        {
                            motorControl->jogMotorRight(LINEAR_MOTOR);
                            traversingDirection = RIGHT;
                        }
                        else
                        {
                            motorControl->jogMotorLeft(LINEAR_MOTOR);
                            traversingDirection = LEFT;
                        }
                    }
                    else
                    {
                        steppingMotion = false;
                        ui.traverseSteps->setText("Traverse");
                        ui.programmedPos->setEnabled(true);
                        ui.addProgPose->setEnabled(true);
                        ui.saveProgrammedList->setEnabled(true);
                        ui.fillCurrentPose->setEnabled(true);
                        ui.clearProgrammedList->setEnabled(true);
                        ui.delayBetweenSteps->setEnabled(true);
                        ui.label_9->setEnabled(true);
                        ui.label_10->setEnabled(true);
                        if(motorControl)
                        {
                            motorControl->stopJogging(LINEAR_MOTOR);
                        }
                    }
                }
            }
        }
    }
    else
        cameraMotorPose = motorPose;
    this->statusBar()->setVisible(true);
    this->statusBar()->showMessage(QString("Linear Pose:%1  Camear Pose:%2 Current Chip:%3").arg(linearMotorPose).arg(cameraMotorPose).arg(currentChip));
}

void MainWindow::warnUser(QString warning,int msgType)
{
    if(!msgBox)
    {
        msgBox = new QMessageBox;
    }
    if(msgType==QMessageBox::Warning)
    {
        msgBox->setWindowTitle("Warning");
        msgBox->setIcon(QMessageBox::Warning);
    }
    else if(msgType==QMessageBox::Critical)
    {
        msgBox->setWindowTitle("Error");
        msgBox->setIcon(QMessageBox::Critical);
    }
    else if(msgType==QMessageBox::Information)
    {
        msgBox->setWindowTitle("Information");
        msgBox->setIcon(QMessageBox::Information);
    }
    msgBox->setText(warning);
    msgBox->exec();
}

void MainWindow::on_actionJog_Left_toggled(bool checked)
{
    if(motorControl)
    {
        if(checked)
        {
            ui.actionJog_Right->setEnabled(false);
            ui.actionLeft->setEnabled(false);
            ui.actionRight->setEnabled(false);
            ui.actionNext_Chip->setEnabled(false);
            ui.actionPrevious_Chip->setEnabled(false);
            ui.actionLoading_Position->setEnabled(false);
            ui.actionHome->setEnabled(false);
            ui.actionEnd->setEnabled(false);
            ui.actionReset_Home->setEnabled(false);
            motorControl->setJoggingSpeed(LINEAR_MOTOR,ASG_PIT::settings().getJoggingSpeed());
            motorControl->jogMotorLeft(LINEAR_MOTOR);
        }
        else
        {
            ui.actionJog_Right->setEnabled(true);
            ui.actionJog_Left->setEnabled(true);
            ui.actionLeft->setEnabled(true);
            ui.actionRight->setEnabled(true);
            ui.actionNext_Chip->setEnabled(true);
            ui.actionPrevious_Chip->setEnabled(true);
            ui.actionLoading_Position->setEnabled(true);
            ui.actionHome->setEnabled(true);
            ui.actionEnd->setEnabled(true);
            ui.actionReset_Home->setEnabled(true);
            motorControl->stopJogging(LINEAR_MOTOR);
        }
    }
}

void MainWindow::on_actionJog_Right_toggled(bool checked)
{
    if(motorControl)
    {
        if(checked)
        {
            ui.actionJog_Left->setEnabled(false);
            ui.actionLeft->setEnabled(false);
            ui.actionRight->setEnabled(false);
            ui.actionNext_Chip->setEnabled(false);
            ui.actionPrevious_Chip->setEnabled(false);
            ui.actionLoading_Position->setEnabled(false);
            ui.actionHome->setEnabled(false);
            ui.actionEnd->setEnabled(false);
            ui.actionReset_Home->setEnabled(false);
            motorControl->setJoggingSpeed(LINEAR_MOTOR,ASG_PIT::settings().getJoggingSpeed());
            motorControl->jogMotorRight(LINEAR_MOTOR);
        }
        else
        {
            ui.actionJog_Left->setEnabled(true);
            ui.actionJog_Right->setEnabled(true);
            ui.actionLeft->setEnabled(true);
            ui.actionRight->setEnabled(true);
            ui.actionNext_Chip->setEnabled(true);
            ui.actionPrevious_Chip->setEnabled(true);
            ui.actionLoading_Position->setEnabled(true);
            ui.actionHome->setEnabled(true);
            ui.actionEnd->setEnabled(true);
            ui.actionReset_Home->setEnabled(true);
            motorControl->stopJogging(LINEAR_MOTOR);
        }
    }
}

void MainWindow::on_actionReset_Home_triggered()
{
    homePose = linearMotorPose;
    if(imageAcquisition)
    {
        imageAcquisition->setHomePose(homePose);
        warnUser(QString("Home location changed to the current position"),QMessageBox::Information);
    }
    if(encapDelamInspection)
        encapDelamInspection->setHomePose(homePose);
}

/*
  Goto Chip
  */
void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    qDebug()<<"Current Index is:"<<index;
    if(motorControl)
    {
        if(index<0 || index>10)
            return;
        linearMotorPose = index*ASG_PIT::settings().getChipStepSize() + homePose;
        autoFocus->reInitializeFocusWindow();
        motorControl->move(LINEAR_MOTOR,linearMotorPose);
    }
}

void MainWindow::on_stageSpeed_valueChanged(int value)
{
    qDebug()<<"Continous Sweeping speed changed to:"<<value;
    this->continousSweepSpeed = value;
    if(motorControl)
    {
            motorControl->setJoggingSpeed(LINEAR_MOTOR,value*1000);
    }
}

void MainWindow::on_continiousSweepBtn_released()
{
    if(ui.continiousSweepBtn->text()==QString("Stop Sweep"))
    {
        if(motorControl)
            motorControl->stopJogging(LINEAR_MOTOR);
        ui.continiousSweepBtn->setText("Start Sweep");
        continousSweeping = false;
    }
    else
    {
        if(ui.startChipCombo->currentIndex() == ui.endChipCombo->currentIndex())
        {
            warnUser(QString("Start chip should not be the same as the end chip"),QMessageBox::Warning);
            return;
        }
        if( ui.startChipCombo->currentIndex() < ui.endChipCombo->currentIndex())
        {
            sweepingStartPose = ui.startChipCombo->currentIndex()*ASG_PIT::settings().getChipStepSize() + homePose;
            sweepingEndPose = (ui.endChipCombo->currentIndex()+ 1)*ASG_PIT::settings().getChipStepSize() + homePose;
        }
        else
        {
            sweepingStartPose = (ui.startChipCombo->currentIndex()+1)*ASG_PIT::settings().getChipStepSize() + homePose;
            sweepingEndPose   = ui.endChipCombo->currentIndex()*ASG_PIT::settings().getChipStepSize() + homePose;
        }
        if(motorControl)
        {
            motorControl->move(LINEAR_MOTOR,sweepingStartPose);
        }
        QMessageBox messageBox;
        messageBox.setText("<b>The Linear stage is going to perform a continuous sweep from start to end</b>");
        messageBox.setInformativeText("Press Ok when you are ready or Cancel to abort");
        messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        messageBox.setDefaultButton(QMessageBox::Ok);
        int retVal = messageBox.exec();
        if(retVal == QMessageBox::Ok)
        {
            if(autoFocus)
            {
                autoFocus->setFocusStatus(false);
                ui.actionAuto_Focus->setChecked(false);
            }
            if(motorControl)
            {
                ui.continiousSweepBtn->setText("Stop Sweep");
                motorControl->setJoggingSpeed(LINEAR_MOTOR,ui.stageSpeed->value()*1000);
                if(sweepingStartPose>sweepingEndPose)
                {
                    motorControl->jogMotorRight(LINEAR_MOTOR);
                    continousSweeping = true;
                }
                else
                {
                    motorControl->jogMotorLeft(LINEAR_MOTOR);
                    continousSweeping = true;
                }
            }
        }
    }
}

void MainWindow::on_fillCurrentPose_released()
{
    ui.programmedPos->setText(QString("%1").arg(linearMotorPose));
}

void MainWindow::on_addProgPose_released()
{
    QStringList labels;
    labels << tr("Name") << tr("Position");
    ui.programmedPositionView->setHorizontalHeaderLabels(labels);
    int rowNum = ui.programmedPositionView->rowCount();
    qDebug()<<"Adding Row, num of current Rows:"<<rowNum;
    QTableWidgetItem *positionNameItem = new QTableWidgetItem(QString("Position %1").arg(rowNum));
    positionNameItem->setFlags(positionNameItem->flags() ^ Qt::ItemIsEditable);
    QTableWidgetItem *positionItem = new QTableWidgetItem(tr("%1").arg(ui.programmedPos->text().toInt()));
    positionItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    positionItem->setFlags(positionItem->flags() | Qt::ItemIsEditable);

    ui.programmedPositionView->insertRow(rowNum);
    ui.programmedPositionView->setItem(rowNum, 0, positionNameItem);
    ui.programmedPositionView->setItem(rowNum, 1, positionItem);
}

void MainWindow::on_saveProgrammedList_released()
{
    QString fileName = QString("%1/programmedPositions.csv").arg(QApplication::applicationDirPath());
    QFile programmedSetFile;
    QTextStream outStream;
    programmedSetFile.setFileName(fileName);
    if (!programmedSetFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to create/open Programmed List output file:"<<fileName;
        return;
    }
    outStream.setDevice(&programmedSetFile);
    for(int i=0;i<ui.programmedPositionView->rowCount();i++)
    {
        outStream<<ui.programmedPositionView->item(i,1)->text()<<"\n";
    }
    outStream.flush();
    programmedSetFile.close();
}

void MainWindow::on_traverseSteps_released()
{
    if(ui.traverseSteps->text()==QString("Traverse"))
    {
        if(ui.programmedPositionView->rowCount()==0)
        {
            warnUser(QString("Add positions to the table then start Again!!!"),QMessageBox::Warning);
            return;
        }
        ui.traverseSteps->setText("Cancel");
        if(autoFocus)
        {
            autoFocus->setFocusStatus(false);
            ui.actionAuto_Focus->setChecked(false);
        }
        ui.programmedPos->setEnabled(false);
        ui.addProgPose->setEnabled(false);
        ui.saveProgrammedList->setEnabled(false);
        ui.fillCurrentPose->setEnabled(false);
        ui.clearProgrammedList->setEnabled(false);
        ui.delayBetweenSteps->setEnabled(false);
        ui.label_9->setEnabled(false);
        ui.label_10->setEnabled(false);
        if(motorControl)
        {
            motorControl->setJoggingSpeed(LINEAR_MOTOR,ui.stageSpeed->value()*1000);
            QMessageBox messageBox;
            messageBox.setText("<b>The Linear stage is going to perform a continuous traverse through the programmed positions</b>");
            messageBox.setInformativeText("Press Ok when you are ready or Cancel to abort");
            messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            messageBox.setDefaultButton(QMessageBox::Ok);
            int retVal = messageBox.exec();
            if(retVal == QMessageBox::Ok)
            {
                steppingMotion = true;
                stepIndex = 0;
                if(linearMotorPose>ui.programmedPositionView->item(stepIndex,1)->text().toInt())
                {
                    motorControl->jogMotorRight(LINEAR_MOTOR);
                    traversingDirection = RIGHT;
                }
                else
                {
                    motorControl->jogMotorLeft(LINEAR_MOTOR);
                    traversingDirection = LEFT;
                }
            }
        }
    }
    else
    {
        ui.traverseSteps->setText("Traverse");
        ui.programmedPos->setEnabled(true);
        ui.addProgPose->setEnabled(true);
        ui.saveProgrammedList->setEnabled(true);
        ui.fillCurrentPose->setEnabled(true);
        ui.clearProgrammedList->setEnabled(true);
        ui.delayBetweenSteps->setEnabled(true);
        ui.label_9->setEnabled(true);
        ui.label_10->setEnabled(true);
        steppingMotion = false;
        if(motorControl)
        {
            motorControl->stopJogging(LINEAR_MOTOR);
        }
    }
}

void MainWindow::on_clearProgrammedList_released()
{
    for(int i=(ui.programmedPositionView->rowCount()-1);i>=0;i--)
    {
        ui.programmedPositionView->removeRow(i);
    }
}

void MainWindow::on_calibrateLightIntensity_released()
{
    if(ui.calibrateLightIntensity->isChecked())
    {
        calibtationStep = 0;
        calibratingIntensity = true;
        ui.calibStepLabel->setText("");
        ui.calibStepLabel->setHidden(false);
        ui.toolBar->setEnabled(false);
        ui.groupBox_13->setEnabled(false);
        ui.groupBox_14->setEnabled(false);
        ui.groupBox_17->setEnabled(false);
        ui.groupBox_18->setEnabled(false);
        ui.groupBox_19->setEnabled(false);
        keysDisabled = true;
        ui.tabWidget->setTabEnabled(1,false);
        gotoHome();
    }
    else
    {
        ui.reAnalyse->setEnabled(false);
        calibratingIntensity = false;
        ui.calibStepLabel->setHidden(true);
        ui.toolBar->setEnabled(true);
        ui.groupBox_13->setEnabled(true);
        ui.groupBox_14->setEnabled(true);
        ui.groupBox_17->setEnabled(true);
        ui.groupBox_18->setEnabled(true);
        ui.groupBox_19->setEnabled(true);
        ui.tabWidget->setTabEnabled(1,true);
        keysDisabled = false;
        warnUser(QString("Calibration Finished."),QMessageBox::Information);
    }
}

void MainWindow::on_reAnalyse_released()
{
    if(frame)
    {
        try
        {
            encapsulationCoverage->processImage(cvCloneImage(frame));
            if(calibtationStep<11)
            {
                if(ASG_PIT::settings().getAutoSkip2Next())
                    gotoNextChip();
            }
            else
            {
                ui.calibrateLightIntensity->setChecked(false);
                warnUser(QString("Calibration Finished."),QMessageBox::Information);
            }
            ui.reAnalyse->setEnabled(false);
        }
        catch(EncapException &encap)
        {
            warnUser(QString("%1. Adjust the light intensity and re-analyse.").arg(encap.what()),QMessageBox::Warning);
            ui.reAnalyse->setEnabled(true);
        }
    }
}

void MainWindow::on_autoSkip2Next_toggled(bool checked)
{
    ASG_PIT::settings().setAutoSkip2Next(checked);
    ui.nextStep->setEnabled(!checked);
}

void MainWindow::on_nextStep_released()
{
    if(calibtationStep<11)
    {
        gotoNextChip();
    }
}

void MainWindow::on_encapOnlineAnalysis_released()
{
    if(ui.encapOnlineAnalysis->text() == "Start Online Analysis")
    {
        if(imageAcquisition)
        {
            imageAcquisition->enableOnlineEncapAnalysis(true);
            if(triggerAcquisition())
            {
                if(cameraGrabber)
                {
                    cameraGrabber->enableAutoShutter(false);
                    cameraGrabber->enableWhiteBalance(false);
                    ui.autoShutterSpeed->setChecked(false);
                }
                ui.encapOnlineAnalysis->setText("Stop Online Analysis");
                ui.actionAcquisition->setEnabled(false);
                ui.tabWidget->widget(1)->setEnabled(false);
                ui.groupBox_14->setEnabled(true);
                ui.groupBox_16->setEnabled(false);
                ui.groupBox_13->setEnabled(false);
                ui.groupBox_20->setEnabled(false);
            }
        }
    }
    else
    {
        if(imageAcquisition)
        {
            imageAcquisition->enableOnlineEncapAnalysis(false);
            triggerAcquisition();
            ui.encapOnlineAnalysis->setText("Start Online Analysis");
            ui.tabWidget->widget(1)->setEnabled(true);
            ui.groupBox_16->setEnabled(true);
            ui.groupBox_13->setEnabled(true);
            ui.groupBox_20->setEnabled(true);
            if(encapsulationCoverage)
                encapsulationCoverage->finishProcessing();
        }
    }
}

void MainWindow::on_shutterSpeed_sliderReleased()
{
    if(cameraGrabber)
    {
        cameraGrabber->setShutterSpeed(ui.shutterSpeed->value());
    }
}

void MainWindow::on_autoShutterSpeed_released()
{
    if(ui.autoShutterSpeed->isChecked())
    {
        if(cameraGrabber)
            cameraGrabber->enableAutoShutter(true);
        ui.shutterSpeed->setEnabled(false);
        ui.label_11->setEnabled(false);
        ui.label_12->setEnabled(false);
    }
    else
    {
        if(cameraGrabber)
            cameraGrabber->enableAutoShutter(false);
        ui.shutterSpeed->setEnabled(true);
        ui.label_11->setEnabled(true);
        ui.label_12->setEnabled(true);
    }
}

void MainWindow::on_shutterSpeedChanged(int value)
{
    ui.shutterSpeed->setValue(value);
}

void MainWindow::on_browseImageSource_released()
{
    QString image;
    QStringList files = QFileDialog::getOpenFileNames(NULL, tr("Chose the Scanned Pattern(s)"),
                                         ASG_PIT::settings().getLastDirPath(),
                                         tr("Images (*.tif)"));
    if(files.size()>0)
    {
        addFiles2TableView(files);
        ASG_PIT::settings().setLastDirPath(QFileInfo(files[0]).path());
    }
}

void MainWindow::on_ledLightIntensitySlider_sliderReleased()
{
    if(ledLightingController)
        ledLightingController->setLightingCurrent(ui.ledLightIntensitySlider->value());
}

void MainWindow::on_whiteBalanceChkBox_released()
{
    if(ui.whiteBalanceChkBox->isChecked())
    {
        if(cameraGrabber)
            cameraGrabber->enableWhiteBalance(true);
    }
    else
    {
        if(cameraGrabber)
            cameraGrabber->enableWhiteBalance(false);
    }
}

void MainWindow::on_actionRecordVideo_triggered(bool checked)
{
    if(ui.actionRecordVideo->text()==QString("Record"))
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Video File"),
                                                        ASG_PIT::settings().getLastDirPath(),
                                                        tr("Images (*.avi)"));
        if(!fileName.isEmpty())
        {
            emit startRecording(fileName);
            ui.actionRecordVideo->setIcon(QIcon(":/stop.png"));
            ui.actionRecordVideo->setText("Stop");
        }
    }
    else
    {
        ui.actionRecordVideo->setIcon(QIcon(":/record.png"));
        ui.actionRecordVideo->setText("Record");
        emit stopRecording();
    }
}

void MainWindow::on_availableCamerasCB_currentIndexChanged(QString cameraSelected)
{
    if(cameraGrabber)
    {
        qDebug()<<"Camera Selected: "<<cameraSelected;
        cameraGrabber->setActiveCamera(cameraSelected);
    }
}

void MainWindow::on_ledLightIntensityMeasurement_valueChanged(int value)
{
    if(ledLightingController)
        ledLightingController->setLightingCurrent(value);
}

void MainWindow::on_actionDelam_Inspection_toggled(bool)
{

}

void MainWindow::on_actionDelam_Inspection_triggered()
{
    bool retVal = false;
    if(encapDelamInspection->isInspecting())
    {
        ui.actionDelam_Inspection->setText("Delam Inspection");
        ui.printHeadCode->setEnabled(true);
        ui.printHeadName->setVisible(true);
        ui.printHeadCode->setVisible(true);
        encapDelamInspection->setInspection(false);
        ui.progressBar->setVisible(false);
        ui.acqProgress->setVisible(false);
        ui.tabWidget->widget(1)->setEnabled(true);
        ui.groupBox_16->setEnabled(true);
        ui.groupBox_14->setEnabled(true);
        ui.groupBox_13->setEnabled(true);
        ui.groupBox_20->setEnabled(true);
        if(cameraSource == Camera::NONE)
        {
            disableCameraRelatedOptions();
        }
        else
        {
            QList<QWidget *> widgets = qFindChildren<QWidget *>(ui.toolBar);
            foreach (QWidget *w, widgets)
                w->setEnabled(true);
        }
        ui.actionOnline_Analysis->setEnabled(false);
    }
    else
    {
        if(getPrintHeadCode())
        {
            ui.tabWidget->setCurrentIndex(1);
            QList<QWidget *> widgets2 = qFindChildren<QWidget *>(ui.spaTab);
            foreach (QWidget *w, widgets2)
                w->setEnabled(false);
            QList<QWidget *> widgets1 = qFindChildren<QWidget *>(ui.toolBar);
            foreach (QWidget *w, widgets1)
                w->setEnabled(false);
            ui.progressBar->setVisible(true);
            ui.acqProgress->setVisible(true);
            ui.printHeadCode->setEnabled(false);
            ui.printHeadName->setVisible(true);
            ui.printHeadCode->setVisible(true);
            //ui.tabWidget->widget(1)->setEnabled(false);
            ui.groupBox_16->setEnabled(false);
            ui.groupBox_14->setEnabled(false);
            ui.groupBox_13->setEnabled(false);
            ui.groupBox_20->setEnabled(false);
            ui.acqProgress->setText("Acquisition Progress:");
            ui.actionDelam_Inspection->setText("Stop Delam Inspection");
            gotoFocusPose();
            encapDelamInspection->setInspection(true);
            ui.actionDelam_Inspection->setEnabled(true);
            ui.actionTimed_Acquisition->setEnabled(true);
            retVal = true;
        }
        else
        {
            warnUser("Invalid printhead code, try again.",QMessageBox::Information);
            retVal = false;
        }
    }
}

void MainWindow::on_delamTable_cellDoubleClicked(int row, int column)
{
    if(row>=0 && row<imagePositions.size())
    {
       encapDelamInspection->setNextPositionIndex(row);
    }
}

void MainWindow::on_saveInspectionBtn_released()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Results Into"),
                                                    ASG_PIT::settings().getLastDirPath(),
                                                    tr("Images (*.csv)"));
    if(fileName.isEmpty())
    {
        warnUser("You have to select a file location First",QMessageBox::Warning);
    }
    ASG_PIT::settings().setLastDirPath(QFileInfo(fileName).absolutePath());

    QFile inspectioResultsFile;
    QTextStream outStream;
    inspectioResultsFile.setFileName(fileName);
    if (!inspectioResultsFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to create/open Programmed List output file:"<<fileName;
        warnUser(QString("Failed to create/open Programmed List output file:")+fileName,QMessageBox::Warning);
        return;
    }
    outStream.setDevice(&inspectioResultsFile);
    outStream<<"Image Position, Status, Die, DieSection\n";
    for(int i=0;i<ui.delamTable->rowCount();i++)
    {
        QComboBox *combo = (QComboBox *)(ui.delamTable->cellWidget(i,1));
        outStream<<ui.delamTable->item(i,0)->text()<<","<<combo->currentText()<<","<<ui.delamTable->item(i,2)->text()<<","<<ui.delamTable->item(i,3)->text()<<"\n";
    }
    outStream.flush();
    inspectioResultsFile.close();
}

void MainWindow::on_resetInspectionBtn_released()
{
    for(int i=0;i<ui.delamTable->rowCount();i++)
    {
        QComboBox *combo = (QComboBox *)(ui.delamTable->cellWidget(i,1));
        combo->clear();
        combo->addItem("No-Delamination");
        combo->addItem("Delamination");
    }
}

void MainWindow::on_shutterSpeed_valueChanged(int value)
{
    if(cameraGrabber)
    {
        cameraGrabber->setShutterSpeed(value);
    }
}

void MainWindow::on_addDelamination_released()
{
    /*
    ui.delamTable->setEnabled(true);
    imagePositions = _imagePositions;
    int sectionsPerDie = ASG_PIT::settings().getInspectionSectionsPerDie();
    for(int i=0;i<imagePositions.size();i++)
    {
        QTableWidgetItem *phPosition = new QTableWidgetItem(QString("%1").arg(i));
        phPosition->setFlags(phPosition->flags() ^ Qt::ItemIsEditable);
        phPosition->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        QComboBox* combo = new QComboBox();
        combo->addItem("No-Delamination");
        combo->addItem("Delamination");
        combo->setCurrentIndex(0);

        int chip = (int)floor(imagePositions[i]/float(ASG_PIT::settings().getChipStepSize()));
        int chipSection = (int)floor(imagePositions[i] - ASG_PIT::settings().getChipStepSize()*chip)/float(ASG_PIT::settings().getChipStepSize()/float(sectionsPerDie));

        QTableWidgetItem *dieNumber = new QTableWidgetItem(QString("%1").arg(chip));
        dieNumber->setFlags(dieNumber->flags() ^ Qt::ItemIsEditable);
        dieNumber->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        QTableWidgetItem *dieSectionNumber = new QTableWidgetItem(QString("%1").arg(chipSection));
        dieSectionNumber->setFlags(dieSectionNumber->flags() ^ Qt::ItemIsEditable);
        dieSectionNumber->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);


        ui.delamTable->insertRow(i);
        ui.delamTable->setItem(i, 0, phPosition);
        ui.delamTable->setCellWidget(i,1,combo);
        ui.delamTable->setItem(i, 2, dieNumber);
        ui.delamTable->setItem(i, 3, dieSectionNumber);
    }
    */
    // labels<<tr("Die")<<tr("Section")<<tr("Ch")<<tr("Row")<<tr("RD");
    int i = ui.delamTable->rowCount();
    QTableWidgetItem *img = new QTableWidgetItem;
    IplImage *imageTmp = cvCloneImage(frame);
    if(frame && frame->imageData)
    {
        if(frame->nChannels==3)
        {
            imageTmp = cvCloneImage(frame);
        }
        else
        {
            imageTmp = cvCreateImage(cvGetSize(frame),frame->depth,3);
            cvCvtColor(frame,imageTmp,CV_GRAY2RGB);
        }
        QImage imageFrame = QImage((const unsigned char *) imageTmp->imageData,imageTmp->width,imageTmp->height,QImage::Format_RGB888).scaledToHeight(50);
        cvReleaseImage(&imageTmp);
        img->setData(1,qVariantFromValue(imageFrame));
    }
    else
        img->setData(1,qVariantFromValue(QImage(":/SBR_logo")));

    QTableWidgetItem *die              = new QTableWidgetItem(QString("%1").arg(currentDie));
    QTableWidgetItem *section          = new QTableWidgetItem(QString("%1").arg(currentDieSection));
    QTableWidgetItem *roofsDelaminated = new QTableWidgetItem("");
    if((linearMotorPose%ASG_PIT::settings().getChipStepSize()) < ASG_PIT::settings().getImageStepSize()/2.0 )
    {
        section->setText("DDT");
    }
    QComboBox* channelCombo = new QComboBox();
    channelCombo->addItem("0");
    channelCombo->addItem("1");
    channelCombo->addItem("2");
    channelCombo->addItem("3");
    channelCombo->addItem("4");
    channelCombo->setCurrentIndex(0);

    QComboBox* rowCombo = new QComboBox();
    rowCombo->addItem("1");
    rowCombo->addItem("2");
    rowCombo->setCurrentIndex(0);

    ui.delamTable->insertRow(i);
    ui.delamTable->setItem(i, 0, die);
    ui.delamTable->setItem(i, 1, section);
    ui.delamTable->setCellWidget(i, 2, channelCombo);
    ui.delamTable->setCellWidget(i, 3, rowCombo);
    ui.delamTable->setItem(i, 4, roofsDelaminated);
    ui.delamTable->setItem(i, 5, img);
    ui.delamTable->resizeRowsToContents();
    ui.delamTable->resizeColumnsToContents();

    QSqlQuery query;
    query.prepare("INSERT INTO die_delam (printhead_id, die, section, channel, row, num_delams) "
                  "VALUES (?, ?, ?, ?, ?, ?)");

    query.bindValue(0, "A00112233");
    query.bindValue(1, 0);
    query.bindValue(2, "DDT");
    query.bindValue(3, 1);
    query.bindValue(4, 2);
    query.bindValue(5, 1);
    if(!query.exec())
        qDebug()<<"Error:"<<defaultDB->lastError();

    /*
    QModelIndex index = ui.view->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui.view->model();

    if(!model->insertRow(index.row()+1, index.parent()))
        return;
    for (int column = 0; column < model->columnCount(index.parent()); ++column)
    {
        QModelIndex child = model->index(index.row()+1, column, index.parent());
        model->setData(child, QVariant("[No data]"), Qt::EditRole);
    }
    */
}
