//
//  MatchT.cpp
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>


#define MATCHT_CPP
#include "MatchT.h"


template <typename M> int MatchT<M>::templateID() const
{
    return templateID_;
}

template <typename M> void MatchT<M>::setTemplateID(int _templateID)
{
    templateID_ = _templateID;
}

template <typename M> int MatchT<M>::groupID() const
{
    return groupID_;
}

template <typename M> void MatchT<M>::setGroupID(int _groupID)
{
    groupID_ = _groupID;
}

template <typename M> const xform& MatchT<M>::alignMtx() const
{
    return alignMtx_;
}

template <typename M> void MatchT<M>::setAlignMtx(const xform& _alignMtx)
{
    alignMtx_ = _alignMtx;
}

template <typename M> int MatchT<M>::nparts()
{
    return nparts_;
}

template <typename M> void MatchT<M>::setNparts(int _nparts)
{
    nparts_ = _nparts;
}

template <typename M> int MatchT<M>::npnts()
{
    return npnts_;
}

template <typename M> void MatchT<M>::setNpnts(int _npnts)
{
    npnts_ = _npnts;
}

template <typename M> std::vector<typename MatchT<M>::Part>& MatchT<M>::parts()
{
    return parts_;
}

template <typename M> const std::vector<typename MatchT<M>::Part>& MatchT<M>::parts() const
{
    return parts_;
}

template <typename M> void MatchT<M>::setParts(const std::vector<Part>& _parts)
{
    parts_ = _parts;
}

template <typename M> const std::vector<typename MatchT<M>::MeshPoint>& MatchT<M>::points() const
{
    return points_;
}

template <typename M> std::vector<typename MatchT<M>::MeshPoint>& MatchT<M>::points()
{
    return points_;
}

template <typename M> void MatchT<M>::setPoints(const std::vector<MeshPoint>& _points)
{
    points_ = _points;
}

template <typename M> const QString& MatchT<M>::meshFilename()
{
    return meshFilename_;
}

template <typename M> void MatchT<M>::setMeshFilename(const QString& _filename)
{
    meshFilename_ = _filename;
}

template <typename M> const OpenMesh::Vec2f& MatchT<M>::descriptor2D()
{
    return descriptor2D_;
}

template <typename M> void MatchT<M>::setDescriptor2D(const OpenMesh::Vec2f& _descriptor2D)
{
    descriptor2D_ = _descriptor2D;
}

template <typename M> const std::vector<float>& MatchT<M>::descriptor()
{
    return descriptor_;
}

template <typename M> void MatchT<M>::setDescriptor(const std::vector<float>& _descriptor)
{
    descriptor_ = _descriptor;
}

template <typename M> double MatchT<M>::fitError() const
{
    return fitError_;
}

template <typename M> void MatchT<M>::setFitError(double _fitError)
{
    fitError_ = _fitError;
}

template <typename M> const OpenMesh::Vec3f& MatchT<M>::meshCentroid()
{
    return meshCentroid_;
}

template <typename M> void MatchT<M>::setMeshCentroid(const OpenMesh::Vec3f& _meshCentroid)
{
    meshCentroid_ = _meshCentroid;
}

template <typename M> float MatchT<M>::meshAvgRadius()
{
    return meshAvgRadius_;
}

template <typename M> void MatchT<M>::setMeshAvgRadius(float _meshAvgRadius)
{
    meshAvgRadius_ = _meshAvgRadius;
}

template <typename M> int MatchT<M>::selectedPartID()
{
    return selectedPartID_;
}

template <typename M> void MatchT<M>::setSelectedPartID(int _selectedPartID)
{
    selectedPartID_ = _selectedPartID;
}

template <typename M> int MatchT<M>::displayMode()
{
    return displayMode_;
}

template <typename M> void MatchT<M>::setDisplayMode(int _displayMode)
{
    displayMode_ = _displayMode;
}

template <typename M> void MatchT<M>::setMesh(const M& _mesh)
{
    ShapeT<M>::mesh_ = _mesh;
}

template <typename M> int MatchT<M>::label() const
{
    return label_;
}

template <typename M> void MatchT<M>::setLabel(double _label)
{
    label_ = _label;
}


template <typename M> void MatchT<M>::openMeshIfNotOpened()
{
    if(OPEN_ORIGINAL_MESH)
    {
        if(ShapeT<M>::mesh_.n_vertices() == 0)
        {
            //qWarning() << "Selected shape's mesh was not loaded before, loading mesh now from: " << meshFilename_ ;
            
            if(!openMesh(meshFilename_.toLocal8Bit()))
            {
                qCritical() << "Cannot read mesh from file: " << meshFilename_;
            }
        }
    }
    else
    {
        typename std::vector<Part>::iterator pIt(parts_.begin()), pEnd(parts_.end());
        
        for( ; pIt != pEnd; ++pIt)
        {
            openPartMeshIfNotOpened(*pIt);
        }

    }
}

// Read the match data from a single .match file
template <typename M> bool MatchT<M>::open(const char *_filename)
{
    qDebug() << "Open match" ;
    FILE * file = fopen(_filename, "rb");

    if(file==NULL)
	{
		qCritical() << "Could not open file " << QString(_filename) ;
		return false;
	}
  
    descriptor_.clear();
    descriptor2D_ = OpenMesh::Vec2f(0,0);
    
    fread(&templateID_, sizeof(int), 1, file);
    
    // xform is stored in column-major order, so is the transformation matrix from the match file
    for (int c=0; c<16; c++)
    {
        fread(&alignMtx_[c], sizeof(double), 1, file);
    }
    
    fread(&nparts_, sizeof(int), 1, file);
    
    int nparts2Remove = 0;
    
    // Go over all parts
    for (int p=0; p < nparts_; p++)
    {
        Part cPart;
        
        fread(&cPart.partID_, sizeof(int), 1, file);
        fread(&cPart.partType_,sizeof(int),1,file);
        
        cPart.pos_ = OpenMesh::Vec3d(0,0,0);
        cPart.scale_ = OpenMesh::Vec3d(0,0,0);
        cPart.axis1_ = OpenMesh::Vec3d(1,0,0);
        cPart.axis2_ = OpenMesh::Vec3d(0,1,0);
        cPart.axis3_ = OpenMesh::Vec3d(0,0,1);
        
        cPart.partShape_.setID(cPart.partID_);
        

        // if we have a box
        if (cPart.partType_ == 1)
        {
            fread(&cPart.pos_[0],sizeof(double),1,file);
            fread(&cPart.pos_[1],sizeof(double),1,file);
            fread(&cPart.pos_[2],sizeof(double),1,file);
            
            fread(&cPart.scale_[0],sizeof(double),1,file);
            fread(&cPart.scale_[1],sizeof(double),1,file);
            fread(&cPart.scale_[2],sizeof(double),1,file);
            
            fread(&cPart.axis1_[0],sizeof(double),1,file);
            fread(&cPart.axis1_[1],sizeof(double),1,file);
            fread(&cPart.axis1_[2],sizeof(double),1,file);
            
            fread(&cPart.axis2_[0],sizeof(double),1,file);
            fread(&cPart.axis2_[1],sizeof(double),1,file);
            fread(&cPart.axis2_[2],sizeof(double),1,file);
            
            fread(&cPart.axis3_[0],sizeof(double),1,file);
            fread(&cPart.axis3_[1],sizeof(double),1,file);
            fread(&cPart.axis3_[2],sizeof(double),1,file);
            
            OpenMesh::Vec3f min = cPart.pos_ - cPart.scale_;
            OpenMesh::Vec3f max = cPart.pos_ + cPart.scale_;
            
            descriptor_.push_back(min[0]);
            descriptor_.push_back(min[1]);
            descriptor_.push_back(min[2]);
            
            descriptor_.push_back(max[0]);
            descriptor_.push_back(max[1]);
            descriptor_.push_back(max[2]);
        }
        
        //Ignore this part if its type is 0
        if(cPart.partType_ == 0)
        {
            nparts2Remove++;
            continue;
        }
        
        parts_.push_back(cPart);
    }
    nparts_ -= nparts2Remove;
    fread(&npnts_,sizeof(int),1,file);
    
    
    // Go over all points
    
    for (int i=0; i<npnts_; i++)
    {
        MeshPoint cPoint;
   
        fread(&cPoint.pos_[0],sizeof(double),1,file);
        fread(&cPoint.pos_[1],sizeof(double),1,file);
        fread(&cPoint.pos_[2],sizeof(double),1,file);
        
        fread(&cPoint.partID_,sizeof(int),1,file);
        
		// Increase partID by 1 because the part ID starts from 2 as there is no 1 (root)
		cPoint.partID_ += 1;
        
        points_.push_back(cPoint);
        
    }
    
    fclose(file);
    
    return true;
}

// Read the match data from a single line string, of an already open collection file
template <typename M> bool MatchT<M>::open(const QString& _values)
{
    
    QStringList lineList = _values.split(",");
    
    int lineListSize = lineList.size();
    
    descriptor_.clear();
    descriptor2D_ = OpenMesh::Vec2f(0,0);
    
    int ind = 0;
    
    ShapeT<M>::filename_ = lineList.at(ind); // name is at 0
    ind++;
    
    templateID_ = lineList.at(ind).toInt(); // template id is at 1
    ind++;
    
    groupID_ = lineList.at(ind).toInt(); // group id is at 2
    ind++;
    
    for (int i=0; i<16; i++)
    {
        // Alignment matrix is between positions 3 - 18
        alignMtx_[i] = lineList.at(ind).toDouble();
        ind++;
    }
    
    nparts_ = lineList.at(ind).toInt(); // nparts is at 19
    ind++;
    
    int nparts2Remove = 0;
    // Go over all parts (from 20 until 20 + nparts*8)
    for (int p=0; p < nparts_; p++)
    {
        Part cPart;
        
        cPart.partID_ = lineList.at(ind).toInt();
        ind++;
        cPart.partType_ = lineList.at(ind).toInt();
        ind++;
        
        cPart.pos_[0] = lineList.at(ind).toDouble();
        ind++;
        cPart.pos_[1] = lineList.at(ind).toDouble();
        ind++;
        cPart.pos_[2] = lineList.at(ind).toDouble();
        ind++;
        cPart.scale_[0] = lineList.at(ind).toDouble();
        ind++;
        cPart.scale_[1] = lineList.at(ind).toDouble();
        ind++;
        cPart.scale_[2] = lineList.at(ind).toDouble();
        ind++;
        
        cPart.partShape_.setID(cPart.partID_);
        
        // Don't add this part to the parts vector if its type is 0
        if(cPart.partType_ == 0)
        {
            nparts2Remove++;
            continue;
        }
        
        OpenMesh::Vec3f min = cPart.pos_ - cPart.scale_;
        OpenMesh::Vec3f max = cPart.pos_ + cPart.scale_;
        
        descriptor_.push_back(min[0]);
        descriptor_.push_back(min[1]);
        descriptor_.push_back(min[2]);
        
        descriptor_.push_back(max[0]);
        descriptor_.push_back(max[1]);
        descriptor_.push_back(max[2]);
        
        parts_.push_back(cPart);
        
    }
    nparts_ -= nparts2Remove;
    npnts_ = lineList.at(ind).toInt(); // npnts is at 20 + nparts*8 + 1
    ind++;
    
    //Go over all points (from 20 + nparts*8 + 2 until 20 + nparts*8 + 2 + npnts*4)
    for (int p=0; p < npnts_; p++)
    {
        MeshPoint cPoint;
        
        cPoint.partID_ = lineList.at(ind).toInt();
        ind++;
        
        cPoint.pos_[0] = lineList.at(ind).toDouble();
        ind++;
        cPoint.pos_[1] = lineList.at(ind).toDouble();
        ind++;
        cPoint.pos_[2] = lineList.at(ind).toDouble();
        ind++;
        
        points_.push_back(cPoint);
        
    }
    
    fitError_ = lineList.at(ind).toDouble();
    ind++;
    
    meshCentroid_[0] = lineList.at(ind).toDouble();
    ind++;
    
    meshCentroid_[1] = lineList.at(ind).toDouble();
    ind++;
    
    meshCentroid_[2] = lineList.at(ind).toDouble();
    ind++;
    
    meshAvgRadius_ = lineList.at(ind).toDouble();
    
    if (OPEN_DESCRIPTOR)
    {
        ind++;
        descriptor2D_[0] = lineList.at(ind).toDouble();
        ind++;
        descriptor2D_[1] = lineList.at(ind).toDouble();
        ind++;
        label_ = lineList.at(ind).toInt();
    }
    
    return true;
}

// Save the match data in a single line of an already open collection file
template<typename M> void MatchT<M>::save(QTextStream& _out)
{
    QString name = ShapeT<M>::filename_.split("/").last();
    
    _out << name << "," << templateID_ << "," << groupID_ << ",";
    
    
    xform tmpIdent;
    
    for (int i=0; i<16; i++)
    {
        // This is just for convenience, we can choose to save the collection with an identity transform for each match file, this means we will apply the original transformation (the one we are not saving) to the match points, as well as to the accompanying mesh (see normaliseMesh function)
        if(ALIGN_MATCH_POINTS_BEFORE_SAVING)
        {
            _out << tmpIdent[i] << "," ; 
        }
        else
        {
            _out << alignMtx_[i] << ",";
        }
    }
    
    
    // Write parts
    _out << nparts_ << ",";
    
    if (RECOMPUTE_BOXES_BEFORE_SAVING)
    {
        recalculateBoxes();
    }
    
    typename std::vector<Part>::const_iterator itPart (parts_.begin()), itPartEnd(parts_.end());
    
    for (; itPart != itPartEnd; ++itPart)
    {
        _out << itPart->partID_ << "," << itPart->partType_ << ",";
        _out << itPart->pos_[0] << "," << itPart->pos_[1] << "," << itPart->pos_[2] << ",";
        _out << itPart->scale_[0] << "," << itPart->scale_[1] << "," << itPart->scale_[2] << ",";
    }
    
    // Write points
    _out << npnts_ << ",";
    
    typename std::vector<MeshPoint>::const_iterator itPoint (points_.begin()), itPointEnd(points_.end());
    
    for (; itPoint != itPointEnd; ++itPoint)
    {
        _out << itPoint->partID_ << ",";
        
        if(ALIGN_MATCH_POINTS_BEFORE_SAVING)
        {
            OpenMesh::Vec3f alignedPos = mv(alignMtx_, itPoint->pos_);
            _out << alignedPos[0] << "," << alignedPos[1] << "," << alignedPos[2] << ",";
        }
        else
        {
            _out << itPoint->pos_[0] << "," << itPoint->pos_[1] << "," << itPoint->pos_[2] << ",";
        }
    }
    
    _out << fitError_ << ",";
    
    _out << meshCentroid_[0] << "," << meshCentroid_[1] << "," << meshCentroid_[2] << ",";
    
    _out << meshAvgRadius_ << ",";
    
    if(SAVE_DESCRIPTOR)
    {
        _out << descriptor2D_[0] << "," << descriptor2D_[1] << "," << 0;
    }
    
    _out << "\n";
    
    
    
}

template <typename M> void MatchT<M>::normaliseMeshToTemplate()
{
    if(LOADED_DATASET ==SID_PLANES)
    {
        qDebug() << "NORMALISING!!" ; 
        ShapeT<M>::normalise();
    
        return;
    }
    
    // Make sure we can normalise first
    float minus_inf = -std::numeric_limits<float>::max();
    
    if(meshCentroid_[0] == minus_inf || meshCentroid_[1] == minus_inf || meshCentroid_[2] == minus_inf || meshAvgRadius_ == minus_inf)
    {
        qCritical() << "Mesh centroid and average radius have not been set, cannot normalise!" ;
        return;
    }
    typename ShapeT<M>::Mesh::VertexIter vIt(ShapeT<M>::mesh_.vertices_begin());
    typename ShapeT<M>::Mesh::VertexIter vEnd(ShapeT<M>::mesh_.vertices_end());

    float scaleFactor = 1.0 / meshAvgRadius_;
    
    for (; vIt!=vEnd; ++vIt)
    {
        ShapeT<M>::mesh_.set_point(vIt, (scaleFactor*(ShapeT<M>::mesh_.point(vIt) - meshCentroid_)));
    }
    
    normalised_ = true;
}

template <typename M> void MatchT<M>::alignMeshToTemplate()
{
    typename ShapeT<M>::Mesh::VertexIter vIt(ShapeT<M>::mesh_.vertices_begin());
    typename ShapeT<M>::Mesh::VertexIter vEnd(ShapeT<M>::mesh_.vertices_end());
    
    for (; vIt!=vEnd; ++vIt)
    {
        ShapeT<M>::mesh_.set_point(vIt, mv(alignMtx_, ShapeT<M>::mesh_.point(vIt)) );
    }
    
    aligned_ = true;
}

template <typename M> bool MatchT<M>::openMesh(const char* _filename)
{
    if(OPEN_ORIGINAL_MESH)
    {
        ShapeT<M>::openMesh(_filename);
    }
    
    if (NORMALISE_MATCH_MESH_AFTER_OPENING && !normalised_)
    {
        normaliseMeshToTemplate();
    }
    if (ALIGN_MATCH_MESH_AFTER_OPENING && !aligned_)
    {
        alignMeshToTemplate();
    }
    
    if (OPEN_PART_MESHES_AFTER_OPENING_MATCH_MESH)
    {
        // Added this extra function to open the part meshes, just in case the mesh was segmented and the part meshes were written in separate files
        openPartMeshes();
    }
    
    return true;
}

template <typename M> void MatchT<M>::pick() const
{
    if(displayMode_ & MESH)
    {
        glDisable(GL_BLEND);
        glDisable(GL_LIGHTING);
        
        glPushMatrix();
        
        if (!disableAlignmtx_)
        {
            glMultMatrixd(alignMtx_);
        }
        
        if(!segmented_)
        {
            if(ShapeT<M>::isMeshOpen())
            {
                ShapeT<M>::pick();
            }
        }
        else
        {
            if(selectedPartID_ >=0)
            {
                // A specific part is chosen, so draw it
                const Part& cPart = parts_.at(selectedPartID_);
                cPart.partShape_.pick();
            }
            else
            {
                typename std::vector<Part>::const_iterator itPart(parts_.begin()), itEnd(parts_.end());
                for (; itPart != itEnd; ++itPart)
                {
                    if(itPart->partType_!=0)
                    {
                        itPart->partShape_.pick();
                    }
                }
            }
        }
        glPopMatrix();
    }
    
    glDisable(GL_LIGHTING);

    if(displayMode_ & BOX)
    {
        // Draw template boxes
        if(selectedPartID_ >=0)
        {
            // A specific part is chosen, so draw it
            const Part& cPart = parts_.at(selectedPartID_);
            cPart.partShape_.pick();
        }
        else
        {
            typename std::vector<Part>::const_iterator itPart(parts_.begin()), itEnd(parts_.end());
            
            for (; itPart != itEnd; ++itPart)
            {
                itPart->partShape_.pick();
            }
        }
    }
}

template <typename M> void MatchT<M>::draw(const std::string& _drawMode) const
{
    // Draw points
    if(displayMode_ & POINTS)
    {
        typename std::vector<MeshPoint>::const_iterator itPoint(points_.begin()), itEnd(points_.end());
        
        for (; itPoint != itEnd; ++itPoint)
        {
            draw_point(*itPoint);
        }
    }

    // draw mesh
    if(displayMode_ & MESH)
    {
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);

        glPushMatrix();
        
        if (!disableAlignmtx_)
        {
            glMultMatrixd(alignMtx_);
        }
       
        if(!segmented_)
        {
            if(ShapeT<M>::isMeshOpen())
            {
                ShapeT<M>::draw(_drawMode);
            }
        }
        else
        {
            if(selectedPartID_ >=0)
            {
                // A specific part is chosen, so draw it
                const Part& cPart = parts_.at(selectedPartID_);
                // part ids start from 2, so subtract 2, but just in case, check it's not smaller than 0
                int col = cPart.partID_-2;
                
                if(col<0)
                {
                    col = 0;
                }
                // Draw all parts with the same color if colorOverride is true
                if (colorOverride_)
                {
                    col = 1;
                    
                    // If all parts are being drawn in the same color and this part is hovered, draw only this part in red
                    if(hoveredPartID_>=0 && cPart.partID_ == hoveredPartID_)
                    {
                        col = 0;
                    }
                }
                
                OpenMesh::Vec4f color(shapeColors[col][0],shapeColors[col][1],shapeColors[col][2],shapeColors[col][3]);
                
                int _f=GL_FRONT_AND_BACK, _m=GL_DIFFUSE;
                
                glMaterialfv(_f, _m, &color[0]);
                
                cPart.partShape_.draw(_drawMode);
            }
            else
            {
                typename std::vector<Part>::const_iterator itPart(parts_.begin()), itEnd(parts_.end());
                
                for (; itPart != itEnd; ++itPart)
                {
                    if(itPart->partType_!=0)
                    {
                        // part ids start from 2, so subtract 2, but just in case, check it's not smaller than 0
                        int col = itPart->partID_-2;
                        
                        if(col<0)
                        {
                            col = 0;
                        }
                        // Draw all parts with the same color if colorOverride is true
                        if (colorOverride_)
                        {
                            col = 1;
                            
                            // If all parts are being drawn in the same color and this part is hovered, draw only this part in red
                            if(hoveredPartID_>=0 && itPart->partID_ == hoveredPartID_)
                            {
                                col = 0;
                            }
                        }
                        OpenMesh::Vec4f color(shapeColors[col][0],shapeColors[col][1],shapeColors[col][2],shapeColors[col][3]);
                        
                        int _f=GL_FRONT_AND_BACK, _m=GL_DIFFUSE;
                        
                        glMaterialfv(_f, _m, &color[0]);

                        itPart->partShape_.draw(_drawMode);
                    }
                }
            }
        }
        
        glPopMatrix();
    }
    
    
    // draw boxes
    if(displayMode_ & BOX)
    {
        glEnable(GL_BLEND);
        glDisable(GL_LIGHTING);
        glShadeModel(GL_SMOOTH);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Draw template boxes
        if(selectedPartID_ >=0)
        {
            // A specific part is chosen, so draw it
            const Part& cPart = parts_.at(selectedPartID_);
            draw_part(cPart);
        }
        else
        {
            typename std::vector<Part>::const_iterator itPart(parts_.begin()), itEnd(parts_.end());
            
            for (; itPart != itEnd; ++itPart)
            {
                draw_part(*itPart);
            }
        }
    }

    
}

template <typename M> void MatchT<M>::draw_point(const MeshPoint& cPoint) const
{
    glPushMatrix();
    
    // part ids start from 2, so subtract 2, but just in case, check it's not smaller than 0
    int col = cPoint.partID_-2;
    
    if(col<0)
    {
        col = 0;
    }
    
    glColor4f(shapeColors[col][0],shapeColors[col][1],shapeColors[col][2],shapeColors[col][3]);
    
    glMultMatrixd(alignMtx_);
    
    glTranslated(cPoint.pos_[0], cPoint.pos_[1], cPoint.pos_[2]);
    
    glutSolidSphere(0.01, 32, 32);
    
    glPopMatrix();
}


template <typename M> void MatchT<M>::draw_part(const Part& cPart) const
{
    glPushMatrix();
    
    // part ids start from 2, so subtract 2, but just in case, check it's not smaller than 0
    int col = cPart.partID_-2;
    
    if(col<0)
    {
        col = 0;
    }
    // Draw all parts with the same color if colorOverride is true
    if (colorOverride_)
    {
        col = 1;
        
        // If all parts are being drawn in the same color and this part is hovered, draw only this part in red
        if(hoveredPartID_>=0 && cPart.partID_ == hoveredPartID_)
        {
            col = 0;
        }
    }
    
    glColor4f(shapeColors[col][0],shapeColors[col][1],shapeColors[col][2],0.5f);
    
    //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,shapeColors[itPart->partID]);
    
    glTranslated(cPart.pos_[0], cPart.pos_[1], cPart.pos_[2]);
    
    glScaled(2.0 * cPart.scale_[0], 2.0 * cPart.scale_[1], 2.0 * cPart.scale_[2]);
    
    glutSolidCube(1);
    
    glPopMatrix();
    
}

template <typename M> void MatchT<M>::pick_part(const Part& cPart)
{
    glLoadName(cPart.partID_);
    
    glPushMatrix();
    
    glTranslated(cPart.pos_[0], cPart.pos_[1], cPart.pos_[2]);
    
    glScaled(2.0 * cPart.scale_[0], 2.0 * cPart.scale_[1], 2.0 * cPart.scale_[2]);
    
    glutSolidCube(1);
    
    glPopMatrix();
    
}


template <typename M> typename ShapeT<M>::BBox MatchT<M>::bbox()
{
    if(ShapeT<M>::isMeshOpen())
    {
        return ShapeT<M>::bbox();
    }
    else
    {
        typename ShapeT<M>::BBox box;
        typename std::vector<Part>::const_iterator itPart(parts_.begin()), partsEnd(parts_.end());
        
        for ( ; itPart != partsEnd; ++itPart)
        {
            OpenMesh::Vec3d min(0,0,0);
            OpenMesh::Vec3d max(0,0,0);
            
            min = itPart->pos_ - itPart->scale_;
            max = itPart->pos_ + itPart->scale_;
            
            box.min.minimize(OpenMesh::vector_cast<typename M::Point>(min));
            box.max.maximize(OpenMesh::vector_cast<typename M::Point>(max));
        }
        
        return box;
    }
}


template <typename M> typename M::value_type sqrDistPoint2Triangle(const M& p0, const M& p1, const M& p2, const M& p)
{
    M diff = p0 - p;
    M edge0 = p1 - p0;
    M edge1 = p2 - p1;
    typename M::value_type a00 = edge0.sqrnorm();
    
    typename M::value_type a01 = edge0 | edge1;
    
    typename M::value_type a11 = edge1.sqrnorm();
    typename M::value_type b0 = diff | edge0;
    typename M::value_type b1 = diff | edge1;
    
    typename M::value_type c = diff.sqrnorm();
    
    typename M::value_type det = fabs(a00*a11 - a01*a01);
    typename M::value_type s = a01*b1 - a11*b0;
    typename M::value_type t = a01*b0 - a00*b1;
    typename M::value_type sqrDistance;
    
    if (s + t <= det)
    {
        if (s < 0)
        {
            if (t < 0)  // region 4
            {
                if (b0 < 0)
                {
                    t = 0;
                    if (-b0 >= a00)
                    {
                        s = 1;
                        sqrDistance = a00 + 2*b0 + c;
                    }
                    else
                    {
                        s = -b0/a00;
                        sqrDistance = b0*s + c;
                    }
                }
                else
                {
                    s = 0;
                    if (b1 >= 0)
                    {
                        t = 0;
                        sqrDistance = c;
                    }
                    else if (-b1 >= a11)
                    {
                        t = 1;
                        sqrDistance = a11 + 2*b1 + c;
                    }
                    else
                    {
                        t = -b1/a11;
                        sqrDistance = b1*t + c;
                    }
                }
            }
            else  // region 3
            {
                s = 0;
                if (b1 >= 0)
                {
                    t = 0;
                    sqrDistance = c;
                }
                else if (-b1 >= a11)
                {
                    t = 1;
                    sqrDistance = a11 + 2*b1 + c;
                }
                else
                {
                    t = -b1/a11;
                    sqrDistance = b1*t + c;
                }
            }
        }
        else if (t < 0)  // region 5
        {
            t = 0;
            if (b0 >= 0)
            {
                s = 0;
                sqrDistance = c;
            }
            else if (-b0 >= a00)
            {
                s = 1;
                sqrDistance = a00 + 2*b0 + c;
            }
            else
            {
                s = -b0/a00;
                sqrDistance = b0*s + c;
            }
        }
        else  // region 0
        {
            // minimum at interior point
            typename M::value_type invDet = 1/det;
            s *= invDet;
            t *= invDet;
            sqrDistance = s*(a00*s + a01*t + 2*b0) + t*(a01*s + a11*t + 2*b1) + c;
        }
    }
    else
    {
        typename M::value_type tmp0, tmp1, numer, denom;
        
        if (s < 0)  // region 2
        {
            tmp0 = a01 + b0;
            tmp1 = a11 + b1;
            if (tmp1 > tmp0)
            {
                numer = tmp1 - tmp0;
                denom = a00 - 2*a01 + a11;
                if (numer >= denom)
                {
                    s = 1;
                    t = 0;
                    sqrDistance = a00 + 2*b0 + c;
                }
                else
                {
                    s = numer/denom;
                    t = 1 - s;
                    sqrDistance = s*(a00*s + a01*t + 2*b0) + t*(a01*s + a11*t + 2*b1) + c;
                }
            }
            else
            {
                s = 0;
                if (tmp1 <= 0)
                {
                    t = 1;
                    sqrDistance = a11 + 2*b1 + c;
                }
                else if (b1 >= 0)
                {
                    t = 0;
                    sqrDistance = c;
                }
                else
                {
                    t = -b1/a11;
                    sqrDistance = b1*t + c;
                }
            }
        }
        else if (t < 0)  // region 6
        {
            tmp0 = a01 + b1;
            tmp1 = a00 + b0;
            if (tmp1 > tmp0)
            {
                numer = tmp1 - tmp0;
                denom = a00 - 2*a01 + a11;
                if (numer >= denom)
                {
                    t = 1;
                    s = 0;
                    sqrDistance = a11 + 2*b1 + c;
                }
                else
                {
                    t = numer/denom;
                    s = 1 - t;
                    sqrDistance = s*(a00*s + a01*t + 2*b0) + t*(a01*s + a11*t + 2*b1) + c;
                }
            }
            else
            {
                t = 0;
                if (tmp1 <= 0)
                {
                    s = 1;
                    sqrDistance = a00 + 2*b0 + c;
                }
                else if (b0 >= 0)
                {
                    s = 0;
                    sqrDistance = c;
                }
                else
                {
                    s = -b0/a00;
                    sqrDistance = b0*s + c;
                }
            }
        }
        else  // region 1
        {
            numer = a11 + b1 - a01 - b0;
            if (numer <= 0)
            {
                s = 0;
                t = 1;
                sqrDistance = a11 + 2*b1 + c;
            }
            else
            {
                denom = a00 - 2*a01 + a11;
                if (numer >= denom)
                {
                    s = 1;
                    t = 0;
                    sqrDistance = a00 + 2*b0 + c;
                }
                else
                {
                    s = numer/denom;
                    t = 1 - s;
                    sqrDistance = s*(a00*s + a01*t + 2*b0) + t*(a01*s + a11*t + 2*b1) + c;
                }
            }
        }
    }
    
    // Account for numerical round-off error.
    if (sqrDistance < 0)
    {
        sqrDistance = 0;
    }
    
    return sqrDistance;
}


template <typename M> void MatchT<M>::split()
{
    if(!ShapeT<M>::isMeshOpen() || parts_.size()==0 || points_.size()==0)
    {
        qCritical() << "Cannot split mesh! Either mesh is not open or parts or points do not exist!" ;
        return;
    }
    
    // Try to find a seg file to use for splitting the mesh
    bool useSegFile = false;
    
    QStringList pathParts = meshFilename_.split("/");
    
    QString segName = pathParts.last().split(".").first().append(".seg");
    
    pathParts.removeLast();
    
    pathParts.push_back(segName);
    
    QString segPath = pathParts.join("/");
    
    QFile file(segPath);
    
    std::vector<int> segLabels;
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Could not open file " << segPath ;
    }
    else
    {
        QTextStream in(&file);
        
        while (!in.atEnd())
        {
            QString line = in.readLine();
            segLabels.push_back(line.toInt());
        }
        
        if (segLabels.size()!= ShapeT<M>::mesh_.n_faces())
        {
            qDebug() << "Mesh has a seg file, but it only contained " << segLabels.size() << " labels for " << ShapeT<M>::mesh_.n_faces() << " faces, so cannot use it!";
        }
        else
        {
            useSegFile = true;
        }
    }
    
    std::map<int, int> partID2partIndex;
    
    std::map<int, std::map<typename M::VertexHandle,typename M::VertexHandle> > partID2vhMap;
    
    typename std::vector<Part>::const_iterator partscIt (parts_.begin()), partscEnd(parts_.end());

    int index =0;
    
    for (; partscIt != partscEnd; ++partscIt)
    {
        partID2partIndex[partscIt->partID_] = index;
        index++;
    }

    
    typename M::ConstFaceIter cfIt(ShapeT<M>::mesh_.faces_begin()), facesEnd(ShapeT<M>::mesh_.faces_end());
    
    int f = 0;
    
    // For every face go over all points of the match and find the nearest point to this face
    for (; cfIt!=facesEnd; ++cfIt)
    {
        int facePartID = -1;
        
        // Get the three vertices of this face (should be 3) so we can get the normal to the face
        typename M::ConstFaceVertexIter cfvIt;
        
        if (useSegFile)
        {
            facePartID = segLabels[ShapeT<M>::indexMap_[f]] + 1; //only works assuming part ids start from 2 and seg ids start from 1
        }
        else
        {
            cfvIt= ShapeT<M>::mesh_.cfv_iter(cfIt);
            
            typename std::vector<MeshPoint>::const_iterator pointsIt(points_.begin()), pointsEnd(points_.end());
            
            double minDistance = std::numeric_limits<double>::max();
          
            typename M::Point p0 = ShapeT<M>::mesh_.point(cfvIt);
            ++cfvIt;
            typename M::Point p1 = ShapeT<M>::mesh_.point(cfvIt);
            ++cfvIt;
            typename M::Point p2 = ShapeT<M>::mesh_.point(cfvIt);
            
            // Go over all points in the match (these have part id labels)
            for( ; pointsIt!=pointsEnd ; ++pointsIt)
            {
                double p2pDistance = sqrDistPoint2Triangle(p0,p1,p2,OpenMesh::vector_cast<typename M::Point>(pointsIt->pos_));
                
                if (p2pDistance < minDistance)
                {
                    minDistance = p2pDistance;
                    facePartID = pointsIt->partID_;
                }
            }
        }
        
        Part& cPart = parts_[partID2partIndex[facePartID]];
        typename ShapeT<M>::Mesh& cMesh = cPart.partShape_.mesh();
        
        std::map< typename M::VertexHandle, typename M::VertexHandle>& cvhMap = partID2vhMap[facePartID];
        
        // reset the face vertex iterator
        cfvIt = ShapeT<M>::mesh_.cfv_iter(cfIt);
        
        std::vector<typename M::VertexHandle> face_vhandles;
        
        int i=0;
        // Go over all vertices of this face (should only be 3 of them), add them to the new mesh if they haven't been added yet, and add their handle to the list of handles for the new face to be added to the new mesh
        for(; cfvIt; ++cfvIt, ++i)
        {
            // check if the vertex has been added to this part
            if(cvhMap.count(cfvIt)<=0)
            {
                typename M::VertexHandle vhandle = cMesh.add_vertex(ShapeT<M>::mesh_.point(cfvIt));
                face_vhandles.push_back(vhandle);
                // add this vertex new handle to the map
                cvhMap[cfvIt] = vhandle;
            }
            else
            {
                face_vhandles.push_back(cvhMap[cfvIt]);
            }
        }
        if( i>3)
        {
            qWarning() << "Warning, found " << i << " vertices for this face instead of 3!!";
        }
        
        cMesh.add_face(face_vhandles);
        
        f++;
    }
    
    // Update normals for all part meshes
    typename std::vector<Part>::iterator partsIt (parts_.begin()), partsEnd(parts_.end());

    for (; partsIt != partsEnd; ++partsIt)
    {
        typename ShapeT<M>::Mesh& cMesh = partsIt->partShape_.mesh();
        
        cMesh.request_face_normals();
        cMesh.request_vertex_normals();
        cMesh.update_face_normals();
        cMesh.update_vertex_normals();
    }
    
    segmented_ = true;
}


template <typename M> bool  MatchT<M>::segmented()
{
    return segmented_;
}

template <typename M>void  MatchT<M>::setSegmented(bool _segmented)
{
    segmented_ = _segmented;
}

template <typename M> void MatchT<M>::recalculateBoxes()
{
    openMeshIfNotOpened();
    
    if (!segmented_)
    {
        qCritical() << "Match has not been segmented into parts, so cannot recalculate its boxes!!" ;
        return;
    }
    
    typename std::vector<Part>::iterator itPart(parts_.begin()), partsEnd(parts_.end());
    
    for(; itPart != partsEnd ; ++itPart)
    {
        M& cMesh = itPart->partShape_.mesh();
        
        typename M::ConstVertexIter vIt(cMesh.vertices_begin()), vEnd(cMesh.vertices_end());
        
        typename ShapeT<M>::BBox cBBox;
        
        for (; vIt!= vEnd; ++vIt)
        {
            OpenMesh::Vec3f p_i = mv(alignMtx_, cMesh.point(vIt));
            
            cBBox.min.minimize(p_i);
            cBBox.max.maximize(p_i);
        }
        
        itPart->scale_ = (cBBox.max - cBBox.min) * 0.5f;
        itPart->pos_ = cBBox.min + itPart->scale_;
    }
}

template <typename M> void MatchT<M>::Part::recalculateBox(const xform& _alignMtx)
{
    typename ShapeT<M>::Mesh & cMesh = partShape_.mesh();
    
    typename TriangleMesh::ConstVertexIter vIt(cMesh.vertices_begin()), vEnd(cMesh.vertices_end());
    
    typename ShapeT<M>::BBox cBBox;
    
    for (; vIt!= vEnd; ++vIt)
    {
        OpenMesh::Vec3f p_i = mv(_alignMtx, cMesh.point(vIt));
        
        cBBox.min.minimize(p_i);
        cBBox.max.maximize(p_i);
    }
    
    scale_ = (cBBox.max - cBBox.min) * 0.5f;
    pos_ = cBBox.min + scale_;
    
}


template <typename M> void MatchT<M>::saveMatchDescriptorToFile(QTextStream& _out)
{
    typename std::vector<Part>::const_iterator pIt(parts_.begin()), pEnd(parts_.end());
    
    // Hack, just to avoid making a new function and calling that and because I am tired
    _out << ShapeT<M>::filename_.split("/").last().split(".").first() << " " << descriptor2D_[0] << " " << descriptor2D_[1] << " " << label_ << "\n";
    
    return;
    
    for( ; pIt != pEnd; ++pIt)
    {
        if (pIt->partType_ == 0)
            continue;
        
        OpenMesh::Vec3d min(0,0,0);
        OpenMesh::Vec3d max(0,0,0);
        
        min = pIt->pos_ - pIt->scale_;
        max = pIt->pos_ + pIt->scale_;
        
        
        if (pIt + 1 == pEnd)
        {
            _out << min[0] << "," << min[1] << "," << min[2] << "," << max[0] << "," << max[1] << "," << max[2];
        }
        else
        {
            _out << min[0] << "," << min[1] << "," << min[2] << "," << max[0] << "," << max[1] << "," << max[2] << ",";
        }
        
//        int descSize = pIt->descF_.size();
//        
//        for (int i=0; i < descSize; i++)
//        {
//            if (pIt + 1 == pEnd && i+1 == descSize)
//            {
//                _out << pIt->descF_[i] ;
//            }
//            else
//            {
//                _out << pIt->descF_[i] << ",";
//            }
//            
//        }
    }
    
    _out << "\n";
    
}

template <typename M> void MatchT<M>::saveMatchPointsToFile(const QString& _filename)
{
    
    QFile file(_filename);
    
    if (!file.open(QIODevice::WriteOnly))
    {
        
        qCritical() << "Could not open file " << _filename;
        return;
        
    }
    
    QTextStream out(&file);
    
    typename std::vector<MeshPoint>::const_iterator itPoint(points_.begin()), itPointEnd(points_.end());
    
    for( ; itPoint != itPointEnd; ++itPoint)
    {
        out << -itPoint->pos_[0] << "\t" << itPoint->pos_[2] << "\t" << -itPoint->pos_[1] << "\t" << itPoint->partID_ << "\n";
    }
    
}

template <typename M> void MatchT<M>::saveMesh(const QString &_directoryPath)
{
    openMeshIfNotOpened();
    
    if (NORMALISE_MATCH_MESH_BEFORE_SAVING && !normalised_)
    {
        normaliseMeshToTemplate();
    }
    
    if (ALIGN_MATCH_MESH_BEFORE_SAVING && !aligned_)
    {
        alignMeshToTemplate();
    }
    
    QString meshName = _directoryPath + "/" + meshFilename_.split("/").last();
    
    OpenMesh::IO::write_mesh(ShapeT<M>::mesh_, meshName.toStdString());
    
}

template <typename M> void MatchT<M>::savePartMeshes()
{
    
    typename std::vector<Part>::const_iterator pIt(parts_.begin()), pEnd(parts_.end());
    
    for( ; pIt != pEnd; ++pIt)
    {
        if(!segmented_)
        {
            qCritical() << "Mesh is not segmented, cannot save part meshes!!" ;
            return;
        }
        
        if (pIt->partType_ == 0)
            continue;
        
        QString partMeshName = meshFilename_+QString(".p%1.off").arg(pIt->partID_);
        
        OpenMesh::IO::write_mesh(pIt->partShape_.mesh(), partMeshName.toStdString());
        
    }
}


template <typename M> void MatchT<M>::savePartMeshes(const QString& _directoryPath)
{
    typename std::vector<Part>::iterator pIt(parts_.begin()), pEnd(parts_.end());
    
    for( ; pIt != pEnd; ++pIt)
    {
        openPartMeshIfNotOpened(*pIt);
        
        if (pIt->partType_ == 0)
            continue;
        
        QString partMeshName = _directoryPath + "/" + ShapeT<M>::filename_.split("/").last().split(".").first() +QString(".p%1.off").arg(pIt->partID_);
        
        std::cout << "mesh name:" << partMeshName.toStdString() << std::endl;
        OpenMesh::IO::write_mesh(pIt->partShape_.mesh(), partMeshName.toStdString());
        
    }
}


template <typename M> bool MatchT<M>::openPartMeshIfNotOpened(Part& _cPart)
{
    M& cMesh = _cPart.partShape_.mesh();
    
    if( cMesh.n_vertices() == 0 )
    {
        QString partMeshName = meshFilename_+QString(".p%1.off").arg(_cPart.partID_);
        
        qWarning() << "Selected part's mesh was not loaded before, loading mesh now from: " << partMeshName ;
        
        _cPart.partShape_.setFilename(partMeshName);
        
        if( !_cPart.partShape_.openMesh(partMeshName.toStdString().c_str()) )
        {
            qCritical() << "Cannot read mesh from file: " << partMeshName;
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return true;
    }

}


template <typename M> void MatchT<M>::openPartMeshes()
{
    typename std::vector<Part>::iterator pIt(parts_.begin()), pEnd(parts_.end());
    
    for( ; pIt != pEnd; ++pIt)
    {
        if (pIt->partType_ == 0)
            continue;
        
        QString partMeshName = meshFilename_+QString(".p%1.off").arg(pIt->partID_);
        
        pIt->partShape_.setFilename(partMeshName);
        
        if( pIt->partShape_.openMesh(partMeshName.toStdString().c_str()) )
        {
            segmented_ = true;
        }
    }
}

template <typename M> void MatchT<M>::disableAlignMtx()
{
    disableAlignmtx_ = true;
}

template <typename M> void MatchT<M>::enableAlignMtx()
{
    disableAlignmtx_ = false;
}

template <typename M> std::vector<typename MatchT<M>::Constraint>& MatchT<M>::constraints()
{
    return constraints_;
}

template <typename M> QTextStream MatchT<M>::openSegFile(const QString &_filename)
{
    QFile file(_filename);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Could not open file " << _filename ;
        return;
    }
    
    QTextStream in(&file);
    
    return in;
}

template <typename M> void MatchT<M>::overrideColor(bool _colorOverride)
{
    colorOverride_ = _colorOverride;
}

template <typename M> void MatchT<M>::setHoveredPartID(int _hoveredPartID)
{
    hoveredPartID_ = _hoveredPartID;
}
