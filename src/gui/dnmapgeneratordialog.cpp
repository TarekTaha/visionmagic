/*
 * dnmapgeneratordialog.cpp
 *
 *  Created on: 06/04/2009
 *      Author: ttaha
 */
#include "dnmapgeneratordialog.h"
#include "ui_dnmapgeneratordialog.h"

DNMapGeneratorDialog::DNMapGeneratorDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::DNMapGeneratorDialog)
{
    m_ui->setupUi(this);
    m_ui->progressBar->setVisible(false);
    connect(&dnmapProbabilisticGenerator,SIGNAL(updateProgress(int)),SLOT(updateProgress(int)));
}

DNMapGeneratorDialog::~DNMapGeneratorDialog()
{
    delete m_ui;
}

void DNMapGeneratorDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DNMapGeneratorDialog::generateMaps()
{
    qDebug() << "Generating DNMAPs";
    int probs[12];
    probs[0] = (int)m_ui->_blockedCount1->value();
    probs[1] = (int)m_ui->_blockedCount2->value();
    probs[2] = (int)m_ui->_blockedCount3->value();
    probs[3] = (int)m_ui->_blockedCount4->value();
    probs[4] = (int)m_ui->_blockedCount5->value();
    probs[5] = (int)m_ui->_blockedCount6->value();
    probs[6] = (int)m_ui->_blockedCount7->value();
    probs[7] = (int)m_ui->_blockedCount8->value();
    probs[8] = (int)m_ui->_blockedCount9->value();
    probs[9] = (int)m_ui->_blockedCount10->value();
    probs[10] =(int)m_ui->_fiberCount->value();
    probs[11] =(int)m_ui->numTests->value();
    dnmapProbabilisticGenerator.setPrefix(m_ui->namePrefix->text());
    dnmapProbabilisticGenerator.generateDNMap(m_ui->dnMapLocation->text(),m_ui->numDNMaps->value(),probs);
}

void DNMapGeneratorDialog::loadDataFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"),
                                                 "./",
                                                 tr("Images (*.csv)"));
    if(fileName!="")
    {
        dnmapProbabilisticGenerator.readFile(fileName);
        m_ui->_blockedCount1->setValue(dnmapProbabilisticGenerator.getBlockedProb(0));
        m_ui->_blockedCount2->setValue(dnmapProbabilisticGenerator.getBlockedProb(1));
        m_ui->_blockedCount3->setValue(dnmapProbabilisticGenerator.getBlockedProb(2));
        m_ui->_blockedCount4->setValue(dnmapProbabilisticGenerator.getBlockedProb(3));
        m_ui->_blockedCount5->setValue(dnmapProbabilisticGenerator.getBlockedProb(4));
        m_ui->_blockedCount6->setValue(dnmapProbabilisticGenerator.getBlockedProb(5));
        m_ui->_blockedCount7->setValue(dnmapProbabilisticGenerator.getBlockedProb(6));
        m_ui->_blockedCount8->setValue(dnmapProbabilisticGenerator.getBlockedProb(7));
        m_ui->_blockedCount9->setValue(dnmapProbabilisticGenerator.getBlockedProb(8));
        m_ui->_blockedCount10->setValue(dnmapProbabilisticGenerator.getBlockedProb(9));
        m_ui->_fiberCount->setValue(dnmapProbabilisticGenerator.getBlockedProb(10));
        m_ui->numTests->setValue(dnmapProbabilisticGenerator.getNumTests());
    }
}

void DNMapGeneratorDialog::selectOutputLocation()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Output Directory"),
                                                    "/home",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks
                                                    | QFileDialog::DontUseNativeDialog);
    if(dir!="")
        m_ui->dnMapLocation->setText(dir);
}

void DNMapGeneratorDialog::updateProgress(int value)
{
    qDebug() << "Value is:"<<value;
    m_ui->progressBar->setVisible(true);
    m_ui->progressBar->setValue(value);
    if(value ==100)
        m_ui->progressBar->setVisible(false);
}
