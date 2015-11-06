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

#define MESHVIEWERWIDGET_CPP

//== INCLUDES =================================================================

#ifdef _MSC_VER
//#  pragma warning(disable: 4267 4311)
#endif

//
#include <iostream>
#include <fstream>
// --------------------

#include <QFileInfo>
#include <QKeyEvent>
// --------------------
#include <OpenMesh/Core/Utils/vector_cast.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <MeshViewerWidgetT.h>

using namespace OpenMesh;
using namespace Qt;


#if defined(_MSC_VER)
#  undef min
#  undef max
#endif


//== IMPLEMENTATION ==========================================================

template <typename M> void MeshViewerWidgetT<M>::draw_scene(const std::string& _draw_mode)
{
    //std::cout << "MeshViewerWidgetT enter drawing" << std::endl;
    
    typename std::vector<Shape*>::iterator shapes_it(shapes_.begin()), shapes_end(shapes_.end());
    
    if (shapes_.size()==0)
    {
        return;
    }
    
    // Draw the global axes first
//    glDisable(GL_LIGHTING);
//    glBegin(GL_LINES);
//    
//    float cradius = radius();
//    OpenMesh::Vec3f origin(0.0,0.0,0.0);
//    OpenMesh::Vec3f x_axis(cradius,0.0,0.0);
//    OpenMesh::Vec3f y_axis(0.0,cradius,0.0);
//    OpenMesh::Vec3f z_axis(0.0,0.0,cradius);
//    
//    glColor3f(1.0f, 0.0f, 0.0f); // red for x
//    
//    glVertex3fv( &origin[0]);
//    
//    glVertex3fv( &x_axis[0]);
//    
//    glColor3f(0.0f, 1.0f, 0.0f); // green for y
//    glVertex3fv( &origin[0]);
//    
//    glVertex3fv( &y_axis[0]);
//    
//    glColor3f(0.0f, 0.0f, 1.0f); // blue for z
//    
//    glVertex3fv( &origin[0]);
//    
//    glVertex3fv( &z_axis[0]);
//    
//    glEnd();
    
    glEnable(GL_LIGHTING);
    if ( _draw_mode == "Points" )
    {
        glDisable(GL_LIGHTING);
        for( ; shapes_it!=shapes_end; ++shapes_it)
            (**shapes_it).draw(_draw_mode);
    }
    
    else if (_draw_mode == "Wireframe")
    {
        glDisable(GL_LIGHTING);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for( ; shapes_it!=shapes_end; ++shapes_it)
            (**shapes_it).draw(_draw_mode);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    else if ( _draw_mode == "Hidden-Line" )
    {
        glDisable(GL_LIGHTING);
        glShadeModel(GL_FLAT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );
        glDepthRange(0.01, 1.0);
        for( ; shapes_it!=shapes_end; ++shapes_it)
            (**shapes_it).draw(_draw_mode);
        
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
        glDepthRange( 0.0, 1.0 );
        for( ; shapes_it!=shapes_end; ++shapes_it)
            (**shapes_it).draw(_draw_mode);
        
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
    }
    
    else if (_draw_mode == "Solid Flat")
    {
        glEnable(GL_LIGHTING);
        glShadeModel(GL_FLAT);
        for( ; shapes_it!=shapes_end; ++shapes_it)
            (**shapes_it).draw(_draw_mode);
    }
    
    else if (_draw_mode == "Solid Smooth"        ||
             _draw_mode == "Strips'n VertexArrays" )
    {
        //std::cout << "MeshViewerWidgetT enter smooth" << std::endl;
        glEnable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        for( ; shapes_it!=shapes_end; ++shapes_it)
            (**shapes_it).draw(_draw_mode);
    }
    
    else if (_draw_mode == "Show Strips")
    {
        glDisable(GL_LIGHTING);
        for( ; shapes_it!=shapes_end; ++shapes_it)
            (**shapes_it).draw(_draw_mode);
    }
    
    else if (_draw_mode == "Colored Vertices" )
    {
        glDisable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        for( ; shapes_it!=shapes_end; ++shapes_it)
            (**shapes_it).draw(_draw_mode);
    }
    
    else if (_draw_mode == "Solid Colored Faces")
    {
        glDisable(GL_LIGHTING);
        glShadeModel(GL_FLAT);
        for( ; shapes_it!=shapes_end; ++shapes_it)
            (**shapes_it).draw(_draw_mode);
        setDefaultMaterial();
    }
    
    else if (_draw_mode == "Smooth Colored Faces" )
    {
        glEnable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        for( ; shapes_it!=shapes_end; ++shapes_it)
            (**shapes_it).draw(_draw_mode);
        setDefaultMaterial();
    }
    
    if (showVnormals_)
    {
        for(shapes_it = shapes_.begin(); shapes_it!=shapes_end; ++shapes_it)
        {
            (**shapes_it).drawNormals("Vertex",normalScale_);
        }
    }
    
    if (showFnormals_)
    {
        glDisable(GL_LIGHTING);
        for( ; shapes_it!=shapes_end; ++shapes_it)
        {
            (**shapes_it).drawNormals("Face",normalScale_);
        }
    }

}


template <typename M> void MeshViewerWidgetT<M>::pick_scene()
{

 	typename std::vector<Shape*>::iterator shapes_it(shapes_.begin()), shapes_end(shapes_.end());
    
    if (shapes_.size()==0)
    {
        return;
    }
    
    glDisable(GL_LIGHTING);
    
    for( ; shapes_it!=shapes_end; ++shapes_it)
    {
        (**shapes_it).pick();
    }

}


//-----------------------------------------------------------------------------

#define TEXMODE( Mode ) \
texMode_ = Mode; qDebug() << "Texture mode set to " << #Mode

template <typename M> void MeshViewerWidgetT<M>::keyPressEvent( QKeyEvent* _event)
{
    typename std::vector<Shape*>::iterator shapes_it(shapes_.begin()), shapes_end(shapes_.end());
    
    switch( _event->key() )
    {
        case Key_N:
            if ( _event->modifiers() & ShiftModifier )
            {
                showFnormals_ = !showFnormals_;
                qDebug() << "show face normals: " << (showFnormals_?"yes\n":"no\n");
            }
            else
            {
                showVnormals_ = !showVnormals_;
                qDebug() << "show vertex normals: " << (showVnormals_?"yes\n":"no\n");
            }
            updateGL();
            break;
            
        case Key_I:
            for( ; shapes_it!=shapes_end; ++shapes_it)
            {
                const Mesh& mesh_ = (**shapes_it).mesh();
                qDebug() << "\n# Vertices     : " << mesh_.n_vertices() ;
                qDebug() << "# Edges        : " << mesh_.n_edges() ;
                qDebug() << "# Faces        : " << mesh_.n_faces() ;
            }
            this->QGLViewerWidget::keyPressEvent( _event );
            break;
        default:
            this->QGLViewerWidget::keyPressEvent( _event );
    }

}

#undef TEXMODE


