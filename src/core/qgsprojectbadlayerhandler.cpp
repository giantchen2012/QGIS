/***************************************************************************
  qgsprojectbadlayerhandler.cpp - QgsProjectBadLayerHandler

 ---------------------
 begin                : 22.10.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprojectbadlayerhandler.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QFileInfo>

void QgsProjectBadLayerHandler::handleBadLayers( const QList<QDomNode>& layers )
{
  QgsMessageLog::instance()->logMessage( QStringLiteral( "%1 bad layers dismissed:" ).arg( layers.size() ) );
  Q_FOREACH ( const QDomNode& layer, layers )
  {
    QgsMessageLog::instance()->logMessage( QStringLiteral( " * %1" ).arg( dataSource( layer ) ) );
  }
}

QgsProjectBadLayerHandler::~QgsProjectBadLayerHandler()
{
}

QgsProjectBadLayerHandler::DataType QgsProjectBadLayerHandler::dataType( const QDomNode& layerNode )
{
  QString type = layerNode.toElement().attribute( QStringLiteral( "type" ) );

  if ( QString::null == type )
  {
    QgsDebugMsg( "cannot find ``type'' attribute" );

    return IS_BOGUS;
  }

  if ( "raster" == type )
  {
    QgsDebugMsg( "is a raster" );

    return IS_RASTER;
  }
  else if ( "vector" == type )
  {
    QgsDebugMsg( "is a vector" );

    return IS_VECTOR;
  }

  QgsDebugMsg( "is unknown type " + type );

  return IS_BOGUS;
}

QString QgsProjectBadLayerHandler::dataSource( const QDomNode& layerNode )
{
  QDomNode dataSourceNode = layerNode.namedItem( QStringLiteral( "datasource" ) );

  if ( dataSourceNode.isNull() )
  {
    QgsDebugMsg( "cannot find datasource node" );

    return QString::null;
  }

  return dataSourceNode.toElement().text();
}

QgsProjectBadLayerHandler::ProviderType QgsProjectBadLayerHandler::providerType( const QDomNode& layerNode )
{
  // XXX but what about rasters that can be URLs?  _Can_ they be URLs?

  switch ( dataType( layerNode ) )
  {
    case IS_VECTOR:
    {
      QString ds = dataSource( layerNode );

      QgsDebugMsg( "datasource is " + ds );

      if ( ds.contains( QLatin1String( "host=" ) ) )
      {
        return IS_URL;
      }
      else if ( ds.contains( QLatin1String( "dbname=" ) ) )
      {
        return IS_DATABASE;
      }
      // be default, then, this should be a file based layer data source
      // XXX is this a reasonable assumption?

      return IS_FILE;
    }

    case IS_RASTER:         // rasters are currently only accessed as
      // physical files
      return IS_FILE;

    default:
      QgsDebugMsg( "unknown ``type'' attribute" );
  }

  return IS_Unknown;
}

void QgsProjectBadLayerHandler::setDataSource( QDomNode& layerNode, const QString& dataSource )
{
  QDomNode dataSourceNode = layerNode.namedItem( QStringLiteral( "datasource" ) );
  QDomElement dataSourceElement = dataSourceNode.toElement();
  QDomText dataSourceText = dataSourceElement.firstChild().toText();

  QgsDebugMsg( "datasource changed from " + dataSourceText.data() );

  dataSourceText.setData( dataSource );

  QgsDebugMsg( "to " + dataSourceText.data() );
}
