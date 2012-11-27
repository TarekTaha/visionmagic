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
    cameraGrabber(NULL),
    frame(NULL),
    displayFrame(NULL),
    offlineImage(NULL),
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
    //Encap
    ui.cvWidget->setEnableROISelection(true);
    ui.actionOnline_Analysis->setVisible(true);
    ui.actionOnline_Analysis->setEnabled(true);
    cameraGrabber->setupGrabbingSource();
    cameraGrabber->start();
    QStringList labels;

    if(ASG_PIT::settings().getSPAChartVersion()==0.4)
        ui.chartVersion4Radio->setChecked(true);
    else
        ui.chartVersion6Radio->setChecked(true);
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
    if(sceneRendering)
        delete sceneRendering;
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
    QString aboutInfo = QString("<b>ACD</b> %1  Built on %2 at %3<br/>Autonomous Contamination Detection,").arg(APP_VERSION_LONG,QLatin1String(__DATE__),QLatin1String(__TIME__))
                        .append(" is a tool developed to facilitate print head inspection and fault ")
                        .append("detection. Developed by: <b>SBR-ASG</b>. For help contact <b>Tarek Taha</b> or <b>David Worboys</b>.");
        QMessageBox::about(this, tr("About ACD"),aboutInfo);
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
         ui.actionRecordVideo->setEnabled(true);
     }
    ui.availableCamerasCB->clear();
    for(int i=0;i<cameraNames.size();i++)
    {
        ui.availableCamerasCB->addItem(cameraNames[i]);
    }
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

void MainWindow::keyPressEvent( QKeyEvent *e )
{
    if(keysDisabled)
        return;
    switch( e->key() )
    {
    case Qt::Key_A:
        break;
    case Qt::Key_B:

        break;
    case Qt::Key_C:
        break;
    case Qt::Key_D:
        break;
    case Qt::Key_K:
        break;
    case Qt::Key_N:
        break;
    case Qt::Key_R:
        break;
    case Qt::Key_W:
        break;
    case Qt::Key_S:
        break;
    case Qt::Key_Home:
        break;
    case Qt::Key_End:
        break;
    case Qt::Key_Space:
            break;
    case Qt::Key_Up:
        break;
    case Qt::Key_Down:
        break;
    case Qt::Key_Escape:
        break;
    case Qt::Key_X:
        break;
    }
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

void MainWindow::saveCurrentImage()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image Into"),
                                                    ASG_PIT::settings().getLastDirPath(),
                                                    tr("Images (*.png *.bmp *.jpg)"));
    if(this->frame && !fileName.isEmpty())
        cvSaveImage(fileName.toAscii(),frame);
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
                settingsDialog->exec(0);
        }
}

void MainWindow::showStatusBarMsg(QString msg)
{
    ui.statusbar->setVisible(true);
    ui.statusbar->showMessage(msg);
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
