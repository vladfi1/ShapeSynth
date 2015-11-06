// ShapelistWidget.cpp
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#include "ShapeListWidget.h"
#include <iostream>


ShapeListWidget::ShapeListWidget(QWidget * parent)
: QListWidget(parent)
{
    
    setSelectionMode(QAbstractItemView::MultiSelection );
    
}


void ShapeListWidget::slotAddShapes()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Open mesh file"),
                                                          tr(""),
                                                          tr("OFF Files (*.off);;"
                                                             "OBJ Files (*.obj);;"
                                                             "STL Files (*.stl);;"
                                                             "Match Files (*.match);;"
                                                             "Match Collection (*.match_coll);;"
                                                             "All Files (*)"));
    if (!fileNames.isEmpty())
    {
        QStringList::const_iterator fn_it(fileNames.constBegin()), fn_end(fileNames.constEnd());
        
        for( ; fn_it!=fn_end; ++fn_it)
        {
            if(!(*fn_it).isEmpty())
            {
                if( (*fn_it).endsWith(".match"))
                {
                    openMatchShape(*fn_it, true);
                }
                else if( (*fn_it).endsWith(".match_coll"))
                {
                    openMatchCollection(*fn_it, false);
                }
                else
                {
                    openPlainShape(*fn_it,true);
                }
            }
        }
    }
}


void ShapeListWidget::slotAddShapesFromDir()
{
    // Ask for a directory
    QString directory_path = QFileDialog::getExistingDirectory(0,"Import Shapes", "../");
    
    if(directory_path.isNull())
    {
        qCritical() << "The directory path was null";
        return;
    }
    
    QStringList nameFilters;
    
    nameFilters << "*.obj" << "*.OBJ" << "*.off" << "*.OFF" << "*.stl" << "*.STL" << "*.ply" << "*.PLY" << "*.match" << "*.match_coll";
    
    //The directory iterator will return all .match files, including in any subdirectories
    QDirIterator directory_walker(directory_path, nameFilters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    
    while(directory_walker.hasNext())
    {
        QString fname = directory_walker.next();
        
        if(!fname.isEmpty())
        {
            if( fname.endsWith(".match"))
            {
                openMatchShape(fname, true);
            }
            else if( fname.endsWith(".match_coll"))
            {
                openMatchCollection(fname, PRELOAD_MODELS);
            }
            else
            {
                openPlainShape(fname,true);
            }
        }
        
    }
    
    qDebug() << "Reading is done! Shapes loaded: " << this->count() ;
    
}

void ShapeListWidget::slotSaveMatchCollection()
{
    
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save match collection to file"),
                                                    tr(""),
                                                    tr("Match Collection Files (*.match_coll);;"
                                                       "All Files (*)"));
    if (filename.isEmpty())
    {
        qCritical() << "No file selected" ;
		return;
    }
    
    QString directory_path = QFileDialog::getExistingDirectory(0,"Choose a directory to save the meshes", "../");
    
    if(directory_path.isNull())
    {
        qCritical() << "The directory path was null";
        return;
    }
    
    saveMatchCollection(filename, directory_path);
    
    
}

void ShapeListWidget::saveMatchCollection(const QString& filename,const QString& _directoryPath)
{
    QFile file(filename);
    
    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical() << "Could not open file " << filename ;
        return;
    }
    
    QTextStream out(&file);
    
    QList<QListWidgetItem*> matchItems = findItems("match",Qt::MatchEndsWith);
    
    QList<QListWidgetItem*>::iterator itMatch(matchItems.begin()), matchesEnd(matchItems.end());
    
    for (; itMatch != matchesEnd; ++itMatch)
	{
        ShapeListWidgetItem<Match>* cItem = static_cast<ShapeListWidgetItem<Match>*>(*itMatch);
        
        if(!cItem)
        {
            qCritical() << "Could not cast to ShapeListWidgetItem<Match>! " ;
            
            return;
        }
        
        cItem->shape().save(out);
        
        // Save the mesh as well?
        //cItem->shape().saveMesh(_directoryPath);
    }
    
    qDebug() << "Done saving collection to file " << filename ;
}

//  Directory structure (e.g. for bikes):
// /pathtohere/bikes.match_coll
// /pathtohere/bikes/
// /pathtohere/bikes/img_matches/
// /pathtohere/bikes/img_models/
// /pathtohere/bikes/models/
void ShapeListWidget::openMatchCollection(const QString& _fname, bool _load_mesh)
{
    QFile file(_fname);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Could not open file " << _fname ;
        return;
    }
    
    QStringList dir_split = _fname.split("/");
    
    QStringList fname_split = dir_split.last().split(".");
    QString coll_name = fname_split.first();
   
    dir_split.removeLast();
    dir_split.push_back(coll_name);
    
    QTextStream in(&file);
    int i=0;
    
    std::vector<Match*> matches;
    
    while (!in.atEnd())
    {
        QString line = in.readLine();
        
        ShapeListWidgetItem<Match>* slwi = new ShapeListWidgetItem<Match>;
        
        if (line.isEmpty() || !slwi->shape().open(line))
        {
            qCritical() << "Cannot read match from string ";
        }
        
        dir_split.push_back("matches");
        QString m_name = slwi->shape().filename();
        dir_split.push_back(m_name);
        
        QString match_name = dir_split.join("/");
        
        slwi->setText(m_name);
        slwi->setToolTip(match_name);
        
        slwi->shape().setFilename(match_name);
        
        // Remove match name and its enclosing directory from the list
        dir_split.removeLast();
        dir_split.removeLast();
        
        // Now add the match snapshot directory
        dir_split.push_back(MATCH_ICON_PATH);
        
        QString snap_name = m_name.split(".").first().append(".jpg");
        
        dir_split.push_back(snap_name);
       
        if (SHOW_SLW)
        {
            QIcon icon(QPixmap(dir_split.join("/")));
        
            slwi->setIcon(icon);
        }
        // Now get the mesh corresponding to this match
        QString mesh_name = m_name.split(".").first().append(".off");
        dir_split.removeLast();
        dir_split.removeLast();
        
        dir_split.push_back(MESH_PATH);
        dir_split.push_back(mesh_name);
        
        QString mesh_fname = dir_split.join("/");
        
        slwi->shape().setMeshFilename(mesh_fname);
        
        slwi->shape().setID(i);
        
        //OpenMesh::Utils::Timer t;
        //t.start();
        if ( _fname.isEmpty() || (_load_mesh && !slwi->shape().openMesh(mesh_fname.toStdString().c_str())) )
        {
            qCritical() << "Cannot read mesh from file: " << mesh_fname;
        }
        //t.stop();
        //std::cout << "Loaded mesh in ~" << t.as_string() << std::endl;
        
        dir_split.removeLast();
        dir_split.removeLast();
        
        this->addItem(slwi);
        
        matches.push_back(&slwi->shape());
    
        i++;
    }
    
    emit matchShapesAdded(matches);
    
    qDebug() << "Reading is done! Matches loaded: " << i ;
}

//  Directory structure (e.g. for bikes):
// /pathtohere/bikes.match_coll
// /pathtohere/bikes/
// /pathtohere/bikes/img_matches/
// /pathtohere/bikes/img_models/
// /pathtohere/bikes/meshes/
// /pathtohere/bikes/matches/
void ShapeListWidget::openMatchShape(const QString& _fname, bool _load_mesh)
{
    ShapeListWidgetItem<Match>* slwi = new ShapeListWidgetItem<Match>;
    
    QStringList fname_split = _fname.split("/");
    QString match_name = fname_split.last();
    slwi->setText(match_name);
    slwi->setToolTip(_fname);
    
    slwi->shape().setFilename(_fname);
    
    // Remove match name and its enclosing directory from the list
    fname_split.removeLast();
    fname_split.removeLast();
    
    // Now add the match snapshot directory
    fname_split.push_back(MATCH_ICON_PATH);
    
    QString snap_name = match_name.split(".").first().append(".jpg");
    
    fname_split.push_back(snap_name);
    
    if (SHOW_SLW)
    {
        QIcon icon(QPixmap(fname_split.join("/")));
    
        slwi->setIcon(icon);
    }
    // Now get the mesh corresponding to this match
    QString mesh_name = match_name.split(".").first().append(".off");
    fname_split.removeLast();
    fname_split.removeLast();
    
    fname_split.push_back(MESH_PATH);
    fname_split.push_back(mesh_name);

    QString mesh_fname = fname_split.join("/");

    if (_fname.isEmpty() || !slwi->shape().open(_fname.toStdString().c_str()))
    {
        qCritical() << "Cannot read match from file: " << _fname;
    }
    
    OpenMesh::Utils::Timer t;
    t.start();
    if ( _fname.isEmpty() || (_load_mesh && !slwi->shape().openMesh(mesh_fname.toStdString().c_str())) )
    {
       qCritical() << "Cannot read mesh from file: " << mesh_fname;
    }
    t.stop();
    
    qDebug() << "Loaded mesh in ~" << t.as_string().c_str() ;
    
    slwi->shape().setID(this->count());
    
    this->addItem(slwi);
    
    std::vector<Match*> matches;
    
    matches.push_back(&slwi->shape());
    
    emit matchShapesAdded(matches);
}

void ShapeListWidget::openPlainShape(const QString& _fname, bool _load_mesh)
{
    ShapeListWidgetItem<Shape>* slwi = new ShapeListWidgetItem<Shape>;
    
    QStringList fname_split = _fname.split("/");
    QString mesh_name = fname_split.last();
    slwi->setText(mesh_name);
    slwi->setToolTip(_fname);
    
    slwi->shape().setFilename(_fname);
    // Remove mesh name and its enclosing directory from the list
    fname_split.removeLast();
    fname_split.removeLast();
    
    // Now add the mesh snapshot directory
    fname_split.push_back("img_models");
    QString snap_name = mesh_name.split(".").first().append(".jpg");
    fname_split.push_back(snap_name);
    
    QIcon icon(QPixmap(fname_split.join("/")));
    
    slwi->setIcon(icon);
    
    OpenMesh::Utils::Timer t;
    t.start();
    if ( _fname.isEmpty() || (_load_mesh && !slwi->shape().openMesh(_fname.toStdString().c_str())) )
    {
        qCritical() << "Cannot read mesh from file: " << _fname;
    }
    
    t.stop();
    qDebug() << "Loaded mesh in ~" << t.as_string().c_str() ;
    
    slwi->shape().setID(this->count());
    
    this->addItem(slwi);
}


void ShapeListWidget::selectionChanged ( const QItemSelection & selected, const QItemSelection & deselected )
{
    QModelIndexList cIndexList = selectedIndexes();
    
    QModelIndexList::const_iterator cind_it(cIndexList.constBegin()), cind_end(cIndexList.constEnd());
    
    std::vector<Shape*> cShapes;
    
    qDebug() << "Selections changed. The following rows are selected: ";
    
    for (; cind_it!=cind_end; ++cind_it )
    {
        ShapeListWidgetItem<Shape>* cItem = static_cast<ShapeListWidgetItem<Shape>*>(this->item(cind_it->row()));
        
        if(!cItem)
        {
            qCritical() << "Could not cast to ShapeListWidgetItem<Shape>! " ;
            return;
        }
        qDebug() << cind_it->row() ;
        
        Shape& cShape = cItem->shape();
        
        cShape.openMeshIfNotOpened();
        
        cShapes.push_back(&cItem->shape());
    }
    
    viewport()->update();
    
    if (CREATE_MVW)
    {
        emit selectedShapesChanged(cShapes,0);
    }
}


