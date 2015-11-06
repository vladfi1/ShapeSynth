//
// TemplateExplorationWidget.h
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#ifndef WIDGETLEFTPANE_HH
#define WIDGETLEFTPANE_HH

#include <unordered_map>

#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QLineEdit>
#include <QSlider>
#include <QScrollArea>
#include <QFile>
#include <QFileDialog>
#include <QAction>
#include <QToolBar>
#include <QPushButton>
#include <QCheckBox>
#include <QDebug>
#include <QDirIterator>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGraphicsScene>
#include <QListWidget>
#include <QToolButton>
#include <QSplitter>
#include <QDateTime>

#include "nanoflann.h"


#include "stdafx.h"
#include <math.h>
#include "alglibinternal.h"
#include "alglibmisc.h"
#include "linalg.h"
#include "statistics.h"
#include "dataanalysis.h"
#include "specialfunctions.h"
#include "solvers.h"
#include "optimization.h"
#include "diffequations.h"
#include "fasttransforms.h"
#include "integration.h"
#include "interpolation.h"


#include "MatchT.h"
#include "Matlab.h"
#include "TemplateExplorationViewItem.h"

using namespace alglib;

class TemplateExplorationWidget : public QWidget
{
	Q_OBJECT
    
public:
    
    typedef ShapeT<TriangleMesh> Shape;
    typedef MatchT<TriangleMesh> Match;
    
//    struct Cluster
//    {
//        int population_ = 0;
//        int representativeIndex_ = -1;
//        OpenMesh::Vec2f centroid_;
//        int label = -1;
//    };
//    
//    struct ClusterFunctor
//    {
//        bool operator() (const Cluster* i, const Cluster* j)
//        {
//            return i->population_ < j->population_;
//        }
//            
//    } CompareClustersFunctor;
    
    struct GroupFunctor
    {
        
        bool operator() (const std::pair<int, int>& i,const std::pair<int, int>& j)
        {
            return i.second > j.second;
        }
            
    } CompareGroupsFunctor;
 
    struct PartScoresFunctor
    {
        
        bool operator() (const std::pair<int, double>& i,const std::pair<int, double>& j)
        {
            return i.second < j.second;
        }
        
    } ComparePartScoresFunctor;
    
    struct MatchFunctor
    {
        
        bool operator() (const Match* i,const Match* j)
        {
            QString iName = i->filename().split("/").last().split(".").first();
            QString jName = j->filename().split("/").last().split(".").first();
            
            return iName < jName;
        }
            
    } CompareMatchesFunctor;

    struct MatchLabelFunctor
    {
        bool operator() (Match* cMatch)
        {
            return cMatch->label() >= label_; //MAX_NUM_CLUSTERS_TO_SHOW;
        }
        
        int label_ = -1;
        
    } CompareMatchLabelFunctor;
            
    struct ExplorationDataState
    {
        bool templateValid = false;
        bool* deformedNearestValid;
        bool deformedNearestOptionsValid = false;
        bool representativeValid = false;
        bool deformedPartOptionsValid = false;
    };
	
            
    TemplateExplorationWidget(QWidget* pParent = 0);
	~TemplateExplorationWidget(void);

	QComboBox* comboBoxTemplateID(void) { return comboBoxTemplateID_; }
	QComboBox* comboBoxPartID(void) { return comboBoxPartID_; }
	QComboBox* comboBoxGroupID(void) { return comboBoxGroupID_; }
    QLineEdit* lineEditErrorThreshold(void) { return lineEditErrorThreshold_; }
    
    QLabel* labelMatchName(void) { return labelMatchName_; }
    QLabel* labelMatchTemplateID(void) { return labelMatchTemplateID_; }
    QLabel* labelMatchGroupID(void) { return labelMatchGroupID_; }
    QLabel* labelMatchFitError(void) { return labelMatchFitError_; }
    
    QLabel* labelLambda1(void) { return labelLambda1_; }
    QLabel* labelLambda2(void) { return labelLambda2_; }
    QSlider* sliderLambda1(void)  { return sliderLambda1_; }
    QSlider* sliderLambda2(void)  { return sliderLambda2_; }
    
	QComboBox* comboBoxExplorationMode(void) { return comboBoxExplorationMode_; }
	QComboBox* comboBoxCalculationMode(void) { return comboBoxCalculationMode_; }

	QComboBox* comboBoxNearestNeighbours(void) { return comboBoxNearestNeighbours_; }
    
    //  std::tr1::unordered_map<int,int>& GetPlotIndexToMatchIndex(void) { return plotIndexToMatchIndex; }
    //
    //  std::tr1::unordered_map<std::string,int>& GetMeshNameToMatchIndex(void) { return meshNameToMatchIndex; }

    QGraphicsScene* scene() { return scene_; }
public slots:
    
    void slotAddMatches(const std::vector<Match*>& _matches);

    void slotCalculateMDS();

    void slotCalculatePCA();

    void slotChangeSelectedPoint(double _posx, double _posy);
            
    void slotDeformTemplate(double _lambda1, double _lambda2);

    void slotDeformKNearestMatches(double _posx, double _posy);

    void slotDeformNearestMatch(int _level);
    
    void slotChangeL1(int _lambda);

    void slotChangeL2(int _lambda);

    void slotChangeSelectedMatch(int _newIndex);

    void slotGroupMatches();

    void slotReadFitErrorFile(void);
            
    void slotReadNormalizationFile();

    void slotReadLabels();
            
    void slotChangeExplorationMode(int _explorationMode);
    
    void slotChangeCalculationMode(int _calculationMode);
    
    void slotChangeDisplayMode(int _displayMode);
            
    void slotChangeNearestNeighbourLevel(int _level);
    
    void slotRecalculatePartBoxes();
    
    void slotSaveMatchPointsToFile();
    
    void slotSaveMatchDescriptorsToFile();
     
    void slotInterpolateSelected(int _lerp);
    
    void slotResetLerp(bool _checked);
    
    void slotSelectCluster(bool _checked);
    
    void slotSplitMatches();
    
    void slotSaveMatchNamesToFile();

    void slotPreservePointScales(float _viewScale);
    
    void slotChangeSelectedCluster(QListWidgetItem * current);
    
    void slotChangeSelectedMatches();
            
    void slotSetPickedPartIDShowNext(unsigned int _id);
    
    void slotSetPickedPartIDShowPrevious(unsigned int _id);
            
    void slotSetHoveredPartID(int _id);
            
    void slotChangeHoveredPoint(double _posx, double _posy);
    
    void slotJoinMatchesInSingleGroup();
        
    void slotToggleConstraints();

    void slotToggleSingleColor();
      
    void slotGoBack();
            
    void slotShowNextNeighbour();
            
    void slotShowPreviousNeighbour();
         
    void slotSaveSynthesizedModel();
            
    void slotSaveGeneralSettings(QTextStream& _out);

    void slotSetForceNeighborIndex(const QString& _text);
      
    void slotSaveSelectedMatches();
            
private slots:

    void slotSetTemplateID(const QString& _id);
    
    void slotSetPartID(const QString& _id);
    
    void slotSetGroupID(const QString& _id);
    
    void slotSetFitErrorThreshold(const QString& _error);
    
    void slotSetLabelID(const QString& _id);
            
    void slotShowNextDeformationOption();
    
    void slotShowNextPartDeformationOption();
        
    void slotShowPreviousPartDeformationOption();
        
    void slotResetIView(int i);
            
    void slotSetNofNN(const QString& _nofNN);

    void slotChangeRecalculateBoxes(int state);
        
    void slotManuallyChangeSelectedPoint();
    
    void slotOptimizeTemplate();

signals:
    
#if defined (APPLE)

    //void setPlotSamples(const QVector<QPointF>& Samples, QString& Title);

    void setPlotSamples(const std::vector<OpenMesh::Vec3f>& _samples, const QString& _title);
            
#elif defined (UNIX)

    void setPlotSamples(const QVector<QwtPoint3D>& Samples, QString& Title);

#endif

    void setEdges(const QVector<QPointF>& Samples, const std::vector<double>& Distances);

    void addPlotMarker(const QPointF& sample, const QColor& color, int _index);

    void deletePlotMarkers();
            
    void deletePlotMarker(int _index);

    void explorationModeChanged(int _explorationMode);

    void lambda1Changed(int _lambda1);

    void lambda2Changed(int _lambda2);

    void lambda1Changed(const QString& _lambda1);

    void lambda2Changed(const QString& _lambda2);
    
    // The following 3 signals did not use to have the _id, this was added to be able to differentiate between different mesh viewer widgets
    // If more than one mesh viewer widgets exist, we can send the id of each in the signal to affect only the one we want
    void selectedShapesChanged(const std::vector<Shape*>& _shapes, int _id);
                
    void toggleVisible(bool _visible, int _id);
            
    void requestRedraw(int _id);
    
    // This only gets emited if we have a qwtplot widget and we need to rebuild its kdtree for nn searching
    void plotPointsChanged();
    
    void sceneChanged();
            
    void interactionModeChanged(const QString& _mode);

    void constraintsChanged(const QString& _constraints);
            
    void nnChanged(const QString& _nn);
    
    void nnUsedChanged(const QString& _nn);
      
    void nIndependentPartsChanged(const QString& _nn);
            
protected:
    
	QComboBox* comboBoxTemplateID_;
	QComboBox* comboBoxPartID_;
	QComboBox* comboBoxGroupID_;
    
    QLineEdit* lineEditErrorThreshold_;
    
    QLabel* labelMatchName_;
    QLabel* labelMatchTemplateID_;
    QLabel* labelMatchGroupID_;
    QLabel* labelMatchFitError_;
    
    QLabel* labelLambda1_;
    QLabel* labelLambda2_;
    QSlider* sliderLambda1_;
    QSlider* sliderLambda2_;
	
    QLabel* labelLerp_;
    QSlider* sliderLerp_;
    QPushButton* btnResetLerp_;
            
    QComboBox* comboBoxExplorationMode_;
	QComboBox* comboBoxCalculationMode_;
	QComboBox* comboBoxNearestNeighbours_;
    QComboBox* comboBoxDisplayMode_;
            
    QPushButton* btnSelectCluster_;
    
    QCheckBox* chkBoxUseANN_;
    
    QPushButton* btnShowNextDefOption_;
    
    QButtonGroup* btnGroup_;
    
    QLineEdit* lnDescriptorSize_;
            
    QCheckBox* chkNN_;
    
    QGraphicsScene* scene_;
            
    QListWidget* templateListWidget_;
            
    QPushButton* btnBack_;
          
    QLabel* nnLabel_;
    
    QLineEdit* lnEdtNofNN_;
            
    QPushButton* btnShowNextDefPartOption_;
            
    QPushButton* btnShowPreviousDefPartOption_;
    
    QCheckBox* chkRecalculateBoxes_;
    
    QLineEdit* lnPointX_;
    
    QLineEdit* lnPointY_;
    
    QPushButton* btnChangeSelectedPoint_;
            
    QWidget* exploScenarioWidget_;
    
    QCheckBox* chkConstraints_;
            
    QCheckBox* chkColorOverride_;
         
    QLineEdit* lnEdtForceNeighbor_;
            
protected slots:
    
    //void slotChangeLambdaLabel(int lambda);
    
private:

    void resetDataState();
    
    void updateGUI(int _explorationMode);
    
    void updatePlot(int _explorationMode);
    
    void updateViewport( int _explorationMode);
            
    void computeExplorationData(int _explorationMode);
       
    void readNormalizationFile(const QString& _filename);

    void readFitErrorFile(const QString& _filename);

    bool filterMatches(int _templateID=-1, int _groupID=-1, int _partID = -1, double _errorThreshold = std::numeric_limits<double>::max(), int _labelID = -1);

    bool calculateMDS();

    bool calculatePCA();

    void groupMatches();

    void setPlotPoints();

    int getNearestPoint(int _level, double _x, double _y);

    NEAREST_POINT* getNearestPoint(double _x, double _y, int _numNeighbors);
            
    void deformNearestMatches(int _numNeighbors);
    
    bool deformNearestPart(Match::Part& tmcPart, Match& nearestMatch);
            
    void rankNeighborPartsUnary(unsigned int _partID);
            
    double partUnaryScore(const Match::Part& tmPart, const Match::Part& nmcPart);
    
    void showPartDeformationOption(unsigned int _partID, int _symmetricPartID,bool isForward);
    
    void updateSliders();

    void updateClusterView();
    
    void updateExplorationView();
            
    QToolBar* createMenu();
    
    void initChairConstraints();
    
    void initBikeConstraints();
    
    void initHelicopterConstraints();
    
    void initPlaneConstraints();
    
    void initPlaneSidConstraints();
            
    ////////////////////////////////////////////////////////////////
    
    // Data
            
    std::vector<Match*>    matches_;

    std::vector<Match*>    filteredMatches_;
    
    std::vector< std::vector<Match*> > filteredMatchesHistory_;
            
    Match                  templateMatch_;
    
    Match*                 deformedNearestMatches_; 
    
    Match                  deformedNearestOption_;
    
    std::vector< std::vector<double> >    pcaBasis_;
    
    std::vector<double>    pcaOrigin_;
    
    OpenMesh::Vec2d pcaMin_;
    OpenMesh::Vec2d pcaMax_;
    
    std::vector<double> avgMatchScale_;
    
    //unsigned int numMatches_ = 0;
    //int numParameters_ = 0;
    
    int selectedTemplateID_ = -1;
    int selectedPartID_ = -1;
    int selectedGroupID_ = -1;
    double selectedFitErrorThreshold_ = std::numeric_limits<double>::max();
    
    int selectedLabelID_ = -1;
            
    int selectedMatchIndex_ = -1;
    
    unsigned int pickedPartID_ = -1;
            
    int currentNumClusters_ = -1;
            
    // 5 representative indices, for left, right, top, bottom, center of the pca 2D cluster
    // OR one for each of the cluster centers
    std::vector<int> representativeIndex_; //[5] = {-1, -1, -1, -1, -1};
    
    std::vector<int> clusterPopulation_;
            
    std::vector<int> lerpIndices_;
    
    std::pair<OpenMesh::Vec2f,OpenMesh::Vec2f> clusterBox_;
            
    EXPLORATION_MODE explorationMode_ = SHOW_TEMPLATE;
    
    CALCULATION_MODE calculationMode_ = CALCULATION_MODE_BOUNDING_BOX;
    
    int displayMode_ = BOX;
            
    NEAREST_POINT* nearestPoints_;
    
    int nearestNeighbourLevel_ = 0;
    
    int nofNN_ =  NUM_OF_NEAREST_NEIGHBOURS;
    
    std::unordered_map< int , std::vector< std::pair< int, double> > > nnIndexPartScoreSorted_;
    
    std::unordered_map< int , int > nnPartScoreVectorIndex_;
    
    std::unordered_map< int, int > nnChosen_;
            
    // This is the selected point inside the plot. This has nothing to do with the selectedMatchIndex_, a selected point might be inside the empty space of the plot
    // We just use store it to avoid recomputing nearest neighbors or deforming them when the same point is clicked on
    OpenMesh::Vec2f selectedPoint_;
    
    std::unordered_map<int,int> plotIndexToMatchIndex_;
    
    std::unordered_map<std::string, int> matchNameToMatchIndex_;
      
    ExplorationDataState dataState_;
        int iview = -1;
            
    TemplateExplorationViewItem* templateMarker_ = 0;
            
    std::vector<int> randomNeighbourIndices_;
            
    int randomNeighbourVectorIndex_ = -1;
    
    bool preserveConstraints_ = true;
            
    //some stats for the modeling session
    int noUp_ = 0;
    int noDown_ = 0;
    
    int nIndependentParts_ = 0;
    int nnUsed_ = 0;
            
    int nSymmetryConstraints_ = 0;
            
    std::unordered_map< int , int > partClicks_;
            
    int forcedNeighborIndex_ = -1;
            
};


#endif
