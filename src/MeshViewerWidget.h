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

#ifndef OPENMESHAPPS_VIEWERWIDGET_HH
#define OPENMESHAPPS_VIEWERWIDGET_HH

//== INCLUDES =================================================================

#include <QWidget>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDirIterator>
#include <QInputDialog>
#include <QVector>
#include <QPointF>
//#include <qwt_scale_map.h>
//#include <qwt_plot.h>
//#include <qwt_plot_curve.h>

#include <OpenMesh/Tools/Utils/getopt.h>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <MeshViewerWidgetT.h>

#include "global.h"

//#include <qwt_plot_spectrocurve.h>

using namespace std;

//== CLASS DEFINITION =========================================================



class MeshViewerWidget : public MeshViewerWidgetT<TriangleMesh>
{
    Q_OBJECT
    
public:
    /// default constructor
    MeshViewerWidget(int _id, QWidget* parent=0) : MeshViewerWidgetT<TriangleMesh>(parent)
    {
        id_ = _id;
        
        
	}
    
public slots:
    
    void slotSetShapes(const std::vector<Shape*>& _shapes, int _id)
    {
        if (id_ != _id)
        {
            return;
        }
        shapes_ = _shapes;
        
        //resetScene();
        
        updateGL();
    }
    
    void slotRedraw(int _id)
    {
        if (id_ != _id)
        {
            return;
        }
        
        //resetScene();
        
        updateGL();
    }
    
    void setViewable(bool _visible, int _id)
    {
        if (id_ != _id)
        {
            return;
        }
        
        setVisible(_visible);
    }
    
    
private:
    
    void resetScene()
    {
        typename std::vector<Shape*>::iterator shapes_it(shapes_.begin()), shapes_end(shapes_.end());
        
        if (shapes_.size()==0)
        {
            //set_scene_pos(OpenMesh::Vec3f(0.0,0.0,0.0), radius()*1.5);
            updateGL();
            return;
        }
        
        Shape::BBox box;
        
        for( ; shapes_it!=shapes_end; ++shapes_it)
        {
            const Shape::BBox& cBox = (**shapes_it).bbox();
            
            box.min.minimize(cBox.min);
            box.max.maximize(cBox.max);
        }
        
        //std::cout << "SCENE bbmin: " << bbMin << std::endl;
        //std::cout << "SCENE bbmax: " << bbMax << std::endl;
        //std::cout << "SCENE POS:" << (bbMin+bbMax)*0.5 << std::endl;
        //std::cout << "SCENE Radius:" << (bbMin-bbMax).norm()*0.5 << std::endl;
        
        // set center and radius
        set_scene_pos( (box.min+box.max)*0.5, (box.min-box.max).norm()*0.5 );
        
        // for normal display
        normalScale_ = (box.max-box.min).min()*0.05f;
    }
    
    int id_ = -1;
 };

#endif
