/***************************************************************************
    qgsprojectbadlayerguihandler.cpp - handle bad layers
    ---------------------
    begin                : December 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectbadlayerguihandler.h"

#include <QApplication>
#include <QDomDocument>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>

#include "qgslogger.h"
#include "qgisgui.h"
#include "qgsproviderregistry.h"
#include "qgsproject.h"

QgsProjectBadLayerGuiHandler::QgsProjectBadLayerGuiHandler()
{
}

bool QgsProjectBadLayerGuiHandler::mIgnore = false;

void QgsProjectBadLayerGuiHandler::handleBadLayers( const QList<QDomNode>& layers )
{
  QgsDebugMsg( QString( "%1 bad layers found" ).arg( layers.size() ) );

  // make sure we have arrow cursor (and not a wait cursor)
  QApplication::setOverrideCursor( Qt::ArrowCursor );

  QMessageBox messageBox;

  QAbstractButton *ignoreButton =
    messageBox.addButton( tr( "Ignore" ), QMessageBox::ActionRole );

  QAbstractButton *okButton = messageBox.addButton( QMessageBox :: Ok );

  messageBox.addButton( QMessageBox :: Cancel );

  messageBox.setWindowTitle( tr( "QGIS Project Read Error" ) );
  messageBox.setText( tr( "Unable to open one or more project layers.\nChoose "
                          "ignore to continue loading without the missing layers. Choose cancel to "
                          "return to your pre-project load state. Choose OK to try to find the "
                          "missing layers." ) );
  messageBox.setIcon( QMessageBox::Critical );
  messageBox.exec();

  QgsProjectBadLayerGuiHandler::mIgnore = false;

  if ( messageBox.clickedButton() == okButton )
  {
    QgsDebugMsg( "want to find missing layers is true" );

    // attempt to find the new locations for missing layers
    // XXX vector file hard-coded -- but what if it's raster?

    QString filter = QgsProviderRegistry::instance()->fileVectorFilters();
    findLayers( filter, layers );
  }
  else if ( messageBox.clickedButton() == ignoreButton )
  {
    QgsProjectBadLayerGuiHandler::mIgnore = true;
  }
  else
  {
    // Do nothing
  }

  QApplication::restoreOverrideCursor();
}

bool QgsProjectBadLayerGuiHandler::findLayer( const QString& fileFilters, const QDomNode& constLayerNode )
{
  // XXX actually we could possibly get away with a copy of the node
  QDomNode& layerNode = const_cast<QDomNode&>( constLayerNode );

  bool retVal = false;

  switch ( providerType( layerNode ) )
  {
    case IS_FILE:
      QgsDebugMsg( "layer is file based" );
      retVal = findMissingFile( fileFilters, layerNode );
      break;

    case IS_DATABASE:
      QgsDebugMsg( "layer is database based" );
      break;

    case IS_URL:
      QgsDebugMsg( "layer is URL based" );
      break;

    case IS_Unknown:
      QgsDebugMsg( "layer has an unknown type" );
      break;
  }
  return retVal;
}
bool QgsProjectBadLayerGuiHandler::findMissingFile( const QString& fileFilters, QDomNode& layerNode )
{
  // Prepend that file name to the valid file format filter list since it
  // makes it easier for the user to not only find the original file, but to
  // perhaps find a similar file.

  QFileInfo originalDataSource( dataSource( layerNode ) );

  QString memoryQualifier;    // to differentiate between last raster and
  // vector directories

  switch ( dataType( layerNode ) )
  {
    case IS_VECTOR:
    {
      memoryQualifier = QStringLiteral( "lastVectorFileFilter" );

      break;
    }
    case IS_RASTER:
    {
      memoryQualifier = QStringLiteral( "lastRasterFileFilter" );

      break;
    }
    default:
      QgsDebugMsg( "unable to determine data type" );
      return false;
  }

  // Prepend the original data source base name to make it easier to pick it
  // out from a list of other files; however the appropriate filter strings
  // for the file type will also be added in case the file name itself has
  // changed, too.

  QString myFileFilters = originalDataSource.fileName() + ";;" + fileFilters;

  QStringList selectedFiles;
  QString enc;
  QString title = QObject::tr( "Where is '%1' (original location: %2)?" )
                  .arg( originalDataSource.fileName(),
                        originalDataSource.absoluteFilePath() );

  bool retVal = QgisGui::openFilesRememberingFilter( memoryQualifier,
                myFileFilters,
                selectedFiles,
                enc,
                title,
                true );

  if ( selectedFiles.isEmpty() )
  {
    return retVal;
  }
  else
  {
    setDataSource( layerNode, selectedFiles.first() );
    if ( ! QgsProject::instance()->read( layerNode ) )
    {
      QgsDebugMsg( "unable to re-read layer" );
    }
  }
  return retVal;
}

void QgsProjectBadLayerGuiHandler::findLayers( const QString& fileFilters, const QList<QDomNode>& layerNodes )
{

  for ( QList<QDomNode>::const_iterator i = layerNodes.begin();
        i != layerNodes.end();
        ++i )
  {
    if ( findLayer( fileFilters, *i ) )
    {
      // If findLayer returns true, the user hit Cancel All button
      break;
    }
  }
}
