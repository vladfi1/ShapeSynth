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
 *   $Date: 2012-10-08 09:30:49 +0200 (Mo, 08 Okt 2012) $                   *
 *                                                                           *
 \*===========================================================================*/


// Modified by Melinos Averkiou

// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#ifndef MESHVIEWERWIDGETT_H
#define MESHVIEWERWIDGETT_H


//== INCLUDES =================================================================

#include <string>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/Utils/GenProg.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Tools/Utils/StripifierT.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <QGLViewerWidget.h>

#include "ShapeT.h"

//== CLASS DEFINITION =========================================================


template <typename M>
class MeshViewerWidgetT : public QGLViewerWidget
{
    
public:
    
    typedef M                             Mesh;
    typedef ShapeT<Mesh>                  Shape;
    
public:
    
    // Default constructor
    MeshViewerWidgetT(QWidget* _parent=0)
    : QGLViewerWidget(_parent),
    showVnormals_(false),
    showFnormals_(false)
    {
        add_draw_mode("Points");
        add_draw_mode("Hidden-Line");
    }
    
    /// destructor
    ~MeshViewerWidgetT() {}
    
public:

   
protected:
    
    // inherited drawing method
    virtual void draw_scene(const std::string& _draw_mode);
    
    // inherited picking method
    virtual void pick_scene();
    
protected: // inherited
    
    virtual void keyPressEvent( QKeyEvent* _event);
    
protected:
    
    bool                   showVnormals_;
    bool                   showFnormals_;
    float                  normalScale_;
    
    // The vector of the shapes to draw
    std::vector<Shape*>    shapes_;
};


//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(MESHVIEWERWIDGET_CPP)
#  define MESHVIEWERWIDGET_TEMPLATES
#  include "MeshViewerWidgetT.cpp"
#endif
//=============================================================================
#endif // OPENMESHAPPS_MESHVIEWERWIDGETT_HH defined
//=============================================================================

