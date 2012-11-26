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
#include "messagelogger.h"
using namespace std;

MessageLogger::MessageLogger(QWidget *parent)
    :QDialog(parent),
    textChanged(false),
    qdebug(false),
    saveBeforeDelete(false)
{
    Logger& logger = Logger::getLogger();
    logger.initialize(QString("%1/pit.log").arg(QApplication::applicationDirPath()), true );
    logger.setLevel( Logger::Debug );
    loggerUI.setupUi(this);
    loggerUI.logTextEdit->clear();
    loggerUI.logTextEdit->setReadOnly(true);
    connect(loggerUI.clearPushButton,SIGNAL(clicked()),SLOT(clearMessages()));
    connect(loggerUI.savePushButton,SIGNAL(clicked()),SLOT(saveClicked()));
    connect(loggerUI.closePushButton,SIGNAL(clicked()),SLOT(close()));
}

MessageLogger::~MessageLogger()
{
    if(saveBeforeDelete)
    {
        file->write((loggerUI.logTextEdit->toPlainText().toAscii()), loggerUI.logTextEdit->toPlainText().length());
        file->close();
        delete file;
    }
}

void MessageLogger::loadLogs()
{
    loggerUI.logTextEdit->clear();
    Logger& lg = Logger::getLogger();
    QString fileName = lg.getFilePath();
    QFile *file;
    file = new QFile(fileName);
    // open File
    if(!file->open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Error",
                             QString("Error while trying to open ") + fileName,
                             QMessageBox::Ok | QMessageBox::Default,
                             QMessageBox::NoButton,
                             QMessageBox::NoButton);
        delete file;
        return;
    }
    loggerUI.logTextEdit->append(file->readAll());
    file->close();
    delete file;
}

void MessageLogger::clearMessages()
{
    // really clear?
    if(QMessageBox::information(this, tr("Message Logger"),
                                "Keep in mind, there's no UNDO...\n\nDelete all messages?",
                                QMessageBox::No | QMessageBox::Default | QMessageBox::Escape,
                                QMessageBox::Yes,
                                QMessageBox::NoButton) == QMessageBox::No) {
        return;
    }

    // clear logging-area
    loggerUI.logTextEdit->clear();
    textChanged = false;
}


void MessageLogger::saveClicked()
{
    saveMessages(false);
}


bool MessageLogger::saved()
{
#ifdef MESSAGELOGGER
    return !textChanged;
#else
    return true;
#endif
}


void MessageLogger::closed()
{
    qdebug = true;
}


void MessageLogger::saveMessages(bool exitApp)
{
    bool fileSelected = false;
    QString fileName;


    do
    {
        #ifdef PIT_ONLY
                fileName = QFileDialog::getSaveFileName(this,tr("Save File"),tr("./acd.log"), tr("*.log"));
        #else
                fileName = QFileDialog::getSaveFileName(this,tr("Save File"),tr("./nha.log"), tr("*.log"));
        #endif
        if(fileName.isNull())
            return;
        file = new QFile(fileName);
        if(file->exists())
        {
            switch(QMessageBox::warning(this, "acd",
                                        "File exists - Overwrite?",
                                        QMessageBox::Yes,
                                        QMessageBox::No | QMessageBox::Default,
                                        QMessageBox::Cancel | QMessageBox::Escape))
            {
            case QMessageBox::Yes:
                fileSelected = true;
                break;
            case QMessageBox::No:
                delete file;
                break;
            case QMessageBox::Cancel:
                delete file;
                return;
                break;
            default:
                break;
            }
        }
        else
        {
             fileSelected = true;
        }
    }
    while(!fileSelected);

    // open File
    if(!file->open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        QMessageBox::warning(this, "Error",
                             QString("Error while trying to open ") + fileName,
                             QMessageBox::Ok | QMessageBox::Default,
                             QMessageBox::NoButton,
                             QMessageBox::NoButton);
        delete file;
        return;
    }

    if(exitApp)
    {
        saveBeforeDelete = true;
        return;
    }

    // write messages to file
    file->write((loggerUI.logTextEdit->toPlainText().toAscii()), loggerUI.logTextEdit->toPlainText().length());
    // close File (and delete object)
    file->close();
    delete file;

    // ask for 'cleaning' the message-area (if not exiting the application)
    if(QMessageBox::information(this, "Message Logger",
                                "Clear messages?",
                                QMessageBox::Yes | QMessageBox::Default,
                                QMessageBox::No | QMessageBox::Escape,
                                QMessageBox::NoButton) == QMessageBox::Yes)
        // clear message-area
        loggerUI.logTextEdit->clear();

    textChanged = false;
}

