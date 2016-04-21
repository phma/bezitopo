project(bezitopo)
cmake_minimum_required(VERSION 2.8)
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
               bezitopo.cpp binio.cpp closure.cpp cogo.cpp cogospiral.cpp color.cpp
               contour.cpp cvtmeas.cpp document.cpp drawobj.cpp
               ellipsoid.cpp geoid.cpp halton.cpp icommon.cpp layer.cpp ldecimal.cpp
               manysum.cpp measure.cpp mkpoint.cpp objlist.cpp plot.cpp pnezd.cpp
               point.cpp pointlist.cpp polyline.cpp projection.cpp ps.cpp qindex.cpp random.cpp
               raster.cpp relprime.cpp scalefactor.cpp spiral.cpp stl.cpp test.cpp
               segment.cpp tin.cpp vcurve.cpp)
add_executable(bezitest absorient.cpp angle.cpp arc.cpp bezier3d.cpp bezier.cpp
               bezitest.cpp bicubic.cpp binio.cpp closure.cpp cogo.cpp cogospiral.cpp
               contour.cpp document.cpp drawobj.cpp ellipsoid.cpp geoid.cpp color.cpp
               halton.cpp histogram.cpp hlattice.cpp layer.cpp ldecimal.cpp manysum.cpp
               measure.cpp objlist.cpp plot.cpp pnezd.cpp point.cpp pointlist.cpp
               polyline.cpp projection.cpp ps.cpp qindex.cpp random.cpp
               raster.cpp refinegeoid.cpp relprime.cpp sourcegeoid.cpp spiral.cpp
               stl.cpp test.cpp segment.cpp tin.cpp vcurve.cpp)
add_executable(convertgeoid angle.cpp arc.cpp bezier.cpp bezier3d.cpp bicubic.cpp
               binio.cpp cogo.cpp convertgeoid.cpp document.cpp drawobj.cpp ellipsoid.cpp
               geoid.cpp histogram.cpp hlattice.cpp ldecimal.cpp
               manysum.cpp measure.cpp pnezd.cpp point.cpp pointlist.cpp polyline.cpp
               ps.cpp qindex.cpp random.cpp raster.cpp refinegeoid.cpp relprime.cpp
               segment.cpp sourcegeoid.cpp spiral.cpp tin.cpp vcurve.cpp)
target_compile_definitions(convertgeoid PUBLIC CONVERTGEOID)

include(TestBigEndian)
include(CheckTypeSize)

set(CMAKE_CXX_FLAGS "-std=c++11")
install(TARGETS bezitopo convertgeoid DESTINATION bin)

test_big_endian(BIGENDIAN)
check_type_size("unsigned long int" UNSIGNED_LONG_INT)
if(UNSIGNED_LONG_INT EQUAL 8)
  set(__ISAAC64 1)
elseif(UNSIGNED_LONG_INT EQUAL 4)
else()
  message(SEND_ERROR "unsigned long int doesn't have right size for ISAAC")
endif()
check_type_size("int" INT)
check_type_size("int *" INT_POINTER)

set(BEZITOPO_MAJOR_VERSION 0)
set(BEZITOPO_MINOR_VERSION 1)
set(BEZITOPO_PATCH_VERSION 0)
set(BEZITOPO_VERSION ${BEZITOPO_MAJOR_VERSION}.${BEZITOPO_MINOR_VERSION}.${BEZITOPO_PATCH_VERSION})

include_directories(${PROJECT_BINARY_DIR})
configure_file (config.h.in config.h)

set(CPACK_PACKAGE_VERSION_MAJOR ${BEZITOPO_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${BEZITOPO_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${BEZITOPO_PATCH_VERSION})
set(CPACK_SOURCE_IGNORE_FILES /\\\\.git;.*~)
include(CPack)
