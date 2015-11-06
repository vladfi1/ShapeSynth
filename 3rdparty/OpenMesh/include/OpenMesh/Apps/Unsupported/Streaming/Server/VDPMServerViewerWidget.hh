/*===========================================================================*\
 *                                                                           *
 *                               OpenMesh                                    *
 *      Copyright (C) 2001-2012 by Computer Graphics Group, RWTH Aachen      *
 *                           www.openmesh.org                                *
 *                                                                           *
 *---------------------------------------------------------------------------* 
 *  This file is part of OpenMesh.                                           *
 *                                                                           *
 *  OpenMesh is free software: you can redistribute it and/or modify         * 
 *  it under the terms of the GNU Lesser General Public License as           *
 *  published by the Free Software Foundation, either version 3 of           *
 *  the License, or (at your option) any later version with the              *
 *  following exceptions:                                                    *
 *                                                                           *
 *  If other files instantiate templates or use macros                       *
 *  or inline functions from this file, or you compile this file and         *
 *  link it with other files to produce an executable, this file does        *
 *  not by itself cause the resulting executable to be covered by the        *
 *  GNU Lesser General Public License. This exception does not however       *
 *  invalidate any other reasons why the executable file might be            *
 *  covered by the GNU Lesser General Public License.                        *
 *                                                                           *
 *  OpenMesh is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU LesserGeneral Public          *
 *  License along with OpenMesh.  If not,                                    *
 *  see <http://www.gnu.org/licenses/>.                                      *
 *                                                                           *
\*===========================================================================*/ 

/*===========================================================================*\
 *                                                                           *             
 *   $Revision: 736 $                                                         *
 *   $Date: 2012-10-08 09:30:49 +0200 (Mo, 08. Okt 2012) $                   *
 *                                                                           *
\*===========================================================================*/

#ifndef OPENMESH_APP_VDPMSTREAMING_SERVER_VDPMSTREAMINGSERVERWIDGET_HH
#define OPENMESH_APP_VDPMSTREAMING_SERVER_VDPMSTREAMINGSERVERWIDGET_HH

#include <qapplication.h>
#include <qwidget.h>
#include <qfiledialog.h>
#include <qstring.h>
#include <OpenMesh/Apps/VDProgMesh/Streaming/Server/ServerSideVDPM.hh>
#include <OpenMesh/Apps/VDProgMesh/Streaming/Server/VDPMServerSocket.hh>
#include <OpenMesh/Apps/VDProgMesh/Streaming/Server/VDPMServerSession.hh>


using OpenMesh::VDPM::set_debug_print;
using OpenMesh::VDPM::debug_print;


class VDPMServerViewerWidget : public QWidget
{
  Q_OBJECT
public:
  //VDPMServerViewerWidget(QWidget *_parent) : QWidget(_parent)
  VDPMServerViewerWidget() : QWidget()
  {
    VDPMServerSocket *server = new VDPMServerSocket(this);
    connect(server, 
	    SIGNAL(newConnect(VDPMServerSession*)), 
	    SLOT(newConnect(VDPMServerSession*)));

    vd_streaming_ = true;
  }

private:
  typedef ServerSideVDPMList::iterator  ServerSideVDPMListIter;
  ServerSideVDPMList  vdpms_;
  bool                vd_streaming_;

public:

  ServerSideVDPM* get_vdpm(const char _vdpm_name[256]);


public:

  bool  open_vd_prog_mesh(const char *_filename);
  bool  vd_streaming() const      { return  vd_streaming_; }

private slots:

  void newConnect(VDPMServerSession *s)
  {
    std::cout << "New connection" << std::endl;

    connect(s, SIGNAL(connectionClosed()), SLOT(connectionClosed()));
  }

  void connectionClosed()
  {
    std::cout << "Client closed connection" << std::endl;
  }

protected:

  virtual void keyPressEvent(QKeyEvent* _event);

};


#endif //OPENMESH_APP_VDPMSTREAMING_SERVER_VDPMSTREAMINGSERVERWIDGET_HH defined


