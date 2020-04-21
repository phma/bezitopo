project(bezitopo)
cmake_minimum_required(VERSION 3.1.0)
# Copyright 2012-2020 Pierre Abbat.
# Copyright 2020 звездочёт.
# This file is part of Bezitopo.
#
# Bezitopo is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# Bezitopo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License and Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# and Lesser General Public License along with Bezitopo. If not, see
# <http://www.gnu.org/licenses/>.

include(TestBigEndian)
include(CheckTypeSize)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/bezitopo)

option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." FALSE)
if (${FORCE_COLORED_OUTPUT})
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options (-fdiagnostics-color=always)
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options (-fcolor-diagnostics)
  endif ()
endif ()

set(FUZZ none)
#Setting FUZZ to boldatni disables reading all geoid formats except boldatni
#in convertgeoid and disables the boldatni magic string check.

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(Qt5 COMPONENTS Core Widgets Gui LinguistTools REQUIRED)
find_package(FFTW)
qt5_add_resources(lib_resources src/viewtin.qrc)
qt5_add_translation(qm_files src/bezitopo_en.ts
                             src/bezitopo_es.ts)
# To update translations, run "lupdate *.cpp -ts *.ts" in the source directory.

set(header_files src/angle.h
                 src/arc.h
                 src/bezier.h
                 src/bezier3d.h
                 src/binio.h
                 src/boundrect.h
                 src/breakline.h
                 src/circle.h
                 src/cogo.h
                 src/cogospiral.h
                 src/color.h
                 src/contour.h
                 src/csv.h
                 src/document.h
                 src/drawobj.h
                 src/ellipsoid.h
                 src/except.h
                 src/geoid.h
                 src/geoidboundary.h
                 src/globals.h
                 src/halton.h
                 src/intloop.h
                 src/latlong.h
                 src/layer.h
                 src/ldecimal.h
                 src/leastsquares.h
                 src/linetype.h
                 src/manyarc.h
                 src/manysum.h
                 src/matrix.h
                 src/measure.h
                 src/minquad.h
                 src/objlist.h
                 src/penwidth.h
                 src/pnezd.h
                 src/point.h
                 src/pointlist.h
                 src/polyline.h
                 src/projection.h
                 src/ps.h
                 src/qindex.h
                 src/quaternion.h
                 src/random.h
                 src/relprime.h
                 src/rootfind.h
                 src/roscat.h
                 src/segment.h
                 src/spiral.h
                 src/spolygon.h
                 src/tin.h
                 src/vball.h
                 src/vcurve.h
                 src/xml.h
                 src/xyz.h
                 src/zoom.h)

# MS Visual C++ cannot build both static and shared libraries with the same name.
# If you ask for a static library, it makes bezitopo.lib. If you ask for a
# shared library, it makes bezitopo.dll, which is the shared library, and
# bezitopo.lib, the import library for the DLL.
# This is not a problem for MinGW, which makes three files: libbezitopo.a,
# the static library; libbezitopo.dll, the shared library; and libbezitopo.dll.a,
# the import library for the DLL.
set(MAKE_STATIC bezilib0)
set(MAKE_SHARED bezilib1)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" AND DEFINED MSVC_STATIC)
  set(MAKE_SHARED "")
endif ()
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" AND NOT DEFINED MSVC_STATIC)
  set(MAKE_STATIC "")
endif ()
set(sourcelib src/angle.cpp
              src/arc.cpp
              src/bezier.cpp
              src/bezier3d.cpp
              src/binio.cpp
              src/boundrect.cpp
              src/breakline.cpp
              src/circle.cpp
              src/cogo.cpp 
              src/cogospiral.cpp
              src/color.cpp
              src/contour.cpp
              src/csv.cpp
              src/document.cpp
              src/drawobj.cpp
              src/ellipsoid.cpp
              src/except.cpp
              src/geoid.cpp
              src/geoidboundary.cpp
              src/halton.cpp
              src/intloop.cpp
              src/latlong.cpp
              src/layer.cpp
              src/ldecimal.cpp
              src/leastsquares.cpp
              src/manyarc.cpp
              src/manysum.cpp
              src/matrix.cpp
              src/measure.cpp
              src/minquad.cpp
              src/objlist.cpp
              src/penwidth.cpp
              src/pnezd.cpp
              src/point.cpp
              src/pointlist.cpp
              src/polyline.cpp
              src/projection.cpp
              src/ps.cpp
              src/qindex.cpp
              src/quaternion.cpp
              src/random.cpp
              src/relprime.cpp
              src/rootfind.cpp
              src/segment.cpp
              src/smooth5.cpp
              src/spiral.cpp
              src/spolygon.cpp
              src/stl.cpp
              src/tin.cpp
              src/vball.cpp
              src/vcurve.cpp
              src/xml.cpp)
if (MAKE_STATIC)
add_library(bezilib0 STATIC ${sourcelib})
endif ()
if (MAKE_SHARED)
add_library(bezilib1 SHARED ${sourcelib})
endif ()
add_executable(bezitopo ${sourcelib}
                        src/absorient.cpp
                        src/bezitopo.cpp
                        src/closure.cpp
                        src/cvtmeas.cpp
                        src/firstarg.cpp
                        src/icommon.cpp
                        src/kml.cpp
                        src/mkpoint.cpp
                        src/plot.cpp
                        src/raster.cpp
                        src/scalefactor.cpp
                        src/test.cpp)
add_executable(bezitest ${sourcelib}
                        src/absorient.cpp
                        src/bezitest.cpp
                        src/bicubic.cpp
                        src/carlsontin.cpp
                        src/crosssection.cpp
                        src/dxf.cpp
                        src/firstarg.cpp
                        src/histogram.cpp
                        src/hlattice.cpp
                        src/hnum.cpp
                        src/kml.cpp
                        src/plot.cpp
                        src/ptin.cpp
                        src/raster.cpp
                        src/rawdata.cpp
                        src/readtin.cpp
                        src/refinegeoid.cpp
                        src/sourcegeoid.cpp
                        src/test.cpp
                        src/textfile.cpp
                        src/tintext.cpp
                        src/zoom.cpp)
add_executable(clotilde ${sourcelib}
                        src/clotilde.cpp
                        src/cmdopt.cpp)
add_executable(convertgeoid ${sourcelib}
                            src/bicubic.cpp
                            src/cmdopt.cpp
                            src/convertgeoid.cpp
                            src/histogram.cpp
                            src/hlattice.cpp
                            src/kml.cpp
                            src/raster.cpp
                            src/refinegeoid.cpp
                            src/sourcegeoid.cpp)
add_executable(viewtin ${sourcelib}
                       src/carlsontin.cpp
                       src/cidialog.cpp
                       src/dxf.cpp
                       src/factordialog.cpp
                       src/firstarg.cpp
                       src/kml.cpp
                       src/linetype.cpp
                       src/llvalidator.cpp
                       src/measurebutton.cpp
                       src/plwidget.cpp
                       src/ptin.cpp
                       src/readtin.cpp
                       src/rendercache.cpp
                       src/test.cpp
                       src/textfile.cpp
                       src/tintext.cpp
                       src/tinwindow.cpp
                       src/topocanvas.cpp
                       src/viewtin.cpp
                       src/zoom.cpp
                       src/zoombutton.cpp
                      ${lib_resources}
                      ${qm_files})
add_executable(sitecheck ${sourcelib}
                         src/carlsontin.cpp
                         src/cidialog.cpp
                         src/dxf.cpp
                         src/factordialog.cpp
                         src/firstarg.cpp
                         src/kml.cpp
                         src/linetype.cpp
                         src/llvalidator.cpp
                         src/measurebutton.cpp
                         src/plwidget.cpp
                         src/ptin.cpp
                         src/readtin.cpp
                         src/rendercache.cpp
                         src/sitecheck.cpp
                         src/sitewindow.cpp
                         src/test.cpp
                         src/textfile.cpp
                         src/tintext.cpp
                         src/topocanvas.cpp
                         src/zoom.cpp
                         src/zoombutton.cpp
                         ${lib_resources}
                         ${qm_files})
add_executable(pangeoid src/geoidwindow.cpp src/pangeoid.cpp src/zoom.cpp)
if (${FFTW_FOUND})
add_executable(transmer ${sourcelib}
                        src/transmer.cpp)
endif (${FFTW_FOUND})
if (MAKE_STATIC)
target_link_libraries(bezilib0 Qt5::Widgets Qt5::Core)
target_compile_definitions(bezilib0 PUBLIC _USE_MATH_DEFINES)
endif ()
if (MAKE_SHARED)
target_link_libraries(bezilib1 Qt5::Widgets Qt5::Core)
target_compile_definitions(bezilib1 PUBLIC _USE_MATH_DEFINES)
endif ()
target_link_libraries(bezitopo Qt5::Widgets Qt5::Core)
target_compile_definitions(bezitopo PUBLIC _USE_MATH_DEFINES)
target_link_libraries(bezitest Qt5::Widgets Qt5::Core)
target_compile_definitions(bezitest PUBLIC _USE_MATH_DEFINES)
target_link_libraries(clotilde Qt5::Widgets Qt5::Core)
target_compile_definitions(clotilde PUBLIC _USE_MATH_DEFINES)
target_link_libraries(convertgeoid Qt5::Widgets Qt5::Core)
target_compile_definitions(convertgeoid PUBLIC _USE_MATH_DEFINES)
target_link_libraries(viewtin Qt5::Widgets Qt5::Core)
target_compile_definitions(viewtin PUBLIC _USE_MATH_DEFINES)
set_target_properties(viewtin PROPERTIES WIN32_EXECUTABLE TRUE)
target_link_libraries(sitecheck Qt5::Widgets Qt5::Core)
target_compile_definitions(sitecheck PUBLIC _USE_MATH_DEFINES)
set_target_properties(sitecheck PROPERTIES WIN32_EXECUTABLE TRUE)
target_link_libraries(pangeoid Qt5::Widgets Qt5::Core)
target_compile_definitions(pangeoid PUBLIC _USE_MATH_DEFINES)
if (${FFTW_FOUND})
target_link_libraries(transmer Qt5::Widgets Qt5::Core ${FFTW_LIBRARIES})
target_compile_definitions(transmer PUBLIC _USE_MATH_DEFINES POINTLIST)
endif (${FFTW_FOUND})
# POINTLIST: the program uses pointlists. Affects BoundRect.
# CONVERTGEOID: the program reads source geoid files. Allows raster output of source geoids.
# NUMSGEOID: the geoquad class needs to count points that are in and out of source geoids.
# FLATTRIANGLE: the program handles only flat triangles.
if (MAKE_STATIC)
target_compile_definitions(bezilib0 PUBLIC POINTLIST)
endif ()
if (MAKE_SHARED)
target_compile_definitions(bezilib1 PUBLIC POINTLIST)
endif ()
target_compile_definitions(convertgeoid PUBLIC CONVERTGEOID NUMSGEOID POINTLIST)
target_compile_definitions(bezitest PUBLIC NUMSGEOID POINTLIST)
target_compile_definitions(bezitopo PUBLIC POINTLIST)
target_compile_definitions(clotilde PUBLIC POINTLIST)
target_compile_definitions(viewtin PUBLIC POINTLIST)
target_compile_definitions(sitecheck PUBLIC POINTLIST FLATTRIANGLE)
#target_compile_definitions(pangeoid PUBLIC CONVERTGEOID NUMSGEOID)

include(CheckIncludeFiles)
check_include_files(time.h HAVE_TIME_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(sys/resource.h HAVE_SYS_RESOURCE_H)
check_include_files(windows.h HAVE_WINDOWS_H)

# Define NO_INSTALL when compiling for fuzzing. This avoids the error
# "The install of the perfecttin target requires changing an RPATH", which
# occurs when using the AFL compiler wrapper with the Ninja generator.
# There is no need to install a binary built for fuzzing.
if (NOT DEFINED NO_INSTALL)
install(TARGETS bezitopo convertgeoid viewtin clotilde DESTINATION bin)
install(TARGETS ${MAKE_SHARED} ${MAKE_STATIC} DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/config.h DESTINATION include/bezitopo)
install(FILES ${qm_files} projections.txt transmer.dat DESTINATION share/bezitopo)
install(FILES ${header_files} DESTINATION include/bezitopo)
install(FILES src/bezitopo.h DESTINATION include)
endif ()

if (WIN32)
include(windeployqt)
windeployqt(sitecheck bin)
endif (WIN32)

test_big_endian(BIGENDIAN)
check_type_size("int" INT)
check_type_size("int *" INT_POINTER)

set(BEZITOPO_MAJOR_VERSION 0)
set(BEZITOPO_MINOR_VERSION 1)
set(BEZITOPO_PATCH_VERSION 5)
set(BEZITOPO_VERSION ${BEZITOPO_MAJOR_VERSION}.${BEZITOPO_MINOR_VERSION}.${BEZITOPO_PATCH_VERSION})
set(COPY_YEAR 2019)

if (MAKE_STATIC)
set_target_properties(bezilib0 PROPERTIES OUTPUT_NAME "bezitopo" VERSION ${BEZITOPO_VERSION})
endif ()
if (MAKE_SHARED)
set_target_properties(bezilib1 PROPERTIES OUTPUT_NAME "bezitopo" VERSION ${BEZITOPO_VERSION})
endif ()

include_directories(${PROJECT_BINARY_DIR})
if (${FFTW_FOUND})
include_directories(${FFTW_INCLUDES})
endif (${FFTW_FOUND})
configure_file (config.h.in config.h)
configure_file (tinytin-txt.dxf tinytin-txt.dxf COPYONLY)
configure_file (tinytin-bin.dxf tinytin-bin.dxf COPYONLY)
configure_file (transmer.dat transmer.dat COPYONLY)

set(CPACK_PACKAGE_VERSION_MAJOR ${BEZITOPO_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${BEZITOPO_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${BEZITOPO_PATCH_VERSION})
set(CPACK_SOURCE_IGNORE_FILES /\\\\.git;.*~)
include(CPack)

include(CTest)
add_test(geom bezitest area3 in intersection invalidintersectionlozenge invalidintersectionaster circle)
add_test(arith bezitest relprime manysum brent newton zoom)
add_test(measure bezitest measure)
add_test(calculus bezitest parabinter derivs)
add_test(random bezitest random)
add_test(matrix bezitest matrix)
add_test(quaternion bezitest quaternion)
add_test(bezier bezitest triangle vcurve trianglecontours grad)
add_test(pointlist bezitest copytopopoints intloop tripolygon)
add_test(maketin bezitest maketin123 maketindouble maketinaster maketinbigaster maketinstraightrow maketinlongandthin maketinlozenge maketinring maketinwheel maketinellipse)
add_test(angle bezitest integertrig angleconv)
add_test(leastsquares bezitest leastsquares)
add_test(minquad bezitest minquad)
add_test(segment bezitest segment)
add_test(arc bezitest arc)
add_test(spiral bezitest spiral spiralarc cogospiral curly manyarc)
add_test(curvefit bezitest curvefit)
add_test(qindex bezitest qindex)
add_test(makegrad bezitest makegrad)
add_test(raster bezitest rasterdraw)
add_test(dirbound bezitest dirbound)
add_test(stl bezitest stl)
add_test(dxf bezitest tindxf)
add_test(halton bezitest halton)
add_test(polyline bezitest polyline alignment)
add_test(bezier3d bezitest bezier3d)
add_test(fileio bezitest csvline pnezd ldecimal)
add_test(geodesy bezitest ellipsoid projection vball geoid geint)
add_test(convertgeoid0 bezitest hlattice bicubic smooth5 quadhash)
add_test(convertgeoid1 bezitest smallcircle cylinterval geoidboundary gpolyline kml)
add_test(layer bezitest layer color)
add_test(contour bezitest contour foldcontour zigzagcontour tracingstop)
add_test(roscat bezitest roscat absorient)
add_test(histogram bezitest histogram)
