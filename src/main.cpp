//
// main.cpp
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#ifdef _MSC_VER
#  pragma warning(disable: 4267 4311)
#endif

#include <iostream>
#include <fstream>
#include <QObject>
#include <QApplication>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QGridLayout>
#include <QSplitter>


#include <QPointer>
#include <QDebug>
#include "LogBrowserDialog.h"

#ifdef ARCH_DARWIN
#include <glut.h>
#else
#include <GL/glut.h>
#endif

#include "MeshViewerWidget.h"
#include "TemplateExplorationWidget.h"

#include "ShapeListWidget.h"

#include "TemplateExplorationView.h"

#include "Matlab.h"
#include "global.h"


QToolBar* create_menu(QMainWindow &w, QWidget& slw);

bool readConfigFile();
void setupColors();

QString COLLECTION_FILE_PATH;
std::string MATLAB_FILE_PATH;
std::string MATLAB_APP_PATH;

QString MESH_PATH;
QString TEMPLATE_ICON_PATH;
QString MATCH_ICON_PATH;
QString MODEL_ICON_PATH;

QString RESULTS_PATH;

bool PRELOAD_MODELS;
DATASET LOADED_DATASET;
float FIT_ERROR;
int MAX_NUM_CLUSTERS_TO_SHOW;
int MIN_CLUSTER_POPULATION;

int NUM_OF_NEAREST_NEIGHBOURS;
int NUM_PARAMS_BOX;
int NUM_PARAMS_POS;
EMBEDDING_TYPES EMBEDDING_MODE;

int NUM_EQUATIONS_SYMMETRY;
int NUM_EQUATIONS_CONTACT;

DEBUG_TYPES DEBUG_MODE;
bool CREATE_SLW;
bool CREATE_TEW;
bool CREATE_QWTPLOTW;
bool CREATE_TEVW;
bool CREATE_MVW;
bool CREATE_LOGW;

bool SHOW_SLW;
bool SHOW_TEW;
bool SHOW_QWTPLOTW;
bool SHOW_TEVW;
bool SHOW_MVW;
bool SHOW_LOGW;
int NOF_MVW;

bool SAVE_DESCRIPTOR;

bool ALIGN_MATCH_POINTS_BEFORE_SAVING;
bool  ALIGN_MATCH_MESH_BEFORE_SAVING;
bool  NORMALISE_MATCH_MESH_BEFORE_SAVING;
bool  RECOMPUTE_BOXES_BEFORE_SAVING;

bool OPEN_DESCRIPTOR;
bool  ALIGN_MATCH_MESH_AFTER_OPENING;
bool NORMALISE_MATCH_MESH_AFTER_OPENING;
bool OPEN_ORIGINAL_MESH;
bool  OPEN_PART_MESHES_AFTER_OPENING_MATCH_MESH;

int APP_WINDOW_WIDTH;
int APP_WINDOW_HEIGHT;

int CLUSTER_VIEW_ICON_HEIGHT;
int CLUSTER_VIEW_ICON_PADDING;
int CLUSTER_VIEW_ICON_FRAME_THICKNESS;
int CLUSTER_VIEW_ICON_SPACING;
int EXPLORATION_VIEW_ICON_HEIGHT;

int PART_DESC_SIZE_ROWS;
int PART_DESC_SIZE_COLS;
int PART_DESC_SIZE;

// Setup some random colors to paint our objects
GLfloat shapeColors[MAX_NUM_OF_COLORS][4];

QColor tewColors[MAX_NUM_OF_COLORS];

QTextBrowser* TIMELOG =0;

Engine* Matlab::matlabEngine_ = 0;

LogBrowserDialog* logBrowser;

void printMessage(QtMsgType type, const char *msg)
{
    if(logBrowser)
        logBrowser->outputMessage( type, msg );
}

class MainWindow : public QMainWindow
{

public:
    
    MainWindow(TemplateExplorationWidget* _tew)
    {
        tew = _tew;
    }
    
    void setWidg(TemplateExplorationWidget* _tew)
    {
        tew = _tew;
    }
    
    void setV(View* _tevw)
    {
        tevw = _tevw;
    }
    
    void setLg(LogBrowserDialog* _lbd)
    {
        lbd = _lbd;
    }
    
protected:
    
    void keyPressEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Backspace)
        {
        	if(tew)
        	{
            	tew->slotGoBack();
            }
        }
        
        if (event->key() == Qt::Key_Right)
        {
        	if(tew)
        	{
            	tew->slotShowNextNeighbour();
            }
        }
        
        if (event->key() == Qt::Key_Left)
        {
        	if(tew)
        	{
            	tew->slotShowPreviousNeighbour();
            }
        }
        
        if (event->key() == Qt::Key_C)
        {
        	if(tew)
        	{
            	tew->slotToggleConstraints();
            }
        }
        
        if (event->key() == Qt::Key_X)
        {
        	if(tew)
        	{
            	tew->slotToggleSingleColor();
            }
        }
        
        QMainWindow::keyPressEvent(event);
        
    }
    
    void closeEvent(QCloseEvent *event)
    {
        if(tew && tevw && lbd)
        {
			QDir().mkpath(RESULTS_PATH);
		
			QString logname(RESULTS_PATH + "general-log.txt");
		
			QFile file(logname  );
		
			if (!file.open(QIODevice::WriteOnly))
			{
				qCritical() << "Could not open file " << logname ;
				return;
			}
		
			QTextStream out(&file);
		
			tew->slotSaveGeneralSettings(out);
			
			dynamic_cast<GraphicsView*>(tevw->view())->saveGeneralSettings(out);
		
			file.close();
		
			lbd->save();
        }
        
        if(TIMELOG)
        {
        	QString timelogname(RESULTS_PATH + "time-log.txt");
		
			QFile file2(timelogname);
		
			if (!file2.open(QIODevice::WriteOnly))
			{
				qCritical() << "Could not open file " << timelogname ;
				return;
			}
			
			QTextStream stream(&file2);
		
			stream << TIMELOG->toPlainText();
		
			file2.close();
        }
        event->accept();
    }
private:
    
    TemplateExplorationWidget* tew = 0;
    View* tevw = 0;
    LogBrowserDialog* lbd = 0;
    
};

int main(int argc, char **argv)
{
    // OpenGL check
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication app(argc,argv);
#if !defined(__APPLE__)
    glutInit(&argc,argv);
#endif
    
    if ( !QGLFormat::hasOpenGL() )
    {
        QString msg = "System has no OpenGL support!";
        QMessageBox::critical( 0, QString("OpenGL"), msg + QString(argv[1]) );
        return -1;
    }
    
    if (!readConfigFile())
    {
        std::cout << "Config file not found. Make sure it is in the same folder as the application!! Exiting!!" << std::endl;
        return -1;
    }
    setupColors();
    
    Matlab::init();
   
    // create main window
    MainWindow mainWin(0);
    
    TIMELOG = new QTextBrowser;
    
    // Left pane
    ShapeListWidget* slw = 0 ;
    
    QToolBar* tools;
    
    if (CREATE_SLW)
    {
        slw = new ShapeListWidget(&mainWin);
        
        slw->setIconSize(QSize(100,100));
        
        if (!SHOW_SLW)
        {
            slw->setVisible(false);
        }
        
        tools = create_menu(mainWin, *slw);
    }

   
    // Middle pane
    
    QSplitter* midPaneSplit = new QSplitter(Qt::Vertical);
    
    QWidget* exploInfoWidget = new QWidget;
    QHBoxLayout* exploInfoLayout = new QHBoxLayout;
    
    exploInfoWidget->setLayout(exploInfoLayout);
    exploInfoWidget->setMaximumHeight(50);
    QLabel* lblExploMode = new QLabel("Interaction mode:");
    exploInfoLayout->addWidget(lblExploMode);
    QLabel* spacing1 = new QLabel("   |   ");
    exploInfoLayout->addWidget(spacing1);
    QLabel* lblExploConstraints = new QLabel("Constraints: ON");
    exploInfoLayout->addWidget(lblExploConstraints);
    QLabel* spacing2 = new QLabel("   |   ");
    exploInfoLayout->addWidget(spacing2);
    QLabel* lblNN = new QLabel(QString("No of Neighbours: %1").arg(NUM_OF_NEAREST_NEIGHBOURS));
    exploInfoLayout->addWidget(lblNN);
    QLabel* spacing3 = new QLabel("   |   ");
    exploInfoLayout->addWidget(spacing3);
    QLabel* lblNNused = new QLabel(QString("No of Neighbours used: 0"));
    exploInfoLayout->addWidget(lblNNused);
    QLabel* spacing4 = new QLabel(" / ");
    exploInfoLayout->addWidget(spacing4);
    QLabel* lblNIP = new QLabel(QString("No of independent parts: 0"));
    exploInfoLayout->addWidget(lblNIP);
    
    exploInfoLayout->addStretch();
    
    midPaneSplit->addWidget(exploInfoWidget);
    
    QWidget* mvWidgets = new QWidget;
    QGridLayout* mvWidgetsLayout = new QGridLayout;
    
    std::vector< MeshViewerWidget* > mvw;
    
    if (CREATE_MVW)
    {
        for (int i=0; i < NOF_MVW; ++i)
        {
            mvw.push_back( new MeshViewerWidget(i,&mainWin));
            
            // Put all widgets in the same line
            mvWidgetsLayout->addWidget(mvw[i], 0, i);
            
            mvw[i]->setMinimumHeight(200);
            mvw[i]->setMinimumWidth(200);
            
            if (!SHOW_MVW)
            {
                mvw[i]->setVisible(false);
                continue;
            }
            
            // Even if we set SHOW_MVW to true, we would like to only show the first mvw for now
            if (i>0)
            {
                mvw[i]->setVisible(false);
            }
        }
        
        mvWidgets->setLayout(mvWidgetsLayout);
        
        midPaneSplit->addWidget(mvWidgets); //&mvw);
    }
    
    
    if (CREATE_LOGW)
    {
        logBrowser = new LogBrowserDialog;
        mainWin.setLg(logBrowser);
        qInstallMsgHandler(printMessage);
        
        logBrowser->setMaximumHeight(200);
        if (!SHOW_LOGW)
        {
            logBrowser->setVisible(false);
        }
        
        midPaneSplit->addWidget(logBrowser);
    }
    
    // Right pane
    
    QSplitter* rightPaneSplit = new QSplitter(Qt::Vertical);
    
    TemplateExplorationWidget* tew = 0;
    
    if (CREATE_TEW)
    {
        tew = new TemplateExplorationWidget;
        mainWin.setWidg(tew);
        
        QAction* saveSynthAct = new QAction("Save Synthesized Model", &mainWin);
        saveSynthAct->setStatusTip("Save Synthesized Model");
        QObject::connect(saveSynthAct, SIGNAL(triggered()), tew, SLOT(slotSaveSynthesizedModel()));
        
        tools->addAction(saveSynthAct);

        if (!SHOW_TEW)
        {
            tew->setVisible(false);
        }
        
        rightPaneSplit->addWidget(tew); //qsa);
    }
    
    View* tevw = 0;
    
    if (CREATE_TEVW)
    {
        tevw = new View("Exploration view");
        mainWin.setV(tevw);
        
        if (CREATE_TEW)
        {
            tevw->view()->setScene(tew->scene());
        }
        
        if (!SHOW_TEVW)
        {
            tevw->setVisible(false);
        }
        
        rightPaneSplit->addWidget(tevw);
    }

   
    QSplitter* qsplit = new QSplitter(Qt::Horizontal);
    
    qsplit->addWidget(rightPaneSplit);
    qsplit->addWidget(midPaneSplit);
    qsplit->addWidget(slw);
    
    // Add everything to the central widget
    QWidget* centralWidget = new QWidget;
    
    mainWin.setCentralWidget(centralWidget);
    
    QGridLayout* centralLayout = new QGridLayout;
    
    centralLayout->addWidget(qsplit);
    centralWidget->setLayout(centralLayout);
    
    mainWin.setWindowTitle("ShapeSynth"); //"Template-based exploration and synthesis");
    mainWin.resize(APP_WINDOW_WIDTH, APP_WINDOW_HEIGHT);
    mainWin.show();
    

    
    if (CREATE_SLW && CREATE_MVW)
    {
        QObject::connect(slw,SIGNAL(selectedShapesChanged(const std::vector<Shape*>&,int)), mvw[0], SLOT(slotSetShapes(const std::vector<Shape*>&,int)));
        
        QObject::connect(slw,SIGNAL(requestRedraw(int)), mvw[0], SLOT(slotRedraw(int)));
        
    }
    if (CREATE_SLW && CREATE_TEW)
    {
        QObject::connect(slw,SIGNAL(matchShapesAdded(const std::vector<Match*>&)), tew, SLOT(slotAddMatches(const std::vector<Match*>&)));
    }
    
    if (CREATE_TEW && CREATE_MVW)
    {
        for (int i=0; i< NOF_MVW; i++)
        {
            QObject::connect(tew,SIGNAL(selectedShapesChanged(const std::vector<Shape*>&, int)), mvw[i], SLOT(slotSetShapes(const std::vector<Shape*>&, int)));
            
            QObject::connect(tew,SIGNAL(requestRedraw(int)), mvw[i], SLOT(slotRedraw(int)));
            
            QObject::connect(tew,SIGNAL(toggleVisible(bool,int)), mvw[i], SLOT(setViewable(bool,int)));
        
            QObject::connect(mvw[i], SIGNAL(pickedShapeShowNext(unsigned int)), tew, SLOT(slotSetPickedPartIDShowNext(unsigned int)));
            
            QObject::connect(mvw[i], SIGNAL(pickedShapeShowPrevious(unsigned int)), tew, SLOT(slotSetPickedPartIDShowPrevious(unsigned int)));
            
            QObject::connect(mvw[i], SIGNAL(hoveredShape(int)), tew, SLOT(slotSetHoveredPartID(int)));
        }
    }
    
    if (CREATE_TEW && CREATE_TEVW)
    {
        QObject::connect(tew, SIGNAL(sceneChanged()), tevw, SLOT(fitScene()));
        
        QObject::connect(tevw->view(), SIGNAL(selectedPointChanged(double,double)), tew, SLOT(slotChangeSelectedPoint(double, double)));
        
        QObject::connect(tevw->view(), SIGNAL(hoveredPointChanged(double,double)), tew, SLOT(slotChangeHoveredPoint(double, double)));

        QObject::connect(tevw->view(), SIGNAL(interactionModeChanged(const QString&)), lblExploMode, SLOT(setText(const QString&)));
        
        QObject::connect(tew, SIGNAL(interactionModeChanged(const QString&)), lblExploMode, SLOT(setText(const QString&)));
        
        QObject::connect(tew, SIGNAL(constraintsChanged(const QString&)), lblExploConstraints, SLOT(setText(const QString&)));
        
        QObject::connect(tew, SIGNAL(nnChanged(const QString&)), lblNN, SLOT(setText(const QString&)));
        
        QObject::connect(tew, SIGNAL(nnUsedChanged(const QString&)), lblNNused, SLOT(setText(const QString&)));
        
        QObject::connect(tew, SIGNAL(nIndependentPartsChanged(const QString&)), lblNIP, SLOT(setText(const QString&)));
        
        QObject::connect(tevw->view(), SIGNAL(nnUsedChanged(const QString&)), lblNNused, SLOT(setText(const QString&)));
        
        QObject::connect(tevw->view(), SIGNAL(nIndependentPartsChanged(const QString&)), lblNIP, SLOT(setText(const QString&)));
    }
    
    if (CREATE_TEW)
    {
        slw->openMatchCollection(COLLECTION_FILE_PATH, PRELOAD_MODELS);
    }
    
    return app.exec();
}


void setupColors()
{
    srand(248504281);
    
    // Just as a precautionary measure, have say 100 random colors, so we never go out of bounds based on labels for coloring
    for(int i=0; i<MAX_NUM_OF_COLORS; i++)
    {
        shapeColors[i][0] = ((float)(rand()%256))/255.f;
        shapeColors[i][1] = ((float)(rand()%256))/255.f;
        shapeColors[i][2] = ((float)(rand()%256))/255.f;
        shapeColors[i][3] = 1.0f;
        
        tewColors[i].setRedF(((float)(rand()%256))/255.f);
        tewColors[i].setGreenF(((float)(rand()%256))/255.f);
        tewColors[i].setBlueF(((float)(rand()%256))/255.f);
        tewColors[i].setAlphaF(1.0f);
    }
    
    // Now actually set the two colormaps we want, one for shapes and one for the plot and gui for the exploration widget
    // colormaps from colorbrewer2.org
    
    // shapeColors, assuming 8 different data classes, i.e. parts, of qualitative nature, so they are not related in any way and we need as distinguishing colors as possible, picked the Set1 color scheme which is also printer-friendly
    
    shapeColors[0][0] = 228.f / 255.f;
    shapeColors[0][1] = 26.f / 255.f;
    shapeColors[0][2] = 28.f /255.f;
    shapeColors[0][3] = 1.0f;
    
    shapeColors[1][0] = 55.f / 255.f;
    shapeColors[1][1] = 126.f / 255.f;
    shapeColors[1][2] = 184.f /255.f;
    shapeColors[1][3] = 1.0f;
    
    shapeColors[2][0] = 77.f / 255.f;
    shapeColors[2][1] = 175.f / 255.f;
    shapeColors[2][2] = 74.f /255.f;
    shapeColors[2][3] = 1.0f;
    
    shapeColors[3][0] = 152.f / 255.f;
    shapeColors[3][1] = 78.f / 255.f;
    shapeColors[3][2] = 163.f /255.f;
    shapeColors[3][3] = 1.0f;
    
    shapeColors[4][0] = 255.f / 255.f;
    shapeColors[4][1] = 127.f / 255.f;
    shapeColors[4][2] = 0.f /255.f;
    shapeColors[4][3] = 1.0f;
    
    shapeColors[5][0] = 255.f / 255.f;
    shapeColors[5][1] = 255.f / 255.f;
    shapeColors[5][2] = 51.f /255.f;
    shapeColors[5][3] = 1.0f;
    
    shapeColors[6][0] = 166.f / 255.f;
    shapeColors[6][1] = 86.f / 255.f;
    shapeColors[6][2] = 40.f /255.f;
    shapeColors[6][3] = 1.0f;
    
    shapeColors[7][0] = 247.f / 255.f;
    shapeColors[7][1] = 129.f / 255.f;
    shapeColors[7][2] = 191.f /255.f;
    shapeColors[7][3] = 1.0f;
    
    // exploration widget colors, assuming 5 different data classes, i.e. clusters, of qualitative nature, so they are not related in any way and we need as distinguishing colors as possible, picked the Dark2 scheme which is also printer-friendly
    tewColors[0].setRed(27);
    tewColors[0].setGreen(158);
    tewColors[0].setBlue(119);
    tewColors[0].setAlpha(255);
    
    tewColors[1].setRed(217);
    tewColors[1].setGreen(95);
    tewColors[1].setBlue(2);
    tewColors[1].setAlpha(255);
    
    tewColors[2].setRed(117);
    tewColors[2].setGreen(112);
    tewColors[2].setBlue(179);
    tewColors[2].setAlpha(255);
    
    tewColors[3].setRed(231);
    tewColors[3].setGreen(41);
    tewColors[3].setBlue(138);
    tewColors[3].setAlpha(255);
    
    tewColors[4].setRed(102);
    tewColors[4].setGreen(166);
    tewColors[4].setBlue(30);
    tewColors[4].setAlpha(255);
    
}



QToolBar* create_menu(QMainWindow &w, QWidget& slw)
{
    QAction* addShapeAct = new QAction(w.tr("Add Shapes"), &w);
    addShapeAct->setStatusTip(w.tr("Add Shape"));
    QObject::connect(addShapeAct, SIGNAL(triggered()), &slw, SLOT(slotAddShapes()));
    
    QAction* addShapesDirAct = new QAction(w.tr("Add Shapes Directory"), &w);
    addShapesDirAct->setStatusTip(w.tr("Add Shapes Directory"));
    QObject::connect(addShapesDirAct, SIGNAL(triggered()), &slw, SLOT(slotAddShapesFromDir()));
    
    QAction* saveMatchCollectionAct = new QAction(w.tr("Save Match Collection"), &w);
    saveMatchCollectionAct->setStatusTip(w.tr("Save Match Collection"));
    QObject::connect(saveMatchCollectionAct, SIGNAL(triggered()), &slw, SLOT(slotSaveMatchCollection()));
    
    QToolBar* toolBar = new QToolBar();

    toolBar->addAction(addShapeAct);
    
    if (SHOW_SLW)
    {
        toolBar->addAction(addShapesDirAct);
        toolBar->addAction(saveMatchCollectionAct);
    }
    w.addToolBar(toolBar);
    return toolBar;
}

bool readConfigFile() //(ShapeListWidget* slw, TemplateExplorationWidget* tew)
{
    // Read config file
 
	QString filename = QString("./config.txt");

    QFile file(filename);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Could not open file " << filename;
        return false;
    }
	else
	{
		QTextStream in(&file);
        int lineno = 0;
		while (!in.atEnd())
		{
			QString line = in.readLine();

            QStringList splitLine = line.split(" = ");
            QString varName = splitLine.first();
            
            QString varValueString = splitLine.last();
            int varValueInt = varValueString.toInt();
            float varValueFloat = varValueString.toFloat();
            
            if (varName == "COLLECTION_FILE_PATH")
            {
                COLLECTION_FILE_PATH = varValueString;
            }
            if (varName == "MATLAB_FILE_PATH")
            {
                MATLAB_FILE_PATH = varValueString.toStdString();
            }
            if (varName == "MATLAB_APP_PATH")
            {
                MATLAB_APP_PATH = varValueString.toStdString();
            }
            if (varName == "MESH_PATH")
            {
                MESH_PATH = varValueString;
            }
            if (varName == "TEMPLATE_ICON_PATH")
            {
                TEMPLATE_ICON_PATH = varValueString;
            }
            if (varName == "MATCH_ICON_PATH")
            {
                MATCH_ICON_PATH = varValueString;
            }
            if (varName == "MODEL_ICON_PATH")
            {
                MODEL_ICON_PATH = varValueString;
            }
            if (varName == "RESULTS_PATH")
            {
                RESULTS_PATH = varValueString;
                RESULTS_PATH += QDateTime::currentDateTime().toString( "yyMMddhhmmss" );
                RESULTS_PATH += "/";
            }
            if (varName == "PRELOAD_MODELS")
            {
                PRELOAD_MODELS = varValueInt>0 ? true: false;
            }
            if (varName == "DATASET")
            {
                LOADED_DATASET = (DATASET)varValueInt;
            }
            if (varName == "FIT_ERROR")
            {
                FIT_ERROR = varValueFloat;
            }
            if (varName == "MAX_NUM_CLUSTERS_TO_SHOW")
            {
                MAX_NUM_CLUSTERS_TO_SHOW = varValueInt;
            }
            if (varName == "MIN_CLUSTER_POPULATION")
            {
                MIN_CLUSTER_POPULATION = varValueInt;
            }
            if (varName == "NUM_NEAREST_NEIGHBOURS")
            {
                NUM_OF_NEAREST_NEIGHBOURS = varValueInt;
            }
            if (varName == "NUM_PARAMS_BOX")
            {
                NUM_PARAMS_BOX = varValueInt;
            }
            if (varName == "NUM_PARAMS_POS")
            {
                NUM_PARAMS_POS = varValueInt;
            }
            if (varName == "EMBEDDING_MODE")
            {
                EMBEDDING_MODE = (EMBEDDING_TYPES)varValueInt;
            }
            if (varName == "NUM_EQUATIONS_SYMMETRY")
            {
                NUM_EQUATIONS_SYMMETRY = varValueInt;
            }
            if (varName == "NUM_EQUATIONS_CONTACT")
            {
                NUM_EQUATIONS_CONTACT = varValueInt;
            }
            if (varName == "DEBUG_MODE")
            {
                DEBUG_MODE = (DEBUG_TYPES)varValueInt;
            }
            if (varName == "CREATE_SLW")
            {
                CREATE_SLW = varValueInt>0 ? true: false;
            }
            if (varName == "CREATE_TEW")
            {
                CREATE_TEW = varValueInt>0 ? true: false;
            }
            if (varName == "CREATE_QWTPLOTW")
            {
                CREATE_QWTPLOTW = varValueInt>0 ? true: false;
            }
            if (varName == "CREATE_TEVW")
            {
                CREATE_TEVW = varValueInt>0 ? true: false;
            }
            if (varName == "CREATE_MVW")
            {
                CREATE_MVW = varValueInt>0 ? true: false;
            }
            if (varName == "CREATE_LOGW")
            {
                CREATE_LOGW = varValueInt>0 ? true: false;
            }
            if (varName == "SHOW_SLW")
            {
                SHOW_SLW = varValueInt>0 ? true: false;
            }
            if (varName == "SHOW_TEW")
            {
                SHOW_TEW = varValueInt>0 ? true: false;
            }
            if (varName == "SHOW_QWTPLOTW")
            {
                SHOW_QWTPLOTW = varValueInt>0 ? true: false;
            }
            if (varName == "SHOW_TEVW")
            {
                SHOW_TEVW = varValueInt>0 ? true: false;
            }
            if (varName == "SHOW_MVW")
            {
                SHOW_MVW = varValueInt>0 ? true: false;
            }
            if (varName == "SHOW_LOGW")
            {
                SHOW_LOGW = varValueInt>0 ? true: false;
            }
            if (varName == "NOF_MVW")
            {
                NOF_MVW = varValueInt;
            }
            if (varName == "SAVE_DESCRIPTOR")
            {
                SAVE_DESCRIPTOR = varValueInt>0 ? true: false;
            }
            if (varName == "ALIGN_MATCH_POINTS_BEFORE_SAVING")
            {
                ALIGN_MATCH_POINTS_BEFORE_SAVING = varValueInt>0 ? true: false;
            }
            if (varName == "ALIGN_MATCH_MESH_BEFORE_SAVING")
            {
                ALIGN_MATCH_MESH_BEFORE_SAVING = varValueInt>0 ? true: false;
            }
            if (varName == "NORMALISE_MATCH_MESH_BEFORE_SAVING")
            {
                NORMALISE_MATCH_MESH_BEFORE_SAVING = varValueInt>0 ? true: false;
            }
            if (varName == "RECOMPUTE_BOXES_BEFORE_SAVING")
            {
                RECOMPUTE_BOXES_BEFORE_SAVING = varValueInt>0 ? true: false;
            }
            if (varName == "OPEN_DESCRIPTOR")
            {
                OPEN_DESCRIPTOR = varValueInt>0 ? true: false;
            }
            
            if (varName == "ALIGN_MATCH_MESH_AFTER_OPENING")
            {
                ALIGN_MATCH_MESH_AFTER_OPENING = varValueInt>0 ? true: false;
            }
            if (varName == "NORMALISE_MATCH_MESH_AFTER_OPENING")
            {
                NORMALISE_MATCH_MESH_AFTER_OPENING = varValueInt>0 ? true: false;
            }
            if (varName == "OPEN_ORIGINAL_MESH")
            {
                OPEN_ORIGINAL_MESH = varValueInt>0 ? true: false;
            }
            if (varName == "OPEN_PART_MESHES_AFTER_OPENING_MATCH_MESH")
            {
                OPEN_PART_MESHES_AFTER_OPENING_MATCH_MESH = varValueInt>0 ? true: false;
            }
            if (varName == "APP_WINDOW_WIDTH")
            {
                APP_WINDOW_WIDTH = varValueInt;
            }
            if (varName == "APP_WINDOW_HEIGHT")
            {
                APP_WINDOW_HEIGHT = varValueInt;
            }
            if (varName == "CLUSTER_VIEW_ICON_HEIGHT")
            {
                CLUSTER_VIEW_ICON_HEIGHT = varValueInt;
            }
            if (varName == "CLUSTER_VIEW_ICON_PADDING")
            {
                CLUSTER_VIEW_ICON_PADDING = varValueInt;
            }
            if (varName == "CLUSTER_VIEW_ICON_FRAME_THICKNESS")
            {
                CLUSTER_VIEW_ICON_FRAME_THICKNESS = varValueInt;
            }
            if (varName == "CLUSTER_VIEW_ICON_SPACING")
            {
                CLUSTER_VIEW_ICON_SPACING = varValueInt;
            }
            if (varName == "EXPLORATION_VIEW_ICON_HEIGHT")
            {
                EXPLORATION_VIEW_ICON_HEIGHT = varValueInt;
            }
            if (varName == "PART_DESC_SIZE_ROWS")
            {
                PART_DESC_SIZE_ROWS = varValueInt;
            }
            if (varName == "PART_DESC_SIZE_COLS")
            {
                PART_DESC_SIZE_COLS = varValueInt;
            }
            if (varName == "PART_DESC_SIZE")
            {
                PART_DESC_SIZE = varValueInt;
            }
        }
	}
    return true;
}



