/*
 * acdsettings.cpp
 *
 *  Created on: 20/02/2009
 *      Author: ttaha
 */
#include "acdsettings.h"

PITSettings::PITSettings( QObject* parent ) :
    AppSettings<QSettings>( parent )
{
    #ifndef WIN32
        QSettings new_config;

        if (!QFile( new_config.fileName() ).exists())
        {
            //attempt to upgrade settings object from old and broken location
            foreach (QString const name, QStringList() << "Client" << "Users" << "Plugins" << "MediaDevices")
            {
                QSettings old_config( QSettings::IniFormat, QSettings::UserScope, "Last.fm", name );
                old_config.setFallbacksEnabled( false );

                if (!QFile::exists( old_config.fileName() ))
                    continue;

                foreach (QString const key, old_config.allKeys()) {
                    if (name != "Client")
                        //Client now becomes [General] group as this makes most sense
                        new_config.beginGroup( name );
                    new_config.setValue( key, old_config.value( key ) );
                    #ifndef QT_NO_DEBUG
                    if (name != "Client") // otherwise qWarning and aborts
                    #endif
                    new_config.endGroup();
                }

                new_config.sync();

                QFile f( old_config.fileName() );
                f.remove();
                QFileInfo(f).dir().rmdir("."); //safe as won't remove a non empty dir
            }
        }
    #endif
}
