/*
 * splashscreen.h
 *
 *  Created on: 12/01/2011
 *      Author: ttaha
 */

#include "SplashScreen.h"
#include "version.h"
SplashScreen::SplashScreen(QPixmap pixmap)
    : QSplashScreen(pixmap)
{

}

SplashScreen::~SplashScreen()
{

}

void SplashScreen::setMessage()
{
    this->mMessages << APPLICATION_NAME << APP_VERSION_LONG << AUTHOR<<YEAR;

    this->mPoints.append(QPointF(20, 100));
    this->mFonts.append(QFont("Helvetica", 25));
    this->mColors.append(QColor(213, 218, 220));

    this->mPoints.append(QPointF(30, 140));
    this->mFonts.append(QFont("Helvetica", 18));
    this->mColors.append(QColor(213, 218, 220));

    //this->mPoints.append(QPointF(110, 322));
    this->mFonts.append(QFont("Helvetica", 18));
    this->mColors.append(QColor(Qt::white));

    repaint();
}

void SplashScreen::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void SplashScreen::drawContents(QPainter *painter)
{
    painter->setFont(this->mFonts.at(0));
    painter->setPen(this->mColors.at(0));
    painter->drawText(this->mPoints.at(0), this->mMessages.at(0));

    painter->setFont(this->mFonts.at(1));
    painter->setPen(this->mColors.at(1));
    painter->drawText(this->mPoints.at(1), this->mMessages.at(1));

    painter->setFont(this->mFonts.at(2));
    painter->setPen(this->mColors.at(2));
    //painter->drawText(this->mPoints.at(2), this->mMessages.at(2));
    QRect r = rect();
    r.setRect(r.x(), r.y(), r.width(), r.height() -12);
    painter->drawText(r, Qt::AlignBottom | Qt::AlignCenter, this->mMessages.at(2));

    painter->setFont(QFont("Verdana", 9));
    QSplashScreen::drawContents(painter);
}
