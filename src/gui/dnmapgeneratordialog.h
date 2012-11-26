/*
 * dnmapgeneratordialog.h
 *
 *  Created on: 06/04/2009
 *      Author: ttaha
 */
#ifndef DNMAPGENERATORDIALOG_H
#define DNMAPGENERATORDIALOG_H

#include <QtGui/QDialog>
#include <QFileDialog>
#include "dnmapprobabilisticgenerator.h"
#include "dlogger.h"

namespace Ui
{
    class DNMapGeneratorDialog;
}

class DNMapGeneratorDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(DNMapGeneratorDialog)
public:
    explicit DNMapGeneratorDialog(QWidget *parent = 0);
    virtual ~DNMapGeneratorDialog();
public slots:
        void selectOutputLocation();
        void loadDataFromFile();
        void generateMaps();
        void updateProgress(int);
protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::DNMapGeneratorDialog *m_ui;
    DNMapProbabilisticGenerator dnmapProbabilisticGenerator;
};

#endif // DNMAPGENERATORDIALOG_H
