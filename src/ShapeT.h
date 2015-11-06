//
//  ShapeT.h
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#ifndef SHAPET_H
#define SHAPET_H

#include <QImage>
#include <QMessageBox>
#include <QDebug>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/Utils/GenProg.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Tools/Utils/StripifierT.hh>

#include <QtOpenGL/qgl.h>

#include "utils.h"


template <typename M> class ShapeT
{
public:
    
    typedef M Mesh;
    
    struct BBox
    {
        typename Mesh::Point min = typename Mesh::Point(std::numeric_limits<typename Mesh::Point::value_type>::max(),std::numeric_limits<typename Mesh::Point::value_type>::max(),std::numeric_limits<typename Mesh::Point::value_type>::max());
        typename Mesh::Point max = typename Mesh::Point(-std::numeric_limits<typename Mesh::Point::value_type>::max(),-std::numeric_limits<typename Mesh::Point::value_type>::max(),-std::numeric_limits<typename Mesh::Point::value_type>::max());
    };
    
    ShapeT()
    {
        
    }
    
    ShapeT(unsigned int _id)
    {
        id_ = _id;
    }
    
    virtual bool open(const QString& _filename);
    
    virtual bool save(const char* _filename);
    
    virtual bool openMesh(const char* _filename);
    
    virtual void openMeshIfNotOpened();
    
    virtual BBox bbox();
    
    virtual void draw(const std::string& _drawMode) const;
    
    virtual void drawNormals(const std::string& _normalDrawMode, float _normalScale);
    
    virtual void pick() const;
    
    virtual void normalise();
    
    unsigned int id();
    
    void setID(unsigned int _id);
    
    Mesh& mesh();
    
    const Mesh& mesh() const;
    
    void setFilename(const QString& _filename);
    
    const QString& filename() const;
    
    void setIconFilename(const QString& _iconFilename);
    
    const QString& iconFilename() const;
    
    bool isMeshOpen() const;
    
public:
    
    double faceArea(const typename Mesh::FaceHandle& _fH);
    
    typename Mesh::Point faceCentroid(const typename Mesh::FaceHandle& _fH);
    
    typename Mesh::Point meshCentroid();
    
    typename Mesh::Point verticesCentroid();
    
    double averageRadius( const typename Mesh::Point& _centroid);
    
    typename Mesh::Point getRandomPoint(const std::vector<double>& _areas);
    
    void getRandomPoints(std::vector<typename Mesh::Point>& _points, int _nPoints);
    
protected:
    
    // Data
    
    Mesh mesh_;
    
    QString filename_;
    
    QString iconFilename_;
    
    unsigned int id_ = -1;
    
    std::map<unsigned int,unsigned int> indexMap_;
};

//=============================================================================
#if !defined(SHAPET_CPP)
#  define SHAPET_TEMPLATES
#  include "ShapeT.cpp"
#endif
//=============================================================================
#endif
