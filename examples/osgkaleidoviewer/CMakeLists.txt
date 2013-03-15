PROJECT(osgKaleidoViewer)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

ADD_EXECUTABLE(osgKaleidoViewer osgkaleidoviewer.cpp)

TARGET_LINK_LIBRARIES(osgKaleidoViewer
	debug     ${OSG_LIBRARY_DEBUG}
	optimized ${OSG_LIBRARY}
	debug     ${OSGGA_LIBRARY_DEBUG}
	optimized ${OSGGA_LIBRARY}
	debug     ${OSGVIEWER_LIBRARY_DEBUG}
	optimized ${OSGVIEWER_LIBRARY}
	osgKaleido
)