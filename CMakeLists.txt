project(bezitopo)
cmake_minimum_required(VERSION 2.8.12)
# Copyright 2012,2013,2014,2015,2016 Pierre Abbat.
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
add_executable(bezitopo absorient.cpp angle.cpp arc.cpp bezier3d.cpp bezier.cpp
               bezitopo.cpp binio.cpp brent.cpp closure.cpp cogo.cpp cogospiral.cpp
               color.cpp contour.cpp cvtmeas.cpp document.cpp drawobj.cpp
               ellipsoid.cpp geoid.cpp geoidboundary.cpp halton.cpp icommon.cpp
               latlong.cpp layer.cpp ldecimal.cpp
               manysum.cpp measure.cpp mkpoint.cpp objlist.cpp plot.cpp pnezd.cpp
               point.cpp pointlist.cpp polyline.cpp projection.cpp ps.cpp qindex.cpp
               quaternion.cpp random.cpp raster.cpp relprime.cpp scalefactor.cpp
               spiral.cpp spolygon.cpp stl.cpp test.cpp segment.cpp
               tin.cpp vball.cpp vcurve.cpp)
add_executable(bezitest absorient.cpp angle.cpp arc.cpp bezier3d.cpp bezier.cpp
               bezitest.cpp bicubic.cpp binio.cpp brent.cpp cogo.cpp cogospiral.cpp
               color.cpp contour.cpp document.cpp drawobj.cpp
               ellipsoid.cpp geoid.cpp geoidboundary.cpp
               halton.cpp histogram.cpp hlattice.cpp hnum.cpp kml.cpp
               latlong.cpp layer.cpp ldecimal.cpp manysum.cpp
               matrix.cpp measure.cpp objlist.cpp plot.cpp pnezd.cpp point.cpp
               pointlist.cpp polyline.cpp projection.cpp ps.cpp qindex.cpp quaternion.cpp
               random.cpp raster.cpp refinegeoid.cpp relprime.cpp sourcegeoid.cpp spiral.cpp spolygon.cpp
               stl.cpp test.cpp segment.cpp tin.cpp vball.cpp vcurve.cpp)
add_executable(convertgeoid angle.cpp arc.cpp bezier.cpp bezier3d.cpp bicubic.cpp
               binio.cpp brent.cpp cogo.cpp convertgeoid.cpp document.cpp drawobj.cpp
               ellipsoid.cpp geoid.cpp geoidboundary.cpp halton.cpp histogram.cpp
               hlattice.cpp kml.cpp latlong.cpp ldecimal.cpp manysum.cpp
               matrix.cpp measure.cpp pnezd.cpp point.cpp pointlist.cpp polyline.cpp
               ps.cpp qindex.cpp quaternion.cpp random.cpp raster.cpp refinegeoid.cpp
               relprime.cpp segment.cpp sourcegeoid.cpp spiral.cpp spolygon.cpp tin.cpp
               vball.cpp vcurve.cpp)
target_compile_definitions(convertgeoid PUBLIC CONVERTGEOID NUMSGEOID)
target_compile_definitions(bezitest PUBLIC NUMSGEOID)

include(TestBigEndian)
include(CheckTypeSize)

set(CMAKE_CXX_FLAGS "-std=c++11")
install(TARGETS bezitopo convertgeoid DESTINATION bin)

test_big_endian(BIGENDIAN)
check_type_size("int" INT)
check_type_size("int *" INT_POINTER)

set(BEZITOPO_MAJOR_VERSION 0)
set(BEZITOPO_MINOR_VERSION 1)
set(BEZITOPO_PATCH_VERSION 1)
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
add_test(arith bezitest relprime manysum brent)
add_test(calculus bezitest parabinter derivs)
add_test(random bezitest random)
add_test(matrix bezitest matrix)
add_test(quaternion bezitest quaternion)
add_test(bezier bezitest triangle vcurve trianglecontours grad)
add_test(pointlist bezitest copytopopoints)
add_test(maketin bezitest maketin123 maketindouble maketinaster maketinbigaster maketinstraightrow maketinlongandthin maketinlozenge maketinring maketinellipse)
add_test(angle bezitest integertrig angleconv)
add_test(minquad bezitest minquad)
add_test(segment bezitest segment)
add_test(arc bezitest arc)
add_test(spiral bezitest spiral spiralarc cogospiral)
add_test(qindex bezitest qindex)
add_test(makegrad bezitest makegrad)
add_test(raster bezitest rasterdraw)
add_test(dirbound bezitest dirbound)
add_test(stl bezitest stl)
add_test(halton bezitest halton)
add_test(polyline bezitest polyline)
add_test(bezier3d bezitest bezier3d)
add_test(fileio bezitest csvline ldecimal)
add_test(geodesy bezitest ellipsoid projection vball geoid geint)
add_test(convertgeoid0 bezitest hlattice bicubic smooth5 quadhash)
add_test(convertgeoid1 bezitest smallcircle cylinterval geoidboundary kml)
add_test(color bezitest color)
add_test(contour bezitest contour foldcontour zigzagcontour tracingstop)
add_test(roscat bezitest roscat absorient)
add_test(histogram bezitest histogram)
