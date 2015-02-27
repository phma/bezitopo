project(bezitopo)
cmake_minimum_required(VERSION 2.8)
add_executable(bezitopo angle.cpp arc.cpp bezier.cpp bezitopo.cpp
               cogo.cpp contour.cpp measure.cpp plot.cpp pnezd.cpp point.cpp
               pointlist.cpp polyline.cpp ps.cpp qindex.cpp random.cpp
               raster.cpp spiral.cpp stl.cpp test.cpp segment.cpp tin.cpp
               vcurve.cpp halton.cpp bezier3d.cpp closure.cpp)
add_executable(bezitest angle.cpp arc.cpp bezier.cpp bezitest.cpp
               cogo.cpp contour.cpp measure.cpp plot.cpp pnezd.cpp point.cpp
               pointlist.cpp polyline.cpp ps.cpp qindex.cpp random.cpp
               raster.cpp spiral.cpp stl.cpp test.cpp segment.cpp tin.cpp
               vcurve.cpp halton.cpp bezier3d.cpp closure.cpp)

include(TestBigEndian)
include(CheckTypeSize)

install(TARGETS bezitopo DESTINATION bin)

test_big_endian(BIGENDIAN)
check_type_size("unsigned long int" UNSIGNED_LONG_INT)
if(UNSIGNED_LONG_INT EQUAL 8)
  set(__ISAAC64 1)
elseif(UNSIGNED_LONG_INT EQUAL 4)
else()
  message(SEND_ERROR "unsigned long int doesn't have right size for ISAAC")
endif()

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
