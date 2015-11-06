//
// TemplateExplorationWidget.cpp
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>


#include "TemplateExplorationWidget.h"


using namespace nanoflann;

// This is the "dataset to kd-tree" adaptor class:
// It takes any dataset of
template <typename Derived>
struct VectorMeshPointAdaptor
{
	typedef typename OpenMesh::Vec3f::value_type coord_t;
    
	const Derived &obj; //!< A const ref to the data set origin
    
	/// The constructor that sets the data set source
	VectorMeshPointAdaptor(const Derived &obj_) : obj(obj_) { }
    
	/// CRTP helper method
	inline const Derived& derived() const { return obj; }
    
	// Must return the number of data points
	inline size_t kdtree_get_point_count() const { return derived().size(); }
    
	// Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
	inline coord_t kdtree_distance(const coord_t *p1, const size_t idx_p2,size_t size) const
	{
		const typename OpenMesh::Vec3f& pnt = derived().at(idx_p2).pos_;
        
		const coord_t d0=p1[0]-pnt[0];
		const coord_t d1=p1[1]-pnt[1];
		const coord_t d2=p1[2]-pnt[2];
		return d0*d0+d1*d1+d2*d2;
	}
    
	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate value, the
	//  "if/else's" are actually solved at compile time.
	inline coord_t kdtree_get_pt(const size_t idx, int dim) const
	{
		const typename OpenMesh::Vec3f& pnt = derived().at(idx).pos_;
        
		if (dim==0) return pnt[0];
		else if (dim==1) return pnt[1];
		else return pnt[2];
	}
    
	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
	//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX &bb) const { return false; }
    
}; // end of VectorMeshPointAdaptor


template <typename Derived>
struct VectorMatchDescriptor2DAdaptor
{
	typedef typename OpenMesh::Vec2f::value_type coord_t;
    
	const Derived &obj; //!< A const ref to the data set origin
    
	/// The constructor that sets the data set source
	VectorMatchDescriptor2DAdaptor(const Derived &obj_) : obj(obj_) { }
    
	/// CRTP helper method
	inline const Derived& derived() const { return obj; }
    
	// Must return the number of data points
	inline size_t kdtree_get_point_count() const { return derived().size(); }
    
	// Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
	inline coord_t kdtree_distance(const coord_t *p1, const size_t idx_p2,size_t size) const
	{
		const typename OpenMesh::Vec2f& pnt = derived().at(idx_p2)->descriptor2D();
        
		const coord_t d0=p1[0]-pnt[0];
		const coord_t d1=p1[1]-pnt[1];
		return d0*d0+d1*d1;
	}
    
	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate value, the
	//  "if/else's" are actually solved at compile time.
	inline coord_t kdtree_get_pt(const size_t idx, int dim) const
	{
		const typename OpenMesh::Vec2f& pnt = derived().at(idx)->descriptor2D();
        
		if (dim==0) return pnt[0];
		else return pnt[1];
	}
    
	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
	//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX &bb) const { return false; }
    
}; // end of VectorMatchDescriptor2DAdaptor


TemplateExplorationWidget::TemplateExplorationWidget(QWidget* pParent)
{
	//setFixedWidth(180);
	QSplitter* verticalSplit = new QSplitter(Qt::Vertical);
    
	QVBoxLayout* boxLayout = new QVBoxLayout;
    
    QWidget* boxWidget = new QWidget;
    
    boxWidget->setLayout(boxLayout);
    
    QToolBar* menu = createMenu();
    
    if (DEBUG_MODE != FULL_DEBUG)
    {
        menu->setVisible(false);
    }
    
    verticalSplit->addWidget(menu);
    
    //boxLayout->addWidget(menu);
    
    templateListWidget_ = new QListWidget;
    templateListWidget_->setViewMode(QListWidget::IconMode);
    //templateListWidget_->setIconSize(QSize(0,0));
    //templateListWidget_->setUniformItemSizes(true);
    templateListWidget_->setResizeMode(QListWidget::Adjust);
    templateListWidget_->setMovement(QListWidget::Static);
    templateListWidget_->setSpacing(CLUSTER_VIEW_ICON_SPACING);
    
    //verticalSplit->addWidget(templateListWidget_);
    
    boxLayout->addWidget(templateListWidget_);
    
    verticalSplit->addWidget(boxWidget);
    
    
    QTabWidget* tabs = new QTabWidget;
    
    if (DEBUG_MODE != FULL_DEBUG)
    {
        tabs->setVisible(false);
    }

    boxLayout->addWidget(tabs);
    
    QWidget* tabFiltering = new QWidget;
    
    QVBoxLayout* tabFilteringLayout = new QVBoxLayout;
    tabFiltering->setLayout(tabFilteringLayout);
    
    // Setup the filtering tab widgets and add them to its layout
	QLabel* labelTemplateID = new QLabel("Template ID");
	comboBoxTemplateID_ = new QComboBox;
  
	for (int i = -1; i < 11; ++i)
	{
        if (i==0)
        {
            continue;
        }
		comboBoxTemplateID_->addItem(QString::number(i));
	}
    
	QLabel* labelPartID = new QLabel("Part ID");
	comboBoxPartID_ = new QComboBox;
  
	for (int i = -1; i < 13; ++i)
	{
		comboBoxPartID_->addItem(QString::number(i));
	}
  
	QLabel* labelGroupID = new QLabel("Group ID");
	comboBoxGroupID_ = new QComboBox;
  
	for (int i = -1; i < 42; ++i)
	{
		comboBoxGroupID_->addItem(QString::number(i));
	}
    
    QLabel* labelErrorThreshold = new QLabel("Error Threshold");
    QLabel* labelMatch = new QLabel("Selected Match");
    QLabel* labelMatchName = new QLabel("Name");
    labelMatchName_ = new QLabel("--");
    QLabel* labelMatchTemplateID = new QLabel("Template ID");
    labelMatchTemplateID_ = new QLabel("--");
    QLabel* labelMatchGroupID = new QLabel("Group ID");
    labelMatchGroupID_ = new QLabel("--");
    QLabel* labelMatchFitError = new QLabel("Fit Error");
    labelMatchFitError_ = new QLabel("--");
    
	lineEditErrorThreshold_ = new QLineEdit;
    lineEditErrorThreshold_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    lineEditErrorThreshold_->setText(QString("oo"));

	QFrame* line00 = new QFrame;
	line00->setFrameShape(QFrame::HLine);
	line00->setFrameShadow(QFrame::Sunken);
	tabFilteringLayout->addWidget(line00);
	tabFilteringLayout->addWidget(labelTemplateID);
	tabFilteringLayout->addWidget(comboBoxTemplateID_);
 
	QFrame* line01 = new QFrame;
	line01->setFrameShape(QFrame::HLine);
	line01->setFrameShadow(QFrame::Sunken);
	tabFilteringLayout->addWidget(line01);
	tabFilteringLayout->addWidget(labelPartID);
	tabFilteringLayout->addWidget(comboBoxPartID_);

	QFrame* line02 = new QFrame;
	line02->setFrameShape(QFrame::HLine);
	line02->setFrameShadow(QFrame::Sunken);
	tabFilteringLayout->addWidget(line02);
	tabFilteringLayout->addWidget(labelGroupID);
	tabFilteringLayout->addWidget(comboBoxGroupID_);

    QFrame* line03 = new QFrame;
	line03->setFrameShape(QFrame::HLine);
	line03->setFrameShadow(QFrame::Sunken);
	tabFilteringLayout->addWidget(line03);
	tabFilteringLayout->addWidget(labelErrorThreshold);
	tabFilteringLayout->addWidget(lineEditErrorThreshold_);
    
    QFrame* line04 = new QFrame;
    line04->setFrameShape(QFrame::HLine);
    line04->setFrameShadow(QFrame::Sunken);
    tabFilteringLayout->addWidget(line04);
    
    // Setup the selected match tab widget
    QWidget* tabSelectedMatch = new QWidget;
    QVBoxLayout* tabSelectedMatchLayout = new QVBoxLayout;
    tabSelectedMatch->setLayout(tabSelectedMatchLayout);
    
    QFrame* line05 = new QFrame;
	line05->setFrameShape(QFrame::StyledPanel);
	line05->setFrameShadow(QFrame::Sunken);
	
    QVBoxLayout* matchBoxLayout = new QVBoxLayout;
    line05->setLayout(matchBoxLayout);
    
    matchBoxLayout->addWidget(labelMatch);
    
    QFrame* line06 = new QFrame;
    line06->setFrameShape(QFrame::HLine);
    line06->setFrameShadow(QFrame::Sunken);
    matchBoxLayout->addWidget(line06);
    
	matchBoxLayout->addWidget(labelMatchName);
	matchBoxLayout->addWidget(labelMatchName_);
    
    matchBoxLayout->addWidget(labelMatchTemplateID);
    matchBoxLayout->addWidget(labelMatchTemplateID_);
    
    matchBoxLayout->addWidget(labelMatchGroupID);
    matchBoxLayout->addWidget(labelMatchGroupID_);
    
    matchBoxLayout->addWidget(labelMatchFitError);
    matchBoxLayout->addWidget(labelMatchFitError_);
    
    tabSelectedMatchLayout->addWidget(line05);
    
	QFrame* line09 = new QFrame;
    line09->setFrameShape(QFrame::HLine);
    line09->setFrameShadow(QFrame::Sunken);
    tabSelectedMatchLayout->addWidget(line09);
    
    // Setup the exploration tab widget

    QWidget* tabExploration = new QWidget;
    QVBoxLayout* tabExplorationLayout = new QVBoxLayout;
    tabExploration->setLayout(tabExplorationLayout);
    
    QLabel* labelNearestNeighbours = new QLabel("NN Level");
    
    tabExplorationLayout->addWidget(labelNearestNeighbours);
    
	comboBoxNearestNeighbours_ = new QComboBox;
    
	for (int i = 0; i < nofNN_; ++i)
	{
		comboBoxNearestNeighbours_->addItem(QString::number(i));
	}
    
    if (DEBUG_MODE == LIGHT_DEBUG)
    {
        boxLayout->addWidget(comboBoxNearestNeighbours_);
        comboBoxNearestNeighbours_->setVisible(false);
    }
	else
    {
        tabExplorationLayout->addWidget(comboBoxNearestNeighbours_);
    }

    chkNN_ = new QCheckBox("Select only this neighbor");
    tabExplorationLayout->addWidget(chkNN_);
    
    QFrame* line07 = new QFrame;
    line07->setFrameShape(QFrame::HLine);
    line07->setFrameShadow(QFrame::Sunken);
    tabExplorationLayout->addWidget(line07);

    QLabel* labelExplorationMode = new QLabel("Exploration Mode");
    tabExplorationLayout->addWidget(labelExplorationMode);
    
	comboBoxExplorationMode_ = new QComboBox;
    comboBoxExplorationMode_->addItem(QString("Template"));
	comboBoxExplorationMode_->addItem(QString("Selected Match"));
    comboBoxExplorationMode_->addItem(QString("Nearest Match"));
    comboBoxExplorationMode_->addItem(QString("Deformed Match"));
    comboBoxExplorationMode_->addItem(QString("Deformed Matches Superimposed"));
    comboBoxExplorationMode_->addItem(QString("Interpolate selected"));
    comboBoxExplorationMode_->addItem(QString("Select cluster"));
    comboBoxExplorationMode_->addItem(QString("Show representative"));
    comboBoxExplorationMode_->addItem(QString("Show deformation options"));
    comboBoxExplorationMode_->addItem(QString("Show groups"));
    comboBoxExplorationMode_->addItem(QString("Show clusters"));
    comboBoxExplorationMode_->addItem(QString("Show cluster"));
    comboBoxExplorationMode_->addItem(QString("Show deformation part options"));
    comboBoxExplorationMode_->addItem(QString("Show matches superimposed (no deformation)"));
    comboBoxExplorationMode_->addItem(QString("Show part options (no deformation)"));
    comboBoxExplorationMode_->addItem(QString("Show random match"));
    
    if (DEBUG_MODE == LIGHT_DEBUG)
    {
        boxLayout->addWidget(comboBoxExplorationMode_);
        comboBoxExplorationMode_->setVisible(false);
    }
	else
    {
       tabExplorationLayout->addWidget(comboBoxExplorationMode_); 
    }

    QCheckBox* chkNorm2Unit = new QCheckBox("Normalise to unit cube");
    chkNorm2Unit->setChecked(true);
    QCheckBox* chkNorm2TBox = new QCheckBox("Normalise to template box");
    QCheckBox* chkNorm2Bbox2Tbox = new QCheckBox("Normalise to bbox, then to template box");
    
    tabExplorationLayout->addWidget(chkNorm2Unit);
    tabExplorationLayout->addWidget(chkNorm2TBox);
    tabExplorationLayout->addWidget(chkNorm2Bbox2Tbox);
    
    lnDescriptorSize_ = new QLineEdit(" ");
    tabExplorationLayout->addWidget( lnDescriptorSize_);
    
    btnGroup_ = new QButtonGroup(this);

    btnGroup_->addButton(chkNorm2Unit, TO_UNIT_CUBE);
    btnGroup_->addButton(chkNorm2TBox, TO_TEMPLATE_BOX);
    btnGroup_->addButton(chkNorm2Bbox2Tbox, TO_BBOX_THEN_TEMPLATE_BOX);
    
    labelLambda1_ = new QLabel("Lambda1: --");
    
    sliderLambda1_ = new QSlider(Qt::Horizontal);
    sliderLambda1_->setMinimum(0);
    sliderLambda1_->setMaximum(100);
    sliderLambda1_->setSingleStep(1);
    sliderLambda1_->setValue(0);

    labelLambda2_ = new QLabel("Lambda2: --");
   
    sliderLambda2_ = new QSlider(Qt::Horizontal);
    sliderLambda2_->setMinimum(0);
    sliderLambda2_->setMaximum(100);
    sliderLambda2_->setSingleStep(1);
    sliderLambda2_->setValue(0);
    
    if (DEBUG_MODE == LIGHT_DEBUG)
    {
        boxLayout->addWidget(labelLambda1_);
        labelLambda1_->setVisible(false);
        
        boxLayout->addWidget(sliderLambda1_);
        sliderLambda1_->setVisible(false);
        
        boxLayout->addWidget(labelLambda2_);
        labelLambda2_->setVisible(false);
        
        boxLayout->addWidget(sliderLambda2_);
        sliderLambda2_->setVisible(false);
    }
    else
    {
        tabExplorationLayout->addWidget(labelLambda1_);
        tabExplorationLayout->addWidget(sliderLambda1_);
        tabExplorationLayout->addWidget(labelLambda2_);
        tabExplorationLayout->addWidget(sliderLambda2_);
    }
        
    QFrame* line08 = new QFrame;
    line08->setFrameShape(QFrame::HLine);
    line08->setFrameShadow(QFrame::Sunken);
    tabExplorationLayout->addWidget(line08);

    QLabel* labelCalculationMode = new QLabel("Calculation Mode");
    tabExplorationLayout->addWidget(labelCalculationMode);
    
	comboBoxCalculationMode_ = new QComboBox;
	comboBoxCalculationMode_->addItem(QString("Bounding Box"));
	comboBoxCalculationMode_->addItem(QString("Position"));
	tabExplorationLayout->addWidget(comboBoxCalculationMode_);
    
    exploScenarioWidget_ = new QWidget;
    
    QHBoxLayout *exploScenarioLayout = new QHBoxLayout;
    
    exploScenarioWidget_->setLayout(exploScenarioLayout);
    
    QLabel* labelDisplayMode = new QLabel("Display Mode");
    
    comboBoxDisplayMode_ = new QComboBox;
    comboBoxDisplayMode_->addItem("Boxes");
    comboBoxDisplayMode_->addItem("Points");
    comboBoxDisplayMode_->addItem("Mesh");
    comboBoxDisplayMode_->addItem("Boxes+Points");
    comboBoxDisplayMode_->addItem("Boxes+Mesh");
    comboBoxDisplayMode_->addItem("Points+Mesh");
    comboBoxDisplayMode_->addItem("Boxes+Points+Mesh");
    
    if (DEBUG_MODE == FULL_DEBUG)
    {
        tabExplorationLayout->addWidget(labelDisplayMode);
        tabExplorationLayout->addWidget(comboBoxDisplayMode_);
    }
    else
    {
        exploScenarioLayout->addWidget(labelDisplayMode);
        exploScenarioLayout->addWidget(comboBoxDisplayMode_);
    }
    
    chkBoxUseANN_ = new QCheckBox("Use ANN");
    
    chkBoxUseANN_->setChecked(false);
    
    tabExplorationLayout->addWidget(chkBoxUseANN_);
    
	// Add vertical spacer to take up the remaining space
	//QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
	//boxLayout->addItem(spacer);

    tabs->addTab(tabFiltering, "Filtering");
    tabs->addTab(tabSelectedMatch, "Selected Match");
    tabs->addTab(tabExploration, "Exploration");
    
    // FROM HERE ON the rest of the widgets are added to the boxlayout and set to not visible because they belong to one of the exploration modes and they will be made visible when we switch to that mode
    btnShowNextDefOption_ = new QPushButton("Next");
    
    btnShowNextDefOption_->setVisible(false);
    
    boxLayout->addWidget(btnShowNextDefOption_);
    
    labelLerp_ = new QLabel("Interpolation slider");
    boxLayout->addWidget(labelLerp_);
    labelLerp_->setVisible(false);
    sliderLerp_ = new QSlider(Qt::Horizontal);
    sliderLerp_->setMinimum(0);
    sliderLerp_->setMaximum(100);
    sliderLerp_->setSingleStep(1);
    sliderLerp_->setValue(0);
    boxLayout->addWidget(sliderLerp_);
    sliderLerp_->setEnabled(false);
    sliderLerp_->setVisible(false);
    sliderLerp_->setTickPosition(QSlider::TicksAbove);
    sliderLerp_->setTickInterval(30);
    //connect(sliderLambda1_,SIGNAL(valueChanged(int)),this, SLOT(slotChangeLambdaLabel(int)));
    btnResetLerp_ = new QPushButton;
    btnResetLerp_->setVisible(false);
    btnResetLerp_->setCheckable(true);
    //btnResetLerp_->setChecked(false);
    btnResetLerp_->setText("Start selecting shapes");
    boxLayout->addWidget(btnResetLerp_);
    
    btnSelectCluster_ = new QPushButton;
    btnSelectCluster_->setVisible(false);
    btnSelectCluster_->setText("Select cluster");
    boxLayout->addWidget(btnSelectCluster_);
       
    // FROM HERE ON ALL WIDGETS ARE RELATED TO THE EXPLORATION SCENARIO WE WANT OUR USERS TO PERFORM
    btnBack_ = new QPushButton("Back");
    btnBack_->setVisible(false);
    
    exploScenarioLayout->addWidget(btnBack_);

    //boxLayout->addWidget(btnBackWidget);  //btnBack_);
    
    nnLabel_ = new QLabel("NN number:");
    
    //nnLabel_->setVisible(false);
    
    //boxLayout->addWidget(nnLabel_);
    exploScenarioLayout->addWidget(nnLabel_);
    
    lnEdtNofNN_ = new QLineEdit;
    lnEdtNofNN_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    lnEdtNofNN_->setText(QString("%1").arg(nofNN_));
    
    //lnEdtNofNN_->setVisible(false);
    exploScenarioLayout->addWidget(lnEdtNofNN_);
    
    //boxLayout->addWidget(lnEdtNofNN_);
    
    btnShowNextDefPartOption_ = new QPushButton("Show next option");
    
    btnShowNextDefPartOption_->setVisible(false);
    
    boxLayout->addWidget(btnShowNextDefPartOption_);
    
    btnShowPreviousDefPartOption_ = new QPushButton("Show previous option");
    
    btnShowPreviousDefPartOption_->setVisible(false);
    
    boxLayout->addWidget(btnShowPreviousDefPartOption_);
    
    chkRecalculateBoxes_ = new QCheckBox("Recalculate boxes");
    
    chkRecalculateBoxes_->setVisible(false);
    
    boxLayout->addWidget(chkRecalculateBoxes_);

    lnPointX_ = new QLineEdit("0");
    
    lnPointX_->setVisible(false);
    
    lnPointY_ = new QLineEdit("0");
    
    lnPointY_->setVisible(false);
    
    btnChangeSelectedPoint_ = new QPushButton("Change point");
    
    btnChangeSelectedPoint_->setVisible(false);
    
    exploScenarioLayout->addWidget(lnPointX_);
    exploScenarioLayout->addWidget(lnPointY_);
    exploScenarioLayout->addWidget(btnChangeSelectedPoint_);
    
    chkConstraints_ = new QCheckBox("Constraints");
    chkConstraints_->setChecked(false);

    exploScenarioLayout->addWidget(chkConstraints_);
    
    chkColorOverride_ = new QCheckBox("Single Color");

    exploScenarioLayout->addWidget(chkColorOverride_);
    
    lnEdtForceNeighbor_ = new QLineEdit(" ");
    lnEdtForceNeighbor_->setVisible(false);
    exploScenarioLayout->addWidget(lnEdtForceNeighbor_);
    
    exploScenarioLayout->addStretch();
    
    //boxLayout->addWidget(lnPointX_);
    //boxLayout->addWidget(lnPointY_);
    //boxLayout->addWidget(btnChangeSelectedPoint_);
    
    exploScenarioWidget_->setVisible(false);
    
    boxLayout->addWidget(exploScenarioWidget_);
    
	//setLayout(boxLayout);
    QVBoxLayout* dummyLayout = new QVBoxLayout;
    dummyLayout->addWidget(verticalSplit);
    setLayout(dummyLayout);
    
    // Connect the signals of the gui elements to the slots of this widget
    QObject::connect(comboBoxTemplateID_, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotSetTemplateID(QString)));
    QObject::connect(comboBoxPartID_, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotSetPartID(QString)));
    QObject::connect(comboBoxGroupID_, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotSetGroupID(QString)));
    QObject::connect(lineEditErrorThreshold_, SIGNAL(textChanged(QString)), this, SLOT(slotSetFitErrorThreshold(QString)));
    QObject::connect(sliderLambda1_, SIGNAL(valueChanged(int)), this, SLOT(slotChangeL1(int)));
    QObject::connect(sliderLambda2_, SIGNAL(valueChanged(int)), this, SLOT(slotChangeL2(int)));
    QObject::connect(sliderLerp_, SIGNAL(valueChanged(int)), this, SLOT(slotInterpolateSelected(int)));
    
    QObject::connect(comboBoxExplorationMode_, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChangeExplorationMode(int)));
    QObject::connect(comboBoxCalculationMode_, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChangeCalculationMode(int)));
    QObject::connect(comboBoxDisplayMode_, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChangeDisplayMode(int)));
    QObject::connect(comboBoxNearestNeighbours_, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChangeNearestNeighbourLevel(int)));

    QObject::connect(btnResetLerp_, SIGNAL(clicked(bool)), this, SLOT(slotResetLerp(bool)));

    QObject::connect(btnSelectCluster_, SIGNAL(clicked(bool)), this, SLOT(slotSelectCluster(bool)));

    QObject::connect(btnShowNextDefOption_, SIGNAL(clicked()), this, SLOT(slotShowNextDeformationOption()) );
        
    QObject::connect(this, SIGNAL(explorationModeChanged(int)), comboBoxExplorationMode_, SLOT(setCurrentIndex(int)));

    QObject::connect(chkNorm2Unit, SIGNAL(stateChanged(int)), this, SLOT(slotResetIView(int)));
    QObject::connect(chkNorm2TBox, SIGNAL(stateChanged(int)), this, SLOT(slotResetIView(int)));
    QObject::connect(chkNorm2Bbox2Tbox, SIGNAL(stateChanged(int)), this, SLOT(slotResetIView(int)));
    
    QObject::connect(templateListWidget_, SIGNAL(itemClicked (QListWidgetItem *)) , this, SLOT(slotChangeSelectedCluster(QListWidgetItem *)));
    
    lerpIndices_.clear();
    
    QObject::connect(btnBack_, SIGNAL(clicked()), this, SLOT(slotGoBack()));
    
    QObject::connect(lnEdtNofNN_, SIGNAL(textChanged(QString)), this, SLOT(slotSetNofNN(QString)));

    QObject::connect(btnShowNextDefPartOption_, SIGNAL(clicked()), this, SLOT(slotShowNextPartDeformationOption()) );
    
    QObject::connect(btnShowPreviousDefPartOption_, SIGNAL(clicked()), this, SLOT(slotShowPreviousPartDeformationOption()) );
    
    QObject::connect(chkRecalculateBoxes_, SIGNAL(stateChanged(int)), this, SLOT(slotChangeRecalculateBoxes(int)));
    
    QObject::connect(btnChangeSelectedPoint_, SIGNAL(clicked()), this, SLOT(slotManuallyChangeSelectedPoint()));
    
    QObject::connect(chkConstraints_, SIGNAL(toggled(bool)), this, SLOT(slotToggleConstraints()));
    
    QObject::connect(chkColorOverride_, SIGNAL(toggled(bool)), this, SLOT(slotToggleSingleColor()));
    
    QObject::connect(lnEdtForceNeighbor_, SIGNAL(textChanged(const QString&)), this, SLOT(slotSetForceNeighborIndex(const QString&)));
    
    // Set initially selected point to -infinity
    selectedPoint_[0] = -std::numeric_limits<float>::max();
    selectedPoint_[1] = -std::numeric_limits<float>::max();

    dataState_.deformedNearestValid = new bool[nofNN_];
    
    for (int i=0; i<nofNN_; ++i)
    {
        dataState_.deformedNearestValid[i] = false;
    }
        
    clusterBox_.first[0] = -std::numeric_limits<float>::max();
    clusterBox_.first[1] = -std::numeric_limits<float>::max();
    
    clusterBox_.second[0] = -std::numeric_limits<float>::max();
    clusterBox_.second[1] = -std::numeric_limits<float>::max();
    
    nearestPoints_ = new NEAREST_POINT[nofNN_];
   
    scene_ = new QGraphicsScene;
 
    QObject::connect(scene_, SIGNAL(selectionChanged()), this, SLOT(slotChangeSelectedMatches()));
    
    deformedNearestMatches_ = new Match[nofNN_];
    
    switch (LOADED_DATASET)
    {
        case CHAIRS:
        {
            initChairConstraints();
        }
            break;
        case BIKES:
        {
            initBikeConstraints();
        }
            break;
        case HELICOPTERS:
        {
            initHelicopterConstraints();
        }
            break;
        case PLANES:
        {
            initPlaneConstraints();
        }
            break;
        case SID_PLANES:
        {
            initPlaneSidConstraints();
        }
            break;
        default:
        {
            
        }
            break;
    }

    preserveConstraints_ = true;
    
    slotSetFitErrorThreshold(QString("%1").arg(FIT_ERROR));
    selectedFitErrorThreshold_ = FIT_ERROR;
}

TemplateExplorationWidget::~TemplateExplorationWidget()
{
    if (nearestPoints_)
    {
        delete [] nearestPoints_;
    }
    
    if (deformedNearestMatches_)
    {
        delete [] deformedNearestMatches_;
    }
}

QToolBar* TemplateExplorationWidget::createMenu()
{
    QAction* calculateMDSAct = new QAction(this->tr("Calculate M&DS..."), this);
    calculateMDSAct->setShortcut(this->tr("Ctrl+D"));
    calculateMDSAct->setStatusTip(this->tr("Calculate MDS"));
    QObject::connect(calculateMDSAct, SIGNAL(triggered()), this, SLOT(slotCalculateMDS()));
    
    QAction* calculatePCAAct = new QAction(this->tr("Calculate P&CA..."),this);
    calculatePCAAct->setShortcut(this->tr("Ctrl+C"));
    calculatePCAAct->setStatusTip(this->tr("Calculate MDS"));
    QObject::connect(calculatePCAAct, SIGNAL(triggered()), this, SLOT(slotCalculatePCA()));
    
    QAction* groupMatchesAct = new QAction(this->tr("&Group Matches..."), this);
    groupMatchesAct->setShortcut(this->tr("Ctrl+G"));
    groupMatchesAct->setStatusTip(this->tr("Group Matches"));
    QObject::connect(groupMatchesAct, SIGNAL(triggered()), this, SLOT(slotGroupMatches()));
    
    QAction* readFitErrorAct = new QAction(this->tr("Read fit e&rror file..."), this);
    readFitErrorAct->setShortcut(this->tr("Ctrl+R"));
    readFitErrorAct->setStatusTip(this->tr("Read fit error file"));
    QObject::connect(readFitErrorAct, SIGNAL(triggered()),this, SLOT(slotReadFitErrorFile()));
    
    QAction* readNormalizationAct = new QAction(this->tr("Read n&ormalization file..."), this);
    readNormalizationAct->setShortcut(this->tr("Ctrl+N"));
    readNormalizationAct->setStatusTip(this->tr("Read normalization file"));
    QObject::connect(readNormalizationAct, SIGNAL(triggered()),this, SLOT(slotReadNormalizationFile()));
    
    QAction* readLabelsAct = new QAction(this->tr("Read &labels..."), this);
    readLabelsAct->setShortcut(this->tr("Ctrl+L"));
    readLabelsAct->setStatusTip(this->tr("Read labels"));
    QObject::connect(readLabelsAct, SIGNAL(triggered()),this, SLOT(slotReadLabels()));
    
    QAction* splitAct = new QAction(this->tr("Split &matches..."), this);
    splitAct->setShortcut(this->tr("Ctrl+S"));
    splitAct->setStatusTip(this->tr("Split matches"));
    QObject::connect(splitAct, SIGNAL(triggered()),this, SLOT(slotSplitMatches()));
    
    QAction* recalcAct = new QAction(this->tr("Recalculate &boxes..."), this);
    recalcAct->setShortcut(this->tr("Ctrl+R"));
    recalcAct->setStatusTip(this->tr("Recalculate boxes"));
    QObject::connect(recalcAct, SIGNAL(triggered()),this, SLOT(slotRecalculatePartBoxes()));
    
    QAction* saveDescAct = new QAction(this->tr("Save &descriptors..."), this);
    saveDescAct->setShortcut(this->tr("Ctrl+D"));
    saveDescAct->setStatusTip(this->tr("Save descriptors"));
    QObject::connect(saveDescAct, SIGNAL(triggered()),this, SLOT(slotSaveMatchDescriptorsToFile()));
    
    QAction* saveNamesAct = new QAction(this->tr("Save &names..."), this);
    saveNamesAct->setShortcut(this->tr("Ctrl+N"));
    saveNamesAct->setStatusTip(this->tr("Save names"));
    QObject::connect(saveNamesAct, SIGNAL(triggered()),this, SLOT(slotSaveMatchNamesToFile()));
  
    QAction* joinMatchGroupAct = new QAction(this->tr("Join &matches in single group..."), this);
    joinMatchGroupAct->setShortcut(this->tr("Ctrl+J"));
    joinMatchGroupAct->setStatusTip(this->tr("Join matches in single group"));
    QObject::connect(joinMatchGroupAct, SIGNAL(triggered()),this, SLOT(slotJoinMatchesInSingleGroup()));
    
    QAction* saveSelectedAct = new QAction(this->tr("Save selected matches"), this);
    saveSelectedAct->setStatusTip(this->tr("Save selected matches"));
    QObject::connect(saveSelectedAct, SIGNAL(triggered()),this, SLOT(slotSaveSelectedMatches()));
    
    
    QToolBar* toolBar = new QToolBar();

    toolBar->addAction(calculateMDSAct);
    toolBar->addAction(calculatePCAAct);
    toolBar->addAction(groupMatchesAct);
    toolBar->addAction(readFitErrorAct);
    toolBar->addAction(readNormalizationAct);
    toolBar->addAction(readLabelsAct);
    toolBar->addAction(splitAct);
    toolBar->addAction(recalcAct);
    toolBar->addAction(saveDescAct);
    toolBar->addAction(saveNamesAct);
    toolBar->addAction(joinMatchGroupAct);
    toolBar->addAction(saveSelectedAct);
    
    return toolBar;
    
}


void TemplateExplorationWidget::slotSetTemplateID(const QString& _id)
{
    selectedTemplateID_ = _id.toInt();
    qDebug() << "Template ID set to: " << selectedTemplateID_ ;
}

void TemplateExplorationWidget::slotSetPartID(const QString& _id)
{
    selectedPartID_ = _id.toInt();
    
    std::vector<Match*>::iterator matchesIt(matches_.begin()), matchesEnd(matches_.end());
    
    for (; matchesIt!=matchesEnd; ++matchesIt)
    {
        (**matchesIt).setSelectedPartID(selectedPartID_);
    }
    qDebug() << "Part ID set to: " << selectedPartID_ ;
    
    if (CREATE_MVW)
    {
        emit requestRedraw(0); // request redraw for mvw 0
    }
}

void TemplateExplorationWidget::slotSetGroupID(const QString& _id)
{
    selectedGroupID_ = _id.toInt();
    qDebug() << "Group ID set to: " << selectedGroupID_ ;
}

void TemplateExplorationWidget::slotSetFitErrorThreshold(const QString& _error)
{
    if(_error == "oo")
        selectedFitErrorThreshold_ = std::numeric_limits<double>::max();
    else
        selectedFitErrorThreshold_ = _error.toDouble();
    
    qDebug() << "Fit error threshold set to: " << selectedFitErrorThreshold_ ;
}


void TemplateExplorationWidget::slotSetLabelID(const QString& _id)
{
    selectedLabelID_ = _id.toInt();
}

void TemplateExplorationWidget::slotChangeL1(int lambda)
{
    // Map integer lambda inside range [0,100] to the range [pcaMin,pcaMax]
    // Get the alpha, i.e. the percentage of the distance from lambda to minSliderValue (i.e. 0) with respect to maxSlideValue - minSliderValue (i.e. 100 - 0)
    double alpha = lambda / 100.0;
    
    // Plot point value will be pcaMin[0] + alpha * (pcaMax[0] - pcaMin[0])
    double point_x = pcaMin_[0] + alpha * (pcaMax_[0] - pcaMin_[0]);
    
    slotChangeSelectedPoint(point_x, selectedPoint_[1]);
    
}

void TemplateExplorationWidget::slotChangeL2(int lambda)
{
    // Map integer lambda inside range [0,100] to the range [pcaMin,pcaMax]
    // Get the alpha, i.e. the percentage of the distance from lambda to minSliderValue (i.e. 0) with respect to maxSlideValue - minSliderValue (i.e. 100 - 0)
    double alpha = lambda / 100.0;
    
    // Plot point value will be pcaMin[1] + alpha * (pcaMax[1] - pcaMin[1])
    double point_y = pcaMin_[1] + alpha * (pcaMax_[1] - pcaMin_[1]);
    
    slotChangeSelectedPoint(selectedPoint_[0], point_y);
}

void TemplateExplorationWidget::slotSaveMatchPointsToFile()
{
    // Ask for a directory
    QString directory_path = QFileDialog::getExistingDirectory(0,"Choose a directory to save the match points", "../");
    
    if(directory_path.isNull())
    {
        qCritical() << "The directory path was null"  ;
        return;
    }
    
    std::vector<Match*>::iterator itMatch(matches_.begin()), itMatchEnd(matches_.end());
    
    for (; itMatch != itMatchEnd; ++itMatch)
    {
        QString fname = (**itMatch).filename().split("/").last().split(".").first();
       (**itMatch).saveMatchPointsToFile(directory_path + "/" + fname + ".txt");
    }
    
}


void TemplateExplorationWidget::slotSaveMatchDescriptorsToFile()
{
    // Ask for a filename
    QString filename = QFileDialog::getSaveFileName(0,"Choose a filename", "../");
    
    QFile file(filename);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical() << "Could not open file " << filename;
        return;
    }
    
    QTextStream out(&file);

    std::vector< std::vector<double> >::iterator pcaBasisIt(pcaBasis_.begin()), pcaBasisEnd(pcaBasis_.end());
    for (; pcaBasisIt!= pcaBasisEnd; ++pcaBasisIt)
    {
        std::vector<double>& cBasis = *pcaBasisIt;
        std::vector<double>::iterator cBasisIt(cBasis.begin()), cBasisEnd(cBasis.end());
        
        for (; cBasisIt!=cBasisEnd; ++cBasisIt)
        {
            out << *cBasisIt << " ";
        }
        out << "\n";
    }
    
    std::vector<double>::iterator pcaOriginIt(pcaOrigin_.begin()), pcaOriginEnd(pcaOrigin_.end());
    
    for (; pcaOriginIt!= pcaOriginEnd; ++pcaOriginIt)
    {
        out << *pcaOriginIt << " ";
    }
    
    out << "\n";
    
    std::vector<Match*>::iterator itMatch(filteredMatches_.begin()), itMatchEnd(filteredMatches_.end());
    
    for (; itMatch != itMatchEnd; ++itMatch)
    {
       (**itMatch).saveMatchDescriptorToFile(out);
    }
    
}



void TemplateExplorationWidget::slotRecalculatePartBoxes()
{
    for (int i = 0; i < nofNN_; ++i)
	{
        int nearestMatchIndex = nearestPoints_[i].index_;
        
        if(nearestMatchIndex<0 || nearestMatchIndex >= filteredMatches_.size())
        {
            qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
            continue;
        }
        
        Match& nearestMatch = *filteredMatches_.at(nearestMatchIndex);
        
        nearestMatch.recalculateBoxes();
    }
    
    if (CREATE_MVW)
    {
        emit requestRedraw(0);
    }
}

void TemplateExplorationWidget::slotChangeNearestNeighbourLevel(int _level)
{
    nearestNeighbourLevel_ = _level;
    
    computeExplorationData(explorationMode_);
    
    // No need to update the gui here
    updateGUI(explorationMode_);
    
    updatePlot(explorationMode_);
    
    updateViewport(explorationMode_);
}

void TemplateExplorationWidget::slotAddMatches(const std::vector<Match*>& _matches)
{
    int mSizeBefore = matches_.size();
    int nmSize = _matches.size();
    matches_.insert( matches_.end(), _matches.begin(), _matches.end() );
    
    int mSizeAfter = matches_.size();
    
    if(mSizeAfter!= mSizeBefore + nmSize)
    {
        qWarning() << "Something is wrong, new match size: " << mSizeAfter << " is not the sum of match size before " << mSizeBefore << " plus the added matches size " << nmSize ;
        return;
    }
   
    // For the new matches that have been added, add an entry mapping their name to their index in the match_ vector
    for( int i = mSizeBefore; i < mSizeAfter ; i++)
    {
        QString matchName = matches_[i]->filename().split("/").last().split(".").first();
        matchNameToMatchIndex_[matchName.toStdString()] = i;
    }
    
    TIMELOG->append(QString("%1 : collection_loaded").arg((qlonglong)QDateTime::currentMSecsSinceEpoch()));
    slotChangeExplorationMode(SHOW_GROUPS);
    
    updateClusterView();
}

void TemplateExplorationWidget::slotGroupMatches()
{
    groupMatches();
}

void TemplateExplorationWidget::groupMatches()
{
    //First sort the matches according to their name so we always get the same group IDs
    std::sort(matches_.begin(),matches_.end(),CompareMatchesFunctor);

    typedef std::vector< std::pair<int, int> > PartIDsTypes;

    std::map< PartIDsTypes, int > groups;

    std::vector<Match*>::iterator itMatch(matches_.begin()), matchesEnd(matches_.end());

    int i = 0;
    
    for (; itMatch != matchesEnd; ++itMatch)
	{
        PartIDsTypes cGroup;

        const std::vector<Match::Part>& mParts = (**itMatch).parts();
        
        std::vector<Match::Part>::const_iterator itPart(mParts.begin()), partsEnd(mParts.end());
        
        for ( ; itPart != partsEnd; ++itPart)
        {
            cGroup.push_back(std::pair<int, int>(itPart->partID_, itPart->partType_) );
        }

        if( groups.count(cGroup) != 1 ) //findGroup == groupsEnd)
        {
            groups[cGroup] = i++;
        }

        (**itMatch).setGroupID( groups[cGroup] );

    }

    qDebug() << "There were " << i << " different groups discovered" ;

    std::map< PartIDsTypes, int >::iterator itGroups, groupsEnd(groups.end());

    for (itGroups= groups.begin(); itGroups!=groupsEnd; ++itGroups)
    {
        QString msg;
        
        msg += "Group ID: ";
        msg += QString("%1").arg(itGroups->second);
        msg += "   ";
        
        const PartIDsTypes& cGroup = itGroups->first;
        
        PartIDsTypes::const_iterator itPartIDsTypes(cGroup.begin()), cIDsTypesEnd(cGroup.end());

        for ( ; itPartIDsTypes!=cIDsTypesEnd; ++itPartIDsTypes)
        {
            msg += QString("%1").arg(itPartIDsTypes->first);
            msg += "->";
            msg += QString("%1").arg(itPartIDsTypes->second);
            msg += " ";
        }
       qDebug() << msg;
    }

}

void TemplateExplorationWidget::readNormalizationFile(const QString& _filename)
{

    QFile file(_filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Could not open file " << _filename;
        return;
    }

    QTextStream in(&file);

	int count = 0;

    while (!in.atEnd())
	{
        QString line = in.readLine();
        QStringList lineList = line.split(" ");

        int lineListSize = lineList.size();

        if(lineListSize!=5)
        {
            qCritical() << "Unexpected line size" ;
            return;
        }

        QString name = lineList.at(0);
        
        if(matchNameToMatchIndex_.count(name.toStdString())!=1)
        {
            qWarning() << "Ignoring match name: " << name << " because it was not found in the collection!" ;
            continue;
        }
        Match& cMatch = *matches_[ matchNameToMatchIndex_[name.toStdString()] ];

        cMatch.setMeshCentroid(OpenMesh::Vec3f(lineList.at(1).toDouble(), lineList.at(2).toDouble(), lineList.at(3).toDouble()));
       
        cMatch.setMeshAvgRadius( lineList.at(4).toDouble() );

        qDebug() << "Match name: " << name << " was mapped to match index: " << matchNameToMatchIndex_[name.toStdString()] ;

        count++;
    }

    qDebug() << "Normalization values loaded (# of entries: " << count << ")" ;

}


bool TemplateExplorationWidget::filterMatches(int _templateID, int _groupID, int _partID, double _errorThreshold, int _labelID)
{
    
    //initialise filteredMatches_ ( should work even if we go down the hierarchy and come back to SHOW_GROUPS mode, but it's an overkill since in that case filteredMatches_ would already be equal to matches from popping the filteredMatchesHistory )
    if (explorationMode_ == SHOW_GROUPS)
    {
        filteredMatches_ = matches_;
    }
    
    filteredMatchesHistory_.push_back(filteredMatches_);
    
    filteredMatches_.clear();
    
    //Three parameters, 8 cases, some of them we don't handle yet
    if (_templateID<0 && _groupID<0 && _partID<0)
    {
        qWarning() << "You are trying to use all the matches for your calculations, this is not possible at the moment! No matches will be selected" ;
        return false;
    }
    
    int numParts = -1;
    
    std::vector<Match*>& matches = filteredMatchesHistory_.back();
    
    std::vector<Match*>::const_iterator itMatch(matches.begin()), matchesEnd(matches.end());

    //Go through the matches to find the number of matches that fit the template/group/part/error filter
    for ( ; itMatch != matchesEnd; ++itMatch)
	{
        if( (_templateID >= 0 && (**itMatch).templateID() != _templateID) || (_groupID >= 0 && (**itMatch).groupID() != _groupID) || (**itMatch).fitError() > _errorThreshold || (_labelID >= 0 && (**itMatch).label() != _labelID))
        {
            continue;
        }

        if(_partID >= 0)
        {
            bool ignore = true;

            const std::vector<Match::Part>& mParts = (**itMatch).parts();
            
            std::vector<Match::Part>::const_iterator itPart(mParts.begin()), partsEnd(mParts.end());
            
            for ( ; itPart != partsEnd; ++itPart)
            {
                if (_partID == itPart->partID_ && itPart->partType_ != 0)
                {
                    ignore = false;
                    break;
                }
            }
            
            if(ignore)
            {
                continue;
            }
        }
        else
        {
            // First time we find a valid match with the template ID and group ID we are looking for, so store its number of parts so we can then assume all other matches with
            // the same template ID and group ID will have the same number of parts
            int mnParts = (**itMatch).nparts();
            if (numParts == -1)
            {
                numParts = mnParts;
            }

            if(mnParts != numParts)
            {
                qCritical() << QString("One of the matches with (template %1, group %2, part %3, error %4) has a different number of parts (%5) than expected (%6). Ignoring it in the filtering!").arg(_templateID).arg(_groupID).arg(_partID).arg(_errorThreshold).arg(mnParts).arg(numParts);
                continue;
            }
        }
        
        filteredMatches_.push_back(*itMatch);
        //numMatches_++;
    }
    
    if(filteredMatches_.size() == 0) //numMatches_ == 0)
    {
       qCritical() << QString("No matches found for template ID: %1, group ID: %2, part ID: %3 and error threshold: %4").arg(_templateID).arg(_groupID).arg(_partID).arg(_errorThreshold);
        return false;
    }

    return true;
}

void TemplateExplorationWidget::slotCalculateMDS()
{
    if( !filterMatches(selectedTemplateID_,selectedGroupID_,selectedPartID_,selectedFitErrorThreshold_,selectedLabelID_) )
    {
        return;
    }
    
	if ( calculateMDS() )
    {
        
        setPlotPoints();
        
        if (CREATE_QWTPLOTW)
        {
            emit plotPointsChanged();
        }
        
        slotChangeSelectedPoint(0.0, 0.0);
    }
}

void TemplateExplorationWidget::slotCalculatePCA()
{
    if( !filterMatches(selectedTemplateID_,selectedGroupID_,selectedPartID_,selectedFitErrorThreshold_,selectedLabelID_) )
    {
        return;
    }

    if (calculatePCA())
    {
        
        setPlotPoints();
        
        if (CREATE_QWTPLOTW)
        {
            emit plotPointsChanged();
        }
        
        slotChangeSelectedPoint(0.0, 0.0);
        
    }
    
}

bool TemplateExplorationWidget::calculateMDS()
{
    std::vector<Match*>::iterator itMatch(filteredMatches_.begin()), fMatchesEnd(filteredMatches_.end());

    int numMatches = filteredMatches_.size();
    
    if (numMatches==0)
    {
        qCritical() << "No matches to calculate MDS!";
        return false;
    }
    // Can safely assume there are matches to embed and their descriptors are all of the same dimensions
    int numParameters = filteredMatches_[0]->descriptor().size();
    
    // Create in and out variables vectors
    std::vector<Matlab::MatlabVariable> ins;
    ins.push_back(Matlab::MatlabVariable());
    
    ins[0].nRows_ = numMatches; //numMatches_;
    ins[0].nColumns_ = numParameters; //numParameters_;
    ins[0].name_ = "descriptors";
    ins[0].data_ = new double[ins[0].nRows_ * ins[0].nColumns_];
    
    std::vector<Matlab::MatlabVariable> outs;
    outs.push_back(Matlab::MatlabVariable());
    
    outs[0].nRows_ = 0;
    outs[0].nColumns_ = 0;
    outs[0].name_ = "projected_descriptors";
    outs[0].data_ = 0;
    
    unsigned int i = 0;
    unsigned int j = 0;

    //Fill the ins[0].data_ array with data
    for ( ; itMatch != fMatchesEnd; ++itMatch)
	{
        j=0;
        
        const std::vector<Match::Part>& mParts = (**itMatch).parts();
        
        std::vector<Match::Part>::const_iterator itPart(mParts.begin()), partsEnd(mParts.end());
        
        const std::vector<float>& cDesc = (**itMatch).descriptor();
        
        std::vector<float>::const_iterator descIt(cDesc.begin()), descEnd(cDesc.end());
        
        for (; descIt!=descEnd; ++descIt)
        {
            ins[0].data_[i + j*ins[0].nRows_] = *descIt;
            j++;
        }
        
        i++;
    }
    
    qDebug() << "Passing " << numMatches << " matches (rows) with " << numParameters << " parameters (columns) each to Matlab!" ;

    std::string code("diss = pdist(descriptors); [pr_desc,eigvalues] = cmdscale(diss); projected_descriptors = pr_desc(:,1:2);");
    
    Matlab::runCode(ins, outs, code);

    qDebug() << "Matlab part is done-returning to C++!" ;

    if(outs[0].nRows_ != numMatches || outs[0].nColumns_ != 2)
    {
        qCritical() << "Something is wrong! Projected descriptors and deformation basis number of rows and columns do not match the expected!" ;
        return false;
    }
        
    i = 0;

    itMatch = filteredMatches_.begin();
    
    //For all matches belonging to the template selected, copy the projected_descriptor values
    for ( ; itMatch != fMatchesEnd; ++itMatch)
	{
        OpenMesh::Vec2f prDes(0.0,0.0);
        
        prDes[0] = outs[0].data_[i];                   // First column
        prDes[1] = outs[0].data_[i + outs[0].nRows_];  // Second column
        
        (**itMatch).setDescriptor2D(prDes);
        
        i++;
    }

    qDebug() << "Done with MDS for template ID:" << selectedTemplateID_ << " group ID:" << selectedGroupID_ << " part ID: " << selectedPartID_ << " error threshold: " << selectedFitErrorThreshold_ ;

    return true;

}

bool TemplateExplorationWidget::calculatePCA()
{
    std::vector<Match*>::iterator itMatch(filteredMatches_.begin()), fMatchesEnd(filteredMatches_.end());
    
    int numMatches = filteredMatches_.size();
    
    if (numMatches==0)
    {
        qCritical() << "No matches to calculate embedding!";
        return false;
    }
    
    // Kind of a hack to reset the number of neighbours if the cluster gets too small, hope it doesn't crash
    if (numMatches < nofNN_)
    {
        nofNN_ = numMatches;
        lnEdtNofNN_->blockSignals(true);
        lnEdtNofNN_->setText(QString("%1").arg(nofNN_));
        lnEdtNofNN_->blockSignals(false);
    }
    
    // Can safely assume there are matches to embed and their descriptors are all of the same dimensions
    int numParameters = filteredMatches_[0]->descriptor().size();
    
    std::vector<Matlab::MatlabVariable> ins;
    ins.push_back(Matlab::MatlabVariable());
    
    ins[0].nRows_ = numMatches;
    ins[0].nColumns_ = numParameters;
    ins[0].name_ = "descriptors";
    ins[0].data_ = new double[ins[0].nRows_ * ins[0].nColumns_];
    
    std::vector<Matlab::MatlabVariable> outs;
    outs.push_back(Matlab::MatlabVariable());
    
    outs[0].nRows_ = 0;
    outs[0].nColumns_ = 0;
    outs[0].name_ = "projected_descriptors";
    outs[0].data_ = 0;
    
    outs.push_back(Matlab::MatlabVariable());
    
    outs[1].nRows_ = 0;
    outs[1].nColumns_ = 0;
    outs[1].name_ = "deformation_basis";
    outs[1].data_ = 0;
    
    outs.push_back(Matlab::MatlabVariable());
    
    outs[2].nRows_ = 0;
    outs[2].nColumns_ = 0;
    outs[2].name_ = "clustCent";
    outs[2].data_ = 0;

    outs.push_back(Matlab::MatlabVariable());
    
    outs[3].nRows_ = 0;
    outs[3].nColumns_ = 0;
    outs[3].name_ = "nofClusters";
    outs[3].data_ = 0;
    
    outs.push_back(Matlab::MatlabVariable());
    
    outs[4].nRows_ = 0;
    outs[4].nColumns_ = 0;
    outs[4].name_ = "point2cluster";
    outs[4].data_ = 0;
    
    
	// TODO: Temp: Compute distance in 3D space
    //std::vector< std::vector<double> > matchDescriptors;
    
    pcaOrigin_.clear();
    pcaBasis_.clear();

	avgMatchScale_.clear();

    templateMatch_.parts().clear();
    templateMatch_.setNparts(0);
    
    representativeIndex_.clear();
    clusterPopulation_.clear();
    
    selectedPoint_[0] = -std::numeric_limits<float>::max();
    selectedPoint_[1] = -std::numeric_limits<float>::max();
    
    for (int i=0; i<numParameters; i++)
    {
        pcaOrigin_.push_back(0.0f);
		avgMatchScale_.push_back(0.0f);
    }

    unsigned int i = 0;
    unsigned int j = 0;

    //Fill the ins[0].data_ array with data
    for ( ; itMatch != fMatchesEnd; ++itMatch)
	{
        j=0;
        
        const std::vector<Match::Part>& mParts = (**itMatch).parts();
        
        std::vector<Match::Part>::const_iterator itPart(mParts.begin()), partsEnd(mParts.end());
        
        for ( ; itPart != partsEnd; ++itPart)
        {
            // If the embedding is to be done based on a specific part ID, then we need to ignore the rest of the parts
            // The code below for filling the array will only run for one part per match
            if (selectedPartID_ >= 0 && selectedPartID_ != itPart->partID_)
            {
                continue;
            }
            
            // Hack just to initialise the part IDs of the parts of the templateMatch (copy the IDs of the first match)
            if(i==0)
            {
                Match::Part cPart;
                cPart.partID_ = itPart->partID_;
                cPart.partType_ = itPart->partType_;
                cPart.pos_ = OpenMesh::Vec3d(0,0,0);
                cPart.scale_ = OpenMesh::Vec3d(0,0,0);
                cPart.partShape_.setID(cPart.partID_);
                
                templateMatch_.parts().push_back(cPart);
                // Will fill out the pos and scale for every part later after we find the average values for the pos and scale
            }
            
            switch (calculationMode_)
			{
				case CALCULATION_MODE_BOUNDING_BOX:
					{
						OpenMesh::Vec3d min(0,0,0);
						OpenMesh::Vec3d max(0,0,0);

						min = itPart->pos_ - itPart->scale_;
						max = itPart->pos_ + itPart->scale_;

                        ins[0].data_[i + j*ins[0].nRows_] = min[0]; // 1st-7th-13th etc column
                        pcaOrigin_[j] += min[0];
						j++;
						ins[0].data_[i + j*ins[0].nRows_] = min[1]; // 2nd-8th-14th etc column
                        pcaOrigin_[j] += min[1];
						j++;
						ins[0].data_[i + j*ins[0].nRows_] = min[2];
                        pcaOrigin_[j] += min[2];
						j++;
						ins[0].data_[i + j*ins[0].nRows_] = max[0];
                        pcaOrigin_[j] += max[0];
						j++;
						ins[0].data_[i + j*ins[0].nRows_] = max[1];
                        pcaOrigin_[j] += max[1];
						j++;
						ins[0].data_[i + j*ins[0].nRows_] = max[2];
                        pcaOrigin_[j] += max[2];
						j++;
					}
					break;
				case CALCULATION_MODE_POSITION:
					{
						ins[0].data_[i + j*ins[0].nRows_] = itPart->pos_[0];
						pcaOrigin_[j] += itPart->pos_[0];
						avgMatchScale_[j] += itPart->scale_[0];
						j++;
                        
						ins[0].data_[i + j*ins[0].nRows_] = itPart->pos_[1];
						pcaOrigin_[j] += itPart->pos_[1];
						avgMatchScale_[j] += itPart->scale_[1];
						j++;
                        
						ins[0].data_[i + j*ins[0].nRows_] = itPart->pos_[2];
						pcaOrigin_[j] += itPart->pos_[2];
						avgMatchScale_[j] += itPart->scale_[2];
						j++;
					}
					break;
			}
        }
        i++;
    }

    qDebug() << "Passing " << numMatches << " matches (rows) with " << numParameters << " parameters (columns) each to Matlab!" ;

    std::string code("addpath('");
    
    code +=MATLAB_FILE_PATH;
    code += "');";
    
    if(EMBEDDING_MODE == PCA)
    {
        code += "descriptors(find(isinf(descriptors) == 1)) = 0; \
                 descriptors(find(isnan(descriptors) == 1)) = 0; \
                 [coefs,pr_desc] = princomp(descriptors); \
                 deformation_basis = coefs(:,1:2); \
                 projected_descriptors = pr_desc(:,1:2); \
                 fraction = 0.05; \
                 bandwidth = .25*compute_spread(projected_descriptors, fraction);  \
                 [clustCent,point2cluster,clustMembsCell] = MeanShiftCluster(projected_descriptors',bandwidth); \
                 point2cluster = point2cluster'; \
                 clustCent = clustCent'; \
                 nofClusters = size(clustCent,1); \
                 [clustCent, point2cluster] = sortClusters(clustCent,point2cluster,nofClusters); ";
    }

    Matlab::runCode(ins, outs, code);
   
    qDebug() << "Matlab part is done-returning to C++!" ;

    if(outs[0].nRows_!=numMatches || outs[0].nColumns_!=2 || outs[1].nRows_ != numParameters || outs[1].nColumns_ != 2)
    {
        qCritical() << "Something is wrong! Projected descriptors and deformation basis number of rows and columns do not match the expected!" ;
        return false;
    }
    //outs[3] holds the number of clusters so should be a 1x1 array
    if (outs[3].nColumns_!=1 || outs[3].nRows_!=1 || outs[2].nRows_ != outs[3].data_[0] || outs[2].nColumns_!=2 || outs[4].nRows_!= numMatches || outs[4].nColumns_!=1)
    {
        qCritical() << "Number of clusters is not a 1x1 array or number of cluster centroids do not match the number of clusters!" ;
        return false;
    }
    
    int numClusters = outs[3].data_[0];
    currentNumClusters_ = numClusters;
    
    if (numClusters<=0)
    {
        qCritical() << "Number of clusters is " << numClusters ;
        return false;
    }
    
    std::vector<OpenMesh::Vec2f> clusterCentroids(numClusters);
    std::vector<float> minDists(numClusters,std::numeric_limits<float>::max());
    std::vector<OpenMesh::Vec2f> clusterCentroidsVerify(numClusters, OpenMesh::Vec2f(0.0,0.0));
    //std::vector<int> clusterPopulation(numClusters,0);
    
    clusterPopulation_.insert(clusterPopulation_.begin(),numClusters,0);
    
    // Copy the cluster centroids
    for (int n=0; n<numClusters; n++)
    {
        clusterCentroids[n][0] = outs[2].data_[n];                   // First column
        clusterCentroids[n][1] = outs[2].data_[n + outs[2].nRows_];  // Second column
    }
    
    // outs[1].data_ holds first two eigenvectors from the PCA
    std::vector<double> basis_1;
    std::vector<double> basis_2;
    
    // Copy the deformation basis values so we can use them to deform the template later
    for (int j=0; j<numParameters; j++)
    {
        basis_1.push_back(outs[1].data_[j]);
        basis_2.push_back(outs[1].data_[j+outs[1].nRows_]);

        // PCA origin is the average of all the selected matches
        pcaOrigin_[j] /= (double)numMatches;

		avgMatchScale_[j] /= (double)numMatches;
    }
    
    pcaBasis_.push_back(basis_1);
    pcaBasis_.push_back(basis_2);

    // Fill the template match parts with values
    templateMatch_.setNparts( templateMatch_.parts().size() );

	switch (calculationMode_)
	{
		case CALCULATION_MODE_BOUNDING_BOX:
			{
                for (int j=0; j<templateMatch_.nparts(); j++)
				{
                    Match::Part& cPart = templateMatch_.parts().at(j);
                    
					OpenMesh::Vec3f min(pcaOrigin_[j*NUM_PARAMS_BOX],
                                        pcaOrigin_[j*NUM_PARAMS_BOX+1],
                                        pcaOrigin_[j*NUM_PARAMS_BOX+2]);
                    
					OpenMesh::Vec3f max(pcaOrigin_[j*NUM_PARAMS_BOX+3],
                                        pcaOrigin_[j*NUM_PARAMS_BOX+4],
                                        pcaOrigin_[j*NUM_PARAMS_BOX+5]);

					cPart.scale_ = (max - min) / 2.0f;
					cPart.pos_ = min + cPart.scale_;
				}
			}
			break;
		case CALCULATION_MODE_POSITION:
			{
				for (int j=0; j<templateMatch_.nparts(); j++)
				{
					Match::Part& cPart = templateMatch_.parts().at(j);

					cPart.scale_[0] = avgMatchScale_[j * NUM_PARAMS_POS];
					cPart.scale_[1] = avgMatchScale_[j * NUM_PARAMS_POS + 1];
					cPart.scale_[2] = avgMatchScale_[j * NUM_PARAMS_POS + 2];

					cPart.pos_[0] = pcaOrigin_[j * NUM_PARAMS_POS];
					cPart.pos_[1] = pcaOrigin_[j * NUM_PARAMS_POS + 1];
					cPart.pos_[2] = pcaOrigin_[j * NUM_PARAMS_POS + 2];
				}
			}
			break;
	}

    pcaMin_ = OpenMesh::Vec2d(std::numeric_limits<double>::max(),std::numeric_limits<double>::max());

    pcaMax_ = OpenMesh::Vec2d(-std::numeric_limits<double>::max(),-std::numeric_limits<double>::max());
    
    i = 0;

    if (numClusters==1)
    {
        representativeIndex_.insert(representativeIndex_.begin(),5,-std::numeric_limits<int>::max());
    }
    else
    {
        representativeIndex_.insert(representativeIndex_.begin(), numClusters,-std::numeric_limits<int>::max());
    }
    
    itMatch = filteredMatches_.begin();

    //For all matches belonging to the template selected, copy the projected_descriptor values
    for ( ; itMatch != fMatchesEnd; ++itMatch)
	{
        int lbl = outs[4].data_[i]-1;
        
        OpenMesh::Vec2f prDes(0.0,0.0);
        
        prDes[0] = outs[0].data_[i];                   // First column
        prDes[1] = outs[0].data_[i + outs[0].nRows_];  // Second column
        
        (**itMatch).setLabel(lbl);
        
        (**itMatch).setDescriptor2D(prDes);
        
        float dist2Centroid = (clusterCentroids[lbl] - prDes).sqrnorm();
        
        if (dist2Centroid < minDists[lbl])
        {
            minDists[lbl] = dist2Centroid;
            representativeIndex_[lbl] = i;
        }
        
        clusterCentroidsVerify[lbl] += prDes;
        
        clusterPopulation_[lbl]++;
    
        if (prDes[0] < pcaMin_[0])
        {
            pcaMin_[0] = prDes[0];
            
            if (numClusters==1)
            {
                representativeIndex_[1] = i;
            }
        }
        
        if (prDes[1] < pcaMin_[1])
        {
            pcaMin_[1] = prDes[1];
            
            if (numClusters==1)
            {
                representativeIndex_[4] = i;
            }
        }
        
        if (prDes[0] > pcaMax_[0])
        {
            pcaMax_[0] = prDes[0];

            if (numClusters==1)
            {
                representativeIndex_[2] = i;
            }
        }
        
        if (prDes[1] > pcaMax_[1])
        {
            pcaMax_[1] = prDes[1];
            
            if (numClusters==1)
            {
                representativeIndex_[3] = i;
            }
        }
        
        i++;

    }
    
    qDebug() << "Done with PCA for template ID:" << selectedTemplateID_ << " group ID:" << selectedGroupID_ << " part ID: " << selectedPartID_ << " error threshold: " << selectedFitErrorThreshold_ ;
    
    
    bool clustersDeleted = false;
    
    int numClustersAfterDeletion = 0;
    
    for (int n=0; n<numClusters; n++)
    {
        clusterCentroidsVerify[n] /= clusterPopulation_[n];
        
        qDebug() << "Cluster " << n << " centroid from Matlab was " << clusterCentroids[n][0] << ", " << clusterCentroids[n][1] << " from C++ it was" << clusterCentroidsVerify[n][0] << ", " << clusterCentroidsVerify[n][1];
        
        qDebug() << "Cluster " << n << " population is " << clusterPopulation_[n];
        
        if (!clustersDeleted && clusterPopulation_[n] < MIN_CLUSTER_POPULATION)
        {
            // If we get here it means one of the clusters has a very small population and needs to be deleted - because clusters are sorted on population, all the clusters after this one will be deleted as well
            representativeIndex_.erase(representativeIndex_.begin()+n, representativeIndex_.end());
            
            MatchLabelFunctor deleteAfterLabelN;
            
            deleteAfterLabelN.label_ = n;
            
            filteredMatches_.erase(std::remove_if(filteredMatches_.begin(), filteredMatches_.end(), deleteAfterLabelN), filteredMatches_.end());
            
            clustersDeleted = true;
            
            numClustersAfterDeletion = n;
            
        }
        
        if (!clustersDeleted && n >= MAX_NUM_CLUSTERS_TO_SHOW)
        {
            representativeIndex_.erase(representativeIndex_.begin() + MAX_NUM_CLUSTERS_TO_SHOW, representativeIndex_.end());
            
            MatchLabelFunctor deleteAfterLabelMAX_NUM;
            
            deleteAfterLabelMAX_NUM.label_ = MAX_NUM_CLUSTERS_TO_SHOW;
            
            filteredMatches_.erase(std::remove_if(filteredMatches_.begin(), filteredMatches_.end(), deleteAfterLabelMAX_NUM), filteredMatches_.end());
            
            clustersDeleted = true;
            
            numClustersAfterDeletion = n;
        }
    }
    
    qDebug() << "Filtered matches after erasing: " << filteredMatches_.size();
    
    if (clustersDeleted)
    {
        i = 0;
        
        // Reset match iterators since they have been invalided after deleting stuff from filteredMatches_
        itMatch = filteredMatches_.begin();
        
        fMatchesEnd = filteredMatches_.end();
        
        // Reset statistics for each cluster (to find min dist again for refinding the representative indices, population etc
        minDists.clear();
        minDists.insert(minDists.begin(), numClustersAfterDeletion,std::numeric_limits<float>::max());
        
        clusterCentroidsVerify.clear();
        clusterCentroidsVerify.insert(clusterCentroidsVerify.begin(), numClustersAfterDeletion, OpenMesh::Vec2f(0.0,0.0));
        
        clusterPopulation_.clear();
        
        clusterPopulation_.insert(clusterPopulation_.begin(), numClustersAfterDeletion, 0);
        
        if (numClustersAfterDeletion == 1)
        {
            representativeIndex_.clear();
            
            representativeIndex_.insert(representativeIndex_.begin(),5,-std::numeric_limits<int>::max());
        }
        
        pcaMin_ = OpenMesh::Vec2d(std::numeric_limits<double>::max(),std::numeric_limits<double>::max());
        
        pcaMax_ = OpenMesh::Vec2d(-std::numeric_limits<double>::max(),-std::numeric_limits<double>::max());
        
        //For all matches belonging to the template selected, copy the projected_descriptor values
        for ( ; itMatch != fMatchesEnd; ++itMatch)
        {
            int lbl = (**itMatch).label();
            
            OpenMesh::Vec2f prDes = (**itMatch).descriptor2D();
            
            float dist2Centroid = (clusterCentroids[lbl] - prDes).sqrnorm();
            
            if (dist2Centroid < minDists[lbl])
            {
                minDists[lbl] = dist2Centroid;
                representativeIndex_[lbl] = i;
            }
            
            clusterCentroidsVerify[lbl] += prDes;
            
            clusterPopulation_[lbl]++;
            
            if (prDes[0] < pcaMin_[0])
            {
                pcaMin_[0] = prDes[0];
                
                if (numClustersAfterDeletion==1)
                {
                    representativeIndex_[1] = i;
                }
            }
            
            if (prDes[1] < pcaMin_[1])
            {
                pcaMin_[1] = prDes[1];
                
                if (numClustersAfterDeletion==1)
                {
                    representativeIndex_[4] = i;
                }
            }
            
            if (prDes[0] > pcaMax_[0])
            {
                pcaMax_[0] = prDes[0];
                
                if (numClustersAfterDeletion==1)
                {
                    representativeIndex_[2] = i;
                }
            }
            
            if (prDes[1] > pcaMax_[1])
            {
                pcaMax_[1] = prDes[1];
                
                if (numClustersAfterDeletion==1)
                {
                    representativeIndex_[3] = i;
                }
            }
            
            i++;
        }
        
        qDebug() << "After recalculating the representative indices!!" ;
        
        for (int n=0; n < numClustersAfterDeletion; n++)
        {
            clusterCentroidsVerify[n] /= clusterPopulation_[n];
            
            qDebug() << "Cluster " << n << " centroid from Matlab was " << clusterCentroids[n][0] << ", " << clusterCentroids[n][1] << " from C++ it was" << clusterCentroidsVerify[n][0] << ", " << clusterCentroidsVerify[n][1];
            
            qDebug() << "Cluster " << n << " population is " << clusterPopulation_[n];
        }
        
        currentNumClusters_ = numClustersAfterDeletion;
    }
    
    std::cout << "PCA min: " << pcaMin_ << " PCA max: " << pcaMax_ << std::endl;

    return true;
}

void TemplateExplorationWidget::setPlotPoints()
{
    std::vector<OpenMesh::Vec3f> samples;

    plotIndexToMatchIndex_.clear();

    std::vector<Match*>::iterator itMatch(filteredMatches_.begin()), fMatchesEnd(filteredMatches_.end());
    
    itMatch = filteredMatches_.begin();
    
    int i = 0;
    int j = 0;
    
    scene_->clear();
    
    if (templateMarker_)
    {
        templateMarker_ = 0;
    }
    
    //Go through the matches to set the plot points
    for (; itMatch != fMatchesEnd; ++itMatch)
	{
        const OpenMesh::Vec2f& pos = (**itMatch).descriptor2D();
        
        int label = (**itMatch).label();
        
        TemplateExplorationViewItem* tmp = new TemplateExplorationViewItem(*itMatch, i);
        
        tmp->setPos(QPointF(pos[0],-pos[1]));
        scene_->addItem(tmp);
        
        if (CREATE_QWTPLOTW)
        {
            samples.push_back(OpenMesh::Vec3f(pos[0],pos[1],(**itMatch).label()));
        }


        plotIndexToMatchIndex_[j] = i;
        j++;
        i++;
    }
    
    if(CREATE_TEVW)
    {
        emit sceneChanged();
    }

    if (CREATE_QWTPLOTW)
    {
     	
        QString plotTitle;
   
        emit setPlotSamples(samples, plotTitle);
    }
}

void TemplateExplorationWidget::slotDeformTemplate(double _lambda1, double _lambda2)
{
    qDebug() << "Deform template: " << _lambda1 << ", " << _lambda2;
    
	int nParamsPerPart = 0;

	switch (calculationMode_)
	{
		case CALCULATION_MODE_BOUNDING_BOX:
        {
            nParamsPerPart = NUM_PARAMS_BOX;
        }
			break;
		case CALCULATION_MODE_POSITION:
        {
            nParamsPerPart = NUM_PARAMS_POS;
        }
			break;
	}

    if(pcaBasis_.size() != 2 || pcaBasis_.at(0).size() != pcaBasis_.at(1).size() || pcaBasis_.at(0).size() != pcaOrigin_.size() || pcaBasis_.at(0).size() != templateMatch_.parts().size()*nParamsPerPart)
    {
        qCritical() << "PCA basis size does not agree with PCA origin or template match number of parameters" ;
        return;
    }

    std::vector<double> deformed_origin;

    int descriptorSize = pcaOrigin_.size();

    for (int i=0; i < descriptorSize; i++)
    {
        double def_i = pcaOrigin_.at(i) + _lambda1 * pcaBasis_.at(0).at(i) + _lambda2 * pcaBasis_.at(1).at(i); // pcaBasis[0][i] is the coordinate of the first eigenvector

        deformed_origin.push_back(def_i);
    }

    double coord_x = 0.0;
    double coord_y = 0.0;

    // Find the projection of the deformed template descriptor on the 2D PCA space
    for (int i =0; i < descriptorSize; i++)
    {
        coord_x += (deformed_origin.at(i) - pcaOrigin_.at(i) ) * pcaBasis_.at(0).at(i);
        coord_y += (deformed_origin.at(i) - pcaOrigin_.at(i) ) * pcaBasis_.at(1).at(i);
    }

    // Store them in templateMatch
    templateMatch_.setDescriptor2D(OpenMesh::Vec2f(coord_x,coord_y));
    
    switch (calculationMode_)
	{
		case CALCULATION_MODE_BOUNDING_BOX:
			{
				for (int i=0; i<templateMatch_.nparts(); i++)
				{
                    Match::Part& cPart = templateMatch_.parts().at(i);
                    
					OpenMesh::Vec3f min(deformed_origin.at(i*nParamsPerPart),
                                        deformed_origin.at(i*nParamsPerPart+1),
                                        deformed_origin.at(i*nParamsPerPart+2));
					
                    OpenMesh::Vec3f max(deformed_origin.at(i*nParamsPerPart+3),
                                        deformed_origin.at(i*nParamsPerPart+4),
                                        deformed_origin.at(i*nParamsPerPart+5));

					cPart.scale_ = (max - min) / 2.0f;
					cPart.pos_ = min + cPart.scale_;
				}
			}
			break;
		case CALCULATION_MODE_POSITION:
			{
				for (int i=0; i<templateMatch_.nparts(); i++)
				{
                    Match::Part& cPart = templateMatch_.parts().at(i);

					cPart.scale_[0] = avgMatchScale_[i * nParamsPerPart];
					cPart.scale_[1] = avgMatchScale_[i * nParamsPerPart + 1];
					cPart.scale_[2] = avgMatchScale_[i * nParamsPerPart + 2];

					cPart.pos_[0] = deformed_origin[i * nParamsPerPart];
					cPart.pos_[1] = deformed_origin[i * nParamsPerPart + 1];
					cPart.pos_[2] = deformed_origin[i * nParamsPerPart + 2];
				}
			}
			break;
	}

    if (preserveConstraints_)
    {
        slotOptimizeTemplate();
    }
}

void TemplateExplorationWidget::slotChangeSelectedMatch(int _newIndex)
{
    selectedMatchIndex_ = plotIndexToMatchIndex_[_newIndex];

    const Match& cMatch = *filteredMatches_.at(selectedMatchIndex_);
    
    QString shortName = cMatch.filename().split("/").last();
    
    qDebug() << "Selected plot index: " << _newIndex << " was mapped to match index: " << selectedMatchIndex_ << " with name: " << shortName;
    
    labelMatchName_->setText(shortName);
    
    labelMatchTemplateID_->setText( QString().setNum(cMatch.templateID()) );
    
    labelMatchGroupID_->setText( QString().setNum(cMatch.groupID()) );
    
    labelMatchFitError_->setText( QString().setNum(cMatch.fitError()) ) ;
    
}

void TemplateExplorationWidget::slotReadFitErrorFile()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Read fit error file"),
                                                    tr(""),
                                                    tr("Text Files  (*.txt);;"
                                                       "All Files (*)"));

	if (filename.isEmpty())
    {
        qCritical() << "No file selected" ;
		return;
    }

    readFitErrorFile(filename);
}

void TemplateExplorationWidget::slotReadNormalizationFile()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Read normalization file"),
                                                    tr(""),
                                                    tr("Text Files  (*.txt);;"
                                                       "All Files (*)"));
    
	if (filename.isEmpty())
    {
        qCritical() << "No file selected" ; 
		return;
    }
    
    readNormalizationFile(filename);
}

void TemplateExplorationWidget::readFitErrorFile(const QString& _filename)
{
    QFile file(_filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Could not open file " << _filename ;
        return;
    }

    QTextStream in(&file);

	int count = 0;

    while (!in.atEnd())
	{
        QString line = in.readLine();
        QStringList lineList = line.split(" ");

        int lineListSize = lineList.size();

        QString name = lineList.at(0);
        
        double fitError = lineList.at(1).toDouble();

        if(matchNameToMatchIndex_.count(name.toStdString())!=1)
        {
            qWarning() << "Ignoring match name: " << name << " because it was not found in the collection!" ;
            continue;
        }
        Match& cMatch = *matches_[ matchNameToMatchIndex_[name.toStdString()] ];
        
        cMatch.setFitError(fitError);
    
        count++;
    }

    qDebug() << "Fit error loaded (# of entries: " << count << ")" ;
}

void TemplateExplorationWidget::slotChangeExplorationMode(int _explorationMode)
{
    int previousExplorationMode = explorationMode_;
    
	switch (_explorationMode)
	{
        case SHOW_TEMPLATE:
        {
            qDebug() << "Changed exploration mode to show template" ;
            explorationMode_ = SHOW_TEMPLATE;
        }
            break;
        case SHOW_SELECTED:
        {
            if(selectedMatchIndex_==-1)
            {
                qWarning() << "No match has been selected yet. Select one first!" ;
                comboBoxExplorationMode_->setCurrentIndex(explorationMode_);
                return;
            }
            qDebug() << "Changed exploration mode to show selected" ;
            explorationMode_= SHOW_SELECTED;
        }
            break;		
        case SHOW_NEAREST:
        {
            qDebug() << "Changed exploration mode to show nearest" ;
            explorationMode_ = SHOW_NEAREST;
        }
			break;
        case SHOW_DEFORMED:
        {
            qDebug() << "Changed exploration mode to show deformed" ;
            explorationMode_ = SHOW_DEFORMED;
        }
			break;
        case SHOW_DEFORMED_SUPERIMPOSED:
        {
            qDebug() << "Changed exploration mode to show deformed superimposed" ;
            explorationMode_ = SHOW_DEFORMED_SUPERIMPOSED;
        }
            break;
        case SHOW_LERP:
        {
            qDebug() << "Changed exploration mode to show lerp" ;
            explorationMode_ = SHOW_LERP;
        }
            break;
        case SELECT_CLUSTER:
        {
            qDebug() << "Changed exploration mode to select cluster" ;
            explorationMode_ = SELECT_CLUSTER;
        }
            break;
        case SHOW_REPRESENTATIVE:
        {
            qDebug() << "Changed exploration mode to show representative" ;
            explorationMode_ = SHOW_REPRESENTATIVE;
        }
            break;
        case SHOW_DEFORMED_OPTIONS:
        {
            qDebug() << "Changed exploration mode to show deformed options" ;
            explorationMode_ = SHOW_DEFORMED_OPTIONS;
        }
            break;
        case SHOW_GROUPS:
        {
            qDebug() << "Changed exploration mode to show groups" ;
            explorationMode_ = SHOW_GROUPS;
            emit interactionModeChanged(QString("Interaction Mode: Exploration"));
        }
            break;
        case SHOW_CLUSTERS:
        {
            qDebug() << "Changed exploration mode to show clusters" ;
            explorationMode_ = SHOW_CLUSTERS;
         }
            break;
        case SHOW_CLUSTER:
        {
            qDebug() << "Changed exploration mode to show cluster" ;
            explorationMode_ = SHOW_CLUSTER;
        }
            break;
        case SHOW_DEFORMED_PART_OPTIONS:
        {
            qDebug() << "Changed exploration mode to show deformed part options" ;
            explorationMode_ = SHOW_DEFORMED_PART_OPTIONS;
        }
            break;
        case SHOW_MATCHES_SUPERIMPOSED:
        {
            qDebug() << "Changed exploration mode to show matches superimposed" ;
            explorationMode_ = SHOW_MATCHES_SUPERIMPOSED;
        }
            break;
        case SHOW_PART_OPTIONS:
        {
            qDebug() << "Changed exploration mode to show part options" ;
            explorationMode_ = SHOW_PART_OPTIONS;
        }
            break;
        case SHOW_RANDOM:
        {
            qDebug() << "Changed exploration mode to show random match" ;
            explorationMode_ = SHOW_RANDOM;
        }
            break;
            
        default:
        {
            
        }
            break;
	}
    if (previousExplorationMode == SHOW_DEFORMED_OPTIONS || previousExplorationMode == SHOW_DEFORMED_PART_OPTIONS || previousExplorationMode == SHOW_PART_OPTIONS || previousExplorationMode == SHOW_CLUSTERS  || previousExplorationMode ==SHOW_CLUSTER)
    {
        dataState_.deformedNearestOptionsValid = false;
        dataState_.deformedPartOptionsValid = false;
    }
    if(explorationMode_ == SHOW_REPRESENTATIVE)
    {
        if (CREATE_MVW)
        {
            // Start from 1, if we do have more than one mvw, the first one at index 0 will always be visible anyway
            for (int i=1; i< NOF_MVW; i++)
            {
                emit toggleVisible(true, i);
            }
        }
    }
    else
    {
        if (CREATE_MVW)
        {
            // Start from 1, if we do have more than one mvw, the first one at index 0 will always be visible anyway
            for (int i=1; i< NOF_MVW; i++)
            {
                emit toggleVisible(false, i);
            }
        }
    }
    randomNeighbourVectorIndex_ = 0;
    computeExplorationData(explorationMode_);
    
    updateGUI(explorationMode_);
    
    updatePlot(explorationMode_);
    
    updateViewport(explorationMode_);
    
    //emit explorationModeChanged(explorationMode_);
}

void TemplateExplorationWidget::slotChangeCalculationMode(int _calculationMode)
{
	switch (_calculationMode)
	{
		case CALCULATION_MODE_BOUNDING_BOX:
			{
				calculationMode_ = CALCULATION_MODE_BOUNDING_BOX;

                qDebug() << "Calculate MDS/PCA on bounding box" ;
			}
			break;
		case CALCULATION_MODE_POSITION:
			{
				calculationMode_ = CALCULATION_MODE_POSITION;

                qDebug() << "Calculate MDS/PCA on position" ;
			}
			break;
	}
}

int TemplateExplorationWidget::getNearestPoint(int _level, double x, double y)
{
	double minDistance = std::numeric_limits<double>::max();
	double threshold = 0.0;
	int index = 0;
	int nearestIndex = -1;


	if (_level > 0)
	{
		threshold = nearestPoints_[_level - 1].distance_;
	}

    std::vector<Match*>::iterator itMatch(filteredMatches_.begin()), fMatchesEnd(filteredMatches_.end());
    
	for (; itMatch != fMatchesEnd; ++itMatch)
	{
        const OpenMesh::Vec2f& pos = (**itMatch).descriptor2D();
        
		double cx = x - pos[0];
		double cy = y - pos[1];
		double distance = cx * cx + cy * cy;

		if (_level == 0)
		{
			if (distance < minDistance)
			{
				minDistance = distance;
				nearestPoints_[_level].index_ = index;
				nearestPoints_[_level].distance_ = distance;
			}
		}
		else if (_level > 0)
		{
			bool isSkip = false;
			for (int i = 0; i < _level; ++i)
			{
				if (index == nearestPoints_[i].index_)
				{
					isSkip = true;
				}
			}

			if (!isSkip && distance < minDistance && distance >= threshold)
			{
				minDistance = distance;
				nearestPoints_[_level].index_ = index;
				nearestPoints_[_level].distance_ = distance;
			}
		}

		++index;
	}

	return nearestPoints_[_level].index_;
}

void TemplateExplorationWidget::slotDeformKNearestMatches(double posx, double posy)
{

    slotDeformTemplate(posx, posy);

    // Deform nearest neighbours according to the deformed template
	for (int i = 0; i < nofNN_; ++i)
	{
        //SetMesh(m_NearestPoints[i]._Index);
		slotDeformNearestMatch(i);
	}

}

void TemplateExplorationWidget::slotDeformNearestMatch(int _level)
{
    int nearestMatchIndex = nearestPoints_[_level].index_;

    if(nearestMatchIndex<0 || nearestMatchIndex >= filteredMatches_.size())
    {
        qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
        return;
    }
    
    //std::cout<< "Nearest match index: " << nearestMatchIndex << " with name: " << matches.at(nearestMatchIndex).name.toStdString() << std::endl;

    //std::cout<< "Nearest match index: " << nearestMatchIndex << " with name: " << matches.at(nearestMatchIndex).name.toStdString() << std::endl;

    Match& nearestMatch = *filteredMatches_.at(nearestMatchIndex);

    if(nearestMatch.nparts() != templateMatch_.nparts())
    {
        qCritical() << "Cannot deform nearest match, number of parts of nearest match and template match is not compatible" ;
        return;
    }

    // Do some copying in order to fill the fields of the deformedNearestMatch
    // Most fields come from the nearest match, but the parts and the plot coordinates come from the template match (it has been deformed already)

    deformedNearestMatches_[_level].setAlignMtx(nearestMatch.alignMtx());
    deformedNearestMatches_[_level].setFitError(nearestMatch.fitError());
    deformedNearestMatches_[_level].setGroupID(nearestMatch.groupID());

    deformedNearestMatches_[_level].setFilename(nearestMatch.filename());
    deformedNearestMatches_[_level].setNparts(nearestMatch.nparts());
    deformedNearestMatches_[_level].setNpnts(nearestMatch.npnts());

    deformedNearestMatches_[_level].setPoints(nearestMatch.points());
    deformedNearestMatches_[_level].setTemplateID(nearestMatch.templateID());

    deformedNearestMatches_[_level].setDescriptor2D(templateMatch_.descriptor2D());
    deformedNearestMatches_[_level].setParts(templateMatch_.parts());
    
    
    deformedNearestMatches_[_level].setSegmented(true);
    
    std::vector<Match::Part>& dnmParts = deformedNearestMatches_[_level].parts();
    
    std::vector<Match::Part>::iterator dnmPartsIt(dnmParts.begin()), dnmPartsEnd(dnmParts.end());
    
    for (; dnmPartsIt!=dnmPartsEnd; ++dnmPartsIt)
    {
        deformNearestPart(*dnmPartsIt, nearestMatch);
    }
    
    return;
}


void TemplateExplorationWidget::slotChangeDisplayMode(int _displayMode)
{
    displayMode_ = 0;
    
    switch (_displayMode)
	{
		case 0:
        {
            displayMode_ |= BOX;
        }
            break;
		case 1:
        {
            displayMode_ |= POINTS;
        }
			break;
        case 2:
        {
            displayMode_ |= MESH;
        }
			break;
        case 3:
        {
            displayMode_ |= BOX;
            displayMode_ |= POINTS;
        }
			break;
        case 4:
        {
            displayMode_ |= BOX;
            displayMode_ |= MESH;
        }
            break;
        case 5:
        {
            displayMode_ |= POINTS;
            displayMode_ |= MESH;
        }
            break;
        case 6:
        {
            displayMode_ |= BOX;
            displayMode_ |= POINTS;
            displayMode_ |= MESH;
        }
            break;
	}
    
    std::vector<Match*>::iterator itMatch(matches_.begin()), matchesEnd(matches_.end());
    
    for( ; itMatch!=matchesEnd; ++itMatch)
    {
        (**itMatch).setDisplayMode(displayMode_);
    }
    
    templateMatch_.setDisplayMode(displayMode_);
    
    for (int i=0; i< nofNN_; ++i)
    {
        deformedNearestMatches_[i].setDisplayMode(displayMode_);
    }
    
    deformedNearestOption_.setDisplayMode(displayMode_);
    
    switch (explorationMode_)
    {
        case SHOW_SELECTED:
            if(selectedMatchIndex_>=0)
            {
                Match* cMatch = filteredMatches_.at(selectedMatchIndex_);
                // Open the mesh if we are in show mesh display mode
                if(displayMode_ & MESH)
                {
                    cMatch->openMeshIfNotOpened();
                }
            }
            break;
        case SHOW_NEAREST:
        {
            int nearestMatchIndex = nearestPoints_[nearestNeighbourLevel_].index_;
            
            if (nearestMatchIndex>=0 && nearestMatchIndex<filteredMatches_.size())
            {
                Match* cMatch = filteredMatches_.at(nearestPoints_[nearestNeighbourLevel_].index_);
                // Open the mesh if we are in show mesh display mode
                if(displayMode_ & MESH)
                {
                    cMatch->openMeshIfNotOpened();
                }
            }
            else
            {
                qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
            }
        }
            break;
        case SHOW_REPRESENTATIVE:
        {
            for (int i=0; i<5; ++i)
            {
                if(representativeIndex_[i]>=0)
                {
                    Match* representative = filteredMatches_.at(representativeIndex_[i]);
                    // Open the mesh if we are in show mesh display mode
                    if(displayMode_ & MESH)
                    {
                        representative->openMeshIfNotOpened();
                    }
                }
                
            }
            
        }
            break;
        case SHOW_MATCHES_SUPERIMPOSED:
            for (int i =0; i < nofNN_; ++i)
            {
                int nearestMatchIndex = nearestPoints_[i].index_;
                
                if(nearestMatchIndex<0 || nearestMatchIndex >= filteredMatches_.size())
                {
                    qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
                }
                else
                {
                    Match* nMatch = filteredMatches_.at(nearestPoints_[i].index_);
                    
                    if (displayMode_ & MESH)
                    {
                        nMatch->openMeshIfNotOpened();
                        nMatch->setDisplayMode(displayMode_);
                    }
                }
            }
        default:
            break;
    }
    
    comboBoxDisplayMode_->blockSignals(true);
    comboBoxDisplayMode_->setCurrentIndex(_displayMode);
    comboBoxDisplayMode_->blockSignals(false);
    
    if (CREATE_MVW)
    {
        emit requestRedraw(0);
    }
}


void TemplateExplorationWidget::slotInterpolateSelected(int _lerp)
{
    
    std::vector<int>::iterator itLerps(lerpIndices_.begin()), lerpsEnd(lerpIndices_.end());
    
    double sumDistances = 0.0;
    
    std::vector<double> distances;
    
    for ( ; itLerps!=lerpsEnd; ++itLerps)
    {
        int cIndex = *itLerps;
        if(itLerps+1 == lerpsEnd)
        {
            break;
        }
        int nIndex = *(itLerps+1);
        Match& cMatch = *filteredMatches_[cIndex];
        const OpenMesh::Vec2f& cPos = cMatch.descriptor2D();
        
        Match& nMatch = *filteredMatches_[nIndex];
        const OpenMesh::Vec2f& nPos = nMatch.descriptor2D();
        
        sumDistances += (nPos - cPos).length();
        
        distances.push_back(sumDistances);
    }
    
    itLerps = lerpIndices_.begin();
    
    std::vector<double>::iterator itDists(distances.begin()) , distsEnd(distances.end());
    
    for ( ; itDists!=distsEnd; ++itDists)
    {
        *itDists = *itDists / sumDistances;
    }

    // _lerp is within the range [0,100], so map it to the range [0,1]
    double alpha = _lerp * 0.01;
    qDebug() << "alpha=" << alpha ;
    
    int index = binarySearch<double>(distances,alpha);
    
    qDebug() << "index=" << index ;

    Match& firstMatch = *filteredMatches_.at(lerpIndices_[index]);
    Match& secondMatch = *filteredMatches_.at(lerpIndices_[index+1]);
    
    const OpenMesh::Vec2f& firstPos = firstMatch.descriptor2D();
    const OpenMesh::Vec2f& secondPos = secondMatch.descriptor2D();
    double a1 = 0.0;
    double a2 = 0.0;
    // Map actual alpha to the piecewise alpha
    if(index!=0)
    {
        a1 = distances[index-1];
    }
    a2 = distances[index];
    
    float alpha2 = (alpha - a1) / (a2 - a1);
    const OpenMesh::Vec2f& lerpPos = secondPos * alpha2 + (1-alpha2) * firstPos;
    
    slotChangeSelectedPoint(lerpPos[0], lerpPos[1]);
}

void TemplateExplorationWidget::slotChangeSelectedPoint( double _posx, double _posy)
{
    
    OpenMesh::Vec2f selectedPoint(_posx,_posy);
    
    if (selectedPoint_ == selectedPoint)
    {
        qDebug() << "Same point was clicked again, no need to recompute anything!" ;
        return;
    }
    
    selectedPoint_[0] = _posx;
    selectedPoint_[1] = _posy;
    
    qDebug() << "Point clicked: " << selectedPoint_[0] << ", " << selectedPoint[1];
    
    // Get nearest neighbours
    // used to check a checkbox to use ann, now we just use it by default
    if(1) //chkBoxUseANN_->isChecked())
    {
        qDebug() << "Using ANN to find nearest neighbours";
        if (nearestPoints_)
        {
            delete [] nearestPoints_;
        }
        
        nearestPoints_ = getNearestPoint(_posx, _posy, nofNN_); //NUM_OF_NEAREST_NEIGHBOURS);
    }
	else
    {
        qDebug() << "Using exhaustive search to find nearest neighbours";
        
        if (nearestPoints_)
        {
            delete [] nearestPoints_;
        }
        
        nearestPoints_ = new NEAREST_POINT[nofNN_];
        
        for (int i = 0; i < nofNN_; ++i) //NUM_OF_NEAREST_NEIGHBOURS; ++i)
        {
            getNearestPoint(i, _posx, _posy);
        }
    }

    // Add some random indices as well, for the SHOW_PART_OPTIONS and SHOW_MATCHES_SUPERIMPOSED modes
    randomNeighbourIndices_.clear();
    
    for (int i = 0 ; i< nofNN_; ++i)
    {
        if(filteredMatches_.size() == 0)
        {
            randomNeighbourIndices_.push_back(-1);
        }
        else
        {
            randomNeighbourIndices_.push_back(rand() % filteredMatches_.size() );
        }
    }
    randomNeighbourVectorIndex_ = 0;
    
    // Selected point has changed so we reset the validity of the template match and the deformed nearest matches so they will be recalculated
    resetDataState();
    
    computeExplorationData(explorationMode_);
    
    // No need to update the gui here
    updateGUI(explorationMode_);
    
    updatePlot(explorationMode_);
    
    updateViewport(explorationMode_);
    
}



// This method computes the data we need to display, depending on the exploration mode passed as parameter (the current exploration mode could be used to avoid using a parameter at all) but lets make it a bit more flexible in case we need to call it in other contexts as well)
// The plan is to compute data for all future exploration modes we may need, but right now only a few exploration modes need actual data to be computed
// In order to avoid checking through the explonentially large state space of (previous_state,next_state) pairs, we store the state of the needed exploration data (i.e. up to now we have the deformed template match and the deformed nearest neighbor matches, more will come in the future) in the dataState_ object and we reset its state whenever these data need to be recomputed, for example when a new point is selected in the plot
// We could reset the dataState object inside this method but in that case we would need to know previous and next state (i.e. was a new point selected or not) so that kind of defied the purpose of using the dataState object and keeping this method simple in terms of checks
void TemplateExplorationWidget::computeExplorationData(int _explorationMode)
{
    qDebug() << "Computing exploration data.." ;
    iview = -1;
    if(selectedPoint_ == OpenMesh::Vec2f(-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max()))
    {
        qDebug() << "No point is selected. Nothing to compute" ;
        return;
    }
    
    switch (_explorationMode)
	{
        case SHOW_TEMPLATE:
        {
            
        }
        case SHOW_SELECTED:
        {
        
        }
        case SHOW_NEAREST:
        {
            if (!dataState_.templateValid)
            {
                slotDeformTemplate(selectedPoint_[0], selectedPoint_[1]);
                dataState_.templateValid = true;
            }
        }
			break;
        case SHOW_DEFORMED:
        {
            if (!dataState_.templateValid)
            {
                slotDeformTemplate(selectedPoint_[0], selectedPoint_[1]);
                dataState_.templateValid = true;
            }
            
            if (!dataState_.deformedNearestValid[nearestNeighbourLevel_])
            {
                slotDeformNearestMatch(nearestNeighbourLevel_);
                dataState_.deformedNearestValid[nearestNeighbourLevel_] = true;
            }
        }
            break;
        case SHOW_DEFORMED_SUPERIMPOSED:
        {
            if (!dataState_.templateValid)
            {
                slotDeformTemplate(selectedPoint_[0], selectedPoint_[1]);
                dataState_.templateValid = true;
            }
            
            for (int i=0; i<=nearestNeighbourLevel_; ++i)
            {
                if (!dataState_.deformedNearestValid[i])
                {
                    slotDeformNearestMatch(i);
                    dataState_.deformedNearestValid[i] = true;
                }
            }
        }
            break;
        case SHOW_LERP:
        {
            // Need a way to check if a match was selected (slotChangeSelectedMatch) right before getting here
            
            if (btnResetLerp_->isChecked() )
            {
                // We could set a flag - check it and then unset it for the next time, but lets try checking if the nearest neighbor is indeed so near that we can assume a match was just selected
                if (nearestPoints_[0].distance_ < 0.0001)
                {
                    lerpIndices_.push_back(selectedMatchIndex_);
                }
            }
            //else
            //{
                if (!dataState_.templateValid)
                {
                    slotDeformTemplate(selectedPoint_[0], selectedPoint_[1]);
                    dataState_.templateValid = true;
                }
            //}
            
        }
            break;
        case SELECT_CLUSTER:
        {
            if (clusterBox_.first == OpenMesh::Vec2f(-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max()))
            {
                clusterBox_.first = selectedPoint_;
            }
            else if (clusterBox_.second == OpenMesh::Vec2f(-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max()))
            {
                clusterBox_.second = selectedPoint_;
            }
            else
            {
                clusterBox_.first = clusterBox_.second;
                clusterBox_.second = selectedPoint_;
            }
            qDebug() << "First: " << clusterBox_.first[0] << " " << clusterBox_.first[1] ;
            qDebug() << "Second: " << clusterBox_.second[0] << " " << clusterBox_.second[1] ;
        }
            break;
        case SHOW_REPRESENTATIVE:
        {
            //nothing to calculate, representativeIndex_ should be calculated every time we calculate PCA or MDS
            if (!dataState_.templateValid)
            {
                slotDeformTemplate(selectedPoint_[0], selectedPoint_[1]);
                dataState_.templateValid = true;
            }
        }
            break;
        case SHOW_DEFORMED_OPTIONS:
        {
            if (!dataState_.templateValid)
            {
                slotDeformTemplate(selectedPoint_[0], selectedPoint_[1]);
                dataState_.templateValid = true;
            }
            if(!dataState_.deformedNearestOptionsValid)
            {
                if(chkNN_->isChecked())
                {
                    deformNearestMatches(1);
                }
                else
                {
                    deformNearestMatches(nearestNeighbourLevel_+1);
                }
                dataState_.deformedNearestOptionsValid = true;
            }
        }
            break;
        case SHOW_GROUPS:
        {
            
        }
            break;
        // Same behaviour for the following three modes
        case SHOW_CLUSTERS:
        {

        }
            
        case SHOW_CLUSTER:
        {

        }
            
        case SHOW_DEFORMED_PART_OPTIONS:
        {
            if (!dataState_.templateValid)
            {
                slotDeformTemplate(selectedPoint_[0], selectedPoint_[1]);
                dataState_.templateValid = true;
            }
            
            if (!dataState_.deformedPartOptionsValid)
            {
                xform ident;
                
                deformedNearestOption_.setAlignMtx(ident);
                
                deformedNearestOption_.setGroupID(selectedGroupID_);
                
                deformedNearestOption_.setNparts(templateMatch_.nparts());
                
                deformedNearestOption_.setTemplateID(selectedTemplateID_);
                
                deformedNearestOption_.setDescriptor2D(templateMatch_.descriptor2D());
                
                deformedNearestOption_.parts().clear();
                
                deformedNearestOption_.setParts(templateMatch_.parts());
                
                // We need to set the segmented property to true in order to render the individual meshes taken from each part, otherwise the renderer will try to render the deformed option's mesh, which is empty
                deformedNearestOption_.setSegmented(true);
                
                deformedNearestOption_.points().clear();
                
                int displayMode = 0;
                displayMode |= BOX;
                displayMode |= MESH;

                std::vector<Match::Part>& cParts = deformedNearestOption_.parts();
                
                std::vector<Match::Part>::iterator partIt(cParts.begin()), partEnd(cParts.end());
                
                nnIndexPartScoreSorted_.clear();
                
                nnChosen_.clear();
                
                partClicks_.clear();
                
                // Do a pass to rank all the neighbors based on their unary score per part
                for (; partIt!=partEnd; ++partIt)
                {
                    rankNeighborPartsUnary(partIt->partID_);
                }
                
                partIt = cParts.begin();
                
                // Check if the part is involved in any symmetry constraints
                std::vector<Match::Constraint>& tmConstraints = templateMatch_.constraints();
                
                std::vector<Match::Constraint>::iterator tmConstraintsIt, tmConstraintsEnd(tmConstraints.end());
                
                int tmConstraintsSize = tmConstraints.size();
                
                int nIparts = cParts.size();
                
                nIndependentParts_ = nIparts - nSymmetryConstraints_;
                
                emit nIndependentPartsChanged(QString("No of independent parts: %1").arg(nIndependentParts_));
                
                // Do another pass to actually pick the top ranked neighbour for each part and enforce symmetries
                for (; partIt!=partEnd; ++partIt)
                {
                    nnPartScoreVectorIndex_[partIt->partID_] = 0;
                    
                    if (preserveConstraints_)
                    {
                        tmConstraintsIt = tmConstraints.begin();
                        
                        int i =0;
                        
                        for (; tmConstraintsIt!= tmConstraintsEnd; ++tmConstraintsIt)
                        {
                            if (tmConstraintsIt->type_ == SYMMETRY)
                            {
                                if( tmConstraintsIt->partIDs_.first == partIt->partID_)
                                {
                                    showPartDeformationOption(partIt->partID_,tmConstraintsIt->partIDs_.second,true);
                                    break;
                                }
                                
                                if(tmConstraintsIt->partIDs_.second == partIt->partID_)
                                {
                                    showPartDeformationOption(partIt->partID_,tmConstraintsIt->partIDs_.first,true);
                                    break;
                                }
                            }
                            i++;
                        }
                        
                        if (i==tmConstraintsSize)
                        {
                            // Means we didn't find any symmetric counterparts, so just deform the part without symmetry
                            showPartDeformationOption(partIt->partID_, -1,true);
                        }
                    }
                    else
                    {
                        showPartDeformationOption(partIt->partID_, -1,true);
                    }
                }
                
                partIt = cParts.begin();
                
                std::unordered_map< int, int> countNeighbors;
                
                // Do another pass to actually count neighbors used
                for (; partIt!=partEnd; ++partIt)
                {
                    countNeighbors [ nnChosen_[partIt->partID_]] ++;
                }
                
                nnUsed_ = countNeighbors.size(); // nnChosen_.size(); //countNeighbors.size() - nSymmetryConstraints_;
                
                emit nnUsedChanged(QString("No of neighbors used: %1").arg(nnUsed_));
                
                dataState_.deformedPartOptionsValid = true;

                TIMELOG->append(QString("%1 : deformed_model top_ranked").arg((qlonglong)QDateTime::currentMSecsSinceEpoch()));
            }
        }
            break;
        case SHOW_MATCHES_SUPERIMPOSED:
        {
            if (!dataState_.templateValid)
            {
                slotDeformTemplate(selectedPoint_[0], selectedPoint_[1]);
                dataState_.templateValid = true;
            }
            
            for (int i=0; i<nofNN_; ++i)
            {
                if (!dataState_.deformedNearestValid[i])
                {
                    dataState_.deformedNearestValid[i] = true;
                }
            }
        }
            break;
        case SHOW_PART_OPTIONS:
        {
            if (!dataState_.templateValid)
            {
                slotDeformTemplate(selectedPoint_[0], selectedPoint_[1]);
                dataState_.templateValid = true;
            }
            
            if (!dataState_.deformedPartOptionsValid)
            {
                xform ident;
                
                deformedNearestOption_.setAlignMtx(ident);
                
                deformedNearestOption_.setGroupID(selectedGroupID_);
                
                deformedNearestOption_.setNparts(templateMatch_.nparts());
                
                deformedNearestOption_.setTemplateID(selectedTemplateID_);
                
                deformedNearestOption_.setDescriptor2D(templateMatch_.descriptor2D());
                
                deformedNearestOption_.parts().clear();
                
                deformedNearestOption_.setParts(templateMatch_.parts());
                
                // We need to set the segmented property to true in order to render the individual meshes taken from each part, otherwise the renderer will try to render the deformed option's mesh, which is empty
                deformedNearestOption_.setSegmented(true);
                
                deformedNearestOption_.points().clear();
                
                
                std::vector<Match::Part>& cParts = deformedNearestOption_.parts();
                
                std::vector<Match::Part>::iterator partIt(cParts.begin()), partEnd(cParts.end());
                
                nnIndexPartScoreSorted_.clear();
                
                // Do a pass to RANDOMLY select a shape from which to pick each part
                for (; partIt!=partEnd; ++partIt)
                {
                    rankNeighborPartsUnary(partIt->partID_);
                    
                    nnPartScoreVectorIndex_[partIt->partID_] = 0;
                    
                    showPartDeformationOption(partIt->partID_, -1,true);
                }

                dataState_.deformedPartOptionsValid = true;
                
                comboBoxDisplayMode_->setCurrentIndex(2); //slotChangeDisplayMode(2); // manually set the display mode to boxes + mesh
                
            }

        }
            break;
        default:
        {
            
        }
            break;
	}
}


void TemplateExplorationWidget::resetDataState()
{
    qDebug() << "Reseting data state.." ;
    
    dataState_.templateValid = false;
    
    for (int i=0; i<nofNN_; ++i)
    {
        dataState_.deformedNearestValid[i] = false;
    }
    
    dataState_.deformedNearestOptionsValid = false;
    dataState_.deformedPartOptionsValid = false;
    pickedPartID_ = -1;
}

void TemplateExplorationWidget::updateGUI(int _explorationMode)
{
    qDebug() << "Updating GUI.." ;
    
    // These two lines may need to be inside the switch. Revisit later!
    labelLambda1_->setText(QString("Lambda1: %1").arg(selectedPoint_[0]));
    labelLambda2_->setText(QString("Lambda2: %1").arg(selectedPoint_[1]));
    
    switch (_explorationMode)
	{
        case SHOW_TEMPLATE:
        {
        }
        case SHOW_SELECTED:
        {
        }
        case SHOW_NEAREST:
        {
        }
        case SHOW_DEFORMED:
        {
        }
        case SHOW_DEFORMED_SUPERIMPOSED:
        {
            updateSliders();
            
            // Hide the widgets related to show lerp
            if (labelLerp_->isVisible())
            {
                labelLerp_->setVisible(false);
            }
            
            if (btnResetLerp_->isVisible())
            {
                btnResetLerp_->setVisible(false);
                slotResetLerp(true);
                btnResetLerp_->setChecked(false);
                btnResetLerp_->setText("Start selecting shapes");
            }
            
            if(sliderLerp_->isVisible())
            {
                sliderLerp_->setVisible(false);
                //sliderLerp_->setEnabled(false);
            }
            
            if (btnSelectCluster_->isVisible())
            {
                btnSelectCluster_->setVisible(false);
            }
            
            if (btnShowNextDefOption_->isVisible())
            {
                btnShowNextDefOption_->setVisible(false);
            }
            
            // Show the widgets related to the rest of the modes
            if(!labelLambda1_->isVisible())
            {
                labelLambda1_->setVisible(true);
            }
            if(!sliderLambda1_->isVisible())
            {
                sliderLambda1_->setVisible(true);
            }
            if(!labelLambda2_->isVisible())
            {
                labelLambda2_->setVisible(true);
            }
            if(!sliderLambda2_->isVisible())
            {
                sliderLambda2_->setVisible(true);
            }
        }
            break;
        case SHOW_LERP:
        {
            // Show the widgets related to show lerp
            if (!labelLerp_->isVisible())
            {
                labelLerp_->setVisible(true);
            }
            if(!sliderLerp_->isVisible())
            {
                sliderLerp_->setVisible(true);
            }
            if (!btnResetLerp_->isVisible())
            {
                btnResetLerp_->setVisible(true);
                //btnResetLerp_->setChecked(false);
            }
            
            if (btnSelectCluster_->isVisible())
            {
                btnSelectCluster_->setVisible(false);
            }
            
            if (btnShowNextDefOption_->isVisible())
            {
                btnShowNextDefOption_->setVisible(false);
            }
            
            // Hide the widgets related to the rest of the modes
            if(labelLambda1_->isVisible())
            {
                labelLambda1_->setVisible(false);
            }
            if(sliderLambda1_->isVisible())
            {
                sliderLambda1_->setVisible(false);
            }
            if(labelLambda2_->isVisible())
            {
                labelLambda2_->setVisible(false);
            }
            if(sliderLambda2_->isVisible())
            {
                sliderLambda2_->setVisible(false);
            }
            
        }
            break;
        case SELECT_CLUSTER:
        {
            // Hide the widgets related to show lerp
            if (labelLerp_->isVisible())
            {
                labelLerp_->setVisible(false);
            }
            
            if (btnResetLerp_->isVisible())
            {
                btnResetLerp_->setVisible(false);
                slotResetLerp(true);
                btnResetLerp_->setChecked(false);
                btnResetLerp_->setText("Start selecting shapes");
            }
            
            if(sliderLerp_->isVisible())
            {
                sliderLerp_->setVisible(false);
                //sliderLerp_->setEnabled(false);
            }
            // Hide the widgets related to the rest of the modes
            if(labelLambda1_->isVisible())
            {
                labelLambda1_->setVisible(false);
            }
            if(sliderLambda1_->isVisible())
            {
                sliderLambda1_->setVisible(false);
            }
            if(labelLambda2_->isVisible())
            {
                labelLambda2_->setVisible(false);
            }
            if(sliderLambda2_->isVisible())
            {
                sliderLambda2_->setVisible(false);
            }
            
            if (btnShowNextDefOption_->isVisible())
            {
                btnShowNextDefOption_->setVisible(false);
            }
            
            if (!btnSelectCluster_->isVisible())
            {
                btnSelectCluster_->setVisible(true);
            }
        }
            break;
        case SHOW_REPRESENTATIVE:
        {
            // Hide the widgets related to show lerp
            if (labelLerp_->isVisible())
            {
                labelLerp_->setVisible(false);
            }
            
            if (btnResetLerp_->isVisible())
            {
                btnResetLerp_->setVisible(false);
                slotResetLerp(true);
                btnResetLerp_->setChecked(false);
                btnResetLerp_->setText("Start selecting shapes");
            }
            if(sliderLerp_->isVisible())
            {
                sliderLerp_->setVisible(false);
                //sliderLerp_->setEnabled(false);
            }
            
            if (btnSelectCluster_->isVisible())
            {
                btnSelectCluster_->setVisible(false);
            }
            
            if (btnShowNextDefOption_->isVisible())
            {
                btnShowNextDefOption_->setVisible(false);
            }
            
            // Hide the widgets related to the rest of the modes
            if(labelLambda1_->isVisible())
            {
                labelLambda1_->setVisible(false);
            }
            if(sliderLambda1_->isVisible())
            {
                sliderLambda1_->setVisible(false);
            }
            if(labelLambda2_->isVisible())
            {
                labelLambda2_->setVisible(false);
            }
            if(sliderLambda2_->isVisible())
            {
                sliderLambda2_->setVisible(false);
            }
        }
            break;
        case SHOW_DEFORMED_OPTIONS:
        {
            // Hide the widgets related to show lerp
            if (labelLerp_->isVisible())
            {
                labelLerp_->setVisible(false);
            }
            
            if (btnResetLerp_->isVisible())
            {
                btnResetLerp_->setVisible(false);
                slotResetLerp(true);
                btnResetLerp_->setChecked(false);
                btnResetLerp_->setText("Start selecting shapes");
            }
            
            if(sliderLerp_->isVisible())
            {
                sliderLerp_->setVisible(false);
                //sliderLerp_->setEnabled(false);
            }
            
            // Hide the widgets related to select cluster
            if (btnSelectCluster_->isVisible())
            {
                btnSelectCluster_->setVisible(false);
            }
            
            // Hide the widgets related to the rest of the modes
            if(labelLambda1_->isVisible())
            {
                labelLambda1_->setVisible(false);
            }
            if(sliderLambda1_->isVisible())
            {
                sliderLambda1_->setVisible(false);
            }
            if(labelLambda2_->isVisible())
            {
                labelLambda2_->setVisible(false);
            }
            if(sliderLambda2_->isVisible())
            {
                sliderLambda2_->setVisible(false);
            }

            if (!btnShowNextDefOption_->isVisible())
            {
                btnShowNextDefOption_->setVisible(true);
            }
        }
            break;
        case SHOW_GROUPS:
        {
            if (btnBack_->isVisible())
            {
                btnBack_->setVisible(false);
            }
            
            if(labelLambda1_->isVisible())
            {
                labelLambda1_->setVisible(false);
            }
            if(sliderLambda1_->isVisible())
            {
                sliderLambda1_->setVisible(false);
            }
            if(labelLambda2_->isVisible())
            {
                labelLambda2_->setVisible(false);
            }
            if(sliderLambda2_->isVisible())
            {
                sliderLambda2_->setVisible(false);
            }

            if (chkRecalculateBoxes_->isVisible())
            {
                chkRecalculateBoxes_->setVisible(false);
            }
            
            if (btnShowNextDefPartOption_->isVisible())
            {
                btnShowNextDefPartOption_->setVisible(false);
            }
            
            if (btnShowPreviousDefPartOption_->isVisible())
            {
                btnShowPreviousDefPartOption_->setVisible(false);
            }
            
            if (nnLabel_->isVisible())
            {
                nnLabel_->setVisible(false);
            }
            
            if (lnEdtNofNN_->isVisible())
            {
                lnEdtNofNN_->setVisible(false);
            }
            
            if (comboBoxDisplayMode_->isVisible())
            {
                comboBoxDisplayMode_->setVisible(false);
            }
            
            if (comboBoxExplorationMode_->isVisible())
            {
                comboBoxExplorationMode_->setVisible(false);
            }
            
            if (lnPointX_->isVisible())
            {
                lnPointX_->setVisible(false);
            }
            
            if (lnPointY_->isVisible())
            {
                lnPointY_->setVisible(false);
            }
            
            if (btnChangeSelectedPoint_->isVisible())
            {
                btnChangeSelectedPoint_->setVisible(false);
            }
            
            if (comboBoxNearestNeighbours_->isVisible())
            {
                comboBoxNearestNeighbours_->setVisible(false);
            }
            
            exploScenarioWidget_->setVisible(false);
        }
            break;
            // Same gui for the next 5 modes
        case SHOW_CLUSTERS:
        {
            
        }
           
        case SHOW_CLUSTER:
        {
            
        }
           
        case SHOW_DEFORMED_PART_OPTIONS:
        {
            
        }
            
        case SHOW_MATCHES_SUPERIMPOSED:
        {

        }
            
        case SHOW_PART_OPTIONS:
        {
            exploScenarioWidget_->setVisible(true);
            
            if (!btnBack_->isVisible())
            {
                btnBack_->setVisible(true);
            }
            
            if (!nnLabel_->isVisible())
            {
                nnLabel_->setVisible(true);
            }
            
            if (!lnEdtNofNN_->isVisible())
            {
                lnEdtNofNN_->setVisible(true);
            }
            
            if (!comboBoxDisplayMode_->isVisible())
            {
                comboBoxDisplayMode_->setVisible(true);
            }
            
            // For light debug and full debug modes
            if (DEBUG_MODE)
            {
                if(!labelLambda1_->isVisible())
                {
                    labelLambda1_->setVisible(true);
                }
                if(!sliderLambda1_->isVisible())
                {
                    sliderLambda1_->setVisible(true);
                }
                if(!labelLambda2_->isVisible())
                {
                    labelLambda2_->setVisible(true);
                }
                if(!sliderLambda2_->isVisible())
                {
                    sliderLambda2_->setVisible(true);
                }
                
                if (!comboBoxExplorationMode_->isVisible())
                {
                    comboBoxExplorationMode_->setVisible(true);
                }
                
                if (!lnPointX_->isVisible())
                {
                    lnPointX_->setVisible(true);
                }
                
                if (!lnPointY_->isVisible())
                {
                    lnPointY_->setVisible(true);
                }
                
                if (!btnChangeSelectedPoint_->isVisible())
                {
                    btnChangeSelectedPoint_->setVisible(true);
                }
                
                if (!comboBoxNearestNeighbours_->isVisible())
                {
                    comboBoxNearestNeighbours_->setVisible(true);
                }
                
                updateSliders();
            }

        }
            
        default:
        {
            
        }
            break;
    }
    
}


void TemplateExplorationWidget::updatePlot(int _explorationMode)
{
    
    qDebug() << "Updating plot.." ;
    switch (_explorationMode)
	{
        case SHOW_TEMPLATE:
        {
            if (dataState_.templateValid && CREATE_QWTPLOTW)
            {
                emit deletePlotMarkers();
                
                const OpenMesh::Vec2f& tmPos = templateMatch_.descriptor2D();
                
                // The template match doesn't really have an index so pass -1 for its index to seperate it from other plot markers
                emit addPlotMarker(QPointF(tmPos[0],tmPos[1]), QColor(255,0,0), -1);
            }
        }
            break;
        case SHOW_SELECTED:
        {
            if (selectedMatchIndex_>=0 && CREATE_QWTPLOTW)
            {
                emit deletePlotMarkers();
                
                const OpenMesh::Vec2f& tmPos = templateMatch_.descriptor2D();
                
                // The template match doesn't really have an index so pass -1 for its index to seperate it from other plot markers
                emit addPlotMarker(QPointF(tmPos[0],tmPos[1]), QColor(255,0,0), -1);

                Match& sMatch = *filteredMatches_.at(selectedMatchIndex_);
                const OpenMesh::Vec2f& smPos = sMatch.descriptor2D();
                
                emit addPlotMarker(QPointF(smPos[0],smPos[1]), QColor(0,0,255), selectedMatchIndex_);
            }
        }
            break;
        case SHOW_NEAREST:
        {
            if (CREATE_QWTPLOTW)
            {
                emit deletePlotMarkers();
                
                const OpenMesh::Vec2f& tmPos = templateMatch_.descriptor2D();
                
                // The template match doesn't really have an index so pass -1 for its index to seperate it from other plot markers
                emit addPlotMarker(QPointF(tmPos[0],tmPos[1]), QColor(255,0,0), -1);
                
                int nearestMatchIndex = nearestPoints_[nearestNeighbourLevel_].index_;
                
                if (nearestMatchIndex>=0 && nearestMatchIndex<filteredMatches_.size())
                {
                    Match& sMatch = *filteredMatches_.at(nearestMatchIndex);
                    
                    const OpenMesh::Vec2f& smPos = sMatch.descriptor2D();
                
                    emit addPlotMarker(QPointF(smPos[0],smPos[1]), QColor(0,255,0), nearestMatchIndex);
                }
                else
                {
                    qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
                }
            }
        }
            break;
        case SHOW_DEFORMED:
        {
            if( dataState_.deformedNearestValid[nearestNeighbourLevel_] && CREATE_QWTPLOTW)
            {
                emit deletePlotMarkers();
                
                const OpenMesh::Vec2f& tmPos = templateMatch_.descriptor2D();
                
                // The template match doesn't really have an index so pass -1 for its index to seperate it from other plot markers
                emit addPlotMarker(QPointF(tmPos[0],tmPos[1]), QColor(255,0,0), -1);
                
                int nearestMatchIndex = nearestPoints_[nearestNeighbourLevel_].index_;
                
                if (nearestMatchIndex>=0 && nearestMatchIndex<filteredMatches_.size())
                {
                    Match& sMatch = *filteredMatches_.at(nearestMatchIndex);
                    
                    const OpenMesh::Vec2f& smPos = sMatch.descriptor2D();
                    
                    emit addPlotMarker(QPointF(smPos[0],smPos[1]), QColor(0,255,0), nearestMatchIndex);
                }
                else
                {
                    qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
                }
            }
        }
            break;
        case SHOW_DEFORMED_SUPERIMPOSED:
        {
            if (CREATE_QWTPLOTW)
            {                
                emit deletePlotMarkers();
                
                const OpenMesh::Vec2f& tmPos = templateMatch_.descriptor2D();
                
                // The template match doesn't really have an index so pass -1 for its index to seperate it from other plot markers
                emit addPlotMarker(QPointF(tmPos[0],tmPos[1]), QColor(255,0,0), -1);
                
                for (int i=0; i<=nearestNeighbourLevel_; ++i)
                {
                    if (dataState_.deformedNearestValid[i])
                    {
                        int nearestMatchIndex = nearestPoints_[i].index_;
                        
                        if (nearestMatchIndex>=0 && nearestMatchIndex< filteredMatches_.size())
                        {
                            Match& sMatch = *filteredMatches_.at(nearestMatchIndex);
                            const OpenMesh::Vec2f& smPos = sMatch.descriptor2D();
                            
                            emit addPlotMarker(QPointF(smPos[0],smPos[1]), QColor(0,255,0), nearestMatchIndex);
                        }
                        else
                        {
                            qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
                        }
                    }
                }
            }
        }
            break;
        case SHOW_LERP:
        {
            if (CREATE_QWTPLOTW)
            {
                emit deletePlotMarkers();
                
                // Add the markers for each of the selected shapes
                std::vector<int>::iterator itLerp(lerpIndices_.begin()), lerpEnd(lerpIndices_.end());
                
                for(; itLerp!=lerpEnd; ++itLerp )
                {
                    Match& cMatch = *filteredMatches_[*itLerp];
                    const OpenMesh::Vec2f& pos = cMatch.descriptor2D();
                    emit addPlotMarker(QPointF(pos[0],pos[1]), tewColors[*itLerp],*itLerp);
                }

                const OpenMesh::Vec2f& tmPos = templateMatch_.descriptor2D();
                
                // The template match doesn't really have an index so pass -1 for its index to seperate it from other plot markers
                emit addPlotMarker(QPointF(tmPos[0],tmPos[1]), QColor(255,0,0), -1);
              
            }
            
        }
            break;
        case SELECT_CLUSTER:
        {
            if (CREATE_QWTPLOTW)
            {
                emit deletePlotMarkers();
                if (clusterBox_.first != OpenMesh::Vec2f(-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max()))
                {
                    //emit deletePlotMarker(-2);
                    emit addPlotMarker(QPointF(clusterBox_.first[0],clusterBox_.first[1]), QColor(0,0,0), -2);
                }
                if (clusterBox_.second != OpenMesh::Vec2f(-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max()))
                {
                    //emit deletePlotMarker(-3);
                    emit addPlotMarker(QPointF(clusterBox_.second[0],clusterBox_.second[1]), QColor(0,0,0), -3);
                }
                qDebug() << "First: " << clusterBox_.first[0] << " " << clusterBox_.first[1] ;
                qDebug() << "Second: " << clusterBox_.second[0] << " " << clusterBox_.second[1] ;
            }

        }
            break;
        case SHOW_REPRESENTATIVE:
        {
            if (CREATE_QWTPLOTW)
            {
                emit deletePlotMarkers();
                
                const OpenMesh::Vec2f& tmPos = templateMatch_.descriptor2D();
                
                // The template match doesn't really have an index so pass -1 for its index to seperate it from other plot markers
                emit addPlotMarker(QPointF(tmPos[0],tmPos[1]), QColor(255,0,0), -1);
                
                for (int i=0; i<5; ++i)
                {
                    if (representativeIndex_[i]>0)
                    {
                        Match* representative = filteredMatches_.at(representativeIndex_[i]);
                        const OpenMesh::Vec2f& rmPos = representative->descriptor2D();
                        
                        emit addPlotMarker(QPointF(rmPos[0],rmPos[1]), tewColors[i], representativeIndex_[i]); // -4 index is for the representative model's dot?
                    }
                }
            }
            
        }
            break;
            
        case SHOW_DEFORMED_OPTIONS:
        {
            if (CREATE_QWTPLOTW)
            {
                emit deletePlotMarkers();
                
                const OpenMesh::Vec2f& tmPos = templateMatch_.descriptor2D();
                
                // The template match doesn't really have an index so pass -1 for its index to seperate it from other plot markers
                emit addPlotMarker(QPointF(tmPos[0],tmPos[1]), QColor(255,0,0), -1);
                if (dataState_.deformedNearestOptionsValid)
                {
                    for (int i=0; i<nofNN_; ++i)
                    {
                        int nearestMatchIndex = nearestPoints_[i].index_;
                        
                        if (nearestMatchIndex>=0 && nearestMatchIndex< filteredMatches_.size())
                        {
                            Match& sMatch = *filteredMatches_.at(nearestMatchIndex);
                            
                            const OpenMesh::Vec2f& smPos = sMatch.descriptor2D();
                            
                            emit addPlotMarker(QPointF(smPos[0],smPos[1]), QColor(0,255,0), nearestMatchIndex);
                        }
                        else
                        {
                            qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
                        }
                        
                    }
                }
            }
        }
            break;
        case SHOW_GROUPS:
        {
            
        }
            break;
        case SHOW_CLUSTERS:
        {
            QList<QGraphicsItem *> lst = scene_->items();
            int lstSize = lst.size();
            if (lstSize==0)
            {
                return;
            }
            
            if (dataState_.templateValid)
            {

                
                if(templateMarker_)
                {
                    templateMarker_->setPos(QPointF(selectedPoint_[0],-selectedPoint_[1]));
                }
            }

        }
            break;
        case SHOW_CLUSTER:
        {

            if (dataState_.templateValid)
            {

                if(templateMarker_)
                {
                    templateMarker_->setPos(QPointF(selectedPoint_[0],-selectedPoint_[1]));
                }

            }

        }
            break;
        // Same behaviour for the following three modes
        case SHOW_DEFORMED_PART_OPTIONS:
        {
            
        }
        case SHOW_MATCHES_SUPERIMPOSED:
        {
            
        }
        case SHOW_PART_OPTIONS:
        {
            if (dataState_.templateValid)
            {
                if(templateMarker_)
                {
                    templateMarker_->setPos(QPointF(selectedPoint_[0],-selectedPoint_[1]));
                }
            }
        }
            break;
            
        default:
        {
            
        }
            break;
	}    
}

void TemplateExplorationWidget::updateViewport(int _explorationMode)
{
    qDebug() << "Updating viewport.." ;
    std::vector<Shape*> shapes;
    
	switch (_explorationMode)
	{
        case SHOW_TEMPLATE:
        {
            if (dataState_.templateValid)
            {
                shapes.push_back(&templateMatch_);
            }
        }
            break;
        case SHOW_SELECTED:
        {
            if(selectedMatchIndex_>=0)
            {
                Match* cMatch = filteredMatches_.at(selectedMatchIndex_);
                // Open the mesh if we are in show mesh display mode
                if(displayMode_ & MESH)
                {
                    cMatch->openMeshIfNotOpened();
                }
                shapes.push_back(cMatch);
            }
        }
            break;
        case SHOW_NEAREST:
        {
            int nearestMatchIndex = nearestPoints_[nearestNeighbourLevel_].index_;
            
            if (nearestMatchIndex>=0 && nearestMatchIndex< filteredMatches_.size())
            {
                Match* cMatch = filteredMatches_.at(nearestMatchIndex);
                // Open the mesh if we are in show mesh display mode
                if(displayMode_ & MESH)
                {
                    cMatch->openMeshIfNotOpened();
                }
                shapes.push_back(cMatch);
            }
            else
            {
                qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
            }
        }
			break;
        case SHOW_DEFORMED:
        {
            if( dataState_.deformedNearestValid[nearestNeighbourLevel_])
            {
                shapes.push_back(&deformedNearestMatches_[nearestNeighbourLevel_]);
            }
        }
			break;
        case SHOW_DEFORMED_SUPERIMPOSED:
        {
            for (int i=0; i <= nearestNeighbourLevel_; ++i)
            {
                if(dataState_.deformedNearestValid[i])
                {
                    shapes.push_back(&deformedNearestMatches_[i]);
                }
            }
        }
            break;
        case SHOW_LERP:
        {
            if(nearestPoints_[0].distance_<0.0001)
            {
                int nearestMatchIndex = nearestPoints_[0].index_;
                
                if (nearestMatchIndex>=0 && nearestMatchIndex< filteredMatches_.size())
                {
                    Match* cMatch = filteredMatches_.at(nearestMatchIndex);
                    cMatch->setDisplayMode(MESH);
                    cMatch->openMeshIfNotOpened();
                    shapes.push_back(cMatch);
                }
                else
                {
                    qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
                }
            }
            else
            {
                if (dataState_.templateValid)
                {
                    shapes.push_back(&templateMatch_);
                }
            }
        }
            break;
            
        case SHOW_REPRESENTATIVE:
        {
            if (representativeIndex_[0]>0)
            {
                Match* representativeCenter = filteredMatches_.at(representativeIndex_[0]);
                if(displayMode_ & MESH)
                {
                    representativeCenter->openMeshIfNotOpened();
                }
                shapes.push_back(representativeCenter);
                if (CREATE_MVW && NOF_MVW>=5)
                {
                    emit selectedShapesChanged(shapes,0);
                }
                shapes.clear();
            }
            if (representativeIndex_[1]>0)
            {
                Match* representativeLeft = filteredMatches_.at(representativeIndex_[1]);
                if(displayMode_ & MESH)
                {
                    representativeLeft->openMeshIfNotOpened();
                }
                shapes.push_back(representativeLeft);
                if (CREATE_MVW && NOF_MVW>=5)
                {
                    emit selectedShapesChanged(shapes,1);
                }
                shapes.clear();
            }
            if (representativeIndex_[2]>0)
            {
                Match* representativeRight = filteredMatches_.at(representativeIndex_[2]);
                if(displayMode_ & MESH)
                {
                    representativeRight->openMeshIfNotOpened();
                }
                shapes.push_back(representativeRight);
                if (CREATE_MVW && NOF_MVW>=5)
                {
                    emit selectedShapesChanged(shapes,2);
                }
                shapes.clear();
            }
            if (representativeIndex_[3]>0)
            {
                Match* representativeTop = filteredMatches_.at(representativeIndex_[3]);
                if(displayMode_ & MESH)
                {
                    representativeTop->openMeshIfNotOpened();
                }
                shapes.push_back(representativeTop);
                if (CREATE_MVW && NOF_MVW>=5)
                {
                    emit selectedShapesChanged(shapes,3);
                }
                shapes.clear();
            }
            if (representativeIndex_[4]>0)
            {
                Match* representativeBottom = filteredMatches_.at(representativeIndex_[4]);
                if(displayMode_ & MESH)
                {
                    representativeBottom->openMeshIfNotOpened();
                }
                shapes.push_back(representativeBottom);
                if (CREATE_MVW && NOF_MVW>=5)
                {
                    emit selectedShapesChanged(shapes,4);
                }
                shapes.clear();
            }
            // Normally should not return, but otherwise we will call emit selectedShapesChanged with no shapes inside which is connected to one of the 5 mesh viewer widgets (the one showing top) so it will reset the shape shown
            return;
        }
            break;
        case SHOW_DEFORMED_OPTIONS:
        {
            if( dataState_.deformedNearestOptionsValid)
            {
                shapes.push_back(&deformedNearestOption_);
            }
            else
            {
                qDebug() << "Cannot display match, deformed nearest options is not valid!" ;
            }
        }
            break;
        case SHOW_GROUPS:
        {
            
        }
            break;
        // Same bahaviour for the following three modes
        case SHOW_CLUSTERS:
        {

            
        }
            
        case SHOW_CLUSTER:
        {

        }
            
        case SHOW_DEFORMED_PART_OPTIONS:
        {
            if( dataState_.deformedPartOptionsValid)
            {
                shapes.push_back(&deformedNearestOption_);
            }
            else
            {
                qDebug() << "Cannot display match, deformed nearest part options is not valid!" ;
            }
        }
            break;
        case SHOW_MATCHES_SUPERIMPOSED:
        {
            for (int i=0; i <= nearestNeighbourLevel_; ++i)
            {
                if(dataState_.deformedNearestValid[i])
                {
                    Match* nMatch = filteredMatches_.at(randomNeighbourIndices_[i]);
                    
                    if (displayMode_ & MESH)
                    {
                        nMatch->openMeshIfNotOpened();
                    }
                    
                    shapes.push_back(nMatch);
                }
            }
        }
            break;
        case SHOW_PART_OPTIONS:
        {
            if( dataState_.deformedPartOptionsValid)
            {
                shapes.push_back(&deformedNearestOption_);
            }
            else
            {
                qDebug() << "Cannot display match, deformed nearest part options is not valid!" ;
            }
        }
            break;
        case SHOW_RANDOM:
        {
            if (randomNeighbourIndices_[nearestNeighbourLevel_]>=0)
            {
                Match* cMatch = filteredMatches_.at(randomNeighbourIndices_[nearestNeighbourLevel_]);
                // Open the mesh if we are in show mesh display mode
                if(displayMode_ & MESH)
                {
                    cMatch->openMeshIfNotOpened();
                }
                shapes.push_back(cMatch);
            }
        }
			break;
        default:
        {
            
        }
            break;
	}
    
    if (CREATE_MVW)
    {
        emit selectedShapesChanged(shapes,0);
    }
}


void TemplateExplorationWidget::slotResetLerp(bool _checked)
{
    
    if (_checked)
    {
        if (CREATE_QWTPLOTW)
        {
            emit deletePlotMarkers();
        
            const OpenMesh::Vec2f& tmPos = templateMatch_.descriptor2D();
        
            // The template match doesn't really have an index so pass -1 for its index to seperate it from other plot markers
            emit addPlotMarker(QPointF(tmPos[0],tmPos[1]), QColor(255,0,0), -1);
        }
        //std::vector<int>::iterator itLerp(lerpIndices_.begin()), lerpEnd(lerpIndices_.end());
        
        // Delete the markers first
        //for(; itLerp!=lerpEnd; ++itLerp )
        //{
        //    emit deletePlotMarker(*itLerp);
        //}
        // Now delete all the indices
        lerpIndices_.clear();
        
        sliderLerp_->blockSignals(true);
        sliderLerp_->setEnabled(false);
        sliderLerp_->setValue(0);
        sliderLerp_->blockSignals(false);
        
        btnResetLerp_->setText("Done");
    }
    else
    {
        btnResetLerp_->setText("Start selecting shapes");
        sliderLerp_->setEnabled(true);
    }
}

void TemplateExplorationWidget::slotSelectCluster(bool _checked)
{
    qDebug() << "Selecting cluster.." ;
    
    qDebug() << "First: " << clusterBox_.first[0] << " " << clusterBox_.first[1] ;
    qDebug() << "Second: " << clusterBox_.second[0] << " " << clusterBox_.second[1] ;
    
    OpenMesh::Vec2f min(0,0), max(0,0);
    
    if (clusterBox_.first[0] > clusterBox_.second[0])
    {
        max[0] =clusterBox_.first[0];
        min[0] = clusterBox_.second[0];
    }
    else
    {
        max[0] = clusterBox_.second[0];
        min[0] = clusterBox_.first[0];
    }
    
    if (clusterBox_.first[1] > clusterBox_.second[1])
    {
        max[1] =clusterBox_.first[1];
        min[1] = clusterBox_.second[1];
    }
    else
    {
        max[1] = clusterBox_.second[1];
        min[1] = clusterBox_.first[1];
    }
    qDebug() << "Min: " << min[0] << " " << min[1] ;
    qDebug() << "Max: " << max[0] << " " << max[1] ;
    
    std::vector<Match*> newFilteredMatches;
    
    std::vector<Match*>::iterator itMatches(filteredMatches_.begin()), matchesEnd(filteredMatches_.end());
    //numMatches_ = 0;
    for (; itMatches!=matchesEnd; ++itMatches)
    {
        const OpenMesh::Vec2f& pos = (**itMatches).descriptor2D();
        
        if (pos[0] >= min[0] && pos[0] <= max[0] && pos[1]>= min[1] && pos[1] <= max[1])
        {
            newFilteredMatches.push_back(*itMatches);
            qDebug() << "Selecting match with pos: " << pos[0] << " " << pos[1] ;
            //numMatches_++;
        }
        
    }
    
    filteredMatches_.clear();
    filteredMatches_ = newFilteredMatches;
    
    int numParts = filteredMatches_[0]->nparts();
    
    if(calculatePCA())
    {
        setPlotPoints();
        
        if (CREATE_QWTPLOTW)
        {
            emit plotPointsChanged();
        }
        slotChangeSelectedPoint(0.0, 0.0);
        
        // Store the index of the representative model for the models in this cluster
        NEAREST_POINT* nearest = getNearestPoint(0.0, 0.0,1);
        representativeIndex_[0] = nearest[0].index_; // store center in index 0

    }
}

NEAREST_POINT* TemplateExplorationWidget::getNearestPoint(double _x, double _y, int _numNeighbors)
{
    int fmSize = filteredMatches_.size();
    
    if (fmSize == 0 || _numNeighbors > fmSize)
    {
        qCritical() << "Trying to build a tree with " << fmSize << " looking for " << _numNeighbors << " is not possible" ;
        NEAREST_POINT* nearestBad = new NEAREST_POINT[_numNeighbors];
        return nearestBad;
    }
    // Build the kd tree
	typedef VectorMatchDescriptor2DAdaptor< std::vector<Match*> > MD2KD;
    
	const MD2KD md2kd( filteredMatches_ ); // The adaptor
    
	// construct a kd-tree index:
	typedef KDTreeSingleIndexAdaptor<
    L2_Simple_Adaptor<OpenMesh::Vec2f::value_type, MD2KD > ,
    MD2KD,
    2 /* dim */
	> my_kd_tree_t;
    
	my_kd_tree_t kdTreeIndex(2 /*dim*/, md2kd, KDTreeSingleIndexAdaptorParams(10 /* max leaf */) );
	kdTreeIndex.buildIndex();
    
    typedef OpenMesh::Vec2f::value_type num_t;
    
    const size_t num_results = _numNeighbors;
    
    size_t nn_index[_numNeighbors];
    
    num_t nn_dist_sqr[_numNeighbors];
    
    OpenMesh::Vec2f p;
    p[0] = _x;
    p[1] = _y;
    
    // Find nearest point inside points
    nanoflann::KNNResultSet<num_t> resultSet(num_results);
    
    resultSet.init(nn_index, nn_dist_sqr);
    
    kdTreeIndex.findNeighbors(resultSet, &p[0], nanoflann::SearchParams(10));
    
    NEAREST_POINT* nearest = new NEAREST_POINT[_numNeighbors];
    
    for (int i=0 ; i < _numNeighbors; ++i)
    {
        nearest[i].index_ = nn_index[i];
        nearest[i].distance_ = nn_dist_sqr[i];
    }
    
    return nearest;
}

void TemplateExplorationWidget::slotReadLabels()
{
    
    // Ask for a directory
    QString directory_path = QFileDialog::getExistingDirectory(0,"Classification directory", "../");
    
    if(directory_path.isNull())
    {
        qCritical() << "The directory path was null";
        return;
    }
    
    QStringList nameFilters;
    
    nameFilters << "*.jpg" << "*.JPG";
    
    QDir classification_dir(directory_path);
    
    QStringList dirNames = classification_dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot,QDir::Name);
    
    std::map<QString, int> dirNames2Labels;
    
    QList<QString>::iterator dirNamesIt(dirNames.begin()),dirNamesEnd(dirNames.end());
    
    for (int i=0; dirNamesIt!=dirNamesEnd; ++dirNamesIt)
    {
        qDebug() << *dirNamesIt << "->" << i;
        dirNames2Labels[*dirNamesIt] = i;
        i++;
    }
    
    //The directory iterator will return all .jpg files, including in any subdirectories
    QDirIterator directory_walker(directory_path, nameFilters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    
    int count = 0;
    int ignoreCount = 0;
    while(directory_walker.hasNext())
    {
        QString fname = directory_walker.next();
        QFileInfo qfi = directory_walker.fileInfo();
        
        if(!fname.isEmpty())
        {
            QString shapeName = fname.split("/").last().split(".").first();
            
            if(matchNameToMatchIndex_.count(shapeName.toStdString())!=1)
            {
                qWarning() << "Ignoring match name: " << shapeName << " because it was not found in the collection!" ;
                ignoreCount++;
                continue;
            }
            Match& cMatch = *matches_[ matchNameToMatchIndex_[shapeName.toStdString()] ];
            
            int label = dirNames2Labels[qfi.dir().dirName()];
            
            cMatch.setLabel(label);
            qDebug() << "Match " << shapeName << " got the label " << label ;
            count++;
        }
    }
    qDebug() << "Labels loaded (# of entries: " << count << ")" ;
    qDebug() << "Ignored " << ignoreCount << " matches that were not found in the loaded collection" ;
    
    std::vector<Match*>::iterator matchesIt(matches_.begin()), matchesEnd(matches_.end());
    count =0;
    for (; matchesIt!=matchesEnd; ++matchesIt)
    {
        if ((**matchesIt).label()==-1)
        {
            qDebug() << "Match with name: " << (**matchesIt).filename().split("/").last() << " did not get a label!";
            count++;
        }
    }
    qDebug() << count << " matches did not get a label";
}


void TemplateExplorationWidget::slotSplitMatches()
{
    std::vector<Match*>::iterator matchesIt(matches_.begin()), matchesEnd(matches_.end());
    
    for (; matchesIt!=matchesEnd; ++matchesIt)
    {
        (**matchesIt).openMeshIfNotOpened();
        
        (**matchesIt).split();
    
        (**matchesIt).savePartMeshes();

    }
    
    return;
    
    // Split nearest neighbours according to the deformed template
	for (int i = 0; i < nofNN_; ++i)
	{
        int nearestMatchIndex = nearestPoints_[i].index_;
    
        if (nearestMatchIndex>=0 && nearestMatchIndex< filteredMatches_.size())
        {            
            Match& nearestMatch = *filteredMatches_.at(nearestMatchIndex);
            
            nearestMatch.openMeshIfNotOpened();
            
            nearestMatch.split();
        }
        else
        {
            qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
        }
        
        // Recalculate the boxes of all the nearest neighbors, given the new segmentations

        //slotRecalculatePartBoxes(nearestMatch);
        
	}

    if (CREATE_MVW)
    {
        emit requestRedraw(0);
    }
}

void TemplateExplorationWidget::deformNearestMatches(int _numNeighbors)
{
    
    xform ident;
    
    deformedNearestOption_.setAlignMtx(ident);

    deformedNearestOption_.setGroupID(selectedGroupID_);

    deformedNearestOption_.setNparts(templateMatch_.nparts());

    deformedNearestOption_.setTemplateID(selectedTemplateID_);
    
    deformedNearestOption_.setDescriptor2D(templateMatch_.descriptor2D());
    
    deformedNearestOption_.parts().clear();
    
    deformedNearestOption_.setParts(templateMatch_.parts());
    
    // We need to set the segmented property to true in order to render the individual meshes taken from each part, otherwise the renderer will try to render the deformed option's mesh, which is empty
    deformedNearestOption_.setSegmented(true);
    
    deformedNearestOption_.setDisplayMode(displayMode_);
    
    // Go over the deformed option's parts (taken from template match) to see which ones have part type!=0. Those that do, will be our parts
    std::vector<Match::Part>& parts = deformedNearestOption_.parts();
    
    int nparts = 0;
    
    std::map<int, int> partID2partIndex;
    
    typename std::vector<Match::Part>::const_iterator partscIt (parts.begin()), partscEnd(parts.end());
    
    int index =0;
    
    for (; partscIt != partscEnd; ++partscIt)
    {
        if (partscIt->partType_!=0)
        {
            partID2partIndex[nparts] = index;
            nparts++;
        }
        index++;
    }
    
    deformedNearestOption_.points().clear();
    
    // For every valid part (remember we may have say 8 parts but only 6 are valid), choose which neighbor we should select it from
    for (int i=0; i<nparts; ++i)
    {
        int neighborIndex = -1;
        
        if(chkNN_->isChecked())
        {
            neighborIndex = nearestNeighbourLevel_;
        }
        else
        {
            neighborIndex = rand() % _numNeighbors;
        }
        
        int partIndex = partID2partIndex[i];
        int partID = parts[ partIndex ].partID_;
       
        std::cout << "Part " << i << " with part ID " << partID << " and part index " << partIndex << " was chosen from neighbor " << neighborIndex << std::endl;
        
        // Get the template part which has the part ID we want
        Match::Part& tmcPart = parts [ partIndex];
        
        // Get the neighbor according to the random index we produced
        int nearestMatchIndex = nearestPoints_[neighborIndex].index_;
        
        if (nearestMatchIndex>=0 && nearestMatchIndex< filteredMatches_.size())
        {
            Match& nearestMatch = *filteredMatches_.at(nearestMatchIndex);
        
            // Calling openMeshIfNotOpened is not needed anymore since the deformNearestPart will just load the part mesh it needs (and only that - not the original mesh and not any other part meshes)
            // If it fails to load the mesh (perhaps didn't find it in the expected location), it will call split on the original mesh to split it using the naive approach
            //nearestMatch.openMeshIfNotOpened(); 
            
            deformNearestPart(tmcPart, nearestMatch);
        }
        else
        {
            qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
        }
    }
    deformedNearestOption_.setNpnts(deformedNearestOption_.points().size());

}

// Given the template part (deformed option object has already got its parts from the deformed template) we want, and the nearest neighbour from which to get the corresponding part (i.e. the part which has the same part ID), deform the nearest neighbour's mesh and points to match the template part
bool TemplateExplorationWidget::deformNearestPart(Match::Part& tmcPart, Match& nearestMatch)
{
    qDebug() << "Calling deform nearest part" ;
    
    std::vector<Match::Part>::iterator itPart(nearestMatch.parts().begin()), partEnd(nearestMatch.parts().end());
    
    //int nmPartIndex = -1;
    
    // We now need to find the neighbor's part which has the same part ID with the part we want to deform
    for (int j=0; itPart != partEnd; ++itPart, ++j)
    {
        Match::Part& nmcPart = *itPart;
        
        if (nmcPart.partID_ == tmcPart.partID_)
        {
            // We need the part's mesh now, so try to open it, if we fail, then segment the mesh using the naive approach
            if(!nearestMatch.openPartMeshIfNotOpened(nmcPart))
            {
                //Open the original mesh first and then split
                nearestMatch.openMeshIfNotOpened();
                nearestMatch.split();
            }
            Shape::Mesh& nMesh = nmcPart.partShape_.mesh();
            Shape::Mesh& tMesh = tmcPart.partShape_.mesh();
            
            if(nMesh.n_vertices() == 0)
            {
                //qWarning() << "Nearest match " << (nearestMatch.meshFilename()+QString(".p%1.off").arg(nmcPart.partID_)) << " has 0 vertices";
                std::cout << "failed to load part for deforming" << std::endl;
                return false;
            }
            
            tMesh = nMesh;
            
            // Deform the part's mesh vertices using the boxes of the parts
            typename TriangleMesh::VertexIter vIt(tMesh.vertices_begin()), vEnd(tMesh.vertices_end());
            
            for(; vIt!=vEnd; ++vIt)
            {
                OpenMesh::Vec3f& p_i = tMesh.point(vIt);
                
                OpenMesh::Vec3f scaleFactor = tmcPart.scale_ / nmcPart.scale_;
                OpenMesh::Vec3f transToOrigin = mv(nearestMatch.alignMtx(), p_i) - nmcPart.pos_;
                OpenMesh::Vec3f scaleToTemplate = transToOrigin * scaleFactor;
                
                OpenMesh::Vec3f newpoint = scaleToTemplate + tmcPart.pos_;
                OpenMesh::Vec3f newpointf(newpoint[0],newpoint[1],newpoint[2]);
                
                tMesh.set_point(vIt, newpointf);
            }
            
            tMesh.request_face_normals();
            tMesh.request_vertex_normals();
            tMesh.update_face_normals();
            tMesh.update_vertex_normals();
            
            break;
        }
        
    }
    
    return true;
}

void TemplateExplorationWidget::slotShowNextDeformationOption()
{
    if(chkNN_->isChecked())
    {
        deformNearestMatches(1);
    }
    else
    {
        deformNearestMatches(nearestNeighbourLevel_+1);
    }
    updateViewport(explorationMode_);
}


void TemplateExplorationWidget::slotSaveMatchNamesToFile()
{
    // Ask for a filename
    QString filename = QFileDialog::getSaveFileName(0,"Choose a filename", "../");
    
    QFile file(filename);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical() << "Could not open file " << filename;
        return;
    }
    
    QTextStream out(&file);
    
    std::vector<Match*>::iterator itMatch(filteredMatches_.begin()), itMatchEnd(filteredMatches_.end());
    
    for (; itMatch != itMatchEnd; ++itMatch)
    {
        QString fname = (**itMatch).filename().split("/").last();
        
        out << fname.split(".").first().append(".jpg") << "\n";
    }
    
}


void TemplateExplorationWidget::slotResetIView(int i)
{
    qDebug() << "iview set to -1";
    
    iview = -1;
}

void TemplateExplorationWidget::slotPreservePointScales(float _viewScale)
{
    QList<QGraphicsItem*> items = scene_->items();
    std::cout << "new scale is " << _viewScale << std::endl;
    for (int i=0; i<items.size(); i++)
    {
        items[i]->setScale(1.0/ _viewScale);
    }
}

void TemplateExplorationWidget::slotChangeSelectedCluster(QListWidgetItem * current)
{
    if (!current)
    {
        return;
    }
    
    noDown_++;
    
    QString id = current->toolTip().split(" ").last();
    
    if (explorationMode_ == SHOW_GROUPS)
    {
        dataState_.representativeValid = false;
        slotSetGroupID(id);
        slotSetLabelID("-1");
        slotCalculatePCA();
        
        if (currentNumClusters_==1)
        {
            slotChangeExplorationMode(SHOW_CLUSTER);
            //slotChangeExplorationMode(SHOW_DEFORMED_PART_OPTIONS);
        }
        else
        {
            slotChangeExplorationMode(SHOW_CLUSTERS);
            //slotChangeExplorationMode(SHOW_DEFORMED_PART_OPTIONS);
        }
    }
    else if (explorationMode_ == SHOW_CLUSTERS)
    {
        dataState_.representativeValid = false;
        slotSetLabelID(id);
        
        slotCalculatePCA();
        
        if (currentNumClusters_==1)
        {
            dataState_.representativeValid = false;
            dataState_.templateValid = false;
            slotChangeExplorationMode(SHOW_CLUSTER);
            //slotChangeExplorationMode(SHOW_DEFORMED_PART_OPTIONS);
        }
        else
        {
            slotChangeExplorationMode(SHOW_CLUSTERS);
            //slotChangeExplorationMode(SHOW_DEFORMED_PART_OPTIONS);
        }
    }
    
    updateClusterView();
    updateExplorationView();
    TIMELOG->append(QString("%1 : moved_down nof:%2").arg((qlonglong)QDateTime::currentMSecsSinceEpoch()).arg(currentNumClusters_));
}

void TemplateExplorationWidget::slotGoBack()
{
    noUp_++;
    
    filteredMatches_ = filteredMatchesHistory_.back();
    filteredMatchesHistory_.pop_back();
    
    dataState_.representativeValid = false;
    
    if (filteredMatchesHistory_.empty())
    {
        filteredMatches_.clear();
        setPlotPoints();
        slotChangeExplorationMode(SHOW_GROUPS);
    }
    else if (calculatePCA())
    {
        // We used to change the selected exploration mode but since both changing the exploration mode and the selected point call the same methods for updating the exploration data, the gui, the plot and the viewport, lets just change the selected point
        //explorationMode_ = SHOW_TEMPLATE;
        
        setPlotPoints();
        
        if (CREATE_QWTPLOTW)
        {
            emit plotPointsChanged();
        }
        
        //slotDeformKNearestMatches(0.0,0.0);
        
        //slotDeformTemplate(0.0, 0.0);
        
        // For lambda1=0 and lambda2=0, find the integers l1 and l2 in order to set the sliders value
        //l1 = (-pcaMin[0] * 200.0) / (pcaMax[0] - pcaMin[0]) - 100.0;
        //l2 = (-pcaMin[1] * 200.0) / (pcaMax[1] - pcaMin[1]) - 100.0;
        
        //emit Lambda1Changed(l1);
        
        //emit Lambda2Changed(l2);
        
		//emit explorationModeChanged(explorationMode_);
        slotChangeSelectedPoint(0.0, 0.0);
        
        // Store the index of the representative model for the models in this cluster
        //NEAREST_POINT* nearest = getNearestPoint(0.0, 0.0,1);
        //representativeIndex_[0] = nearest[0].index_; // store center in index 0
        
        if (currentNumClusters_==1)
        {
            slotChangeExplorationMode(SHOW_CLUSTER);
            //slotChangeExplorationMode(SHOW_DEFORMED_PART_OPTIONS);
        }
        else
        {
            slotChangeExplorationMode(SHOW_CLUSTERS);
            //slotChangeExplorationMode(SHOW_DEFORMED_PART_OPTIONS);
        }
    }
    updateClusterView();
    updateExplorationView();
    
    if (explorationMode_ == SHOW_GROUPS)
    {
        TIMELOG->append(QString("%1 : moved_up top_level").arg((qlonglong)QDateTime::currentMSecsSinceEpoch()));
    }
    else
    {
        TIMELOG->append(QString("%1 : moved_up nof:%2").arg((qlonglong)QDateTime::currentMSecsSinceEpoch()).arg(currentNumClusters_));
    }
    
}


void TemplateExplorationWidget::slotSetPickedPartIDShowNext(unsigned int _id)
{
    unsigned int previousPickedPartID = pickedPartID_;
    
    pickedPartID_ = _id;
    
//    if (explorationMode_!=SHOW_DEFORMED_PART_OPTIONS)
//    {
//        slotChangeExplorationMode(SHOW_DEFORMED_PART_OPTIONS);
//    }
    
    if (explorationMode_ == SHOW_DEFORMED_PART_OPTIONS || explorationMode_ == SHOW_CLUSTERS || explorationMode_ == SHOW_CLUSTER)
    {
//        if (previousPickedPartID != pickedPartID_)
//        {
//            rankNeighborPartsUnary(pickedPartID_);
//        }
        slotShowNextPartDeformationOption();
    }
    
    if (explorationMode_ == SHOW_PART_OPTIONS)
    {
        nnPartScoreVectorIndex_[pickedPartID_] = -1;
    }
}

void TemplateExplorationWidget::slotSetPickedPartIDShowPrevious(unsigned int _id)
{
    unsigned int previousPickedPartID = pickedPartID_;
    
    pickedPartID_ = _id;
    
    //    if (explorationMode_!=SHOW_DEFORMED_PART_OPTIONS)
    //    {
    //        slotChangeExplorationMode(SHOW_DEFORMED_PART_OPTIONS);
    //    }
    
    if (explorationMode_ == SHOW_DEFORMED_PART_OPTIONS || explorationMode_ == SHOW_CLUSTERS || explorationMode_ == SHOW_CLUSTER)
    {
        //        if (previousPickedPartID != pickedPartID_)
        //        {
        //            rankNeighborPartsUnary(pickedPartID_);
        //        }
        slotShowPreviousPartDeformationOption();
    }
    
    if (explorationMode_ == SHOW_PART_OPTIONS)
    {
        nnPartScoreVectorIndex_[pickedPartID_] = -1;
    }
}

void TemplateExplorationWidget::slotSetHoveredPartID(int _id)
{
    
    deformedNearestOption_.setHoveredPartID(_id);
}

void TemplateExplorationWidget::slotSetNofNN(const QString& _nofNN)
{
    int nofnn = _nofNN.toInt();
    
    if (nofnn <=0)
    {
        qWarning() << "Number of nearest neighbors should be at least 1. You entered: " << nofnn << " Defaulting to the previous number of neighbours" ;
        lnEdtNofNN_->blockSignals(true);
        lnEdtNofNN_->setText(QString("%1").arg(nofNN_));
        lnEdtNofNN_->blockSignals(false);
        //nofnn = 1;
        return;
    }
    
    if (nofnn > filteredMatches_.size())
    {
        qWarning() << "Number of nearest neighbors should be smaller than the number of shapes in the cluster. You entered: " << nofnn << " Defaulting to the previous number of neighbours" ;
        lnEdtNofNN_->blockSignals(true);
        lnEdtNofNN_->setText(QString("%1").arg(nofNN_));
        lnEdtNofNN_->blockSignals(false);
        //nofnn = 1;
        return;
    }
    
    if (nofNN_ == nofnn)
    {
        qWarning() << "New number of neighbours is the same as before, no change needed!" ;
        return;
    }
    
    nofNN_ = nofnn;
    
    // Get nearest neighbours
    // used to check a checkbox to use ann, now we just use it by default
    if(1) //chkBoxUseANN_->isChecked())
    {
        qDebug() << "Using ANN to find nearest neighbours";
        if (nearestPoints_)
        {
            delete [] nearestPoints_;
        }
        
        nearestPoints_ = getNearestPoint(selectedPoint_[0], selectedPoint_[1], nofNN_); //NUM_OF_NEAREST_NEIGHBOURS);
    }
	else
    {
        qDebug() << "Using exhaustive search to find nearest neighbours";
        
        if (nearestPoints_)
        {
            delete [] nearestPoints_;
        }
        
        nearestPoints_ = new NEAREST_POINT[nofNN_];
        
        for (int i = 0; i < nofNN_; ++i) //NUM_OF_NEAREST_NEIGHBOURS; ++i)
        {
            getNearestPoint(i, selectedPoint_[0], selectedPoint_[1]);
        }
    }
    
    dataState_.deformedPartOptionsValid = false;
    
    // Just to be safe, we delete the flags for the deformed nearest neighbour validity, re-allocate them and set them to false
    if (dataState_.deformedNearestValid)
    {
        delete [] dataState_.deformedNearestValid;
    }
    
    dataState_.deformedNearestValid = new bool[nofNN_];
    
    for (int i=0; i<nofNN_; ++i)
    {
        dataState_.deformedNearestValid[i] = false;
    }
    
    // Same for the deformed nearest neighbour match objects
    if (deformedNearestMatches_)
    {
        delete [] deformedNearestMatches_;
    }
    
    deformedNearestMatches_ = new Match[nofNN_];
    
    qDebug() << "NOF NN set to: " << nofNN_ ;
    
    comboBoxNearestNeighbours_->clear();
    
    for (int i = 0; i < nofNN_; ++i)
	{
		comboBoxNearestNeighbours_->addItem(QString::number(i));
	}
    
    randomNeighbourIndices_.clear();
    
    for (int i = 0 ; i< nofNN_; ++i)
    {
        randomNeighbourIndices_.push_back(rand() % filteredMatches_.size() );
    }
    randomNeighbourVectorIndex_ = 0;
    
    if (explorationMode_ == SHOW_CLUSTER)
    {
        slotChangeExplorationMode(SHOW_CLUSTER);
    }

    if (explorationMode_ == SHOW_CLUSTERS)
    {
        slotChangeExplorationMode(SHOW_CLUSTERS);
    }
    
    if (explorationMode_ == SHOW_DEFORMED_PART_OPTIONS)
    {
        slotChangeExplorationMode(SHOW_DEFORMED_PART_OPTIONS);
    }
    
    emit nnChanged(QString("No of Nearest Neighbours: %1").arg(nofNN_));

}


void TemplateExplorationWidget::rankNeighborPartsUnary(unsigned int _partID)
{
    std::vector< std::pair<int, double> >& nnScoreSorted = nnIndexPartScoreSorted_[_partID];
    
    nnScoreSorted.clear();
    
    //nnPartScoreVectorIndex_ = -1;
    
    if(_partID <=0)
    {
        qCritical() << "Picked part ID is " << _partID ;
        return;
    }
    // Unfortunately we only know the part ID of the currently selected part, so we need to go through the list of parts of the deformed template and locate the part with this part ID
    std::vector<Match::Part>::const_iterator tmPartIt(templateMatch_.parts().begin()), tmPartEnd(templateMatch_.parts().end());
    
    int tmPartIndex = 0;
    
    for (int j=0; tmPartIt != tmPartEnd; ++tmPartIt, ++j)
    {
        const Match::Part& tmcPart = *tmPartIt;
        
        if (tmcPart.partID_ == _partID)
        {
            break;
        }
        
        tmPartIndex++;
    }
    

    for (int i=0; i<nofNN_; ++i)
    {
        int nearestMatchIndex = nearestPoints_[i].index_;
        
        if (nearestMatchIndex<0 || nearestMatchIndex>=filteredMatches_.size())
        {
            qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
            nnScoreSorted.push_back(std::pair<int, double>(-1,std::numeric_limits<double>::max()));
            continue;
        }
        
        Match& nearestMatch = *filteredMatches_.at(nearestMatchIndex);
        
        std::vector<Match::Part>::iterator itPart(nearestMatch.parts().begin()), partEnd(nearestMatch.parts().end());
        
        // Unfortunately we only know the part ID of the currently selected part, so we need to go through the list of parts of the currently evaluated nearest neighbor and locate the part with this part ID
        for (int j=0; itPart != partEnd; ++itPart, ++j)
        {
            Match::Part& nmcPart = *itPart;
            
            if (nmcPart.partID_ == _partID)
            {
                // We now know both the template's part and the nearest neighbor's part, so we can find the unary score
                const Match::Part& tmPart = templateMatch_.parts().at(tmPartIndex);
                
                if (chkRecalculateBoxes_->isChecked())
                {
                    // We need the part's mesh now, so try to open it, if we fail, then segment the mesh using the naive approach
                    if(!nearestMatch.openPartMeshIfNotOpened(nmcPart))
                    {
                        // Open the original mesh first and then split
                        nearestMatch.openMeshIfNotOpened();
                        nearestMatch.split();
                    }
                    
                    nmcPart.recalculateBox(nearestMatch.alignMtx());
                }
                
                double score = partUnaryScore(tmPart,nmcPart);
                
                // Push the score and nearest neighbor index into a vector we can then sort according to score
                nnScoreSorted.push_back(std::pair<int, double>(nearestMatchIndex,score));
            }

        }
    }
    
    // Scores vector should have the same size as number of nearest neighbors
    if (nnScoreSorted.size()!=nofNN_)
    {
        qCritical() << "The vector of nearest neighbor indices sorted based on their unary score for part ID " << pickedPartID_ << " does not have the same size ( " << nnScoreSorted.size() << " ) as the currently selected number of nearest neighbors ( " << nofNN_ << " ). Resulting synthesis suggestions will be invalid!" ;
        nnScoreSorted.clear();
        return;
    }
    
    std::sort(nnScoreSorted.begin(),nnScoreSorted.end(), ComparePartScoresFunctor);
    
    qDebug() << "Currently selected point: " << selectedPoint_[0] << " , " << selectedPoint_[1] ;
    qDebug() << "Ranking neighbors for part id: " << _partID;
    
    for (int i=0; i< nnScoreSorted.size(); ++i)
    {
        int nearestMatchIndex = nnScoreSorted[i].first;
        
        if (nearestMatchIndex<0 || nearestMatchIndex>=filteredMatches_.size())
        {
            qDebug() << "Neighbor " << i << " with index " << nnScoreSorted[i].first << " , name INVALID" << " and score INVALID" << nnScoreSorted[i].second ;
        }
        else
        {
            qDebug() << "Neighbor " << i << " with index " << nnScoreSorted[i].first << " , name " << filteredMatches_.at(nearestMatchIndex)->filename().split("/").last() << " and score " << nnScoreSorted[i].second ;
        }
    }

}

// Calculate the unary score for two parts
// In the current setup, tmPart would have been chosen from the deformed template and nmcPart would have been chosen from the currently evaluated nearest neighbor to the deformed template, and they would of course have the same part ID to allow meaningful comparison
// The function returns the squared norm of the 6D descriptor calculated as the (min,max) coordinates of the boxes of the two parts
double TemplateExplorationWidget::partUnaryScore(const Match::Part& tmPart, const Match::Part& nmcPart)
{
    OpenMesh::Vec3f nmMin = nmcPart.pos_ - nmcPart.scale_;
    OpenMesh::Vec3f nmMax = nmcPart.pos_ + nmcPart.scale_;
    
    OpenMesh::Vec6d nmDescriptor(nmMin[0],nmMin[1],nmMin[2],nmMax[0],nmMax[1],nmMax[2]);
    
    OpenMesh::Vec3f tmMin = tmPart.pos_ - tmPart.scale_;
    OpenMesh::Vec3f tmMax = tmPart.pos_ + tmPart.scale_;
    
    OpenMesh::Vec6d tmDescriptor(tmMin[0],tmMin[1],tmMin[2],tmMax[0],tmMax[1],tmMax[2]);
    
    double score = (nmDescriptor - tmDescriptor).sqrnorm();
    
    return score;
}


void TemplateExplorationWidget::showPartDeformationOption(unsigned int _partID, int _symmetricPartID, bool isForward)
{
    if (_partID <=0 || nnPartScoreVectorIndex_[_partID]<0)
    {
        qCritical() << " Picked part ID: " << pickedPartID_ << ", Part score vector index: " << nnPartScoreVectorIndex_[_partID] ;
        return;
    }
    
    // Get the deformed option part, we know the selected part ID
    std::vector<Match::Part>& parts = deformedNearestOption_.parts();
    
    int nparts = 0;
    
    typename std::vector<Match::Part>::iterator partscIt (parts.begin()), partscEnd(parts.end());
    
    int index =0;
    
    for (; partscIt != partscEnd; ++partscIt)
    {
        if (partscIt->partType_!=0 && partscIt->partID_ == _partID)
        {
            Match::Part& tmcPart = *partscIt;
            
            int nearestMatchIndex = -1;
            
            bool notDeformed = true;
            int i=0;
            while (notDeformed)
            {
                //if(explorationMode_ == SHOW_DEFORMED_PART_OPTIONS)
                if(explorationMode_ == SHOW_CLUSTERS || explorationMode_ == SHOW_CLUSTER || explorationMode_ == SHOW_DEFORMED_PART_OPTIONS)
                {
                    // Get the neighbor according to the sorted index we produced
                    
                    std::vector< std::pair<int, double> >& partRank = nnIndexPartScoreSorted_[_partID];
                    
                    // If a valid symmetric part id was passed as a second argument, get the score for the neighbor of this part and the score for the neighbor of the symmetric part and choose the neighbor with the smallest score
                    if (_symmetricPartID>=0)
                    {
                        std::vector< std::pair<int, double> >& symmetricPartRank = nnIndexPartScoreSorted_[_symmetricPartID];
                        
                        double score1 = partRank.at(nnPartScoreVectorIndex_[_partID]).second;
                        
                        double score2 = symmetricPartRank.at(nnPartScoreVectorIndex_[_partID]).second;
                        
                        if (score1 <= score2)
                        {
                            nearestMatchIndex = partRank.at(nnPartScoreVectorIndex_[_partID]).first;
                        }
                        else
                        {
                            nearestMatchIndex = symmetricPartRank.at(nnPartScoreVectorIndex_[_partID]).first;
                        }
                    }
                    else
                    {
                        nearestMatchIndex = partRank.at(nnPartScoreVectorIndex_[_partID]).first;
                    }
                }
                
                if (explorationMode_ == SHOW_PART_OPTIONS)
                {
                    // Just get a random point
                    nearestMatchIndex = randomNeighbourIndices_[randomNeighbourVectorIndex_];
                    randomNeighbourVectorIndex_++;
                    //rand() % filteredMatches_.size();
                    //nearestPoints_[nnPartScoreVectorIndex_].index_;
                }
                
                if (nearestMatchIndex<0 || nearestMatchIndex>=filteredMatches_.size())
                {
                    qCritical() << "Nearest neighbour index is invalid" << nearestMatchIndex;
                    break;
                }
                
                // Hack just to force a specific neighbor to be chosen for this part
                if(forcedNeighborIndex_>0 && forcedNeighborIndex_ < filteredMatches_.size())
                    nearestMatchIndex = forcedNeighborIndex_;
                
                Match& nearestMatch = *filteredMatches_.at(nearestMatchIndex);
                
                qDebug() << "Nearest neighbor to pick part " << _partID << " from is " << nearestMatchIndex << " with name " << nearestMatch.filename().split("/").last() << " Circular index is " << nnPartScoreVectorIndex_[_partID];
                
                nnChosen_[_partID] = nearestMatchIndex;
                
                if(explorationMode_ == SHOW_CLUSTERS || explorationMode_ == SHOW_CLUSTER || explorationMode_ == SHOW_DEFORMED_PART_OPTIONS)
                {
                    notDeformed = ! deformNearestPart(tmcPart, nearestMatch);
                    if (notDeformed)
                    {
                        if(isForward)
                        {
                            nnPartScoreVectorIndex_[_partID] = (nnPartScoreVectorIndex_[_partID] + 1) % nofNN_;
                        }
                        else
                        {
                            nnPartScoreVectorIndex_[_partID] = (nnPartScoreVectorIndex_[_partID] - 1);
                            
                            if (nnPartScoreVectorIndex_[_partID] < 0)
                            {
                                nnPartScoreVectorIndex_[_partID] = nofNN_ - 1;
                            }
                            
                            qDebug() << " Circular index for part is " << nnPartScoreVectorIndex_[_partID];
                        }
                        
                        if (_symmetricPartID>=0)
                        {
                            if(isForward)
                            {
                                nnPartScoreVectorIndex_[_symmetricPartID] = (nnPartScoreVectorIndex_[_symmetricPartID] + 1) % nofNN_;
                            }
                            else
                            {
                                nnPartScoreVectorIndex_[_symmetricPartID] = (nnPartScoreVectorIndex_[_symmetricPartID] - 1);
                                
                                if (nnPartScoreVectorIndex_[_symmetricPartID] < 0)
                                {
                                    nnPartScoreVectorIndex_[_symmetricPartID] = nofNN_ - 1;
                                }
                                
                                qDebug() << " Circular index for symmetric part is " << nnPartScoreVectorIndex_[_symmetricPartID];
                            }
                        }
                    }
                }
                
                if (explorationMode_ == SHOW_PART_OPTIONS)
                {
                    std::vector<Match::Part>::iterator itPart(nearestMatch.parts().begin()), partEnd(nearestMatch.parts().end());
                    
                    // We now need to find the neighbor's part which has the same part ID with the part we want to deform
                    for (int j=0; itPart != partEnd; ++itPart, ++j)
                    {
                        Match::Part& nmcPart = *itPart;
                        
                        if (nmcPart.partID_ == tmcPart.partID_)
                        {
                            // We need the part's mesh now, so try to open it, if we fail, then segment the mesh using the naive approach
                            if(!nearestMatch.openPartMeshIfNotOpened(nmcPart))
                            {
                                //Open the original mesh first and then split
                                nearestMatch.openMeshIfNotOpened();
                                nearestMatch.split();
                            }
                            qDebug() << "Replacing part with part from neighbor, no deformation made";
                            tmcPart = nmcPart;
                            
                            break;
                        }
                    }
                }
                
                i++;
                
                // 5? JUST TO BE SAFE ?
                if (i> (nofNN_ + 5) )
                {
                    qWarning() << "All neighbor part meshes for part ID " << _partID << "were empty, aborting deformation!";
                    break;
                }
            }
            break;
        }
    }
    
}

void TemplateExplorationWidget::slotShowNextPartDeformationOption()
{
    nnPartScoreVectorIndex_[pickedPartID_] = (nnPartScoreVectorIndex_[pickedPartID_] + 1) % nofNN_;
    
    partClicks_[pickedPartID_] ++;
    
    // Check if the part is involved in any symmetry constraints
    std::vector<Match::Constraint>& tmConstraints = templateMatch_.constraints();
    
    std::vector<Match::Constraint>::iterator tmConstraintsIt(tmConstraints.begin()), tmConstraintsEnd(tmConstraints.end());
    
    int tmConstraintsSize = tmConstraints.size();

    if (preserveConstraints_)
    {
        int i =0;
            
        for (; tmConstraintsIt!= tmConstraintsEnd; ++tmConstraintsIt)
        {
            if (tmConstraintsIt->type_ == SYMMETRY)
            {
                if( tmConstraintsIt->partIDs_.first == pickedPartID_)
                {
                    nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.second] = (nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.second] + 1) % nofNN_;
                    partClicks_[tmConstraintsIt->partIDs_.second] ++;
                    showPartDeformationOption(pickedPartID_,tmConstraintsIt->partIDs_.second,true);
                    showPartDeformationOption(tmConstraintsIt->partIDs_.second,pickedPartID_,true);
                    break;
                }
                
                if(tmConstraintsIt->partIDs_.second == pickedPartID_)
                {
                    nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.first] = (nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.first] + 1) % nofNN_;
                    partClicks_[tmConstraintsIt->partIDs_.first] ++;
                    showPartDeformationOption(pickedPartID_,tmConstraintsIt->partIDs_.first,true);
                    showPartDeformationOption(tmConstraintsIt->partIDs_.first,pickedPartID_,true);
                    break;
                }
            }
            i++;
        }
        
        if (i==tmConstraintsSize)
        {
            // Means we didn't find any symmetric counterparts, so just deform the part without symmetry
            showPartDeformationOption(pickedPartID_, -1,true);
            
        }
    }
    else
    {
        showPartDeformationOption(pickedPartID_, -1,true);
    }
    
    std::vector<Match::Part>& cParts = deformedNearestOption_.parts();
    
    std::vector<Match::Part>::iterator partIt(cParts.begin()), partEnd(cParts.end());
    
    std::unordered_map< int, int> countNeighbors;
    
    // Do another pass to actually count neighbors used
    for (; partIt!=partEnd; ++partIt)
    {
        countNeighbors [ nnChosen_[partIt->partID_]] ++;
    }
    
    nnUsed_ = countNeighbors.size(); // nnChosen_.size(); //countNeighbors.size() - nSymmetryConstraints_;
    
    emit nnUsedChanged(QString("No of neighbors used: %1").arg(nnUsed_));
    
    //showPartDeformationOption(pickedPartID_);
    
    updateViewport(explorationMode_);
    
    TIMELOG->append(QString("%1 : advanced_next_part part_id:%2").arg((qlonglong)QDateTime::currentMSecsSinceEpoch()).arg(pickedPartID_));

}

void TemplateExplorationWidget::slotShowPreviousPartDeformationOption()
{
    nnPartScoreVectorIndex_[pickedPartID_] = (nnPartScoreVectorIndex_[pickedPartID_] - 1);
    
    if (nnPartScoreVectorIndex_[pickedPartID_] < 0)
    {
        nnPartScoreVectorIndex_[pickedPartID_] = nofNN_ - 1;
    }

    partClicks_[pickedPartID_] ++;
    
    // Check if the part is involved in any symmetry constraints
    std::vector<Match::Constraint>& tmConstraints = templateMatch_.constraints();
    
    std::vector<Match::Constraint>::iterator tmConstraintsIt(tmConstraints.begin()), tmConstraintsEnd(tmConstraints.end());
    
    int tmConstraintsSize = tmConstraints.size();
    
    if (preserveConstraints_)
    {
        int i =0;
        
        for (; tmConstraintsIt!= tmConstraintsEnd; ++tmConstraintsIt)
        {
            if (tmConstraintsIt->type_ == SYMMETRY)
            {
                if( tmConstraintsIt->partIDs_.first == pickedPartID_)
                {
                    nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.second] = (nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.second] - 1);
                    
                    if (nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.second] < 0)
                    {
                        nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.second] = nofNN_ - 1;
                    }
                    
                    partClicks_[tmConstraintsIt->partIDs_.second] ++;
                    
                    showPartDeformationOption(pickedPartID_,tmConstraintsIt->partIDs_.second,false);
                    showPartDeformationOption(tmConstraintsIt->partIDs_.second,pickedPartID_,false);
                    break;
                }
                
                if(tmConstraintsIt->partIDs_.second == pickedPartID_)
                {
                    nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.first] = (nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.first] - 1);
                    
                    if (nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.first] < 0)
                    {
                        nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.first] = nofNN_ - 1;
                    }
                    
                    partClicks_[tmConstraintsIt->partIDs_.first] ++;
                    
                    showPartDeformationOption(pickedPartID_,tmConstraintsIt->partIDs_.first,false);
                    showPartDeformationOption(tmConstraintsIt->partIDs_.first,pickedPartID_,false);
                    break;
                }
            }
            i++;
        }
        
        if (i==tmConstraintsSize)
        {
            // Means we didn't find any symmetric counterparts, so just deform the part without symmetry
            showPartDeformationOption(pickedPartID_, -1,false);
            
        }
    }
    else
    {
        showPartDeformationOption(pickedPartID_, -1,false);
    }
    
    std::vector<Match::Part>& cParts = deformedNearestOption_.parts();
    
    std::vector<Match::Part>::iterator partIt(cParts.begin()), partEnd(cParts.end());
    
    std::unordered_map< int, int> countNeighbors;
    
    // Do another pass to actually count neighbors used
    for (; partIt!=partEnd; ++partIt)
    {
        countNeighbors [ nnChosen_[partIt->partID_]] ++;
    }
    
    nnUsed_ = countNeighbors.size(); // nnChosen_.size(); //countNeighbors.size() - nSymmetryConstraints_;
    
    emit nnUsedChanged(QString("No of neighbors used: %1").arg(nnUsed_));
    
    //showPartDeformationOption(pickedPartID_);
    
    updateViewport(explorationMode_);

    TIMELOG->append(QString("%1 : advanced_previous_part part_id:%2").arg((qlonglong)QDateTime::currentMSecsSinceEpoch()).arg(pickedPartID_));
}



void TemplateExplorationWidget::slotChangeRecalculateBoxes(int state)
{
    if (state == Qt::Unchecked)
    {
        qDebug() << "Recalculate boxes policy switched to false!" ;
    }
    else
    {
        qDebug() << "Recalculate boxes policy switched to true!" ;
    }
    
    dataState_.deformedPartOptionsValid = false;
    
    // Really shouldn't do change exploration mode here, but seems I have no other choice
    if (explorationMode_ == SHOW_DEFORMED_PART_OPTIONS)
    {
        if (currentNumClusters_==1)
        {
            slotChangeExplorationMode(SHOW_CLUSTER);
        }
        else
        {
            slotChangeExplorationMode(SHOW_CLUSTERS);
        }
    }
}



void TemplateExplorationWidget::slotManuallyChangeSelectedPoint()
{
    double posx = lnPointX_->text().toDouble();
    double posy = lnPointY_->text().toDouble();
    
    slotChangeSelectedPoint(posx, posy);
}



void TemplateExplorationWidget::updateSliders()
{
    // Find the integer to update the slider's value now that it's going to be visible
    // Transform from the range [pcaMin,pcaMax] to the range [0,100]
    // Get the alpha, i.e. the percentage of the distance from pcaMax to pcaMin (on x axis) covered by the x coordinate of the currently selected point [0,1]
    double alpha1 = (selectedPoint_[0] - pcaMin_[0]) / (pcaMax_[0] - pcaMin_[0]);
    if(alpha1<0 || alpha1>100)
    {
        qWarning() << "The point you have clicked on is outside [pcaMin,pcaMax] so slider will be set at minimum or maximum" ;
    }
    // Slider value will be minSliderValue (i.e. 0) + alpha*SliderRange (i.e 100) and cast to integer
    int sliderValue1 = (int) (alpha1 * 100.0);
    sliderLambda1_->blockSignals(true);
    sliderLambda1_->setValue(sliderValue1);
    sliderLambda1_->blockSignals(false);
    
    // Find the integer to update the slider's value now that it's going to be visible
    // Transform from the range [pcaMin,pcaMax] to the range [0,100]
    // Get the alpha, i.e. the percentage of the distance from pcaMax to pcaMin (on x axis) covered by the x coordinate of the currently selected point [0,1]
    double alpha2 = (selectedPoint_[1] - pcaMin_[1]) / (pcaMax_[1] - pcaMin_[1]);
    if(alpha2<0 || alpha2>100)
    {
        qWarning() << "Warning, the point you have clicked on is outside [pcaMin,pcaMax] so slider will be set at minimum or maximum" ;
    }
    // Slider value will be minSliderValue (i.e. 0) + alpha*SliderRange (i.e 100) and cast to integer
    int sliderValue2 = (int) (alpha2 * 100.0);
    sliderLambda2_->blockSignals(true);
    sliderLambda2_->setValue(sliderValue2);
    sliderLambda2_->blockSignals(false);

}

void TemplateExplorationWidget::updateClusterView()
{
    templateListWidget_->clear();
    
    switch (explorationMode_)
    {
        case SHOW_GROUPS:
        {
            std::map<int, int> groupIDmap;
            
            std::vector<Match*>::iterator matchesIt(matches_.begin()), matchesEnd(matches_.end());

            QStringList tmplIconPath = (*matchesIt)->filename().split("/");
            tmplIconPath.removeLast();
            tmplIconPath.removeLast();
            tmplIconPath.push_back(TEMPLATE_ICON_PATH);
            
            for (; matchesIt!=matchesEnd; ++matchesIt)
            {
                // Do some basic filtering based on the fit error, if the fit error of the match is bigger than the maximum allowable fit error, just leave it out on the calculation of the group IDs - hopefully if no bug exists, it will be left out of the later PCA calculations
                if ((**matchesIt).fitError()>selectedFitErrorThreshold_)
                {
                    continue;
                }
                int groupID = (*matchesIt)->groupID();
                //check if group id has been found before, if not, add a new template icon to templateListWidget_
                if (groupIDmap.count(groupID) == 0)
                {
                    groupIDmap[groupID] = 1;
                }
                else
                {
                    groupIDmap[groupID]++;
                }
            }
            
            std::vector<std::pair<int, int> > groupIDvector;
            copy(groupIDmap.begin(),
                 groupIDmap.end(),
                 std::back_inserter<std::vector<std::pair<int, int> > >(groupIDvector));
            
            std::sort (groupIDvector.begin(), groupIDvector.end(), CompareGroupsFunctor);
            
            std::vector< std::pair<int, int> >::iterator grIt(groupIDvector.begin()), grEnd(groupIDvector.end());
            
            int groupIndex = 0;
            
            for (; grIt!=grEnd; ++grIt)
            {
                //std::cout << "Group " << (*grIt).first << " has " << (*grIt).second << " models " << std::endl;
                if (groupIndex < MAX_NUM_CLUSTERS_TO_SHOW)
                {
                    QStringList iconPath = tmplIconPath;
                    
                    iconPath.push_back(QString("group%1.jpg").arg((*grIt).first));
                    
                    QString icName = iconPath.join("/");
                    
                    qDebug() << "Icon to load from: " << icName;
                    
                    QPixmap tmplIcon(icName);
                    
                    tmplIcon = tmplIcon.scaledToHeight(CLUSTER_VIEW_ICON_HEIGHT); //scaled(100,100, Qt::IgnoreAspectRatio);
                    
                    ///////////////////////////////////// setup the custom widget to add to the template list widget
                    
                    QFrame* clusterFrame = new QFrame;
                    
                    QVBoxLayout* clusterItemLayout = new QVBoxLayout;
                    
                    QColor frameColor = tewColors[groupIndex];
                    
                    clusterFrame->setStyleSheet(QString(".QFrame { border: %4px solid rgb(%1,%2,%3) }").arg(frameColor.red()).arg(frameColor.green()).arg(frameColor.blue()).arg(CLUSTER_VIEW_ICON_FRAME_THICKNESS));
                    
                    QLabel* itemPixmap = new QLabel;
                    itemPixmap->setPixmap(tmplIcon);
                    
                    QLabel* itemLabel = new QLabel(QString("%1 models").arg((*grIt).second));
                    itemLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                    
                    clusterItemLayout->addWidget(itemPixmap);
                    clusterItemLayout->addWidget(itemLabel);
                    
                    clusterFrame->setLayout(clusterItemLayout);
                    
                    int iconwidth = tmplIcon.width();
                    int iconheight = tmplIcon.height();
                    
                    /////////////////////////////////////////////
                    
                    QListWidgetItem* qlwi = new QListWidgetItem;
                    
                    qlwi->setToolTip(QString("Group %1").arg((*grIt).first));
                    
                    qlwi->setSizeHint(QSize(iconwidth + CLUSTER_VIEW_ICON_PADDING, iconheight + CLUSTER_VIEW_ICON_PADDING) );
                
                    //qlwi->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
                    
                    templateListWidget_->addItem(qlwi);

                    templateListWidget_->setItemWidget(qlwi, clusterFrame);
                }
                else
                {
                    break;
                }
                groupIndex++;
            }

        }
            break;
        case SHOW_CLUSTERS:
        {
            std::vector<int>::iterator reprIndIt(representativeIndex_.begin()), reprIndEnd(representativeIndex_.end());
            int lbl=0;
            for (; reprIndIt!=reprIndEnd;  ++reprIndIt)
            {
                if(*reprIndIt>=0)
                {
                    Match* representative = filteredMatches_.at(*reprIndIt);
                    QStringList matchIconPath = representative->filename().split("/");
                    QString matchName = matchIconPath.last().split(".").first();
                    matchIconPath.removeLast();
                    matchIconPath.removeLast();
                    matchIconPath.push_back(MODEL_ICON_PATH);
                    matchIconPath.push_back(matchName + ".jpg");
                    
                    
                    QString icName = matchIconPath.join("/");
                    
                    qDebug() << "Icon to load from: " << icName;
                    
                    QPixmap matchIcon(icName);
                    
                    matchIcon = matchIcon.scaledToHeight(CLUSTER_VIEW_ICON_HEIGHT); //scaled(100,100, Qt::IgnoreAspectRatio);
                    
                    ///////////////////////////////////// setup the custom widget to add to the template list widget
                    
                    QFrame* clusterFrame = new QFrame;
                    
                    QVBoxLayout* clusterItemLayout = new QVBoxLayout;
                    
                    QColor frameColor = tewColors[representative->label()];
                    
                    clusterFrame->setStyleSheet(QString(".QFrame { border: %4px solid rgb(%1,%2,%3) }").arg(frameColor.red()).arg(frameColor.green()).arg(frameColor.blue()).arg(CLUSTER_VIEW_ICON_FRAME_THICKNESS));
                    
                    QLabel* itemPixmap = new QLabel;
                    itemPixmap->setPixmap(matchIcon);
                    
                    QLabel* itemLabel = new QLabel(QString("%1 models").arg(clusterPopulation_[lbl]));
                    itemLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                    
                    clusterItemLayout->addWidget(itemPixmap);
                    clusterItemLayout->addWidget(itemLabel);
                    
                    clusterFrame->setLayout(clusterItemLayout);
                    
                    int iconwidth = matchIcon.width();
                    int iconheight = matchIcon.height();
                    
                    /////////////////////////////////////////////
                    
                    QListWidgetItem* qlwi = new QListWidgetItem;
                    
                    qlwi->setToolTip(QString("Cluster %1").arg(lbl));
                    
                    qlwi->setSizeHint(QSize(iconwidth + CLUSTER_VIEW_ICON_PADDING, iconheight + CLUSTER_VIEW_ICON_PADDING) );
                    
                    qlwi->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
                    
                    qlwi->setForeground(QColor(tewColors[representative->label()]));
                    
                    templateListWidget_->addItem(qlwi);
                    
                    templateListWidget_->setItemWidget(qlwi, clusterFrame);
                }
                lbl++;
            }

        }
            break;
            
        case SHOW_CLUSTER:
        {
            std::vector<int>::iterator reprIndIt(representativeIndex_.begin()), reprIndEnd(representativeIndex_.end());
            
            int i=0;
            
            for (; reprIndIt!=reprIndEnd;  ++reprIndIt)
            {
                if(*reprIndIt>=0)
                {
                    Match* representative = filteredMatches_.at(*reprIndIt);
                    QStringList matchIconPath = representative->filename().split("/");
                    QString matchName = matchIconPath.last().split(".").first();
                    matchIconPath.removeLast();
                    matchIconPath.removeLast();
                    matchIconPath.push_back(MODEL_ICON_PATH);
                    matchIconPath.push_back(matchName + ".jpg");
                    
                    QString icName = matchIconPath.join("/");
                    
                    qDebug() << "Icon to load from: " << icName;
                    
                    QPixmap matchIcon(icName);
                    
                    matchIcon = matchIcon.scaledToHeight(CLUSTER_VIEW_ICON_HEIGHT);
                    
                    QString text;
                    switch (i)
                    {
                        case 0:
                            text = "Center";
                            break;
                        case 1:
                            text = "Left";
                            break;
                        case 2:
                            text = "Right";
                            break;
                        case 3:
                            text = "Top";
                            break;
                        case 4:
                            text = "Bottom";
                            break;
                        default:
                            break;
                    }
                    
                    ///////////////////////////////////// setup the custom widget to add to the template list widget
                    
                    QFrame* clusterFrame = new QFrame;
                    
                    QVBoxLayout* clusterItemLayout = new QVBoxLayout;
                    
                    QColor frameColor = tewColors[i];
                    
                    clusterFrame->setStyleSheet(QString(".QFrame { border: %4px solid rgb(%1,%2,%3) }").arg(frameColor.red()).arg(frameColor.green()).arg(frameColor.blue()).arg(CLUSTER_VIEW_ICON_FRAME_THICKNESS));
                    
                    QLabel* itemPixmap = new QLabel;
                    itemPixmap->setPixmap(matchIcon);
                    
                    QLabel* itemLabel = new QLabel(text);
                    itemLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                    
                    clusterItemLayout->addWidget(itemPixmap);
                    clusterItemLayout->addWidget(itemLabel);
                    
                    clusterFrame->setLayout(clusterItemLayout);
                    
                    int iconwidth = matchIcon.width();
                    int iconheight = matchIcon.height();
                    
                    /////////////////////////////////////////////

                    QListWidgetItem* qlwi = new QListWidgetItem;
                    
                    qlwi->setSizeHint(QSize(iconwidth + CLUSTER_VIEW_ICON_PADDING, iconheight + CLUSTER_VIEW_ICON_PADDING) );
                    
                    qlwi->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
                    qlwi->setForeground(QColor(tewColors[i]));
                    
                    templateListWidget_->addItem(qlwi);
                    
                    templateListWidget_->setItemWidget(qlwi, clusterFrame);

                }
                i++;
            }
        }
            break;
        default:
            break;
    }
}

void TemplateExplorationWidget::slotChangeHoveredPoint(double _posx, double _posy)
{
    
    if (explorationMode_!= SHOW_CLUSTER && explorationMode_!= SHOW_CLUSTERS)
    {
        return;
    }
    
    slotDeformTemplate(_posx, _posy);
    
    std::vector<Shape*> shapes;
    
    shapes.push_back(&templateMatch_);
    
    int displayMode = 0;
    
    displayMode |= BOX;
    templateMatch_.setDisplayMode(displayMode);
    
    if (CREATE_MVW)
    {
        emit selectedShapesChanged(shapes,0);
    }
    
}


void TemplateExplorationWidget::slotJoinMatchesInSingleGroup()
{
    std::vector<Match*>::iterator matchesIt(matches_.begin()), matchesEnd( matches_.end());
    
    matchesIt = matches_.begin();
    int i=0;
    std::cout << "Before joining" << std::endl;
    
    for (; matchesIt!= matchesEnd; ++matchesIt)
    {
        std::vector<Match::Part>& cMatchParts = (**matchesIt).parts();
        
        std::vector<Match::Part>::iterator partsIt(cMatchParts.begin()), partsEnd(cMatchParts.end());
        std::cout << i << ": ";
        for (; partsIt!=partsEnd; ++partsIt)
        {
            std::cout << partsIt->partID_ << "->" << partsIt->partType_ << " , ";
        }
        std::cout << " " << std::endl;
        i++;
    }
    
    std::map<int, int> partIDCounts;
    
    matchesIt = matches_.begin();
    
    for (; matchesIt!= matchesEnd; ++matchesIt)
    {
        std::vector<Match::Part>& cMatchParts = (**matchesIt).parts();
        
        std::vector<Match::Part>::iterator partsIt(cMatchParts.begin()), partsEnd(cMatchParts.end());
        
        for (; partsIt!=partsEnd; ++partsIt)
        {
            partIDCounts[partsIt->partID_]++;
        }
    }
    
    std::cout << "Discovered parts:" << std::endl;
    
    std::map<int, int>::iterator partIDCountsIt, partIDCountsEnd(partIDCounts.end());
    partIDCountsIt = partIDCounts.begin();
    
    for (; partIDCountsIt!= partIDCountsEnd; ++partIDCountsIt)
    {
        std::cout << partIDCountsIt->first <<"->" << partIDCountsIt->second << " ";
    }
    std::cout << std::endl;
    
    matchesIt = matches_.begin();
    
    for (; matchesIt!= matchesEnd; ++matchesIt)
    {
        std::vector<Match::Part>& cMatchParts = (**matchesIt).parts();
        
        partIDCountsIt = partIDCounts.begin();
     
        (**matchesIt).setGroupID(0);
        
        for (; partIDCountsIt!=partIDCountsEnd; ++partIDCountsIt)
        {
            std::vector<Match::Part>::iterator partsIt(cMatchParts.begin()), partsEnd(cMatchParts.end());
        
            int sortedPartID = partIDCountsIt->first;
            
            // There are 4 cases for the id of the part we want to insert in the vector of parts
            // 1. It already exists, so we need to iterate and find it (if we had a map of the part id's stored already it would be faster)
            // 2. It does not exist, and its id is smaller than the id of the first part in the vector, so just push it in the front of the vector - we are assuming parts are placed in the vector sorted by their part id which seems like a valid assumption given our data
            // 3. It does not exist, and its id is bigger than the id of the last part in the vector, so just push if at the back of the vector - we are assuming parts are placed in the vector sorted by their part id which seems like a valid assumption given our data
            // 4. It does not exist and it's id should go between the id's of two existing parts in the vector, so find these parts and add it in between them - we are assuming parts are placed in the vector sorted by their part id which seems like a valid assumption given our data
            
            // Might not get to add this to the vector, but doesn't hurt to have it preallocated in case we do need to add it
            Match::Part nPart;
            
            nPart.partID_ = sortedPartID;
            nPart.partType_ = 5;
            nPart.pos_[0] = 0;
            nPart.pos_[1] = 0;
            nPart.pos_[2] = 0;
            
            nPart.scale_[0] = 0;
            nPart.scale_[1] = 0;
            nPart.scale_[2] = 0;
            
            // Case 2: it does not exist and its id is smaller than the id of the first part in the vector
            if (sortedPartID < partsIt->partID_)
            {
                cMatchParts.insert(partsIt, nPart);
                continue;
            }
            
            // Case 3: it does not exist and its id is bigger than the id of the last part of the vector
            if (sortedPartID > (partsEnd-1)->partID_)
            {
                cMatchParts.push_back(nPart);
                continue;
            }

            for (; partsIt!=partsEnd; ++partsIt)
            {
                int cMatchPartID = partsIt->partID_;
                
                if (partsIt + 1 == partsEnd)
                {
                    // shouldn't reach here anyway, but just a sanity check to make sure we want dereference outside the vector
                    break;
                }
                
                int nMatchPartID = (partsIt + 1)->partID_;
                
                // Case 1: we have found the id of the part we want to insert in the vector, so no need to insert anything
                if ( cMatchPartID == sortedPartID || nMatchPartID == sortedPartID)
                {
                    break;
                }
                
                // Case 4: id of the part is bigger than the current part and smaller than the next part, so insert it between them, i.e. before the next part
                if (sortedPartID > cMatchPartID && sortedPartID < nMatchPartID)
                {
                    cMatchParts.insert(partsIt+1, nPart);
                }
            }
        }
        
        (**matchesIt).setNparts(cMatchParts.size());
    }
    
    matchesIt = matches_.begin();
    i=0;
    std::cout << "After joining" << std::endl;
    
    for (; matchesIt!= matchesEnd; ++matchesIt)
    {
        std::vector<Match::Part>& cMatchParts = (**matchesIt).parts();
        
        std::vector<Match::Part>::iterator partsIt(cMatchParts.begin()), partsEnd(cMatchParts.end());
        qDebug() << i;
        for (; partsIt!=partsEnd; ++partsIt)
        {
            std::cout << partsIt->partID_ << "->" << partsIt->partType_ << " , ";
        }
        std::cout << " " << std::endl;
        i++;
    }
}

void TemplateExplorationWidget::slotOptimizeTemplate()
{
    std::vector<Match::Part>& tmParts = templateMatch_.parts();
    
    std::vector<Match::Constraint>& tmConstraints = templateMatch_.constraints();

    std::vector<Match::Part>::iterator tmPartsIt (tmParts.begin()), tmPartsEnd(tmParts.end());
    
    std::vector<Match::Constraint>::iterator tmConstraintsIt(tmConstraints.begin()), tmConstraintsEnd(tmConstraints.end());
    
    int N = tmParts.size() * NUM_PARAMS_BOX;
    
    double _x[N];
    
    for (int i=0; tmPartsIt!=tmPartsEnd; ++tmPartsIt, i+=NUM_PARAMS_BOX)
    {
        _x[i] = tmPartsIt->pos_[0];
        _x[i+1] = tmPartsIt->pos_[1];
        _x[i+2] = tmPartsIt->pos_[2];
        
        _x[i+3] = tmPartsIt->scale_[0];
        _x[i+4] = tmPartsIt->scale_[1];
        _x[i+5] = tmPartsIt->scale_[2];
    }
    
    // prepare array data for the optimizer, the optimizer solves for the quadratic function:
    // f(x) = 1/2*x^tAx + b*x^t, subject to Cx = d;
    // A is a N*N matrix, b is a 1*N vector,
    // C is a M*N matrix, d is a M*1 vector
    double A[N][N];
    
    double b[N];

    for (int i=0; i<N; ++i)
    {
        // init A 
        for (int j=0; j<N; ++j)
        {
            A[i][j] = 0.0;
        }
        
        // Then set A to diagonal matrix
        A[i][i] = 2.0; // factor 0.5 is multiplied before, so instead of A being identity, we make it 2I
        
        b[i] = -2.0 * _x[i];
    }
    
    
    int M = 0;
    int J = 0;
    
    // Unfortunately we don't know how many symmetry or contact or whatever type of constraints we have, so we need to go over the constraints vector once to discover them and increase M which is the number of constraint equations based on the number of equations we need per constraint
    for (; tmConstraintsIt!=tmConstraintsEnd; ++tmConstraintsIt)
    {
        // each symmetry imposes 7 equations, each contact imposes 3 equations
        if (tmConstraintsIt->type_ == SYMMETRY)
        {
            M+=NUM_EQUATIONS_SYMMETRY; 
        }
        if (tmConstraintsIt->type_ == CONTACT)
        {
            M+=NUM_EQUATIONS_CONTACT;
        }
    }
    
    double C[M][N+1]; // the N+1'th column is to store the values of d
    
    // init C matrix
    for (int i = 0; i<M; ++i)
    {
        for (int j=0; j<N+1; ++j)
        {
            C[i][j] = 0.0;
        }
    }
    OpenMesh::Vec3d o(0.0,0.0,0.0); // reflectional symmetry plane center -- to be defined
    
    OpenMesh::Vec3d n(1.0,0.0,0.0); // reflectional symmetry plane normal
    
    if (LOADED_DATASET == PLANES)
    {
        n[0] = 0.0;
        n[1] = 1.0;
        n[2] = 0.0;
    }
    
    tmConstraintsIt = tmConstraints.begin();
    
    
    int Q[8][3] =
    {
        {1,1,1},
        {1,-1,1},
        {-1,-1,1},
        {-1,1,1},
        {1,1,-1},
        {1,-1,-1},
        {-1,-1,-1},
        {-1,1,-1}
    };
    

    for (; tmConstraintsIt!=tmConstraintsEnd; ++tmConstraintsIt)
    {
        // Get the starting indices of the two parts involved in the constraint
        int _index1 = tmConstraintsIt->partIndices_.first * NUM_PARAMS_BOX;
        int _index2 = tmConstraintsIt->partIndices_.second * NUM_PARAMS_BOX;
        
        // each symmetry imposes 7 equations, each contact imposes 3 equations
        if (tmConstraintsIt->type_ == SYMMETRY)
        {
            // centers
			// 1. n*(c_i+c_j) = 2o*n
			C[J][_index1 + 0] = n[0]; C[J][_index2 + 0] = n[0];
			C[J][_index1 + 1] = n[1]; C[J][_index2 + 1] = n[1];
			C[J][_index1 + 2] = n[2]; C[J][_index2 + 2] = n[2];
            
			C[J][N] = 2 * (n|o);
			J++;
            
			// 2. nx(c_i-c_j) = 0
			C[J][_index1 + 0] = n[2]; C[J][_index2 + 0] = -n[2]; C[J][_index1 + 2] = -n[0]; C[J][_index2 + 2] = n[0]; // the N+1 term is already 0
			J++;
			C[J][_index1 + 1] = n[0]; C[J][_index2 + 1] = -n[0]; C[J][_index1 + 0] = -n[1]; C[J][_index2 + 0] = n[1]; // the N+1 term is already 0
			J++;
			C[J][_index1 + 2] = n[1]; C[J][_index2 + 2] = -n[1]; C[J][_index1 + 1] = -n[2]; C[J][_index2 + 1] = n[2]; // the N+1 term is already 0
			J++;
            
			// scales
			C[J][_index1 + 3] = 1.0; C[J][_index2 + 3] = -1.0; // the N+1 term is already 0
			J++;
			C[J][_index1 + 4] = 1.0; C[J][_index2 + 4] = -1.0;
			J++;
			C[J][_index1 + 5] = 1.0; C[J][_index2 + 5] = -1.0;
			J++;
        }
        
        if (tmConstraintsIt->type_ == CONTACT)
        {
            xform T, R;
            
            double minDist = std::numeric_limits<double>::max();
            
            for (int j=0; j < 8; ++j)
            {
                xform _T;
                _T[0] = Q[j][0];
				_T[5] = Q[j][1];
				_T[10] = Q[j][2];
                
				OpenMesh::Vec3f p = tmParts[ tmConstraintsIt->partIndices_.first ].pos_ + mv(_T, tmParts[ tmConstraintsIt->partIndices_.first ].scale_);
                
				for (int k = 0; k < 8; ++k)
				{
					xform _R;
					_R[0] = Q[k][0];
					_R[5] = Q[k][1];
					_R[10] = Q[k][2];
					OpenMesh::Vec3f q = tmParts[ tmConstraintsIt->partIndices_.second ].pos_ + mv(_R,  tmParts[ tmConstraintsIt->partIndices_.second ].scale_);
					double dis = (q - p).norm();
                    
					if (dis < minDist)
					{
						minDist = dis;
						T = _T;
						R = _R;
					}
				}
            }
            C[J][_index1 + 0] = 1.0; C[J][_index2 + 0] = -1.0; C[J][_index1 + 3] = T[0]; C[J][_index2 + 3] = -R[0]; // the N+1 term is already 0
            J++;
            C[J][_index1 + 1] = 1.0; C[J][_index2 + 1] = -1.0; C[J][_index1 + 4] = T[5]; C[J][_index2 + 4] = -R[5]; // the N+1 term is already 0
            J++;
            C[J][_index1 + 2] = 1.0; C[J][_index2 + 2] = -1.0; C[J][_index1 + 5] = T[10]; C[J][_index2 + 5] = -R[10]; // the N+1 term is already 0
            J++;
        }
    }
    
    // -- feed to the optimizer
    long int ct[M];
    double x[N];
    
    for (int i=0; i<M; ++i)
    {
        ct[i] = 0;
    }
    for (int i=0; i<N; ++i)
    {
        x[i] = 0.0;
    }
    minqpstate state;
    minqpreport rep;
    
    real_2d_array A_alglib;
    A_alglib.setcontent(N, N, &A[0][0]);
    
    real_1d_array b_alglib;
    b_alglib.setcontent(N, &b[0]);
    
    real_2d_array C_alglib;
    
    C_alglib.setcontent(M, N+1, &C[0][0]);
    
    integer_1d_array ct_alglib;
    
    ct_alglib.setcontent(M, &ct[0]);
    
    real_1d_array x_alglib;
    
    x_alglib.setcontent(N, &x[0]);
    
    // create solver, set quadratic/linear terms
    minqpcreate(N,state);
    minqpsetquadraticterm(state, A_alglib);
    minqpsetlinearterm(state, b_alglib);
    minqpsetlc(state, C_alglib, ct_alglib); // set linear constraints
    
    // Set scale of the parameters.
    // It is strongly recommended that you set scale of your variables.
    // Knowing their scales is essential for evaluation of stopping criteria
    // and for preconditioning of the algorithm steps.
    // You can find more information on scaling at http://www.alglib.net/optimization/scaling.php
    //double[] s = new double[N]; for (int i = 0; i < N; ++i) s[i] = 1.0;
    //alglib.minqpsetscale(state, s);
    
    
    // solve problem with Cholesky-based QP solver
    minqpsetalgocholesky(state);
    minqpoptimize(state);
    minqpresults(state,x_alglib,rep);
    
    
    //// solve problem with BLEIC-based QP solver
    //// default stopping criteria are used.
    //alglib.minqpsetalgobleic(state, 0.0, 0.0, 0.0, 0);
    //alglib.minqpoptimize(state);
    //alglib.minqpresults(state, out x, out rep);

    tmPartsIt = tmParts.begin();
    
    for (int i=0; tmPartsIt!=tmPartsEnd; ++tmPartsIt, i+=NUM_PARAMS_BOX)
    {
        tmPartsIt->pos_[0] = x_alglib[i];
        tmPartsIt->pos_[1] = x_alglib[i+1];
        tmPartsIt->pos_[2] = x_alglib[i+2];
        
        tmPartsIt->scale_[0] = x_alglib[i+3];
        tmPartsIt->scale_[1] = x_alglib[i+4];
        tmPartsIt->scale_[2] = x_alglib[i+5];
    }

}

void TemplateExplorationWidget::updateExplorationView()
{
    
    QList<QGraphicsItem *> lst = scene_->items();
    
    int lstSize = lst.size();
    
    if (lstSize==0)
    {
        return;
    }
    
    // Add the template marker back to the scene because it should have been cleared when the poins were added
//    if (dataState_.templateValid)
//    {
    if (!templateMarker_)
    {
        templateMarker_ = new TemplateExplorationViewItem(&templateMatch_,-1);
    }
    
    templateMarker_->setPos(QPointF(selectedPoint_[0],-selectedPoint_[1]));
    
    scene_->addItem(templateMarker_);
//    }
    
    // Mark the representative items depending on the mode we are in (show clusters/cluster)
    std::vector<int>::iterator reprIndIt(representativeIndex_.begin()), reprIndEnd(representativeIndex_.end());
    
    int c=0;
    
    for (; reprIndIt!=reprIndEnd;  ++reprIndIt)
    {
        if(*reprIndIt>=0)
        {
            for (int i=0; i<lstSize; i++)
            {
                TemplateExplorationViewItem* marker = dynamic_cast<TemplateExplorationViewItem*> (lst[i]);
                
                if (marker && marker->index()== *reprIndIt)
                {
                    if (explorationMode_ == SHOW_CLUSTERS)
                    {
                        marker->setRepresentative(true);
                    }
                    else if(explorationMode_ == SHOW_CLUSTER)
                    {
                        marker->setRepresentative(true);
                        marker->colorOverride(tewColors[c]);
                    }
                    
                    break;
                }
            }
        }
        c++;
    }
}

void TemplateExplorationWidget::slotChangeSelectedMatches()
{
    QList<QGraphicsItem *> selList = scene_->selectedItems();
    
    int lstSize = selList.size();
    
    if (lstSize==0)
    {
        return;
    }
    
    filteredMatchesHistory_.push_back(filteredMatches_);
    
    filteredMatches_.clear();
    
    for (int i=0; i<lstSize; i++)
    {
        TemplateExplorationViewItem* marker = dynamic_cast<TemplateExplorationViewItem*> (selList[i]);
        
        if (marker)
        {
            filteredMatches_.push_back(marker->match());
        }
    }
    
    if (calculatePCA())
    {
        setPlotPoints();
        
        if (CREATE_QWTPLOTW)
        {
            emit plotPointsChanged();
        }
    
        slotChangeSelectedPoint(0.0, 0.0);

        if (currentNumClusters_==1)
        {
            slotChangeExplorationMode(SHOW_CLUSTER);
            //slotChangeExplorationMode(SHOW_DEFORMED_PART_OPTIONS);
        }
        else
        {
            slotChangeExplorationMode(SHOW_CLUSTERS);
            //slotChangeExplorationMode(SHOW_DEFORMED_PART_OPTIONS);
        }
    }
    updateClusterView();
    updateExplorationView();
}

void TemplateExplorationWidget::initChairConstraints()
{
    std::vector<Match::Constraint>& tmConstraints = templateMatch_.constraints();
    
    //CHAIR CONSTRAINTS
    Match::Constraint sym1;
    sym1.type_ = SYMMETRY;
    sym1.partIndices_ = std::pair<int, int>(2,5);
    sym1.partIDs_ = std::pair<int, int>(4,7);
    
    Match::Constraint sym2;
    sym2.type_ = SYMMETRY;
    sym2.partIndices_ = std::pair<int, int>(3,4);
    sym2.partIDs_ = std::pair<int, int>(5,6);
    
    Match::Constraint con1;
    con1.type_ = CONTACT;
    con1.partIndices_ = std::pair<int, int>(1,0);
    con1.partIDs_ = std::pair<int, int>(3,2);
    
    Match::Constraint con2;
    con2.type_ = CONTACT;
    con2.partIndices_ = std::pair<int, int>(2,0);
    con2.partIDs_ = std::pair<int, int>(4,2);
    
    Match::Constraint con3;
    con3.type_ = CONTACT;
    con3.partIndices_ = std::pair<int, int>(3,0);
    con3.partIDs_ = std::pair<int, int>(5,2);
    
    Match::Constraint con4;
    con4.type_ = CONTACT;
    con4.partIndices_ = std::pair<int, int>(4,0);
    con4.partIDs_ = std::pair<int, int>(6,2);
    
    Match::Constraint con5;
    con5.type_ = CONTACT;
    con5.partIndices_ = std::pair<int, int>(5,0);
    con5.partIDs_ = std::pair<int, int>(7,2);
    
    
    tmConstraints.push_back(sym1);
    tmConstraints.push_back(sym2);
    tmConstraints.push_back(con1);
    tmConstraints.push_back(con2);
    tmConstraints.push_back(con3);
    tmConstraints.push_back(con4);
    tmConstraints.push_back(con5);
    
    nSymmetryConstraints_ = 2;
    
}

void TemplateExplorationWidget::initBikeConstraints()
{
    std::vector<Match::Constraint>& tmConstraints = templateMatch_.constraints();
    
    // BIKE CONSTRAINTS
    Match::Constraint sym1;
    sym1.type_ = SYMMETRY;
    sym1.partIndices_ = std::pair<int, int>(2,3);
    sym1.partIDs_ = std::pair<int, int>(4,5);

    Match::Constraint sym2;
    sym2.type_ = SYMMETRY;
    sym2.partIndices_ = std::pair<int, int>(3,4);
    sym2.partIDs_ = std::pair<int, int>(5,6);

    Match::Constraint sym3;
    sym3.type_ = SYMMETRY;
    sym3.partIndices_ = std::pair<int, int>(1,2);
    sym3.partIDs_ = std::pair<int, int>(3,4);

    Match::Constraint con1;
    con1.type_ = CONTACT;
    con1.partIndices_ = std::pair<int, int>(5,0);
    con1.partIDs_ = std::pair<int, int>(7,2);

    Match::Constraint con2;
    con2.type_ = CONTACT;
    con2.partIndices_ = std::pair<int, int>(6,0);
    con2.partIDs_ = std::pair<int, int>(8,2);

    Match::Constraint con3;
    con3.type_ = CONTACT;
    con3.partIndices_ = std::pair<int, int>(7,0);
    con3.partIDs_ = std::pair<int, int>(9,2);

    Match::Constraint con4;
    con4.type_ = CONTACT;
    con4.partIndices_ = std::pair<int, int>(3,7);
    con4.partIDs_ = std::pair<int, int>(5,9);

    Match::Constraint con5;
    con5.type_ = CONTACT;
    con5.partIndices_ = std::pair<int, int>(4,7);
    con5.partIDs_ = std::pair<int, int>(6,9);

    Match::Constraint con6;
    con6.type_ = CONTACT;
    con6.partIndices_ = std::pair<int, int>(1,5);
    con6.partIDs_ = std::pair<int, int>(3,7);

    Match::Constraint con7;
    con7.type_ = CONTACT;
    con7.partIndices_ = std::pair<int, int>(2,6);
    con7.partIDs_ = std::pair<int, int>(4,8);

    tmConstraints.push_back(sym1);
//    tmConstraints.push_back(sym2);
//    tmConstraints.push_back(sym3);
//    tmConstraints.push_back(con1);
//    tmConstraints.push_back(con2);
//    tmConstraints.push_back(con3);
//    tmConstraints.push_back(con4);
//    tmConstraints.push_back(con5);
//    tmConstraints.push_back(con6);
//    tmConstraints.push_back(con7);
    
    nSymmetryConstraints_ = 1;
    

}

void TemplateExplorationWidget::initHelicopterConstraints()
{
    
}

void TemplateExplorationWidget::initPlaneConstraints()
{
    std::vector<Match::Constraint>& tmConstraints = templateMatch_.constraints();
    
    //PLANE CONSTRAINTS
    Match::Constraint sym1;
    sym1.type_ = SYMMETRY;
    sym1.partIndices_ = std::pair<int, int>(1,2);
    sym1.partIDs_ = std::pair<int, int>(3,4);
    
    Match::Constraint sym2;
    sym2.type_ = SYMMETRY;
    sym2.partIndices_ = std::pair<int, int>(4,5);
    sym2.partIDs_ = std::pair<int, int>(6,7);
    
    tmConstraints.push_back(sym1);
    tmConstraints.push_back(sym2);
    
}

void TemplateExplorationWidget::initPlaneSidConstraints()
{
    std::vector<Match::Constraint>& tmConstraints = templateMatch_.constraints();
    
    //PLANE CONSTRAINTS
    Match::Constraint sym1;
    sym1.type_ = SYMMETRY;
    sym1.partIndices_ = std::pair<int, int>(5,6);
    sym1.partIDs_ = std::pair<int, int>(7,8);

    Match::Constraint sym2;
    sym2.type_ = SYMMETRY;
    sym2.partIndices_ = std::pair<int, int>(3,4);
    sym2.partIDs_ = std::pair<int, int>(5,6);

    Match::Constraint sym3;
    sym3.type_ = SYMMETRY;
    sym3.partIndices_ = std::pair<int, int>(1,2);
    sym3.partIDs_ = std::pair<int, int>(3,4);

    Match::Constraint sym4;
    sym4.type_ = SYMMETRY;
    sym4.partIndices_ = std::pair<int, int>(7,8);
    sym4.partIDs_ = std::pair<int, int>(9,10);
    
    Match::Constraint sym5;
    sym5.type_ = SYMMETRY;
    sym5.partIndices_ = std::pair<int, int>(9,10);
    sym5.partIDs_ = std::pair<int, int>(11,12);
    
    Match::Constraint sym6;
    sym6.type_ = SYMMETRY;
    sym6.partIndices_ = std::pair<int, int>(11,12);
    sym6.partIDs_ = std::pair<int, int>(13,14);
    
    Match::Constraint sym7;
    sym7.type_ = SYMMETRY;
    sym7.partIndices_ = std::pair<int, int>(14,15);
    sym7.partIDs_ = std::pair<int, int>(16,17);
    
    Match::Constraint sym8;
    sym8.type_ = SYMMETRY;
    sym8.partIndices_ = std::pair<int, int>(16,17);
    sym8.partIDs_ = std::pair<int, int>(18,19);
    
    Match::Constraint sym9;
    sym9.type_ = SYMMETRY;
    sym9.partIndices_ = std::pair<int, int>(18,19);
    sym9.partIDs_ = std::pair<int, int>(20,21);
    
    Match::Constraint sym10;
    sym10.type_ = SYMMETRY;
    sym10.partIndices_ = std::pair<int, int>(20,21);
    sym10.partIDs_ = std::pair<int, int>(22,23);
    
    Match::Constraint sym11;
    sym11.type_ = SYMMETRY;
    sym11.partIndices_ = std::pair<int, int>(22,23);
    sym11.partIDs_ = std::pair<int, int>(24,25);
    
    Match::Constraint sym12;
    sym12.type_ = SYMMETRY;
    sym12.partIndices_ = std::pair<int, int>(24,25);
    sym12.partIDs_ = std::pair<int, int>(26,27);
    
    Match::Constraint sym13;
    sym13.type_ = SYMMETRY;
    sym13.partIndices_ = std::pair<int, int>(26,27);
    sym13.partIDs_ = std::pair<int, int>(28,29);
    
    
    Match::Constraint con1;
    con1.type_ = CONTACT;
    con1.partIndices_ = std::pair<int, int>(5,0);
    con1.partIDs_ = std::pair<int, int>(7,2);

    Match::Constraint con2;
    con2.type_ = CONTACT;
    con2.partIndices_ = std::pair<int, int>(6,0);
    con2.partIDs_ = std::pair<int, int>(8,2);

    Match::Constraint con3;
    con3.type_ = CONTACT;
    con3.partIndices_ = std::pair<int, int>(7,0);
    con3.partIDs_ = std::pair<int, int>(9,2);

    Match::Constraint con4;
    con4.type_ = CONTACT;
    con4.partIndices_ = std::pair<int, int>(3,7);
    con4.partIDs_ = std::pair<int, int>(5,9);

    Match::Constraint con5;
    con5.type_ = CONTACT;
    con5.partIndices_ = std::pair<int, int>(4,7);
    con5.partIDs_ = std::pair<int, int>(6,9);

    Match::Constraint con6;
    con6.type_ = CONTACT;
    con6.partIndices_ = std::pair<int, int>(1,5);
    con6.partIDs_ = std::pair<int, int>(3,7);

    Match::Constraint con7;
    con7.type_ = CONTACT;
    con7.partIndices_ = std::pair<int, int>(2,6);
    con7.partIDs_ = std::pair<int, int>(4,8);

    tmConstraints.push_back(sym1);
    tmConstraints.push_back(sym2);
    tmConstraints.push_back(sym3);
    tmConstraints.push_back(sym4);
    tmConstraints.push_back(sym5);
    tmConstraints.push_back(sym6);
    tmConstraints.push_back(sym7);
    tmConstraints.push_back(sym8);
    tmConstraints.push_back(sym9);
    tmConstraints.push_back(sym10);
    tmConstraints.push_back(sym11);
    tmConstraints.push_back(sym12);
    tmConstraints.push_back(sym13);
//    tmConstraints.push_back(con1);
//    tmConstraints.push_back(con2);
//    tmConstraints.push_back(con3);
//    tmConstraints.push_back(con4);
//    tmConstraints.push_back(con5);
//    tmConstraints.push_back(con6);
//    tmConstraints.push_back(con7);
    
    nSymmetryConstraints_ = 13;
    
}

void TemplateExplorationWidget::slotToggleConstraints()
{
    preserveConstraints_ = !preserveConstraints_;
    
    emit constraintsChanged(preserveConstraints_ ? QString("Constraints: ON") : QString("Constraints: OFF") );
}

void TemplateExplorationWidget::slotToggleSingleColor()
{
    deformedNearestOption_.overrideColor(chkColorOverride_->isChecked());
    
    filteredMatches_.at(nearestPoints_[nearestNeighbourLevel_].index_)->overrideColor(chkColorOverride_->isChecked());
    
    //qDebug() << "Override color for neighbor" << filteredMatches_.at(nearestPoints_[nearestNeighbourLevel_].index_)->filename();
    
    emit requestRedraw(0);
}

void TemplateExplorationWidget::slotShowNextNeighbour()
{
    if(explorationMode_ == SHOW_CLUSTERS || explorationMode_ == SHOW_CLUSTER)
    {
        nearestNeighbourLevel_ = (nearestNeighbourLevel_ + 1) % nofNN_;
        
        int nnIndex = nearestPoints_[nearestNeighbourLevel_].index_;
        
        if (nnIndex>=0 && nnIndex<filteredMatches_.size())
        {
            Match* nMatch = filteredMatches_.at(nnIndex);
            nMatch->openMeshIfNotOpened();
            int displayMode = 0;
            
            displayMode |= MESH;
            nMatch->setDisplayMode(displayMode);
            
            std::vector<Shape*> shapes;
            shapes.push_back(nMatch);
            
            emit selectedShapesChanged(shapes,0);
            
            emit interactionModeChanged(QString("Interaction mode: Showing neighbour %1 of %2").arg(nearestNeighbourLevel_+1).arg(nofNN_));
            
        }
        
    }
}

void TemplateExplorationWidget::slotShowPreviousNeighbour()
{
    if(explorationMode_ == SHOW_CLUSTERS || explorationMode_ == SHOW_CLUSTER)
    {
        nearestNeighbourLevel_ = nearestNeighbourLevel_ -  1;
        if (nearestNeighbourLevel_ < 0)
        {
            nearestNeighbourLevel_ = nofNN_ - 1;
        }
        
        int nnIndex = nearestPoints_[nearestNeighbourLevel_].index_;
        
        if (nnIndex>=0 && nnIndex<filteredMatches_.size())
        {
            Match* nMatch = filteredMatches_.at(nnIndex);
            nMatch->openMeshIfNotOpened();
            int displayMode = 0;
            
            displayMode |= MESH;
            nMatch->setDisplayMode(displayMode);
            
            std::vector<Shape*> shapes;
            shapes.push_back(nMatch);
            
            emit selectedShapesChanged(shapes,0);
            
            emit interactionModeChanged(QString("Interaction mode: Showing neighbour %1 of %2").arg(nearestNeighbourLevel_+1).arg(nofNN_));
        }
    }
}

void TemplateExplorationWidget::slotSaveSynthesizedModel()
{
    QString dirpath = RESULTS_PATH;
    dirpath+= QDateTime::currentDateTime().toString( "yyMMddhhmmss" );
    dirpath+= "/";
    
    QDir().mkpath(dirpath);
    
    QString name = dirpath + "syn-model-";
    name += QDateTime::currentDateTime().toString( "yyMMddhhmmss" );
    
    QString logname = name;
    
    name += ".off";
    
    logname += ".log.txt";
    
    deformedNearestOption_.setMeshFilename(name);
    
    deformedNearestOption_.savePartMeshes();
    
    QFile file(logname);
    
    if (!file.open(QIODevice::WriteOnly))
    {
        qCritical() << "Could not open file " << logname ;
        return;
    }
    
    QTextStream out(&file);
    
    out << "Number of neighbors chosen: " << nnUsed_ << "\n";
    out << "Number of independent parts: " << nIndependentParts_ << "\n";
    out << "Location: " << selectedPoint_[0] << " , " << selectedPoint_[1] << "\n";
    
    Shape::Mesh cMesh;
    
    std::vector<Match::Part>& cParts = deformedNearestOption_.parts();
    
    std::vector<Match::Part>::iterator partIt(cParts.begin()), partEnd(cParts.end());
    
    int vertexOffset = 0;
    
    for (; partIt!=partEnd; ++partIt)
    {
        Shape::Mesh& cPartMesh = partIt->partShape_.mesh();
        
        typename Shape::Mesh::ConstVertexIter vIt(cPartMesh.vertices_begin()), vEnd(cPartMesh.vertices_end());
        
        for (; vIt!=vEnd; ++vIt)
        {
            cMesh.add_vertex(cPartMesh.point(vIt));
        }
        
        typename Shape::Mesh::ConstFaceIter fIt(cPartMesh.faces_begin()), fEnd(cPartMesh.faces_end());
        
        for (; fIt!=fEnd; ++fIt)
        {
            typename Shape::Mesh::ConstFaceVertexIter fvIt = cPartMesh.cfv_iter(fIt.handle());
            
            typename Shape::Mesh::VertexHandle v0 = fvIt;
            ++fvIt;
            typename Shape::Mesh::VertexHandle v1 = fvIt;
            ++fvIt;
            typename Shape::Mesh::VertexHandle v2 = fvIt;
            ++fvIt;
            
            cMesh.add_face(Shape::Mesh::VertexHandle(v0.idx()+vertexOffset),Shape::Mesh::VertexHandle(v1.idx()+vertexOffset),Shape::Mesh::VertexHandle(v2.idx()+vertexOffset));
        }
        
        vertexOffset+=cPartMesh.n_vertices();
        
        std::vector< std::pair<int, double> >& partRank = nnIndexPartScoreSorted_[partIt->partID_];
        
        if (preserveConstraints_)
        {
            // Check if the part is involved in any symmetry constraints
            std::vector<Match::Constraint>& tmConstraints = templateMatch_.constraints();
            
            std::vector<Match::Constraint>::iterator tmConstraintsIt(tmConstraints.begin()), tmConstraintsEnd(tmConstraints.end());

            int tmConstraintsSize = tmConstraints.size();
            int i=0;
            
            for (; tmConstraintsIt!= tmConstraintsEnd; ++tmConstraintsIt)
            {
                if (tmConstraintsIt->type_ == SYMMETRY)
                {
                    if( tmConstraintsIt->partIDs_.first == partIt->partID_)
                    {
                        std::vector< std::pair<int, double> >& symmetricPartRank = nnIndexPartScoreSorted_[tmConstraintsIt->partIDs_.second];
                        
                        double score1 = partRank.at(nnPartScoreVectorIndex_[partIt->partID_]).second;
                        
                        double score2 = symmetricPartRank.at(nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.second]).second;
                        
                        double finalScore = 0;
                        
                        int nearestMatchIndex = -1;
                        
                        if (score1 <= score2)
                        {
                            nearestMatchIndex = partRank.at(nnPartScoreVectorIndex_[partIt->partID_]).first;
                            finalScore = score1;
                        }
                        else
                        {
                            nearestMatchIndex = symmetricPartRank.at(nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.second]).first;
                            finalScore = score2;
                        }
                        
                        if (nearestMatchIndex<0 || nearestMatchIndex>=filteredMatches_.size())
                        {
                            qCritical() << "CANNOT SAVE STATS FOR Part id: " << partIt->partID_ << " nearest neighbor index is invalid!!";
                        }
                        else
                        {
                            Match* nMatch = filteredMatches_.at(nearestMatchIndex);
                            out << "Part ID: " << partIt->partID_ << " , " << nMatch->filename().split("/").last() << " , " << finalScore << " , " << partClicks_[partIt->partID_] << "\n";
                        }
                        break;
                    }
                        
                    if(tmConstraintsIt->partIDs_.second == partIt->partID_)
                    {
                        std::vector< std::pair<int, double> >& symmetricPartRank = nnIndexPartScoreSorted_[tmConstraintsIt->partIDs_.first];
                        
                        double score1 = partRank.at(nnPartScoreVectorIndex_[partIt->partID_]).second;
                        
                        double score2 = symmetricPartRank.at(nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.first]).second;
                        
                        double finalScore = 0;
                        
                        int nearestMatchIndex = -1;
                        
                        if (score1 <= score2)
                        {
                            nearestMatchIndex = partRank.at(nnPartScoreVectorIndex_[partIt->partID_]).first;
                            finalScore = score1;
                        }
                        else
                        {
                            nearestMatchIndex = symmetricPartRank.at(nnPartScoreVectorIndex_[tmConstraintsIt->partIDs_.first]).first;
                            finalScore = score2;
                        }
                        
                        if (nearestMatchIndex<0 || nearestMatchIndex>=filteredMatches_.size())
                        {
                            qCritical() << "CANNOT SAVE STATS FOR Part id: " << partIt->partID_ << " nearest neighbor index is invalid!!";
                        }
                        else
                        {
                            Match* nMatch = filteredMatches_.at(nearestMatchIndex);
                            out << "Part ID: " << partIt->partID_ << " , " << nMatch->filename().split("/").last() << " , " << finalScore << " , " << partClicks_[partIt->partID_] << "\n";
                        }
                        break;
                    }
                }
                i++;
            }
            
            if (i == tmConstraintsSize)
            {
                int nearestMatchIndex = partRank.at(nnPartScoreVectorIndex_[partIt->partID_]).first;
                
                if (nearestMatchIndex<0 || nearestMatchIndex>=filteredMatches_.size())
                {
                    qCritical() << "CANNOT SAVE STATS FOR Part id: " << partIt->partID_ << " nearest neighbor index is invalid!!";
                }
                else
                {
                    Match* nMatch = filteredMatches_.at(nearestMatchIndex);
                    out << "Part ID: " << partIt->partID_ << " , " << nMatch->filename().split("/").last() << " , " << partRank.at(nnPartScoreVectorIndex_[partIt->partID_]).second << " , " << partClicks_[partIt->partID_] << "\n";
                }
            }

        }
        else
        {
            int nearestMatchIndex = partRank.at(nnPartScoreVectorIndex_[partIt->partID_]).first;
            
            if (nearestMatchIndex<0 || nearestMatchIndex>=filteredMatches_.size())
            {
                qCritical() << "CANNOT SAVE STATS FOR Part id: " << partIt->partID_ << " nearest neighbor index is invalid!!";
            }
            else
            {
                Match* nMatch = filteredMatches_.at(nearestMatchIndex);
                out << "Part ID: " << partIt->partID_ << " , " << nMatch->filename().split("/").last() << " , " << partRank.at(nnPartScoreVectorIndex_[partIt->partID_]).second << " , " << partClicks_[partIt->partID_] << "\n";
            }

        }
    }

    OpenMesh::IO::write_mesh(cMesh, name.toStdString());
    
    TIMELOG->append(QString("%1 : saved_synthesized_model").arg((qlonglong)QDateTime::currentMSecsSinceEpoch()));
}


void TemplateExplorationWidget::slotSaveGeneralSettings(QTextStream& _out)
{
    _out << "Number ups: " << noUp_ << "\n";
    _out << "Number downs: " << noDown_ << "\n";
    
}

void TemplateExplorationWidget::slotSetForceNeighborIndex(const QString& _text)
{
    
    forcedNeighborIndex_ = _text.toInt();
}


void TemplateExplorationWidget::slotSaveSelectedMatches()
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
    
    QFile file(filename);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical() << "Could not open file " << filename;
        return;
    }
    
    
    
    QTextStream out(&file);
    
    std::vector<Match*>::iterator itMatches(filteredMatches_.begin()), itMatchesEnd(filteredMatches_.end());
    
    for (; itMatches!=itMatchesEnd; ++itMatches)
    {
//        if ((**itMatches).fitError() < FIT_ERROR)
//        {
//            out << (**itMatches).filename().split("/").last().split(".").first() << "\n";
//        }
        (**itMatches).save(out);
        //std::cout << "dir path:" << directory_path.toStdString() << std::endl;
        
        (**itMatches).savePartMeshes(directory_path);
    }

    //return;
    
    // Ask for a filename
    QString filename2 = QFileDialog::getSaveFileName(0,"Choose a filename", "../");
    
    QFile file2(filename2);
    
    if (!file2.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qCritical() << "Could not open file " << filename2;
        return;
    }
    
    QTextStream out2(&file2);
    
    std::vector< std::vector<double> >::iterator pcaBasisIt(pcaBasis_.begin()), pcaBasisEnd(pcaBasis_.end());
    
    for (; pcaBasisIt!= pcaBasisEnd; ++pcaBasisIt)
    {
        std::vector<double>& cBasis = *pcaBasisIt;
        std::vector<double>::iterator cBasisIt(cBasis.begin()), cBasisEnd(cBasis.end());
        
        for (; cBasisIt!=cBasisEnd; ++cBasisIt)
        {
            out2 << *cBasisIt << ",";
        }
        out2 << "\n";
    }
    
    std::vector<double>::iterator pcaOriginIt(pcaOrigin_.begin()), pcaOriginEnd(pcaOrigin_.end());
    
    for (; pcaOriginIt!= pcaOriginEnd; ++pcaOriginIt)
    {
        out2 << *pcaOriginIt << ",";
    }
    
    out2 << "\n";
    
    

    
}

