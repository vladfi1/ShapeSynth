Linux "port" of ShapeSynth. ShapeSynth is from http://vecg.cs.ucl.ac.uk/Projects/SmartGeometry/shape_synth/shapeSynth_eg14.html.

Notes
=====
* Because ShapeSynth uses a custom version of OpenMesh, we use the headers and libs in the 3rdparty folder. The original source comes with Mac (BSD) libraries, so we recompiled on Ubuntu.
* A few bits of source code had to change.
* CMake expects Matlab to be in /usr/local/matlab-7sp1, so either install there or add a symlink.

