//
//  ShapeT.cpp
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#define SHAPET_CPP

#include "ShapeT.h"

//using namespace OpenMesh;

template <typename M> bool ShapeT<M>::open(const QString& _filename)
{
    filename_ = _filename;
    
    QStringList fname_split = filename_.split("/");
    QString mesh_name = fname_split.last();
    
    // Remove mesh name and its enclosing directory from the list
    fname_split.removeLast();
    
    QString snap_name = mesh_name.split(".").first().append(".jpg");
    fname_split.push_back(snap_name);
    
    iconFilename_ = fname_split.join("/");
    
    return openMesh(_filename.toStdString().c_str());
}

template <typename M> bool ShapeT<M>::save(const char* _filename)
{
    QString fname(_filename);
    
    // If a name was not given then use the stored filename of the shape
    if(fname.isEmpty())
    {
        return OpenMesh::IO::write_mesh(mesh_, filename_.toStdString());
    }
    else
    {
        return OpenMesh::IO::write_mesh(mesh_, _filename);
    }
}

template <typename M> bool ShapeT<M>::openMesh(const char* _filename)
{
    mesh_.request_face_normals();
    
    mesh_.request_vertex_normals();
    
    
    qDebug() << "Loading from file '" << _filename ;
    
    OpenMesh::Utils::Timer t1;
    t1.start();
    
    OpenMesh::IO::Options opt;
    
    opt += OpenMesh::IO::Options::VertexNormal;
    opt += OpenMesh::IO::Options::FaceNormal;
    
    indexMap_.clear();
    
    if ( OpenMesh::IO::read_mesh(mesh_, std::string(_filename), opt, false, &indexMap_))
    {
        // Update face and vertex normals
        if ( ! opt.check( OpenMesh::IO::Options::FaceNormal ) )
        {
            mesh_.update_face_normals();
        }
        else
        {
            qDebug() << "File provides face normals";
        }
        if ( ! opt.check( OpenMesh::IO::Options::VertexNormal ) )
        {
            mesh_.update_vertex_normals();
        }
        else
        {
            qDebug() << "File provides vertex normals";
        }
        
        t1.stop();
        
        qDebug() << "Mesh loaded in: " << t1.as_string().c_str() ;
        qDebug() << mesh_.n_vertices() << " vertices, " << mesh_.n_edges() << " edge, " << mesh_.n_faces() << " faces";
        
        return true;
    }
    else
    {
        qDebug() << "Mesh " << _filename << " was not found";
        return false;
    }
    
}

template <typename M> unsigned int ShapeT<M>::id()
{
    return id_;
}

template <typename M> void ShapeT<M>::setID(unsigned int _id)
{
    id_ = _id;
}

template <typename M> typename ShapeT<M>::Mesh& ShapeT<M>::mesh()
{
    return mesh_;
}

template <typename M> const typename ShapeT<M>::Mesh& ShapeT<M>::mesh() const
{
    return mesh_;
}

template <typename M> void ShapeT<M>::setFilename(const QString& _filename)
{
    filename_ = _filename;
}

template <typename M> const QString& ShapeT<M>::filename() const
{
    return filename_;
}

template <typename M> void ShapeT<M>::setIconFilename(const QString& _iconFilename)
{
    iconFilename_ = _iconFilename;
}

template <typename M> const QString& ShapeT<M>::iconFilename() const
{
    return iconFilename_;
}

template <typename M> bool ShapeT<M>::isMeshOpen() const
{
    if (mesh_.n_vertices() == 0)
        return false;
    else
        return true;
}

template <typename M> void ShapeT<M>::openMeshIfNotOpened()
{
    if(mesh_.n_vertices() == 0)
    {
        qWarning() << "Selected shape's mesh was not loaded before, loading mesh now from: " << filename_ ;
        
        if(!openMesh(filename_.toLocal8Bit()))
        {
            qCritical() <<  "Cannot read mesh from file: " << filename_;
        }
    }
}

template <typename M> void ShapeT<M>::pick() const
{
    typename Mesh::ConstFaceIter fIt(mesh_.faces_begin()), fEnd(mesh_.faces_end());
    
    typename Mesh::ConstFaceVertexIter fvIt;
    
    glLoadName(id_);
    
    glBegin(GL_TRIANGLES);
    
    for (; fIt!=fEnd; ++fIt)
    {
        fvIt = mesh_.cfv_iter(fIt.handle());
        glVertex3fv( &mesh_.point(fvIt)[0] );
        ++fvIt;
        glVertex3fv( &mesh_.point(fvIt)[0] );
        ++fvIt;
        glVertex3fv( &mesh_.point(fvIt)[0] );
    }
    
    glEnd();
}

template <typename M> void ShapeT<M>::draw(const std::string& _drawMode) const
{
    if (mesh_.n_vertices() == 0)
    {
        return;
    }
    
    typename Mesh::ConstFaceIter fIt(mesh_.faces_begin()), fEnd(mesh_.faces_end());
    
    typename Mesh::ConstFaceVertexIter fvIt;
    
    if (_drawMode == "Wireframe")
    {
        glBegin(GL_TRIANGLES);
        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = mesh_.cfv_iter(fIt.handle());
            glVertex3fv( &mesh_.point(fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(fvIt)[0] );
        }
        glEnd();
    }
    
    else if (_drawMode == "Solid Flat")
    {
        glBegin(GL_TRIANGLES);
        for (; fIt!=fEnd; ++fIt)
        {
            glNormal3fv( &mesh_.normal(fIt)[0] );
            
            fvIt = mesh_.cfv_iter(fIt.handle());
            glVertex3fv( &mesh_.point(fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(fvIt)[0] );
            ++fvIt;
            glVertex3fv( &mesh_.point(fvIt)[0] );
        }
        glEnd();
    }
    
    else if (_drawMode == "Solid Smooth")
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, mesh_.points());
        
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());
        
        glBegin(GL_TRIANGLES);
        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = mesh_.cfv_iter(fIt.handle());
            glArrayElement(fvIt.handle().idx());
            ++fvIt;
            glArrayElement(fvIt.handle().idx());
            ++fvIt;
            glArrayElement(fvIt.handle().idx());
        }
        glEnd();
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    
    else if (_drawMode == "Colored Vertices") // --------------------------------
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, mesh_.points());
        
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());
        
        if ( mesh_.has_vertex_colors() )
        {
            glEnableClientState( GL_COLOR_ARRAY );
            glColorPointer(3, GL_UNSIGNED_BYTE, 0,mesh_.vertex_colors());
        }
        
        glBegin(GL_TRIANGLES);
        for (; fIt!=fEnd; ++fIt)
        {
            fvIt = mesh_.cfv_iter(fIt.handle());
            glArrayElement(fvIt.handle().idx());
            ++fvIt;
            glArrayElement(fvIt.handle().idx());
            ++fvIt;
            glArrayElement(fvIt.handle().idx());
        }
        glEnd();
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }
    
    else if (_drawMode == "Solid Colored Faces") // -----------------------------
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, mesh_.points());
        
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());
        
        glBegin(GL_TRIANGLES);
        for (; fIt!=fEnd; ++fIt)
        {
            //glColor( fIt.handle() );
            glColor3ubv( &mesh_.color(fIt.handle())[0] );
            
            fvIt = mesh_.cfv_iter(fIt.handle());
            glArrayElement(fvIt.handle().idx());
            ++fvIt;
            glArrayElement(fvIt.handle().idx());
            ++fvIt;
            glArrayElement(fvIt.handle().idx());
        }
        glEnd();
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    
    else if (_drawMode == "Smooth Colored Faces") // ---------------------------
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, mesh_.points());
        
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());
        
        glBegin(GL_TRIANGLES);
        for (; fIt!=fEnd; ++fIt)
        {
            OpenMesh::Vec3f c=OpenMesh::color_cast<OpenMesh::Vec3f>(mesh_.color(fIt.handle()));
            OpenMesh::Vec4f m( c[0], c[1], c[2], 1.0f );
            int _f=GL_FRONT_AND_BACK, _m=GL_DIFFUSE;
            
            glMaterialfv(_f, _m, &m[0]);
            
            fvIt = mesh_.cfv_iter(fIt.handle());
            glArrayElement(fvIt.handle().idx());
            ++fvIt;
            glArrayElement(fvIt.handle().idx());
            ++fvIt;
            glArrayElement(fvIt.handle().idx());
        }
        glEnd();
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    
    else if( _drawMode == "Points" ) // -----------------------------------------
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, mesh_.points());
        
        if (mesh_.has_vertex_colors())
        {
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(3, GL_UNSIGNED_BYTE, 0, mesh_.vertex_colors());
        }
        
        glDrawArrays( GL_POINTS, 0, mesh_.n_vertices() );
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }
    
    
}

template <typename M> void ShapeT<M>::drawNormals(const std::string& _normalDrawMode, float _normalScale)
{
    if( _normalDrawMode == "Vertex")
    {
        typename Mesh::ConstVertexIter vIt(mesh_.vertices_begin()), vEnd(mesh_.vertices_end());
        
        glBegin(GL_LINES);
        glColor3f(1.000f, 0.803f, 0.027f); // orange
        
        for(; vIt!=vEnd; ++vIt)
        {
            glVertex3fv( &mesh_.point( vIt )[0] );
            
            glVertex3fv( &(mesh_.point( vIt ) + _normalScale*mesh_.normal( vIt ))[0] );
            
        }
        glEnd();
    }
    else if (_normalDrawMode == "Face")
    {
        typename Mesh::ConstFaceIter fIt(mesh_.faces_begin()), fEnd(mesh_.faces_end());
        glBegin(GL_LINES);
        glColor3f(0.705f, 0.976f, 0.270f); // greenish
        
        for(; fIt!= fEnd; ++fIt)
        {
            // Find the com of the face to draw the normal from
            typename Mesh::FaceVertexIter fvIt(mesh_.fv_iter(fIt));
            
            typename Mesh::Point v(0,0,0);
            int n=0;
            for(; fvIt; ++fvIt)
            {
                v += mesh_.point(fvIt);
                n++;
            }
            
            v *= 1.0f/n;
            
            glVertex3fv(&v[0]);
            
            glVertex3fv( &(v + _normalScale*mesh_.normal( fIt ))[0]);
        }
        
        glEnd();
    }
    
}

template <typename M> void ShapeT<M>::normalise()
{
    
    typename Mesh::VertexIter vIt(mesh_.vertices_begin()), vEnd(mesh_.vertices_end());
    
    typename Mesh::Point centroid = meshCentroid();
    
    for(; vIt!=vEnd; ++vIt)
    {
        mesh_.set_point(vIt, mesh_.point(vIt) - centroid);
    }
    
    float scaleFactor = 1.0 / averageRadius(centroid);
    
    vIt = mesh_.vertices_begin();
    
    for(; vIt!=vEnd; ++vIt)
    {
        mesh_.set_point(vIt, scaleFactor * mesh_.point(vIt) );
    }
}

template <typename M> double ShapeT<M>::faceArea(const typename Mesh::FaceHandle& _fH)
{
    typename Mesh::ConstFaceVertexIter fvIt(mesh_.cfv_iter(_fH));
    
    const typename Mesh::Point& p0 = mesh_.point(fvIt);
    ++fvIt;
    const typename Mesh::Point& p1 = mesh_.point(fvIt);
    ++fvIt;
    const typename Mesh::Point& p2 = mesh_.point(fvIt);
    ++fvIt;
    
    if(fvIt)
    {
        // TODO: Handle non-triangle meshes possibly?
        qWarning() << "Mesh face appears to have more than 3 vertices. Setting face area to zero!" ;
        return 0.0f;
    }
    else
    {
        return ((p1 - p0) % (p2 - p0)).norm();
    }
}

template <typename M> typename M::Point ShapeT<M>::faceCentroid(const typename Mesh::FaceHandle& _fH)
{
    typename Mesh::ConstFaceVertexIter fvIt(mesh_.cfv_iter(_fH));
    typename Mesh::Point centroid(0,0,0);
    
    int i=0;
    
    for( ; fvIt; ++fvIt, ++i)
    {
        centroid += mesh_.point(fvIt);
    }
    
    if(i>3)
    {
        qWarning() << "Mesh face has " << i << " vertices instead of 3 vertices!" ;
    }
    
    return centroid /= i;
}

template <typename M> typename M::Point ShapeT<M>::meshCentroid()
{
    //openMeshIfNotOpened();
    
    double area = 0.0;
    
    typename Mesh::Point centroid(0,0,0);
    
    typename Mesh::ConstFaceIter fIt(mesh_.faces_begin()), fEnd(mesh_.faces_end());
    
    for(; fIt!=fEnd; ++fIt)
    {
        float fArea = faceArea(fIt);
        
        // Note: we do not handle the case if face area is zero (face has more than 3 vertices). In this case the face is simply ignored
        centroid += fArea * faceCentroid(fIt);
        
        area += fArea;
    }
    
    if(area == 0.0)
    {
        return centroid;
    }
    else
    {
        return centroid / area;
    }
}

template <typename M> typename M::Point ShapeT<M>::verticesCentroid()
{
    //openMeshIfNotOpened();
    
    typename Mesh::Point centroid(0,0,0);
    
    typename Mesh::ConstVertexIter vIt(mesh_.vertices_begin()), vEnd(mesh_.vertices_end());
    
    for(; vIt!=vEnd; ++vIt)
    {
        centroid += mesh_.point(vIt);
    }
    
    int nV = mesh_.n_vertices();
    
    if (nV==0)
    {
        return centroid;
    }
    else
    {
        return centroid / nV;
    }
    
}

template <typename M> double ShapeT<M>::averageRadius(const typename Mesh::Point& _centroid)
{
    //openMeshIfNotOpened();
    
    double area = 0.0;
    
    double distance = 0.0;
    
    typename Mesh::ConstFaceIter fIt(mesh_.faces_begin()), fEnd(mesh_.faces_end());
    
    for(; fIt!=fEnd; ++fIt)
    {
        double fArea = faceArea(fIt);
        
        // Note: we do not handle the case if face area is zero (face has more than 3 vertices). In this case the face is simply ignored
        distance += fArea * (faceCentroid(fIt) - _centroid).norm();
        
        area += fArea;
    }
    
    if(area==0.0)
    {
        return 0.0;
    }
    else
    {
        return distance / area;
    }
}

template <typename M> typename M::Point ShapeT<M>::getRandomPoint(const std::vector<double>& _areas)
{
    
    typename Mesh::Point randomPoint(0,0,0);
    int nF = mesh_.n_faces();
    
    if(nF!= _areas.size())
    {
        qCritical() << "The number of faces in the mesh (" << nF << ") is not the same as the number of entries in the cumulative area distribution! Cannot get random point!" ;
        return randomPoint;
    }
    //Since the areas array has the cumulative area up to every face, it's safe to assume the last position has the total area of the mesh
    float totalArea = _areas.at(nF -1);
    
    //Get a random number between 0 and the total area
    double randomArea= random<double>(0.0f,totalArea);
    float random1 = random<float>(0.0f,1.0f);
    float random2 = random<float>(0.0f,1.0f);
    int faceIndex= binarySearch<double>(_areas, randomArea);
    
    typename Mesh::ConstFaceVertexIter fvIt(mesh_.cfv_iter(mesh_.face_handle(faceIndex)));
    
    const typename Mesh::Point& p0 = mesh_.point(fvIt);
    ++fvIt;
    const typename Mesh::Point& p1 = mesh_.point(fvIt);
    ++fvIt;
    const typename Mesh::Point& p2 = mesh_.point(fvIt);
    ++fvIt;
    
    
    float squareRandom1 = sqrt(random1);
    
    randomPoint = (1 - squareRandom1)*p0 + squareRandom1*(1-random2)*p1+ squareRandom1*random2*p2;
    
    if(fvIt)
    {
        // TODO: Handle non-triangle meshes possibly?
        qWarning() << "Mesh face appears to have more than 3 vertices! Only using the first 3 vertices to get a random point on the face" ;
    }
    
    return randomPoint;
}

template <typename M> void ShapeT<M>::getRandomPoints(std::vector<typename Mesh::Point>& _points, int _nPoints)
{
    
    _points.clear();
    
    typename Mesh::ConstFaceIter fIt(mesh_.faces_begin()), fEnd(mesh_.faces_end());
    
    std::vector<double> cumulativeAreas(0,mesh_.n_faces());
    
    double area = 0.0;
    
    for(; fIt!=fEnd; ++fIt)
    {
        double fArea = faceArea(fIt);
        
        area += fArea;
        
        cumulativeAreas.push_back(area);
    }
    
    for(int i=0;i<_nPoints;i++)
    {
        _points.push_back( getRandomPoint(cumulativeAreas) );
    }
}

template <typename M> typename ShapeT<M>::BBox ShapeT<M>::bbox()
{
    BBox box;
    
    typename Mesh::ConstVertexIter vIt(mesh_.vertices_begin());
    typename Mesh::ConstVertexIter vEnd(mesh_.vertices_end());
    
    for ( ; vIt!=vEnd; ++vIt)
    {
        box.min.minimize( mesh_.point(vIt));
        box.max.maximize( mesh_.point(vIt));
    }
    
    return box;
}

