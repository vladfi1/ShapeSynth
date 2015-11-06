//
//  ShapeListWidgetItem.h
//  ShapeAligner
//
//  Created by localadmin on 20/05/2013.
//
//

#ifndef SHAPELISTWIDGETITEM_H
#define SHAPELISTWIDGETITEM_H


#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/Utils/GenProg.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Tools/Utils/StripifierT.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#include <QListWidgetItem>
#include <QDebug>

#include "global.h"

template<typename S> class ShapeListWidgetItem : public QListWidgetItem
{
    
public:
    
    ShapeListWidgetItem(QListWidget* parent=0) : QListWidgetItem(parent, QListWidgetItem::UserType)
    {
        
    }
    
    typedef S Shape;
    
    Shape& shape()
    {
        return shape_;
    }
    
    const Shape& shape() const
    {
        return shape_;
    }
    
protected:
    
    Shape shape_;
    
    
};

#endif
