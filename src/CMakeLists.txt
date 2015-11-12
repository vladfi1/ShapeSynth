cmake_minimum_required (VERSION 2.6)

project(ShapeSynth)

# add our macro directory to cmake search path
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/../cmake)
set (CMAKE_DEBUG_POSTFIX "d")


include (ACGCommon)
include (ACGOutput)

acg_get_version()


if (WIN32)
	add_definitions(-D_USE_MATH_DEFINES -DNOMINMAX)
	SET(CMAKE_FIND_LIBRARY_PREFIXES "")
	SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")
elseif (APPLE)
   add_definitions(-DAPPLE)
   SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
   SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib" ".a")
elseif (UNIX)
   add_definitions(-DUNIX)
   SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
   SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
endif ()

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
find_package(OpenMesh REQUIRED)
find_package(Matlab REQUIRED)
find_package(Alglib REQUIRED)

set(QT_USE_QTOPENGL 1)
include (${QT_USE_FILE})

if (WIN32)
	FILE(GLOB files_install_app_dlls "${CMAKE_BINARY_DIR}/build/*.dll")
	INSTALL(FILES ${files_install_app_dlls} DESTINATION .)
endif()


include_directories (
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${GLUT_INCLUDE_DIR}
  ${QT_INCLUDE_DIRS}
  ${OPENMESH_INCLUDE_DIRS}
  ${MATLAB_INCLUDE_DIR}
  ${ALGLIB_INCLUDE_DIRS}
)

set (targetName ShapeSynth)

# source code directories
set (directories .)

# collect all header and source files
acg_append_files (headers "*.h" ${directories})
acg_append_files (sources "*.cpp" ${directories})
acg_append_files (ui "*.ui" ${directories})

# genereate uic and moc targets
acg_qt4_autouic (uic_targets ${ui})
acg_qt4_automoc (moc_targets ${headers})


if (WIN32)
  acg_add_executable (${targetName} WIN32 ${uic_targets} ${sources} ${headers} ${moc_targets})
  # link to qtmain library to get WinMain function for a non terminal app
  target_link_libraries (${targetName} ${QT_QTMAIN_LIBRARY})
else ()
  acg_add_executable (${targetName} ${uic_targets} ${sources} ${headers} ${moc_targets})
endif ()


target_link_libraries (${targetName}
	${OPENGL_LIBRARIES}
	${GLUT_LIBRARIES}
	${QT_LIBRARIES}
	${OPENMESH_LIBRARIES}
	${MATLAB_LIBRARIES}
	${ALGLIB_LIBRARIES}
)

SET( CMAKE_CXX_FLAGS "-std=c++11 -w -Wl,-rpath-link,/usr/local/matlab-7sp1/bin/glnxa64/" )

acg_print_configure_header(ShapeSynth "ShapeSynth")
