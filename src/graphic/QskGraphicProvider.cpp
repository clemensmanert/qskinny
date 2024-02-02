/******************************************************************************
 * QSkinny - Copyright (C) The authors
 *           SPDX-License-Identifier: BSD-3-Clause
 *****************************************************************************/

#include "QskGraphicProvider.h"
#include "QskGraphicProviderMap.h"
#include "QskGraphic.h"
#include "QskSkinManager.h"
#include "QskSkin.h"

#include <qmutex.h>
#include <qcache.h>
#include <qdebug.h>
#include <qurl.h>

static QskGraphicProviderMap qskGraphicProviders;

class QskGraphicProvider::PrivateData
{
  public:
    // caching of graphics
    QCache< QString, const QskGraphic > cache;
    QMutex mutex;
};

QskGraphicProvider::QskGraphicProvider( QObject* parent )
    : QObject( parent )
    , m_data( new PrivateData() )
{
}

QskGraphicProvider::~QskGraphicProvider()
{
}

void QskGraphicProvider::setCacheSize( int size )
{
    if ( size < 0 )
        size = 0;

    QMutexLocker locker( &m_data->mutex );
    m_data->cache.setMaxCost( size );
}

int QskGraphicProvider::cacheSize() const
{
    QMutexLocker locker( &m_data->mutex );
    return m_data->cache.maxCost();
}

void QskGraphicProvider::clearCache()
{
    QMutexLocker locker( &m_data->mutex );
    m_data->cache.clear();
}

const QskGraphic* QskGraphicProvider::requestGraphic( const QString& id ) const
{
    const QskGraphic* graphic = nullptr;

    {
        QMutexLocker locker( &m_data->mutex );
        graphic = m_data->cache.object( id );
    }

    if ( graphic == nullptr )
    {
        graphic = loadGraphic( id );

        if ( graphic == nullptr )
        {
            qWarning() << "QskGraphicProvider: can't load" << id;
            return nullptr;
        }

        {
            QMutexLocker locker( &m_data->mutex );

            if( auto cached = m_data->cache.object( id ) )
            {
                delete graphic;
                graphic = cached;
            }
            else
            {
                const int cost = 1; // TODO ...
                m_data->cache.insert( id, graphic, cost );
            }
        }
    }

    return graphic;
}

void Qsk::addGraphicProvider(
    const QString& providerId, QskGraphicProvider* provider )
{
    qskGraphicProviders.insert( providerId, provider );
}

QskGraphicProvider* Qsk::graphicProvider( const QString& providerId )
{
    if ( auto skin = qskSkinManager->skin() )
    {
        if ( auto provider = skin->graphicProvider( providerId ) )
            return provider;
    }

    return qskGraphicProviders.provider( providerId );
}

QskGraphic Qsk::loadGraphic( const char* source )
{
    return loadGraphic( QUrl( source ) );
}

QskGraphic Qsk::loadGraphic( const QUrl& url )
{
    static QskGraphic nullGraphic;

    QString imageId = url.toString( QUrl::RemoveScheme |
        QUrl::RemoveAuthority | QUrl::NormalizePathSegments );

    if ( imageId.isEmpty() )
        return nullGraphic;

    if ( imageId[ 0 ] == '/' )
        imageId = imageId.mid( 1 );

    const QString providerId = url.host();

    const QskGraphic* graphic = nullptr;

    if ( const auto provider = Qsk::graphicProvider( providerId ) )
        graphic = provider->requestGraphic( imageId );

    return graphic ? *graphic : nullGraphic;
}

#include "moc_QskGraphicProvider.cpp"
