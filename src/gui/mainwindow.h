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
#include "cv.h"
#include "highgui.h"
#include "QOpenCVWidget.h"
#include "scenevisualization.h"
#include "templatemodel.h"
#include "messagelogger.h"
#include "version.h"
#include "acdsettings.h"
#include "settingsdialog.h"
#include "cameragrabber.h"
#include "splashscreen.h"
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
    void getFreshDisplayFrame();
    void enableMask(bool enable);
signals:
    void startRecording(QString);
    void stopRecording();
public slots:
    void about();
    void helpManual();
    void showProgress(QString label,int progress);
    void hideProgress();
    void showSettingsDialog();
    void warnUser(QString warning,int);
    void processingImage(QString);
    void processingImage(QString,int);
    virtual void keyPressEvent( QKeyEvent *e );
    void saveCurrentImage();
    void showStatusBarMsg(QString msg);
    void recieveFrame(IplImage *);
    void camerasFound(QVector<QString>);
private:
    QScriptEngine scriptEngine;
    void addFiles2TableView(QStringList files);
    void disableAllWidgetsBut(QList<QWidget *> toBeEnabled);
    void disableCameraRelatedOptions();
    void resetPlot();
    QStringList traverseDirTree(QString rootDir,QRegExp rx);
    void setEnableMotorRelatedOptions(bool enable);
    void setup2DPlotting();
    ImageViewer *imageViewer;
    SettingsDialog *settingsDialog;
    Ui::MainWindowClass ui;
    CameraGrabber *cameraGrabber;
    IplImage * frame,* displayFrame, *offlineImage;
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
};

#endif // MAINWINDOW_H
