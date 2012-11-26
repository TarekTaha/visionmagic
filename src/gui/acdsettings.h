/*
 * acdsettings.h
 *
 *  Created on: 20/02/2009
 *      Author: ttaha
 */
#ifndef ACDSETTINGS_H
#define ACDSETTINGS_H
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include "settings.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
/**
 * Note we operate on separate QSettings objects always to be thread-safe.
 * Note the silly classes for each group are because you can't copy QSettings objects
 * so we couldn't just return one through a function, which would be neater.
 */
enum
{
    ACD,
    PIT_SMALL_TEMPS,
    THUNDERBIRD,
    NOVO
};
#ifdef WIN32
    class HklmSettings : public QSettings
    {
    public:
        HklmSettings( QObject* parent = 0 ) :
                QSettings( "HKEY_LOCAL_MACHINE\\Software\\SBR\\ISG\\ACD",
                           QSettings::NativeFormat,
                           parent )
        {}
    };

    /** Due to historical reasons, we store windows settings "wrongly", but
      * migrate settings on other platforms to the "correct" location. To make
      * the code all the same though we use this class below and macro it to
      * the QSettings token */
    class HkcuSettings : public QSettings
    {
    public:
        HkcuSettings( QObject* parent = 0 ) :
            QSettings( "HKEY_CURRENT_USER\\Software\\SBR\\ISG\\ACD",
                       QSettings::NativeFormat,
                       parent )
        {}
    };

    // set QSettings to HkcuSettings
    #define QSettings HkcuSettings
#else
    // set HklmSettings to QSettings
    typedef QSettings HklmSettings;
#endif // WIN32
/**
 * Settings that apply to all applications we are likely to write. Not stored
 * on a per-user basis.
 */
class CameraSettings : public HkcuSettings
{
public:
    CameraSettings()
    {
        beginGroup( "Camera" );
    }
};


class MotorSettings : public HkcuSettings
{
public:
    MotorSettings()
    {
        beginGroup( "Motor" );
    }
};

class FocusSettings : public HkcuSettings
{
public:
    FocusSettings()
    {
        beginGroup( "Focus" );
    }
};

class AcquisitionSettings : public HkcuSettings
{
public:
    AcquisitionSettings()
    {
        beginGroup( "Acquisition" );
    }
};

class AnalysisSettings : public HkcuSettings
{
public:
    AnalysisSettings()
    {
        beginGroup( "Analysis" );
    }
};

class LedLightingSettings : public HkcuSettings
{
public:
    LedLightingSettings()
    {
        beginGroup( "LedLighting" );
    }
};

class MESSettings : public HkcuSettings
{
public:
    MESSettings()
    {
        beginGroup( "MES" );
    }
};

class SPASettings : public HkcuSettings
{
public:
    SPASettings()
    {
        beginGroup( "SPA" );
    }
};

class EncapsulationSettings : public HkcuSettings
{
public:
    EncapsulationSettings()
    {
        beginGroup( "Encapsulation" );
    }
};

class ContactAngleMeasurement : public HkcuSettings
{
public:
    ContactAngleMeasurement()
    {
        beginGroup( "ContactAngleMeasurement" );
    }
};

class DelaminationInspection : public HkcuSettings
{
public:
    DelaminationInspection()
    {
        beginGroup( "DelaminationInspection" );
    }
};


class GlobalSettings : public HkcuSettings
{
public:
    GlobalSettings()
    {
        beginGroup( "Global" );
    }
};

class PITSettings : public AppSettings<QSettings>
{
    Q_OBJECT
    PITSettings( QObject* parent );
    friend PITSettings &ASG_PIT::settings();
    // NOTE private as app object has control, the LoginWidget friend is
    // unsavoury but the code is safe as long as SettingsDialog is the only
    // object that calls save( false ) on it. This sucks on many levels.
    friend class ACDApplication;
public:
    QByteArray splitterState() const { return QSettings().value( "splitterState" ).toByteArray(); }
    void setSplitterState( QByteArray state ) { QSettings().setValue( "splitterState", state ); }

    int sidebarWidth() const
    {
        return QSettings().value( "sidebarWidth", 190 ).toInt();
    }
    void setSidebarWidth( const int width )
    {
        QSettings().setValue( "sidebarWidth", width );
    }

    QString browser()            const  { return QSettings().value( "Browser" ).toString(); }
    void setBrowser( QString browser )  { QSettings().setValue( "Browser", browser ); }

    /*! Global Settings */
    void setLastDirPath(QString path)    { GlobalSettings().setValue("LastDirPath",path); }
    void setConnectionTimeOut(int tOut)  { GlobalSettings().setValue("ConnectionTimeOut",tOut); }
    void setImagesSource(int source)     { GlobalSettings().setValue("ImagesSource", source ); }

    int  getConnectionTimeOut()          { return GlobalSettings().value("ConnectionTimeOut",2000).toInt();}
    int  getImagesSource()               { return GlobalSettings().value("ImagesSource",PIT_SMALL_TEMPS).toInt();}
    QString getLastDirPath()             { return GlobalSettings().value("LastDirPath",QString("%1/").arg(QApplication::applicationDirPath())).toString(); }
    QString getCurrentWorkingDirectory() { return GlobalSettings().value("CurrentWorkingDirectory",QApplication::applicationDirPath()).toString(); }

    /*! Focus Settings */
    void setFocusWindowPosX(int  x)  { FocusSettings().setValue("DynFocusWindowPosX", x ); }
    void setFocusWindowPosY(int  y)  { FocusSettings().setValue("DynFocusWindowPosY", y ); }
    void setFocusWindowWidth(int x)  { FocusSettings().setValue("DynFocusWindowWidth", x ); }
    void setFocusWindowHeight(int y) { FocusSettings().setValue("DynFocusWindowHeight", y ); }
    void setUpperBoundary(int x)     { FocusSettings().setValue("UpperBoundary", x ); }
    void setLowerBoundary(int x)     { FocusSettings().setValue("LowerBoundary", x ); }
    void setDynamicWindow(int x)     { FocusSettings().setValue("DynamicWindow", x ); }

    int getFocusWindowPosX()         { return FocusSettings().value("DynFocusWindowPosX",640).toInt(); }
    int getFocusWindowPosY()         { return FocusSettings().value("DynFocusWindowPosY",480).toInt(); }
    int getFocusWindowWidth()        { return FocusSettings().value("DynFocusWindowWidth",200).toInt(); }
    int getFocusWindowHeight()       { return FocusSettings().value("DynFocusWindowHeight",200).toInt(); }
    int getUpperBoundary()           { return FocusSettings().value("UpperBoundary",3000).toInt();}
    int getLowerBoundary()           { return FocusSettings().value("LowerBoundary",-5000).toInt();}
    int getDynamicWindow()           { return FocusSettings().value("DynamicWindow",200).toInt();}

    /*! Motor Settings*/
    void setMotorsComPort(QString comPort){ MotorSettings().setValue("ComPort",comPort); }
    void setRigSettingsFileName(QString file){ MotorSettings().setValue("SettingsFile",file); }
    void setBaudRate(int baudRate)      { MotorSettings().setValue("BaudRate",baudRate); }
    void setVelocity(int speed)         { MotorSettings().setValue("Velocity",speed); }
    void setAcceleration(int acc)       { MotorSettings().setValue("Acceleration",acc); }
    void setJoggingSpeed(int speed)     { MotorSettings().setValue("JoggingSpeed",speed); }

    QString getMotorsComPort()          { return MotorSettings().value("ComPort","COM1").toString(); }
    QString getRigSettingsFileName()    { return MotorSettings().value("SettingsFile",QString("%1/rigSettings.xml").arg(QApplication::applicationDirPath())).toString(); }
    int getBaudRate()                   { return MotorSettings().value("BaudRate",115200).toInt(); }
    int getVelocity()                   { return MotorSettings().value("Velocity",70000).toInt(); }
    int getAcceleration()               { return MotorSettings().value("Acceleration",200000).toInt(); }
    int getJoggingSpeed()               { return MotorSettings().value("JoggingSpeed",10000).toInt(); }

    /*! CameraSettings */
    void setFPS(int fps)                { CameraSettings().setValue("FPS",fps); }
    void setResolutionH(int height)     { CameraSettings().setValue("ResolutionH",height); }
    void setResolutionW(int width)      { CameraSettings().setValue("ResolutionW",width); }
    int  getFramesPerSec()              { return CameraSettings().value("FPS",15).toInt(); }
    int  getResolutionH()               { return CameraSettings().value("ResolutionH",960).toInt(); }
    int  getResolutionW()               { return CameraSettings().value("ResolutionW",1280).toInt(); }

    /*! AcquisitionSettings */
    void setImagesPath(QString path)    { AcquisitionSettings().setValue("ImagesPath",path); }
    void setOverlapPerc(int overlap)    { AcquisitionSettings().setValue("OverlapPerc",overlap); }
    void setImageStepSize(int stepSize){  AcquisitionSettings().setValue("ImageStepSize",stepSize);}
    void setAcqLastPath(QString s)      { AcquisitionSettings().setValue("AcqLastPath",s);}
    QString  getImagesPath() const      { return AcquisitionSettings().value("ImagesPath",QString("%1/capturedImages").arg(QApplication::applicationDirPath())).toString(); }
    QString  getAcqLastPath() const     { return AcquisitionSettings().value("AcqLastPath",QString("./")).toString();}
    int  getOverLapPerc()               { return AcquisitionSettings().value("OverlapPerc",10).toInt(); }
    int  getImageStepSize()             { return AcquisitionSettings().value("ImageStepSize",1735).toInt(); } // 1258 for the colored camera
    int  getChipStepSize()              { return AcquisitionSettings().value("ChipStepSize",20325).toInt(); }

    /*! LedLightingSettings */
    void setLedLightingComPort(QString comPort)    { LedLightingSettings().setValue("ComPort",comPort); }
    void setLedLightingDefaultCurrent(int current) { LedLightingSettings().setValue("DefaultCurrent",current); }
    QString getLedLightingComPort()                { return LedLightingSettings().value("ComPort","COM2").toString(); }
    int  getLedLightingDefaultCurrent()            { return LedLightingSettings().value("DefaultCurrent",100).toInt(); }

    /*! Analysis Settings */
    void setAnalysisFileName(QString fileName)  { AnalysisSettings().setValue("AnalysisFileName",fileName); }
    void setDNMapFileName(QString fileName)     { AnalysisSettings().setValue("DNMapFileName",fileName); }
    void setGenerateDNMap(bool state)           { AnalysisSettings().setValue("GenerateDNMap",state); }
    void setEnableSecondMatchLevel(bool state)  { AnalysisSettings().setValue("SecondMatchLevel",state); }
    void setMatchingAccuracy(int accuracy)      { AnalysisSettings().setValue("MatchingAccuracy",accuracy); }
    void setSecondMatchingAccuracy(int accuracy){ AnalysisSettings().setValue("SecondMatchingAccuracy",accuracy); }
    void setEdgeMatchingAccuracy(int accuracy)  { AnalysisSettings().setValue("EdgeMatchingAccuracy",accuracy); }
    void setAnalysisType(int mode)              { AnalysisSettings().setValue("AnalysisType",mode); }
    QString getAnalysisFileName()               { return AnalysisSettings().value("AnalysisFileName","ImageAnalysisResults").toString(); }
    QString getDNMapFileName()                  { return AnalysisSettings().value("DNMapFileName","DeadNozzleMap").toString(); }
    bool getGenerateDNMap()                     { return AnalysisSettings().value("GenerateDNMap",0).toBool(); }
    bool getSecondMatchLevelEnabled()           { return AnalysisSettings().value("SecondMatchLevel",0).toBool(); }
    int  getMatchingAccuracy()                  { return AnalysisSettings().value("MatchingAccuracy",85).toInt(); }
    int  getSecondMatchingAccuracy()            { return AnalysisSettings().value("SecondMatchingAccuracy",85).toInt(); }
    int  getEdgeMatchingAccuracy()              { return AnalysisSettings().value("EdgeMatchingAccuracy",95).toInt(); }
    int  getAnalysisType()                      { return AnalysisSettings().value("AnalysisType",0).toInt(); }

    /*! Encapsulation Settings */
    void setEncapsulationImgSrc(QString path)  { EncapsulationSettings().setValue("EncapsulationImgSrc",path); }
    QString getEncapsulationImgSrc()           { return EncapsulationSettings().value("EncapsulationImgSrc","./").toString(); }
    void setAutoSkip2Next(bool state)          { AnalysisSettings().setValue("AutoSkip2Next",state); }
    void setScalingFactor(double scale)        { AnalysisSettings().setValue("ScalingFactor",scale);}
    bool getAutoSkip2Next()                    { return AnalysisSettings().value("AutoSkip2Next",0).toBool(); }
    double getScalingFactor()                  { return AnalysisSettings().value("ScalingFactor",1.34375).toDouble();}
    double getDefaultScalingFactor()           { return AnalysisSettings().value("DefaultScalingFactor",1.34375).toDouble();}

    /*! ACD MES Settings */
    void setProcessSegmentID(QString segmentID) { MESSettings().setValue("ProcessSegmentID",segmentID); }
    void setBasePath(QString basePath)          { MESSettings().setValue("BasePath",basePath); }
    void setEquipmentID(QString equipmentID)    { MESSettings().setValue("EquipmentID",equipmentID); }
    void setPersonID(QString personID)          { MESSettings().setValue("PersonID",personID); }
    void setSiteLocation(QString siteLocation)  { MESSettings().setValue("SiteLocation",siteLocation); }
    void setInboxLocation(QString inboxLocation){ MESSettings().setValue("InboxLocation",inboxLocation); }
    void setNumOfDieBlockageToFail(int num)     { MESSettings().setValue("NumDieFailure",num); }
    int  getNumOfDieBlockageToFail()            { return MESSettings().value("NumDieFailure",3).toInt(); }
    void setNumOfClusterSizeToFail(int num)     { MESSettings().setValue("ClusterSize",num); }
    int  getNumOfClusterSizeToFail()            { return MESSettings().value("ClusterSize",2).toInt(); }
    QString  getProcessSegmentID() const        { return MESSettings().value("ProcessSegmentID","PR010027/1").toString(); }
    QString  getBasePath() const
    {
        return MESSettings().value("BasePath","\\\\MES2-SQL-01.nrm.memjetmes.local\\ProductionData\\MES").toString().replace("\\","/");
    }
    QString  getEquipmentID() const             { return MESSettings().value("EquipmentID","EID-07730").toString(); }
    QString  getPersonID() const
    {
        QStringList environment = QProcess::systemEnvironment();
        QString usernameString;
        for(int i=0;i<environment.size();i++)
        {
            QString ss = environment.at(i);
            if(ss.contains("USERNAME"))
            {
                usernameString = ss;
                break;
            }
        }
        return usernameString.section("=",1,1);
    }
    QString  getSiteLocation() const            { return MESSettings().value("SiteLocation","NRM").toString(); }
    QString  getInboxLocation() const
    {
        return MESSettings().value("InboxLocation","\\\\MES2-SQL-01.nrm.memjetmes.local\\ftp\\MES\\InBox").toString().replace("\\","/");
    }

    /*! SPA */
    void setSPATolerance2FiducialSize(int accuracy)  { SPASettings().setValue("SPATolerance2FiducialSize",accuracy); }
    void setSPATolerance2SquareShape(int accuracy)   { SPASettings().setValue("SPATolerance2SquareShape",accuracy); }
    void setSPATolerance2LinePatternSize(int acc)    { SPASettings().setValue("SPATolerance2LinePatternSize",acc); }
    void setSPATolerance2Offset(int accuracy)        { SPASettings().setValue("SPATolerance2Offset",accuracy); }
    void setSPATopChipFiducialXRef(int ref)          { SPASettings().setValue("TopChipFiducialXRef",ref); }
    void setSPAButtomChipFiducialXRef(int ref)       { SPASettings().setValue("ButtomChipFiducialXRef",ref); }
    void setSPADistanceBetweenInnerFids(int dist)    { SPASettings().setValue("DistanceBetweenInnerFids",dist); }
    void setSPAGaussianConstant(int ref)             { SPASettings().setValue("GaussianConstant",ref); }
    void setSPAAdaptiveFilterBlockSize(int ref)      { SPASettings().setValue("AdaptiveFilterBlockSize",ref); }
    void setSPASaveFailureSnapshots(int status)      { SPASettings().setValue("SaveFailureSnapshots",status); }
    void setSPASaveDebugImage(int status)            { SPASettings().setValue("SaveDebugImage",status); }
    void setSPAShowAbsPos(int status)                { SPASettings().setValue("ShowAbsPos",status); }
    void setSPAShowGrid(int status)                  { SPASettings().setValue("ShowGrid",status); }
    void setSPASaveHealthInfo(int status)            { SPASettings().setValue("SaveHealthInfo",status); }
    void setSPAShowScale(int status)                 { SPASettings().setValue("ShowScale",status); }
    void setSPAShowYPos(int status)                  { SPASettings().setValue("ShowYPos",status); }
    void setSPAShowChipNum(int status)               { SPASettings().setValue("ShowChipNum",status); }
    void setSPAOverLayResult(int status)             { SPASettings().setValue("OverLayResult",status); }
    void setSPARotationAngle(int status)             { SPASettings().setValue("RotationAngle",status); }
    void setSPASaveResults(int status)               { SPASettings().setValue("SaveResults",status); }
    void setSPAFilterBySize(int status)              { SPASettings().setValue("FilterBySize",status); }
    void setSPAFilterByOffset(int status)            { SPASettings().setValue("FilterByOffset",status); }
    void setSPAFilterEdges(int status)               { SPASettings().setValue("FilterEdges",status); }
    void setSPADNMapSuffix(QString dnmapSuffix)      { SPASettings().setValue("DNMapSuffix",dnmapSuffix); }
    void setSPAScannedPrefix(QString scannedPrefix)  { SPASettings().setValue("ScannedPatternPrefix",scannedPrefix); }
    void setSPAContoursSuffix(QString contourSuffix) { SPASettings().setValue("ContoursSuffix",contourSuffix); }
    void setSPALowResScanning(int res)               { SPASettings().setValue("LowResScanning",res); }
    void setSPAHighResScanning(int res)              { SPASettings().setValue("HighResScanning",res); }
    void setSPAChartVersion(double ver)              { SPASettings().setValue("ChartVersion",ver); }
    void setSPAPlumpingOder(QString pOrder)          { SPASettings().setValue("PlumpingOder",pOrder); }

    int  getSPATolerance2FiducialSize()              { return SPASettings().value("SPATolerance2FiducialSize",40).toInt(); }
    int  getSPATolerance2SquareShape()               { return SPASettings().value("SPATolerance2SquareShape",20).toInt(); }
    int  getSPATolerance2LinePatternSize()           { return SPASettings().value("SPATolerance2LinePatternSize",40).toInt(); }
    int  getSPATolerance2Offset()                    { return SPASettings().value("SPATolerance2Offset",4).toInt(); }
    int  getSPATopChipFiducialXRef()                 { return SPASettings().value("TopChipFiducialXRef",912).toInt(); }
    int  getSPAButtomChipFiducialXRef()              { return SPASettings().value("ButtomChipFiducialXRef",400).toInt(); }
    int  getSPADistanceBetweenInnerFids()            { return SPASettings().value("DistanceBetweenInnerFids",128).toInt(); }
    int  getSPAGaussianConstant()                    { return SPASettings().value("GaussianConstant",5).toInt(); }
    int  getSPAAdaptiveFilterBlockSize()             { return SPASettings().value("AdaptiveFilterBlockSize",15).toInt(); }
    int  getSPARotationAngle()                       { return SPASettings().value("RotationAngle",0).toInt(); }
    int  getSPALowResScanning()                      { return SPASettings().value("LowResScanning",100).toInt(); }
    int  getSPAHighResScanning()                     { return SPASettings().value("HighResScanning",2400).toInt(); }
    bool getSPASaveDebugImage()                      { return SPASettings().value("SaveDebugImage",1).toBool(); }
    bool getSPASaveFailureSnapshots()                { return SPASettings().value("SaveFailureSnapshots",0).toBool(); }
    bool getSPAShowAbsPos()                          { return SPASettings().value("ShowAbsPos",0).toBool(); }
    bool getSPAShowGrid()                            { return SPASettings().value("ShowGrid",0).toBool(); }
    bool getSPASaveHealthInfo()                      { return SPASettings().value("SaveHealthInfo",1).toBool(); }
    bool getSPAShowScale()                           { return SPASettings().value("ShowScale",0).toBool(); }
    bool getSPAShowYPos()                            { return SPASettings().value("ShowYPos",0).toBool(); }
    bool getSPAShowChipNum()                         { return SPASettings().value("ShowChipNum",0).toBool(); }
    bool getSPAOverLayResult()                       { return SPASettings().value("OverLayResult",1).toBool(); }
    bool getSPASaveResults()                         { return SPASettings().value("SaveResults",0).toBool(); }
    bool getSPAFilterBySize()                        { return SPASettings().value("FilterBySize",0).toBool(); }
    bool getSPAFilterByOffset()                      { return SPASettings().value("FilterByOffset",0).toBool(); }
    bool getSPAFilterEdges()                         { return SPASettings().value("FilterEdges",1).toBool(); }
    QString getSPAPlumpingOder()                     { return SPASettings().value("PlumpingOder","YKCKM").toString(); }
    QString getSPADNMapSuffix()                      { return SPASettings().value("DNMapSuffix","DNMap").toString(); }
    QString getSPAContoursSuffix()                   { return SPASettings().value("ContoursSuffix","Contours").toString(); }
    QString getSPAScannedPrefix()                    { return SPASettings().value("ScannedPatternPrefix","cropped_pattern").toString(); }
    double getSPAChartVersion()                      { return SPASettings().value("ChartVersion",0.4).toDouble();}
    /*! Encap MES Settings */
    void setEncapProcessSegmentID(QString segmentID) { MESSettings().setValue("EncapProcessSegmentID",segmentID); }
    void setEncapProcessSegmentVer(int ver)          { MESSettings().setValue("EncapProcessSegmentVer",ver); }
    void setEncapBasePath(QString basePath)          { MESSettings().setValue("EncapBasePath",basePath); }
    void setEncapEquipmentID(QString equipmentID)    { MESSettings().setValue("EncapEquipmentID",equipmentID); }
    void setEncapPersonID(QString personID)          { MESSettings().setValue("EncapPersonID",personID); }
    void setEncapSiteLocation(QString siteLocation)  { MESSettings().setValue("EncapSiteLocation",siteLocation); }
    void setEncapInboxLocation(QString inboxLocation){ MESSettings().setValue("EncapInboxLocation",inboxLocation); }

    void setEncapUpperSpecLimit_Die_1(int num)       { MESSettings().setValue("EncapUpperSpecLimit_Die_1",num); }
    int  getEncapUpperSpecLimit_Die_1()              { return MESSettings().value("EncapUpperSpecLimit_Die_1",285).toInt(); }
    void setEncapLowerSpecLimit_Die_1(int num)       { MESSettings().setValue("EncapLowerSpecLimit_Die_1",num); }
    int  getEncapLowerSpecLimit_Die_1()              { return MESSettings().value("EncapLowerSpecLimit_Die_1",60).toInt(); }
    void setEncapUpperSpecLimit_Die_2_11(int num)    { MESSettings().setValue("EncapUpperSpecLimit_Die_2_11",num); }
    int  getEncapUpperSpecLimit_Die_2_11()           { return MESSettings().value("EncapUpperSpecLimit_Die_2_11",255).toInt(); }
    void setEncapLowerSpecLimit_Die_2_11(int num)    { MESSettings().setValue("EncapLowerSpecLimit_Die_2_11",num); }
    int  getEncapLowerSpecLimit_Die_2_11()           { return MESSettings().value("EncapLowerSpecLimit_Die_2_11",60).toInt(); }
    void setEncapUpperSpecLimit_Flat(int num)        { MESSettings().setValue("EncapUpperSpecLimit_Flat",num); }
    int  getEncapUpperSpecLimit_Flat()               { return MESSettings().value("EncapUpperSpecLimit_Flat",300).toInt(); }
    void setEncapLowerSpecLimit_Flat(int num)        { MESSettings().setValue("EncapLowerSpecLimit_Flat",num); }
    int  getEncapLowerSpecLimit_Flat()               { return MESSettings().value("EncapLowerSpecLimit_Flat",60).toInt(); }

    QString  getEncapProcessSegmentID() const        { return MESSettings().value("EncapProcessSegmentID","PR010134").toString(); }
    int      getEncapProcessSegmentVer() const       { return MESSettings().value("EncapProcessSegmentVer","0").toInt(); }
    QString  getEncapBasePath() const
    {
        return MESSettings().value("BasePath","\\\\MES2-SQL-01.nrm.memjetmes.local\\ProductionData\\MES").toString().replace("\\","/");
    }
    QString  getEncapEquipmentID() const             { return MESSettings().value("EncapEquipmentID","EID-07711").toString(); }
    QString  getEncapPersonID() const
    {
        QStringList environment = QProcess::systemEnvironment();
        QString usernameString;
        for(int i=0;i<environment.size();i++)
        {
            QString ss = environment.at(i);
            if(ss.contains("USERNAME"))
            {
                usernameString = ss;
                break;
            }
        }
        return usernameString.section("=",1,1);
    }
    QString  getEncapSiteLocation() const            { return MESSettings().value("EncapSiteLocation","NRM").toString(); }
    QString  getEncapInboxLocation() const
    {
        return MESSettings().value("EncapInboxLocation","\\\\MES2-SQL-01.nrm.memjetmes.local\\ftp\\MES\\InBox").toString().replace("\\","/");
    }
    /*!
      Contact Angle Measurement
      */
    void   setContactAngleMeasurementScalingFactor(double num) { ContactAngleMeasurement().setValue("ScalingFactor",num); }
    double getContactAngleMeasurementScalingFactor()           { return ContactAngleMeasurement().value("ScalingFactor",1.5).toDouble(); }
    void   setContactAngleMeasurementDropVolume(double num) { ContactAngleMeasurement().setValue("DropVolume",num); }
    double getContactAngleMeasurementDropVolume()           { return ContactAngleMeasurement().value("DropVolume",2.5).toDouble(); }


    void   setInspectionSectionsPerDie(int num)             { DelaminationInspection().setValue("SectionsPedDie",num); }
    double getInspectionSectionsPerDie()                    { return DelaminationInspection().value("SectionsPedDie",4).toInt(); }
};

namespace ASG_PIT
{
    inline PITSettings &settings()
    {
        //TODO maybe better to have a static instantiate() function
        // thus we lose the need for a mutex

        static QMutex mutex;
        static PITSettings* settings = 0;

        QMutexLocker locker( &mutex );

        if (!settings)
        {
            settings = QCoreApplication::instance()->findChild<PITSettings*>( "Settings-Instance" );
            if (!settings)
            {
                settings = new PITSettings( QCoreApplication::instance() );
                settings->setObjectName( "Settings-Instance" );
            }
        }
        return *settings;
    }

}

#endif // ACDSETTINGS_H
