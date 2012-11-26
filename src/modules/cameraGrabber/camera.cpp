/**************************************************************************
*                         Vision Magic                                    *
*   Copyright (C) 2012 by:                                                *
*      Tarek Taha  <tarek@tarektaha.com>                                  *
*                                                                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not,If not, see                           *
*    <http://www.gnu.org/licenses/>.                                      *
***************************************************************************/
#include "camera.h"
#include "avtcamera.h"
#include "opencvcamera.h"

Camera::Camera(QObject *parent) :
    QThread(parent)
{
}

Camera* Camera::factory(const QString& type,int camId) throw(BadCameraCreation)
{
  if(type == "AVT")
      return new AVTCamera(camId);
  if(type == "OPENCV_CAM")
      return new OpenCVCamera(camId);
  throw BadCameraCreation(type);
}
