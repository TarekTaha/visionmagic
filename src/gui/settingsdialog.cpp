/*
 * imageacquisition.h
 *
 *  Created on: 25/02/2009
 *      Author: ttaha
 */

#ifdef WIN32
    #include <windows.h>

#endif
#include "settingsdialog.h"

SettingsDialog::SettingsDialog( QWidget *parent )
        : QDialog( parent ),
          m_populating( false )
{
    ui.setupUi( this );

    setWindowTitle( tr( "ACD Options" ) );

    // Create pages and add them to stack widget
    QWidget* cameraWidget = new QWidget();
    ui_camera.setupUi( cameraWidget );
    ui.pageStack->addWidget( cameraWidget );

    QWidget* focusWidget = new QWidget();
    ui_focus.setupUi( focusWidget );
    ui.pageStack->addWidget( focusWidget );

    QWidget* motorsWidget = new QWidget();
    ui_motors.setupUi( motorsWidget );
    ui.pageStack->addWidget( motorsWidget );

    QWidget* acquisitionWidget = new QWidget();
    ui_acquisition.setupUi( acquisitionWidget );
    ui.pageStack->addWidget( acquisitionWidget );

    QWidget* analysisWidget = new QWidget();
    ui_analysis.setupUi( analysisWidget );
    ui.pageStack->addWidget( analysisWidget );

    QWidget* ledLightingWidget = new QWidget();
    ui_ledlighting.setupUi( ledLightingWidget );
    ui.pageStack->addWidget( ledLightingWidget );

    QWidget* mesWidget = new QWidget();
    ui_acd_mes.setupUi( mesWidget );
    ui.pageStack->addWidget( mesWidget );

    QWidget* encapMesWidget = new QWidget();
//    ui_encap_mes.setupUi( encapMesWidget );
    ui.pageStack->addWidget( encapMesWidget );

    QWidget* spaWidget = new QWidget();
    ui_spa.setupUi( spaWidget );
    ui.pageStack->addWidget( spaWidget );

    adjustSize();
    getAvaliablePorts();
    //Main
    connect( ui.pageList, SIGNAL(currentRowChanged( int )), SLOT(pageSwitched( int )) );
    connect( ui.okButton, SIGNAL(clicked()), SLOT(onOkClicked()) );
    connect( ui.applyButton, SIGNAL(clicked()), SLOT(applyPressed()) );
    //Acquisition
    connect(ui_acquisition.imagesLocation,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
    connect(ui_acquisition.overlapPercentage,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(ui_acquisition.browseBtn,SIGNAL(clicked()),this,SLOT(choseFolder()));
    connect(ui_acquisition.imageStepSizeSlider,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    //Analysis
    connect(ui_analysis.analysisFileName,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
    connect(ui_analysis.dnmapFileName,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
    //Motors
    connect(ui_motors.motorVelocity,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
    connect(ui_motors.motorAcceleration,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
    connect(ui_motors.baudRate,SIGNAL(currentIndexChanged(int)),this,SLOT(configChanged()));
    connect(ui_motors.comPort,SIGNAL(currentIndexChanged(int)),this,SLOT(configChanged()));
    connect(ui_motors.timeOutSlider,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(ui_motors.joggingSpeedSlider,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(ui_motors.settingFileButton,SIGNAL(clicked()),this,SLOT(selectSettingsFile()));
    connect(ui_motors.connectMotors,SIGNAL(clicked()),this,SLOT(connect2Motors()));
    //Focus
    connect(ui_focus.focusWindowH,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(ui_focus.focusWindowW,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(ui_focus.focusWindowX,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(ui_focus.focusWindowY,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(ui_focus.dynamicPoseWindow,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(ui_focus.focusUpperBound,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(ui_focus.focusLowerBound,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    //LedLighting
    connect(ui_ledlighting.currentValue,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(ui_ledlighting.comPort,SIGNAL(currentIndexChanged(int)),this,SLOT(configChanged()));
    //ACD MES
    connect(ui_acd_mes.personID,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
    connect(ui_acd_mes.equipmentID,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
    connect(ui_acd_mes.basePath,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
    connect(ui_acd_mes.processSegmentID,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
    connect(ui_acd_mes.siteLocation,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
    connect(ui_acd_mes.inboxLocation,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
    connect(ui_acd_mes.numDie2Fail,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
    connect(ui_acd_mes.clusterSize2Fail,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));

    //Encap MES
//    connect(ui_encap_mes.personID,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
//    connect(ui_encap_mes.equipmentID,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
//    connect(ui_encap_mes.basePath,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
//    connect(ui_encap_mes.processSegmentID,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
//    connect(ui_encap_mes.siteLocation,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
//    connect(ui_encap_mes.inboxLocation,SIGNAL(textChanged(QString)),this,SLOT(configChanged()));
//    connect(ui_encap_mes.processSegmentVer,SIGNAL(valueChanged(int)),this,SLOT(configChanged()));
}

void SettingsDialog::applyPressed()
{
    if (m_pagesToSave.contains( 0 )) { saveCamera(); }
    if (m_pagesToSave.contains( 1 )) { saveFocus(); }
    if (m_pagesToSave.contains( 2 )) { saveMotors(); }
    if (m_pagesToSave.contains( 3 )) { saveAcquisition(); }
    if (m_pagesToSave.contains( 4 )) { saveAnalysis(); }
    if (m_pagesToSave.contains( 5 )) { saveLedLighting(); }
    if (m_pagesToSave.contains( 6 )) { saveMES(); }
    if (m_pagesToSave.contains( 7 )) { saveEncapMES(); }

    ui.applyButton->setEnabled( false );
}

void SettingsDialog::configChanged()
{
    if ( m_populating )
        return;
    if( !m_pagesToSave.contains( ui.pageList->currentRow() ) && ui.pageList->currentRow() >= 0 )
          m_pagesToSave.insert( ui.pageList->currentRow() );

    ui_ledlighting.portName->setText(portsFriendlyName[ui_ledlighting.comPort->currentIndex()]);
    ui_motors.portName->setText(portsFriendlyName[ui_motors.comPort->currentIndex()]);

    ui.applyButton->setEnabled( true );
}

void SettingsDialog::connect2Motors()
{
    ASG_PIT::settings().setConnectionTimeOut(ui_motors.timeOutSlider->value());
    emit connect2MotorOnPort(ui_motors.comPort->currentText());
}

void SettingsDialog::choseFolder()
{
    acquisitionFolder =  QFileDialog::getExistingDirectory(this, "Chose Directory with the Images",
                                                    "./",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks
                                                    | QFileDialog::DontUseNativeDialog);
    if(acquisitionFolder!="")
    {
        ui_acquisition.imagesLocation->setText(acquisitionFolder);
        configChanged();
    }

}

int SettingsDialog::exec( int startPage )
{
    // This bool is just to prevent signals emitted by the programmatic
    // populating of widgets from being mistakenly interpreted as user
    // interaction.
    m_populating = true;

    populateCamera();
    populateFocus();
    populateMotors();
    populateAcquisition();
    populateAnalysis();
    populateLedLighting();
    populateMES();
    populateEncapMES();

    m_populating = false;

    pageSwitched( startPage );
    ui.applyButton->setEnabled( false );

    return QDialog::exec();
}

void SettingsDialog::getAvaliablePorts()
{
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    for (int i = 0; i < ports.size(); i++)
    {
        ui_motors.comPort->addItem(ports.at(i).portName.toLocal8Bit().constData());
        ui_ledlighting.comPort->addItem(ports.at(i).portName.toLocal8Bit().constData());
        portsFriendlyName.push_back(ports.at(i).friendName.toLocal8Bit().constData());
        ui_ledlighting.portName->setText(portsFriendlyName[i]);
        ui_motors.portName->setText(portsFriendlyName[i]);
    }
}

void SettingsDialog::onOkClicked()
{
    accept();

    if( m_pagesToSave.size() > 0 )
        applyPressed();
}

void SettingsDialog::pageSaved(int page)
{
    m_pagesToSave.remove( page );
}

void SettingsDialog::pageSwitched( int currentRow )
{
    ui.pageList->setCurrentRow( currentRow );
    ui.pageStack->setCurrentIndex( currentRow + 1 );
}

void SettingsDialog::populateCamera()
{
//    ui_camera.fps
}


void SettingsDialog::populateFocus()
{
    ui_focus.focusWindowH->setValue(ASG_PIT::settings().getFocusWindowHeight());
    ui_focus.focusWindowW->setValue(ASG_PIT::settings().getFocusWindowWidth());
    ui_focus.focusWindowX->setValue(ASG_PIT::settings().getFocusWindowPosX());
    ui_focus.focusWindowY->setValue(ASG_PIT::settings().getFocusWindowPosY());
    ui_focus.dynamicPoseWindow->setValue(ASG_PIT::settings().getDynamicWindow());
    ui_focus.focusUpperBound->setValue(ASG_PIT::settings().getUpperBoundary());
    ui_focus.focusLowerBound->setValue(ASG_PIT::settings().getLowerBoundary());
}

void SettingsDialog::populateEncapMES()
{
//    ui_encap_mes.equipmentID->setText(ASG_PIT::settings().getEncapEquipmentID());
//    ui_encap_mes.processSegmentID->setText(ASG_PIT::settings().getEncapProcessSegmentID());
//    ui_encap_mes.personID->setText(ASG_PIT::settings().getEncapPersonID());
//    ui_encap_mes.basePath->setText(ASG_PIT::settings().getEncapBasePath());
//    ui_encap_mes.siteLocation->setText(ASG_PIT::settings().getEncapSiteLocation());
//    ui_encap_mes.inboxLocation->setText(ASG_PIT::settings().getEncapInboxLocation());
//    ui_encap_mes.upperSpecLimit_die_1->setValue(ASG_PIT::settings().getEncapUpperSpecLimit_Die_1());
//    ui_encap_mes.lowerSpecLimit_die_1->setValue(ASG_PIT::settings().getEncapLowerSpecLimit_Die_1());
//    ui_encap_mes.upperSpecLimit_die_2_11->setValue(ASG_PIT::settings().getEncapUpperSpecLimit_Die_2_11());
//    ui_encap_mes.lowerSpecLimit_die_2_11->setValue(ASG_PIT::settings().getEncapLowerSpecLimit_Die_2_11());
//    ui_encap_mes.upperSpecLimit_flat->setValue(ASG_PIT::settings().getEncapUpperSpecLimit_Flat());
//    ui_encap_mes.lowerSpecLimit_flat->setValue(ASG_PIT::settings().getEncapLowerSpecLimit_Flat());
//    ui_encap_mes.processSegmentVer->setValue(ASG_PIT::settings().getEncapProcessSegmentVer());

}

void SettingsDialog::populateMES()
{
    ui_acd_mes.equipmentID->setText(ASG_PIT::settings().getEquipmentID());
    ui_acd_mes.processSegmentID->setText(ASG_PIT::settings().getProcessSegmentID());
    ui_acd_mes.personID->setText(ASG_PIT::settings().getPersonID());
    ui_acd_mes.basePath->setText(ASG_PIT::settings().getBasePath());
    ui_acd_mes.siteLocation->setText(ASG_PIT::settings().getSiteLocation());
    ui_acd_mes.inboxLocation->setText(ASG_PIT::settings().getInboxLocation());
    ui_acd_mes.numDie2Fail->setValue(ASG_PIT::settings().getNumOfDieBlockageToFail());
    ui_acd_mes.clusterSize2Fail->setValue(ASG_PIT::settings().getNumOfClusterSizeToFail());
}

void SettingsDialog::populateMotors()
{
    ui_motors.motorVelocity->setText(QString("%1").arg(ASG_PIT::settings().getVelocity()));
    ui_motors.motorAcceleration->setText(QString("%1").arg(ASG_PIT::settings().getAcceleration()));
    ui_motors.settingsFile->setText(QString("%1").arg(ASG_PIT::settings().getRigSettingsFileName()));
    ui_motors.timeOutSlider->setValue(ASG_PIT::settings().getConnectionTimeOut());
    ui_motors.joggingSpeedSlider->setValue(ASG_PIT::settings().getJoggingSpeed()/1000);
    int index = ui_motors.comPort->findText(ASG_PIT::settings().getMotorsComPort());
    if(index!=-1)
    {
        ui_ledlighting.comPort->setCurrentIndex(index);
        if(portsFriendlyName.size()>0)
        ui_motors.portName->setText(portsFriendlyName[index]);
    }
    else
        ui_ledlighting.comPort->addItem(ASG_PIT::settings().getLedLightingComPort());
}


void SettingsDialog::populateAcquisition()
{
    ui_acquisition.imagesLocation->setText(ASG_PIT::settings().getImagesPath());
    ui_acquisition.overlapPercentage->setValue(ASG_PIT::settings().getOverLapPerc());
    ui_acquisition.imageStepSizeSlider->setValue(ASG_PIT::settings().getImageStepSize());
}


void SettingsDialog::populateAnalysis()
{
    ui_analysis.analysisFileName->setText(ASG_PIT::settings().getAnalysisFileName());
    ui_analysis.dnmapFileName->setText(ASG_PIT::settings().getDNMapFileName());
}

void SettingsDialog::populateLedLighting()
{
    ui_ledlighting.currentValue->setValue(ASG_PIT::settings().getLedLightingDefaultCurrent());
    int index = ui_ledlighting.comPort->findText(ASG_PIT::settings().getLedLightingComPort());
    if(index!=-1)
    {
        ui_ledlighting.comPort->setCurrentIndex(index);
        if(portsFriendlyName.size()>0)
        ui_ledlighting.portName->setText(portsFriendlyName[index]);
    }
    else
        ui_ledlighting.comPort->addItem(ASG_PIT::settings().getLedLightingComPort());
}

void SettingsDialog::populateSPA()
{
    ui_spa.toleranceToFiducialSize->setValue(ASG_PIT::settings().getSPATolerance2FiducialSize());
    ui_spa.toleranceToFiducialSquares->setValue(ASG_PIT::settings().getSPATolerance2SquareShape());
    ui_spa.toleranceToLinePatternSize->setValue(ASG_PIT::settings().getSPATolerance2LinePatternSize());
    ui_spa.toleranceToOffset->setValue(ASG_PIT::settings().getSPATolerance2Offset());
    ui_spa.gaussConstant->setValue(ASG_PIT::settings().getSPAGaussianConstant());
    ui_spa.blockSize->setValue(ASG_PIT::settings().getSPAAdaptiveFilterBlockSize());
    ui_spa.lowResScan->setValue(ASG_PIT::settings().getSPALowResScanning());
    ui_spa.highResScan->setValue(ASG_PIT::settings().getSPAHighResScanning());
    ui_spa.scanningPrefix->setText(ASG_PIT::settings().getSPAScannedPrefix());
}

void SettingsDialog::saveCamera()
{

    pageSaved( 0 );
}


void SettingsDialog::saveFocus()
{
    ASG_PIT::settings().setFocusWindowHeight(ui_focus.focusWindowH->value());
    ASG_PIT::settings().setFocusWindowWidth(ui_focus.focusWindowW->value());
    ASG_PIT::settings().setFocusWindowPosX(ui_focus.focusWindowX->value());
    ASG_PIT::settings().setFocusWindowPosY(ui_focus.focusWindowY->value());
    ASG_PIT::settings().setDynamicWindow(ui_focus.dynamicPoseWindow->value());
    ASG_PIT::settings().setUpperBoundary(ui_focus.focusUpperBound->value());
    ASG_PIT::settings().setLowerBoundary(ui_focus.focusLowerBound->value());

    pageSaved( 1 );
}

void SettingsDialog::saveAcquisition()
{
    ASG_PIT::settings().setImagesPath(ui_acquisition.imagesLocation->text());
    ASG_PIT::settings().setOverlapPerc(ui_acquisition.overlapPercentage->value());
    ASG_PIT::settings().setImageStepSize(ui_acquisition.imageStepSizeSlider->value());
    pageSaved( 3 );
}

void SettingsDialog::saveAnalysis()
{
    ASG_PIT::settings().setAnalysisFileName(ui_analysis.analysisFileName->text());
    ASG_PIT::settings().setDNMapFileName(ui_analysis.dnmapFileName->text());
    pageSaved( 4 );
}

void SettingsDialog::saveLedLighting()
{
    ASG_PIT::settings().setLedLightingComPort(ui_ledlighting.comPort->currentText());
    ASG_PIT::settings().setLedLightingDefaultCurrent(ui_ledlighting.currentValue->value());
    pageSaved( 5 );
    emit ledLightingCurrentValueChanged();
}

void SettingsDialog::saveEncapMES()
{
//    ASG_PIT::settings().setEncapEquipmentID(ui_encap_mes.equipmentID->text());
//    ASG_PIT::settings().setEncapPersonID(ui_encap_mes.personID->text());
//    ASG_PIT::settings().setEncapBasePath(ui_encap_mes.basePath->text());
//    ASG_PIT::settings().setEncapProcessSegmentID(ui_encap_mes.processSegmentID->text());
//    ASG_PIT::settings().setEncapSiteLocation(ui_encap_mes.siteLocation->text());
//    ASG_PIT::settings().setEncapInboxLocation(ui_encap_mes.inboxLocation->text());

//    ASG_PIT::settings().setEncapUpperSpecLimit_Die_1(ui_encap_mes.upperSpecLimit_die_1->value());
//    ASG_PIT::settings().setEncapLowerSpecLimit_Die_1(ui_encap_mes.lowerSpecLimit_die_1->value());
//    ASG_PIT::settings().setEncapUpperSpecLimit_Die_2_11(ui_encap_mes.upperSpecLimit_die_2_11->value());
//    ASG_PIT::settings().setEncapLowerSpecLimit_Die_2_11(ui_encap_mes.lowerSpecLimit_die_2_11->value());
//    ASG_PIT::settings().setEncapUpperSpecLimit_Flat(ui_encap_mes.upperSpecLimit_flat->value());
//    ASG_PIT::settings().setEncapLowerSpecLimit_Flat(ui_encap_mes.lowerSpecLimit_flat->value());
//    ASG_PIT::settings().setEncapProcessSegmentVer(ui_encap_mes.processSegmentVer->value());
    pageSaved( 7 );
}

void SettingsDialog::saveMES()
{
    ASG_PIT::settings().setEquipmentID(ui_acd_mes.equipmentID->text());
    ASG_PIT::settings().setPersonID(ui_acd_mes.personID->text());
    ASG_PIT::settings().setBasePath(ui_acd_mes.basePath->text());
    ASG_PIT::settings().setProcessSegmentID(ui_acd_mes.processSegmentID->text());
    ASG_PIT::settings().setSiteLocation(ui_acd_mes.siteLocation->text());
    ASG_PIT::settings().setInboxLocation(ui_acd_mes.inboxLocation->text());
    ASG_PIT::settings().setNumOfDieBlockageToFail(ui_acd_mes.numDie2Fail->value());
    ASG_PIT::settings().setNumOfClusterSizeToFail(ui_acd_mes.clusterSize2Fail->value());
    pageSaved( 6 );
}

void SettingsDialog::saveMotors()
{
    ASG_PIT::settings().setVelocity(ui_motors.motorVelocity->text().toInt());
    ASG_PIT::settings().setAcceleration(ui_motors.motorAcceleration->text().toInt());
    ASG_PIT::settings().setMotorsComPort(ui_motors.comPort->currentText());
    ASG_PIT::settings().setRigSettingsFileName(ui_motors.settingsFile->text());
    ASG_PIT::settings().setConnectionTimeOut(ui_motors.timeOutSlider->value());
    ASG_PIT::settings().setJoggingSpeed(ui_motors.joggingSpeedSlider->value()*1000);
    pageSaved( 2 );
}

void SettingsDialog::saveSPA()
{
    ASG_PIT::settings().setSPATolerance2FiducialSize(ui_spa.toleranceToFiducialSize->value());
    ASG_PIT::settings().setSPATolerance2SquareShape(ui_spa.toleranceToFiducialSquares->value());
    ASG_PIT::settings().setSPATolerance2LinePatternSize(ui_spa.toleranceToLinePatternSize->value());
    ASG_PIT::settings().setSPATolerance2Offset(ui_spa.toleranceToOffset->value());
    ASG_PIT::settings().setSPAAdaptiveFilterBlockSize(ui_spa.blockSize->value());
    ASG_PIT::settings().setSPAGaussianConstant(ui_spa.gaussConstant->value());
    ASG_PIT::settings().setSPAScannedPrefix(ui_spa.scanningPrefix->text());
    ASG_PIT::settings().setSPALowResScanning(ui_spa.lowResScan->value());
    ASG_PIT::settings().setSPAHighResScanning(ui_spa.highResScan->value());
    pageSaved( 7 );
}

void SettingsDialog::selectSettingsFile()
{
    settingsFile = QFileDialog::getOpenFileName(this, tr("Rig Settings File"),
                                                 "./",
                                                 tr("Settings XML (*.xml)"));
    if(settingsFile!="")
    {
        ui_motors.settingsFile->setText(settingsFile);
        configChanged();
    }
}
