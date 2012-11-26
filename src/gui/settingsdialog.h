/*
 * imageacquisition.h
 *
 *  Created on: 25/02/2009
 *      Author: ttaha
 */

#ifndef __SETTINGSDIALOG_H__
#define __SETTINGSDIALOG_H__

#include <QHeaderView>
#include <QProcess>
#include <qextserialport.h>
#include <qextserialenumerator.h>
#include "ui_settingsdialog.h"
#include "ui_settingsdialog_camera.h"
#include "ui_settingsdialog_focus.h"
#include "ui_settingsdialog_motors.h"
#include "ui_settingsdialog_ledlighting.h"
#include "ui_settingsdialog_acquisition.h"
#include "ui_settingsdialog_analysis.h"
#include "ui_settingsdialog_mes.h"
#include "ui_settingsdialog_spa.h"
#include "dlogger.h"
#include "settingsdialog.h"
#include "acdsettings.h"

class LoginWidget;
class MainWindow;
class SettingsDialog : public QDialog
{
    Q_OBJECT
    public:
        SettingsDialog( QWidget *parent = 0 );
        int exec( int startPage = 0 );
        Ui::SettingsDialogFocus     ui_focus;
        signals:
        void ledLightingCurrentValueChanged();
        void connect2MotorOnPort(QString);
    public slots:
        void applyPressed();
        void connect2Motors();
    private slots:
        void configChanged();
        void pageSwitched( int currentRow );
        void pageSaved( int page );
        void onOkClicked();
        void choseFolder();
        void selectSettingsFile();
    private:
        void populateCamera();
        void populateFocus();
        void populateLedLighting();
        void populateEncapMES();
        void populateMES();
        void populateMotors();
        void populateAcquisition();
        void populateAnalysis();
        void populateSPA();
        void getAvaliablePorts();

        void saveCamera();
        void saveFocus();
        void saveMotors();
        void saveEncapMES();
        void saveMES();
        void saveAcquisition();
        void saveAnalysis();
        void saveLedLighting();
        void saveSPA();

        Ui::SettingsDialog ui;
        Ui::SettingsDialogCamera        ui_camera;
        Ui::SettingsDialogMotors        ui_motors;
        Ui::SettingsDialogLedLighting   ui_ledlighting;
        Ui::SettingsDialogAcquisition   ui_acquisition;
        Ui::SettingsDialogAnalysis      ui_analysis;
        Ui::SettingsDialogMES           ui_acd_mes;
        Ui::SettingsDialogSPA           ui_spa;
//        Ui::SettingsDialogEncapMES      ui_encap_mes;
        QVector <QString> portsFriendlyName;
        QSet<int> m_pagesToSave;
        bool m_populating;
        QString acquisitionFolder,settingsFile;
        friend class MainWindow;
};

#endif
