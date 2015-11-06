//
//  Match.h
//  
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>


#ifndef MATCHT_H
#define MATCHT_H

#include <QTextStream>
#include <QFile>
#include <QLabel>
#include <QDialog>
#include <QHBoxLayout>

#include <unordered_map>

#include <XForm.h>

#include "ShapeT.h"
#include "global.h"
#include <GL/glut.h>

template <typename M> class MatchT: public ShapeT<M>
{

public:
    
    struct Constraint
    {
        int type_ = -1;
        std::pair<int, int> partIndices_;
        std::pair<int, int> partIDs_;
    };
    
    struct Part
    {
        int partID_;
        int partType_;
        OpenMesh::Vec3f pos_;
        OpenMesh::Vec3f scale_;
        OpenMesh::Vec3f axis1_;
        OpenMesh::Vec3f axis2_;
        OpenMesh::Vec3f axis3_;
        ShapeT<M> partShape_;
        
        void recalculateBox(const xform& _alignMtx);
    };
    
    struct MeshPoint
    {
        int partID_; //ID of the template part to which this point corresponds (label id)
        OpenMesh::Vec3f pos_; // point position
    };
    
    MatchT()
    {
        templateID_ = -1;
        groupID_ = -1;
        nparts_ = -1;
        npnts_ = -1;
        fitError_ = 0.0;
        
        selectedPartID_ = -1;
        
        displayMode_ = BOX;
        
        float minus_inf = -std::numeric_limits<float>::max();
        
        meshCentroid_[0] = minus_inf;
        meshCentroid_[1] = minus_inf;
        meshCentroid_[2] = minus_inf;
        
        meshAvgRadius_ = minus_inf;
    }
    
    MatchT(unsigned int _id)
    {
        MatchT();
        ShapeT<M>::id_ = _id;
    }
    
    virtual bool open(const char* _filename);
    
    bool open(const QString& _values);
    
    void save(QTextStream& _out);
    
    virtual bool openMesh(const char* _filename);
    
    virtual void openMeshIfNotOpened();
    
    virtual void draw(const std::string& _drawMode) const;
    
    virtual void pick() const;
    
    virtual typename ShapeT<M>::BBox bbox();
    
    int templateID() const;
    
    void setTemplateID(int _templateID);
    
    int groupID() const;
    
    void setGroupID(int _groupID);
    
    const xform& alignMtx() const;
    
    void setAlignMtx(const xform& _alignMtx);
    
    int nparts();
    
    void setNparts(int _nparts);
    
    int npnts();
    
    void setNpnts(int _npnts);
    
    std::vector<Part>& parts();
    
    const std::vector<Part>& parts() const;
    
    void setParts(const std::vector<Part>& _parts);
    
    const std::vector<MeshPoint>& points() const;
    
    std::vector<MeshPoint>& points();
    
    void setPoints(const std::vector<MeshPoint>& _points);
    
    const QString& meshFilename();
    
    void setMeshFilename(const QString& _filename);
    
    const OpenMesh::Vec2f& descriptor2D();
    
    void setDescriptor2D(const OpenMesh::Vec2f& _descriptor2D);
    
    const std::vector<float>& descriptor();
    
    void setDescriptor(const std::vector<float>& _descriptor);
    
    double fitError() const;
    
    void setFitError(double _fitError);
    
    const OpenMesh::Vec3f& meshCentroid();
    
    void setMeshCentroid(const OpenMesh::Vec3f& _meshCentroid);
    
    float meshAvgRadius();
    
    void setMeshAvgRadius(float _meshAvgRadius);
    
    int selectedPartID();
    
    void setSelectedPartID(int _selectedPartID);
    
    int displayMode();
    
    void setDisplayMode(int _displayMode);
    
    void setMesh(const M& _mesh);
    
    int label() const;
    
    void setLabel(double _label);

    void split();
    
    bool segmented();
    
    void setSegmented(bool _segmented);
    
    void recalculateBoxes();
    
    void saveMatchDescriptorToFile(QTextStream& _out);
    
    void saveMatchPointsToFile(const QString& _filename);
    
    void savePartMeshes();
    
    void savePartMeshes(const QString& _directoryPath);
    
    void saveMesh(const QString& _directoryPath);
    
    void openPartMeshes();
   
    bool openPartMeshIfNotOpened(Part& _cPart);
    
    void disableAlignMtx();
    
    void enableAlignMtx();
    
    void showPartDescriptors(int i);
    
    std::vector<Constraint>& constraints();
    
    QTextStream openSegFile(const QString& _filename);
    
    void overrideColor(bool _colorOverride);
    
    void setHoveredPartID(int _hoveredPartID);
    
private:
    
    void normaliseMeshToTemplate();

    void alignMeshToTemplate();
    
    void draw_point(const MeshPoint& cPoint) const;
    
    void draw_part(const Part& cPart) const;
    
    void pick_part(const Part& cPart);
    
    
    // DATA
    int templateID_;
    
    int groupID_;
    
    xform alignMtx_;
    
    int nparts_;
    std::vector<Part> parts_;
    
    int npnts_;
    std::vector<MeshPoint> points_;
    
    QString meshFilename_;
    
    std::vector< float> descriptor_;
    
    OpenMesh::Vec2f descriptor2D_;
    
	double fitError_;
    
    OpenMesh::Vec3f meshCentroid_;
    
    float meshAvgRadius_;
    
    int selectedPartID_;
    
    int displayMode_;
    
    int label_;

    bool segmented_ = false;
    
    bool disableAlignmtx_ = false;
    
    bool normalised_ = false;
    
    bool aligned_ = false;
    
    std::vector< Constraint > constraints_;
    
    bool colorOverride_ = false;
    
    int hoveredPartID_ = -1;
};

//=============================================================================
#if !defined(MATCHT_CPP)
#  define MATCHT_TEMPLATES
#  include "MatchT.cpp"
#endif
//=============================================================================
#endif
