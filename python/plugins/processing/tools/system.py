# -*- coding: utf-8 -*-

"""
***************************************************************************
    py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str


__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import time
import sys
import uuid
import math

from qgis.PyQt.QtCore import QDir
from qgis.core import QgsApplication

numExported = 1


def userFolder():
    userDir = os.path.join(QgsApplication.qgisSettingsDirPath(), 'processing')
    if not QDir(userDir).exists():
        QDir().mkpath(userDir)

    return str(QDir.toNativeSeparators(userDir))


def defaultOutputFolder():
    folder = os.path.join(userFolder(), 'outputs')
    if not QDir(folder).exists():
        QDir().mkpath(folder)

    return str(QDir.toNativeSeparators(folder))


def isWindows():
    return os.name == 'nt'


def isMac():
    return sys.platform == 'darwin'

_tempFolderSuffix = uuid.uuid4().hex


def tempFolder():
    tempDir = os.path.join(str(QDir.tempPath()), 'processing' + _tempFolderSuffix)
    if not QDir(tempDir).exists():
        QDir().mkpath(tempDir)

    return str(os.path.abspath(tempDir))


def setTempOutput(out, alg):
    if hasattr(out, 'directory'):
        out.value = getTempDirInTempFolder()
    else:
        ext = out.getDefaultFileExtension(alg)
        out.value = getTempFilenameInTempFolder(out.name + '.' + ext)


def getTempFilename(ext=None):
    tmpPath = tempFolder()
    t = time.time()
    m = math.floor(t)
    uid = '{:8x}{:05x}'.format(m, int((t - m) * 1000000))
    if ext is None:
        filename = os.path.join(tmpPath, '{}{}'.format(uid, getNumExportedLayers()))
    else:
        filename = os.path.join(tmpPath, '{}{}.{}'.format(uid, getNumExportedLayers(), ext))
    return filename


def getTempFilenameInTempFolder(basename):
    """Returns a temporary filename for a given file, putting it into
    a temp folder but not changing its basename.
    """

    path = tempFolder()
    path = os.path.join(path, uuid.uuid4().hex)
    mkdir(path)
    basename = removeInvalidChars(basename)
    filename = os.path.join(path, basename)
    return filename


def getTempDirInTempFolder():
    """Returns a temporary directory, putting it into a temp folder.
    """

    path = tempFolder()
    path = os.path.join(path, uuid.uuid4().hex)
    mkdir(path)
    return path


def removeInvalidChars(string):
    validChars = \
        'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:.'
    string = ''.join(c for c in string if c in validChars)
    return string


def getNumExportedLayers():
    global numExported
    numExported += 1
    return numExported


def mkdir(newdir):
    newdir = newdir.strip('\n\r ')
    if os.path.isdir(newdir):
        pass
    else:
        (head, tail) = os.path.split(newdir)
        if head and not os.path.isdir(head):
            mkdir(head)
        if tail:
            os.mkdir(newdir)
