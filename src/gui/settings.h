/*
 * settings.h
 *
 *  Created on: 20/02/2009
 *      Author: ttaha
 */
#ifndef SETTINGS_H
#define SETTINGS_H
#include <QMap>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QStringList>
#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>

/**
 * Settings that apply to all applications we are likely to write but where the
 * actual data is different for each application. This is to be viewed as a
 * repository for shared settings-related code. Not stored on a per-user basis.
 *
 * Inherit from this class when writing your own app.
 *
 * Parameterised on the QSettings object type to use so that a subclass can tell
 * us what the correct storage location is.
 */

template <typename T>
class AppSettings : public QObject
{
public:
    typedef T QSettings;
    AppSettings( QObject* ) { }
    QString path() const
    {
        return QSettings().value( "Path" ).toString();
    }
    void setPath( QString p ) { QSettings().setValue( "Path", p ); }
    QString version() const
    {
        return QSettings().value( "Version", "unknown" ).toString();
    }
    void setVersion( QString v ) { QSettings().setValue( "Version", v ); }
    QByteArray containerGeometry() const
    {
        return QSettings().value( "MainWindowGeometry" ).toByteArray();
    }
    void setContainerGeometry( QByteArray state )
    {
        QSettings().setValue( "MainWindowGeometry", state );
    }

    Qt::WindowState containerWindowState() const
    {
        return (Qt::WindowState) QSettings().value( "MainWindowState" ).toInt();
    }
    void setContainerWindowState( int state )
    {
        QSettings().setValue( "MainWindowState", state );
    }
};
class PITSettings;

namespace ASG_PIT
{
    PITSettings& settings();
}

#endif // SETTINGS_H
