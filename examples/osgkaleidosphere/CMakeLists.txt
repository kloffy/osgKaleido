PROJECT(osgKaleidoSphere)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

FILE(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(osgKaleidoSphere osgkaleidosphere.cpp)

TARGET_LINK_LIBRARIES(osgKaleidoSphere
	debug     ${OSG_LIBRARY_DEBUG}
	optimized ${OSG_LIBRARY}
	debug     ${OSGGA_LIBRARY_DEBUG}
	optimized ${OSGGA_LIBRARY}
	debug     ${OSGVIEWER_LIBRARY_DEBUG}
	optimized ${OSGVIEWER_LIBRARY}
	osgKaleido
)