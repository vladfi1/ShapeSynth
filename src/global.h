//
// global.h
//
// Copyright (c) 2013-2014 Melinos Averkiou <m.averkiou@cs.ucl.ac.uk>

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtOpenGL/qgl.h>

#include <QTextBrowser>

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <XForm.h>

// MATLAB
#include <engine.h>
#include <matrix.h>

struct MeshTraits : public OpenMesh::DefaultTraits
{
  HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
};

typedef OpenMesh::TriMesh_ArrayKernelT<MeshTraits>  TriangleMesh;

enum EXPLORATION_MODE
{
	SHOW_TEMPLATE,
    SHOW_SELECTED,
    SHOW_NEAREST,
    SHOW_DEFORMED,
    SHOW_DEFORMED_SUPERIMPOSED,
    SHOW_LERP,
    SELECT_CLUSTER,
    SHOW_REPRESENTATIVE,
    SHOW_DEFORMED_OPTIONS,
    SHOW_GROUPS,
    SHOW_CLUSTERS,
    SHOW_CLUSTER,
    SHOW_DEFORMED_PART_OPTIONS,
    SHOW_MATCHES_SUPERIMPOSED,
    SHOW_PART_OPTIONS,
    SHOW_RANDOM
};

enum DISPLAY_MODE
{
    BOX = 0x01,
    POINTS = 0x02,
    MESH = 0x04
};

enum CALCULATION_MODE
{
	CALCULATION_MODE_BOUNDING_BOX,
	CALCULATION_MODE_POSITION
};

enum BOX_NORMALISATION_MODE
{
    TO_UNIT_CUBE = 0,
    TO_TEMPLATE_BOX = 1,
    TO_BBOX_THEN_TEMPLATE_BOX = 2
};

enum DEBUG_TYPES
{
	NO_DEBUG = 0,
	LIGHT_DEBUG = 1,
    FULL_DEBUG = 2
};

enum CONSTRAINT_TYPE
{
    SYMMETRY = 0,
    CONTACT = 1
};

enum DATASET
{
    CHAIRS = 0,
    BIKES = 1,
    HELICOPTERS = 2,
    PLANES = 3,
    SID_PLANES = 4
};

typedef struct 
{
	int index_ = -1;
	double distance_ = std::numeric_limits<double>::max();
    
} NEAREST_POINT;

enum EMBEDDING_TYPES
{
    PCA = 0,
    FAST_SPECTRAL = 1
};

//#define USE_SLOW_WRONG 1
#define USE_SOFTWARE_RASTERIZATION 2
//#define USE_HARDWARE_RASTERIZATION 3

static const int MAX_NUM_OF_COLORS = 100;

extern QString COLLECTION_FILE_PATH;
extern std::string MATLAB_FILE_PATH;
extern std::string MATLAB_APP_PATH;

extern QString MESH_PATH;
extern QString TEMPLATE_ICON_PATH;
extern QString MATCH_ICON_PATH;
extern QString MODEL_ICON_PATH;

extern QString RESULTS_PATH;

extern bool PRELOAD_MODELS;

extern DATASET LOADED_DATASET;
extern float FIT_ERROR;
extern int MAX_NUM_CLUSTERS_TO_SHOW;
extern int MIN_CLUSTER_POPULATION;

extern int NUM_OF_NEAREST_NEIGHBOURS;
extern int NUM_PARAMS_BOX;
extern int NUM_PARAMS_POS;
extern EMBEDDING_TYPES EMBEDDING_MODE;

extern int NUM_EQUATIONS_SYMMETRY;
extern int NUM_EQUATIONS_CONTACT;

extern DEBUG_TYPES DEBUG_MODE;
extern bool CREATE_SLW;
extern bool CREATE_TEW;
extern bool CREATE_QWTPLOTW;
extern bool CREATE_TEVW;
extern bool CREATE_MVW;
extern bool CREATE_LOGW;

extern bool SHOW_SLW;
extern bool SHOW_TEW;
extern bool SHOW_QWTPLOTW;
extern bool SHOW_TEVW;
extern bool SHOW_MVW;
extern bool SHOW_LOGW;
extern int NOF_MVW;

extern bool SAVE_DESCRIPTOR;
extern bool ALIGN_MATCH_POINTS_BEFORE_SAVING;
extern bool  ALIGN_MATCH_MESH_BEFORE_SAVING;
extern bool  NORMALISE_MATCH_MESH_BEFORE_SAVING;
extern bool  RECOMPUTE_BOXES_BEFORE_SAVING;

extern bool OPEN_DESCRIPTOR;
extern bool  ALIGN_MATCH_MESH_AFTER_OPENING;
extern bool NORMALISE_MATCH_MESH_AFTER_OPENING;
extern bool OPEN_ORIGINAL_MESH;
extern bool  OPEN_PART_MESHES_AFTER_OPENING_MATCH_MESH;

extern int APP_WINDOW_WIDTH;
extern int APP_WINDOW_HEIGHT;

extern int CLUSTER_VIEW_ICON_HEIGHT;
extern int CLUSTER_VIEW_ICON_PADDING;
extern int CLUSTER_VIEW_ICON_FRAME_THICKNESS;
extern int CLUSTER_VIEW_ICON_SPACING;

extern int EXPLORATION_VIEW_ICON_HEIGHT;

extern int PART_DESC_SIZE_ROWS;
extern int PART_DESC_SIZE_COLS;
extern int PART_DESC_SIZE;

// Setup some random colors to paint our objects
extern GLfloat shapeColors[MAX_NUM_OF_COLORS][4];

extern QColor tewColors[MAX_NUM_OF_COLORS];

extern QTextBrowser* TIMELOG;

#endif
