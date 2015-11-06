// ShapeListWidget.h
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#ifndef SHAPELISTWIDGET_H
#define SHAPELISTWIDGET_H

//== INCLUDES =================================================================

#include <QWidget>
#include <QDockWidget>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDirIterator>
#include <QInputDialog>
#include <QListWidget>
#include <QDebug>

#include <QDir>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/Utils/GenProg.hh>
#include <OpenMesh/Tools/Utils/getopt.h>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>

#include "ShapeListWidgetItem.h"
#include "ShapeT.h"
#include "MatchT.h"


//== CLASS DEFINITION =========================================================

class ShapeListWidget : public QListWidget
{
    Q_OBJECT
    
public:
    
    typedef ShapeT<TriangleMesh> Shape;
    typedef MatchT<TriangleMesh> Match;
    
    // Default constructor
    ShapeListWidget(QWidget* parent=0);
    
    void openMatchCollection(const QString& _fname, bool _load_mesh);
    
    void saveMatchCollection(const QString& _fname, const QString& _directoryPath);
    
    void openMatchShape(const QString& _fname, bool _load_mesh);
    
    void openPlainShape(const QString& _fname, bool _load_mesh);
    
    // Re-implemented from QListWidget - emits the signal with the currently selected shapes
    void selectionChanged( const QItemSelection & selected, const QItemSelection & deselected );
    
public slots:
    
    void slotAddShapes();
    
    void slotAddShapesFromDir();
    
    void slotSaveMatchCollection();
   
signals:
    
    void selectedShapesChanged(const std::vector<Shape*>& _current,int _id);
    
    void requestRedraw(int _id);
    
    void matchShapesAdded(const std::vector<Match*>& _matches);
    

};

#endif
