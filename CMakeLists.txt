project(bezitopo)
cmake_minimum_required(VERSION 3.1.0)
# Copyright 2012,2013,2014,2015,2016,2017 Pierre Abbat.
# This file is part of Bezitopo.
# 
# Bezitopo is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Bezitopo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Bezitopo. If not, see <http://www.gnu.org/licenses/>.

include(TestBigEndian)
include(CheckTypeSize)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Gui)
qt5_add_resources(lib_resources viewtin.qrc)

add_executable(bezitopo absorient.cpp angle.cpp arc.cpp bezier3d.cpp bezier.cpp
               bezitopo.cpp binio.cpp breakline.cpp circle.cpp closure.cpp cogo.cpp
               cogospiral.cpp color.cpp contour.cpp csv.cpp cvtmeas.cpp document.cpp
               drawobj.cpp ellipsoid.cpp geoid.cpp geoidboundary.cpp halton.cpp
               icommon.cpp kml.cpp latlong.cpp layer.cpp ldecimal.cpp
               manysum.cpp measure.cpp mkpoint.cpp objlist.cpp plot.cpp pnezd.cpp
               point.cpp pointlist.cpp polyline.cpp projection.cpp ps.cpp qindex.cpp
               quaternion.cpp random.cpp raster.cpp relprime.cpp rootfind.cpp
               scalefactor.cpp spiral.cpp spolygon.cpp stl.cpp test.cpp segment.cpp
               tin.cpp vball.cpp vcurve.cpp)
add_executable(bezitest absorient.cpp angle.cpp arc.cpp bezier3d.cpp bezier.cpp
               bezitest.cpp bicubic.cpp binio.cpp breakline.cpp circle.cpp cogo.cpp
               cogospiral.cpp color.cpp contour.cpp csv.cpp document.cpp drawobj.cpp
               ellipsoid.cpp geoid.cpp geoidboundary.cpp
               halton.cpp histogram.cpp hlattice.cpp hnum.cpp kml.cpp
               latlong.cpp layer.cpp ldecimal.cpp manysum.cpp
               matrix.cpp measure.cpp objlist.cpp plot.cpp pnezd.cpp point.cpp
               pointlist.cpp polyline.cpp projection.cpp ps.cpp qindex.cpp quaternion.cpp
               random.cpp raster.cpp refinegeoid.cpp relprime.cpp rootfind.cpp
               sourcegeoid.cpp spiral.cpp spolygon.cpp
               stl.cpp test.cpp segment.cpp tin.cpp vball.cpp vcurve.cpp zoom.cpp)
add_executable(convertgeoid angle.cpp arc.cpp bezier.cpp bezier3d.cpp bicubic.cpp
               binio.cpp cogo.cpp contour.cpp
               convertgeoid.cpp csv.cpp document.cpp drawobj.cpp
               ellipsoid.cpp geoid.cpp geoidboundary.cpp halton.cpp histogram.cpp
               hlattice.cpp kml.cpp latlong.cpp ldecimal.cpp manysum.cpp
               matrix.cpp measure.cpp pnezd.cpp point.cpp pointlist.cpp polyline.cpp
               projection.cpp ps.cpp qindex.cpp quaternion.cpp random.cpp raster.cpp
               refinegeoid.cpp relprime.cpp rootfind.cpp segment.cpp sourcegeoid.cpp
               spiral.cpp spolygon.cpp tin.cpp vball.cpp vcurve.cpp)
add_executable(viewtin angle.cpp arc.cpp bezier.cpp bezier3d.cpp binio.cpp breakline.cpp
               cidialog.cpp circle.cpp cogo.cpp color.cpp
               contour.cpp csv.cpp document.cpp drawobj.cpp ellipsoid.cpp
               geoid.cpp geoidboundary.cpp kml.cpp
               latlong.cpp ldecimal.cpp linetype.cpp
               manysum.cpp measure.cpp measurebutton.cpp
               pnezd.cpp point.cpp pointlist.cpp polyline.cpp projection.cpp
               ps.cpp qindex.cpp quaternion.cpp random.cpp relprime.cpp
               rootfind.cpp segment.cpp spiral.cpp spolygon.cpp test.cpp tin.cpp
               tinwindow.cpp vball.cpp vcurve.cpp viewtin.cpp zoom.cpp zoombutton.cpp
               ${lib_resources})
add_executable(pangeoid geoidwindow.cpp pangeoid.cpp zoom.cpp)
target_link_libraries(bezitopo Qt5::Widgets Qt5::Core)
target_link_libraries(bezitest Qt5::Widgets Qt5::Core)
target_link_libraries(convertgeoid Qt5::Widgets Qt5::Core)
target_link_libraries(viewtin Qt5::Widgets Qt5::Core)
target_link_libraries(pangeoid Qt5::Widgets Qt5::Core)
target_compile_definitions(convertgeoid PUBLIC CONVERTGEOID NUMSGEOID)
target_compile_definitions(bezitest PUBLIC NUMSGEOID)
#target_compile_definitions(pangeoid PUBLIC CONVERTGEOID NUMSGEOID)

include(CheckIncludeFiles)
check_include_files(time.h HAVE_TIME_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(sys/resource.h HAVE_SYS_RESOURCE_H)
check_include_files(windows.h HAVE_WINDOWS_H)

install(TARGETS bezitopo convertgeoid DESTINATION bin)

test_big_endian(BIGENDIAN)
check_type_size("int" INT)
check_type_size("int *" INT_POINTER)

set(BEZITOPO_MAJOR_VERSION 0)
set(BEZITOPO_MINOR_VERSION 1)
set(BEZITOPO_PATCH_VERSION 2)
set(BEZITOPO_VERSION ${BEZITOPO_MAJOR_VERSION}.${BEZITOPO_MINOR_VERSION}.${BEZITOPO_PATCH_VERSION})

include_directories(${PROJECT_BINARY_DIR})
configure_file (config.h.in config.h)

set(CPACK_PACKAGE_VERSION_MAJOR ${BEZITOPO_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${BEZITOPO_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${BEZITOPO_PATCH_VERSION})
set(CPACK_SOURCE_IGNORE_FILES /\\\\.git;.*~)
include(CPack)

include(CTest)
add_test(geom bezitest area3 in intersection invalidintersectionlozenge invalidintersectionaster)
add_test(arith bezitest relprime manysum brent newton zoom)
add_test(measure bezitest measure)
add_test(calculus bezitest parabinter derivs)
add_test(random bezitest random)
add_test(matrix bezitest matrix)
add_test(quaternion bezitest quaternion)
add_test(bezier bezitest triangle vcurve trianglecontours grad)
add_test(pointlist bezitest copytopopoints)
add_test(maketin bezitest maketin123 maketindouble maketinaster maketinbigaster maketinstraightrow maketinlongandthin maketinlozenge maketinring maketinwheel maketinellipse)
add_test(angle bezitest integertrig angleconv)
add_test(minquad bezitest minquad)
add_test(segment bezitest segment)
add_test(arc bezitest arc)
add_test(spiral bezitest spiral spiralarc cogospiral manyarc)
add_test(qindex bezitest qindex)
add_test(makegrad bezitest makegrad)
add_test(raster bezitest rasterdraw)
add_test(dirbound bezitest dirbound)
add_test(stl bezitest stl)
add_test(halton bezitest halton)
add_test(polyline bezitest polyline)
add_test(bezier3d bezitest bezier3d)
add_test(fileio bezitest csvline pnezd ldecimal)
add_test(geodesy bezitest ellipsoid projection vball geoid geint)
add_test(convertgeoid0 bezitest hlattice bicubic smooth5 quadhash)
add_test(convertgeoid1 bezitest smallcircle cylinterval geoidboundary kml)
add_test(color bezitest color)
add_test(contour bezitest contour foldcontour zigzagcontour tracingstop)
add_test(roscat bezitest roscat absorient)
add_test(histogram bezitest histogram)
