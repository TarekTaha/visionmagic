/*
 * splashscreen.h
 *
 *  Created on: 12/01/2011
 *      Author: ttaha
 */
#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QSplashScreen>
#include <QPixmap>
#include <QPainter>

class SplashScreen : public QSplashScreen
{
private:
    QStringList mMessages;
    QList<QPointF> mPoints;
    QList<QFont> mFonts;
    QList<QColor> mColors;
public:
    SplashScreen(QPixmap pixmap);
    ~SplashScreen();

    void setMessage();
protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void drawContents(QPainter *painter);
};

#endif // SPLASHSCREEN_H
