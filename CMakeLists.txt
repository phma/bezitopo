project(bezitopo)
cmake_minimum_required(VERSION 2.8)
add_executable(bezitopo absorient.cpp angle.cpp arc.cpp bezier3d.cpp bezier.cpp
               bezitopo.cpp binio.cpp closure.cpp cogo.cpp color.cpp contour.cpp
               cvtmeas.cpp document.cpp drawobj.cpp
               ellipsoid.cpp geoid.cpp halton.cpp icommon.cpp layer.cpp ldecimal.cpp
               manysum.cpp measure.cpp mkpoint.cpp objlist.cpp plot.cpp pnezd.cpp
               point.cpp pointlist.cpp polyline.cpp ps.cpp qindex.cpp random.cpp
               raster.cpp relprime.cpp spiral.cpp stl.cpp test.cpp segment.cpp tin.cpp
               vcurve.cpp)
add_executable(bezitest absorient.cpp angle.cpp arc.cpp bezier3d.cpp bezier.cpp
               bezitest.cpp binio.cpp closure.cpp cogo.cpp color.cpp contour.cpp
               document.cpp drawobj.cpp
               ellipsoid.cpp geoid.cpp halton.cpp layer.cpp ldecimal.cpp manysum.cpp
               measure.cpp objlist.cpp plot.cpp pnezd.cpp
               point.cpp pointlist.cpp polyline.cpp ps.cpp qindex.cpp random.cpp
               raster.cpp relprime.cpp spiral.cpp stl.cpp test.cpp segment.cpp tin.cpp
               vcurve.cpp)
add_executable(convertgeoid angle.cpp arc.cpp bezier.cpp bezier3d.cpp binio.cpp cogo.cpp
               convertgeoid.cpp document.cpp drawobj.cpp geoid.cpp hlattice.cpp ldecimal.cpp
               manysum.cpp measure.cpp pnezd.cpp point.cpp pointlist.cpp polyline.cpp
               ps.cpp qindex.cpp random.cpp raster.cpp relprime.cpp segment.cpp
               sourcegeoid.cpp spiral.cpp tin.cpp vcurve.cpp)
target_compile_definitions(convertgeoid PUBLIC CONVERTGEOID)

include(TestBigEndian)
include(CheckTypeSize)

set(CMAKE_CXX_FLAGS "-std=c++11")
install(TARGETS bezitopo DESTINATION bin)

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
