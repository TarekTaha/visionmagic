#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QTextStream>
#include <QDateTime>
#include <QRadioButton>
#include <QCheckBox>
#include <QtWebKit>
#include <QtScript>
#include <QTableWidgetItem>
#include <qwt_plot_renderer.h>
#include <qwt_painter.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_canvas.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_math.h>
#include <qwt_color_map.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include <qwt_counter.h>
#include <qwt_picker_machine.h>
#include "ui_mainwindow.h"
#include "arthurwidgets.h"
#include "cv.h"
#include "highgui.h"
#include "QOpenCVWidget.h"
#include "motorcontrol.h"
#include "autofocus.h"
#include "imageacquisition.h"
#include "imageanalysis.h"
#include "scenevisualization.h"
#include "templatemodel.h"
#include "messagelogger.h"
#include "version.h"
#include "acdsettings.h"
#include "settingsdialog.h"
#include "dnmapprobabilisticgenerator.h"
#include "dnmapgeneratordialog.h"
#include "ledlightingcontroller.h"
#include "comportdetection.h"
#include "configparser.h"
#include "nozzlehealthviewer.h"
//#include "scannerInterface.h"
#include "encapsulationcoverage.h"
#include "calibrationdialog.h"
#include "cameragrabber.h"
#include "splashscreen.h"
#include "encapdelaminspection.h"
#include "imageviewer.h"

enum {NOZZEL, EDGE_START,EDGE_END};
enum Direction{LEFT,RIGHT};
enum Graphs{ANGLE_VS_TIME,AREA_VS_TIME,REGULARITY_VS_TIME,ROUNDNESS_VS_TIME};
class Messagelogger;
class QSqlDatabase;
class MyZoomer: public QwtPlotZoomer
{
public:
    MyZoomer(QwtPlotCanvas *canvas):
        QwtPlotZoomer(canvas)
    {
        setTrackerMode(AlwaysOn);
    }

    virtual QwtText trackerTextF(const QPointF &pos) const
    {
        QColor bg(Qt::white);
        bg.setAlpha(200);

        QwtText text = QwtPlotZoomer::trackerTextF(pos);
        text.setBackgroundBrush( QBrush( bg ));
        return text;
    }
};
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(SplashScreen *splashScreen,QWidget *parent = 0);
    ~MainWindow();
    bool getPrintHeadCode();
    void initializeFireCamera();
    void getFreshDisplayFrame();
    void finishedAddingTemplate();
    bool isTemplateSouceAvailable();
    void copyOldTemplates(QString source);
    void populateSPAGui();
    void setupImageAnalysis();
    void setupOfflineAnalysisTab();
    void loadProgrammedList();
    void enableMask(bool enable);
signals:
    void setAnalysisPause(bool);
    void propagateNozzelSelected(const QPoint &locationPressed,int );
    void scriptSignalTest();
    void triggerScriptStart();
    void startRecording(QString);
    void stopRecording();
public slots:
    void changeFocusedTab(int);
    void changeFocusStatus();
    void changeFineFocusStatus();
    void detectComPortsDialog();
    void gotoEnd();
    void gotoFocusPose();
    void gotoHome();
    bool triggerAcquisition();
    void acquisitionEnded(bool encapEnabled);
    void updateMotorPose(int motorId,int motorPose);
    void motorMovingChanged(int motorId,bool,int motorPose);
    void isFocused(bool);
    void savingImages();
    void startOfflineAnalysis();
    void analysisFinished();
    void motorsInitialized(bool);
    void matchingValueChanged(int);
    void dnmapGenerationChanged(int);
    void triggerAnalysis();
    void nozzelSelected(const QPoint &,int);
    void drawTemplateMask(const QPoint &);
    void onlineAnalysisChanged();
    void addNozzelTemplateBtnClicked();
    void addEdgeStartTemplateBtnClicked();
    void addEdgeEndTemplateBtnClicked();
    void about();
    void helpManual();
    void getOfflineImage(IplImage *);
    void printHeadCodeChanged(const QString&);
    void showProgress(QString label,int progress);
    void hideProgress();
    void triggerAnalysisType();
    void showLog();
    void changeImagesSource();
    void changeImagesSource(int);
    void clearNozzelTempList();
    void clearEdgeStartTempList();
    void clearEdgeEndTempList();
    void closeDialog();
    void changeNozzelDisplay();
    void showSettingsDialog();
    void analyseSPA();
    void selectSPAImage();
    void specifyDNMapSuffix(QString);
    void showProbabilisticDNMap();
    void changeSPASettings();
    void specifySPAResultsSuffix(QString);
    void extractContours();
    void warnUser(QString warning,int);
    void ledLightingCurrentValueChanged();
    void processingImageAnalysis();
    void finishedProcessingImageAnalysis();
    void processingImage(QString);
    void processingImage(QString,int);
    void startComDetection();
    void initiateLedLighting(QString);
    void initiateMotors(QString);
    void comDetectionStatus(bool,bool);
    void scanPattern();
    void spaFinished();
    void mesBottonPressed();
    virtual void keyPressEvent( QKeyEvent *e );
    void generateBorealisTNFPattern();
    QRectF getRegionOfInterest(QString);
    void showImageDetails(QTableWidgetItem*);
    void showDetails(QTableWidgetItem*);
    void clearTableSelection();
    void extractSPAImagesFromDirTree();
    void gotoLoadingPosition();
    void goLeft();
    void gotoNextChip();
    void gotoPrevChip();
    void goRight();
    void refocus();
    void detectTriangles();
    void saveCurrentImage();
    void analyseLastAcquiredImageSet();
    void startOfflineAnalysis(QString dir);
    void runScriptEngine();
    void runTestScript();
    void startTimedAquisition();
    void edgeMatchingValueChanged(int value);
    void chartVersionChanged();
    void setEncapsulationImagesSource();
    void processEncapsulationImages();
    void secondMatchingSliderValueChanged(int value);
    void secondMatchLevelToggled(bool);
    void showStatusBarMsg(QString msg);
    void setPlumpingOder(QString);
    void recieveFrame(IplImage *);
    void camerasFound(QVector<QString>);
    void on_shutterSpeedChanged(int);
    void finishedTraversingProgrammedSet();
    void runScript();
    void newImagePositions(QVector<int>);
    void locationChanged(int,int,int);
private:
    QScriptEngine scriptEngine;
    void addFiles2TableView(QStringList files);
    void disableAllWidgetsBut(QList<QWidget *> toBeEnabled);
    void disableCameraRelatedOptions();
    void resetPlot();
    QStringList traverseDirTree(QString rootDir,QRegExp rx);
    void setEnableMotorRelatedOptions(bool enable);
    void setup2DPlotting();
    void setupNHAViewerTab();
    ImageViewer *imageViewer;
    SettingsDialog *settingsDialog;
    ComPortDetection * comPortDetection;
    ComPortDetectionThread * comPortDetectionThread;
    LedLightingController * ledLightingController;
    DNMapGeneratorDialog * dnMapGeneratorDialog;
    EncapDelamInspection * encapDelamInspection;
    Ui::MainWindowClass ui;
    AutoFocus *autoFocus;
    MotorControl *motorControl;
    ImageAcquisition *imageAcquisition;
    ImageAnalysis * imageAnalysis;
    CameraGrabber *cameraGrabber;
    IplImage * frame,* displayFrame, *offlineImage;
    NozzleHealthViewer * nozzleHealthViewer;
    EncapsulationCoverage *encapsulationCoverage;
    QLabel *matchValue,*secondMatchingValue,*edgeTempMatchingValue;
    QPushButton *addNozzelTemplateBtn,*addEdgeStartTemplateBtn,*addEdgeEndTemplateBtn;
    QPushButton *clearNozzelTempListBtn,*clearEdgeStartTempListBtn,*clearEdgeEndTempListBtn, *mesPushDataBtn;
    QRadioButton * manualAnalysis, *semiAutoAnalysis, *autoAnalysis, *acdImagesSource, *acdImagesSourceSmall, *zebraFishImagesSource,* novoImagesSource,*thunderBirdImageSource;
    QCheckBox *showDeadNozzels, *showClearNozzels, *showNumbers, *generateDNMapChkBox,*enableSecondLevelMatch;
    QGroupBox *analysisSetting,*analysisTypeGrp,*analysisDisplay,*matchPercentageGrp,*nozzelTempGrp,*imagesSourceGrp,*displayNozzelsGrp,*mesGrp;
    QTabWidget *templatesTab;
    QSlider *secondMatchingSlider ,*matchingSlider,*edgeTempMatchSlider;
    int linearMotorPose,homePose;
    int cameraMotorPose,currentChip,distanceBetweenChips,analysisType,cameraSource;
    SceneVisualization *sceneRendering;
    QTime timer;
    QTimer acquisitionTimer;
    QPoint tempSize;
    QMessageBox *msgBox;
    QMutex mutex;
    QString onlineAnalysisPath,nozzelTempPath,edgeStartPath,edgeEndPath;
    MessageLogger *logger;
    bool useOpenCVGrabber,openCVInitialized,AVTInitialized,analysingOffline,threadsInitialized,focused,analysingOnline,addingTemplate,repeatAnalysis,
         generateDNMap,analysingImage,motorMoving,scriptEngineStarted,testLoopStarted,timedAcqusition,alreadyGotHeadCode,continousSweeping,calibratingIntensity,
         focusWindowSet,steppingMotion;
    int templateType,calibtationStep,keysDisabled,stepIndex,traversingDirection,graph2Plot,currentDie,currentDieSection;
    static const int totalCalibrationSteps = 11;
    TemplateModel *nozzelTempModel,*edgeStartTempModel,*edgeEndTempModel;
    int imagesSource,continousSweepSpeed,sweepingStartPose,sweepingEndPose,currentImagePosition;
    QwtPlotCurve *angle1Data,*angle2Data,*averageAngleData,*cumilativeAngleData;
    QVector <double *> plotDataPointers;
    QVector<int> imagePositions;
    QSqlDatabase * defaultDB;
private slots:
    void on_actionRecordVideo_triggered(bool checked);
    void on_whiteBalanceChkBox_released();
    void on_ledLightIntensitySlider_sliderReleased();
    void on_browseImageSource_released();
    void on_autoShutterSpeed_released();
    void on_shutterSpeed_sliderReleased();
    void on_encapOnlineAnalysis_released();
    void on_nextStep_released();
    void on_autoSkip2Next_toggled(bool checked);
    void on_reAnalyse_released();
    void on_calibrateLightIntensity_released();
    void on_clearProgrammedList_released();
    void on_traverseSteps_released();
    void on_saveProgrammedList_released();
    void on_addProgPose_released();
    void on_fillCurrentPose_released();
    void on_continiousSweepBtn_released();
    void on_stageSpeed_valueChanged(int value);
    void on_comboBox_currentIndexChanged(int index);
    void on_actionReset_Home_triggered();
    void on_actionJog_Right_toggled(bool );
    void on_actionJog_Left_toggled(bool );
    void on_availableCamerasCB_currentIndexChanged(QString cameraSelected);
    void on_ledLightIntensityMeasurement_valueChanged(int value);
    void on_actionDelam_Inspection_toggled(bool arg1);
    void on_actionDelam_Inspection_triggered();
    void on_delamTable_cellDoubleClicked(int row, int column);
    void on_saveInspectionBtn_released();
    void on_resetInspectionBtn_released();
    void on_shutterSpeed_valueChanged(int value);
    void on_addDelamination_released();
};

#endif // MAINWINDOW_H
