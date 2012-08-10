project(bezitopo)
cmake_minimum_required(VERSION 2.8)
add_executable(bezitopo angle.cpp arc.cpp bezier.cpp bezitopo.cpp cogo.cpp contour.cpp measure.cpp pnezd.cpp point.cpp pointlist.cpp polyline.cpp ps.cpp qindex.cpp test.cpp tin.cpp)
add_executable(bezitest angle.cpp arc.cpp bezier.cpp bezitest.cpp cogo.cpp contour.cpp measure.cpp pnezd.cpp point.cpp pointlist.cpp polyline.cpp ps.cpp qindex.cpp test.cpp tin.cpp)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
#find_package(LibXML++ REQUIRED)
#include_directories(${LibXML++_INCLUDE_DIRS})
#set(LIBS ${LIBS} ${LibXML++_LIBRARIES})
#target_link_libraries(foncvrate ${LIBS})

install(TARGETS bezitopo DESTINATION bin)

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
