Linux "port" of ShapeSynth. ShapeSynth is from http://vecg.cs.ucl.ac.uk/Projects/SmartGeometry/shape_synth/shapeSynth_eg14.html.

=========================================================================================================================================================================

Dependencies:

1. OpenMesh (tested with OpenMesh 2.4) 
   Since OpenMesh has been slightly edited for our purposes, we provide a compiled version of OpenMesh 2.4 for Mac OS (built on Mac OS 10.9) inside folder /3rdparty.
   
2. alglib (tested with alglib 3.8)
   We provide a compiled version of alglib 3.8 for Mac OS (built on Mac OS 10.9) inside folder /3rdparty
   
3. QT (tested with QT 4.8 - will probably work with QT 5 but the CMakeLists.txt script will need to be edited)

4. Matlab (tested with Matlab 2013b for Mac)

5. OpenGL and glut

=========================================================================================================================================================================

How to build:

1. Use cmake (minimum version required is 2.6) to build the makefiles/xcode/visual studio project files.
   Cmake scripts will look for OpenMesh and alglib in /usr/local since the application is supported for Mac OS.
   They will also try the /3rdparty folder inside the application folder. 
   The cmake script to find Matlab does not work for Mac OS, you will need to set the path to Matlab libraries manually.
   You can manually set the paths to all dependencies, or edit the cmake scripts if you like.

2. Compile using the makefiles/xcode/visual studio project files produced from cmake.


=========================================================================================================================================================================

How to run:

1. Place config.txt in the same folder as the compiled application.

2. Edit config.txt to change the paths for COLLECTION_FILE_PATH, MATLAB_FILE_PATH, MATLAB_APP_PATH

3. Mesh data (meshes, icons etc), will need to be placed in a data folder with the same name as the collection file. 
   This data folder will also be in the same folder as the collection file.
   Inside the data folder you can have the following subfolders:
   a. a MESH_PATH which is the name of the folder containing the meshes (provided).
   b. a TEMPLATE_ICON_PATH which is the name of the folder containing the template icons (provided).
   c. a MATCH_ICON_PATH which is the name of the folder containing the icons for each match - not required or used, can be left as an empty folder.
   d. a MODEL_ICON_PATH which is the name of the folder containing the icons for each mesh (provided). 
   
4. Run the compiled application, which should load the collection file. 
   The collection should take a few seconds to load depending on the number of models. 
   Do not try adding more models at runtime, this will not work.
   

=========================================================================================================================================================================
   
Notes:


* Because ShapeSynth uses a custom version of OpenMesh, we use the headers and libs in the 3rdparty folder. The original source comes with Mac (BSD) libraries, so we recompiled on Ubuntu.
* A few bits of source code had to change.
* CMake expects Matlab to be in /usr/local/matlab-7sp1, so either install there or add a symlink.

