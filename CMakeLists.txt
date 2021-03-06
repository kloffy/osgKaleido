PROJECT(osgKaleido)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Add a postfix, usually d on windows.")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "Add a postfix, usually empty on windows.")
SET(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "Add a postfix, usually empty on windows.")
SET(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "Add a postfix, usually empty on windows.")

IF(CMAKE_BUILD_TYPE MATCHES "Release")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_MINSIZEREL_POSTFIX}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELWITHDEBINFO_POSTFIX}")
ELSEIF(CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
ELSE()
    SET(CMAKE_BUILD_POSTFIX "")
ENDIF()

# Does nothing in cmake 2.8.10, set manually.
SET(CMAKE_VS_PLATFORM_TOOLSET "v120_CTP_Nov2012" CACHE STRING "" FORCE)

IF (NOT EXISTS ${CMAKE_BINARY_DIR}/bin)
	FILE (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
ENDIF ()
IF (NOT EXISTS ${CMAKE_BINARY_DIR}/lib)
    FILE (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
ENDIF ()

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "Runtime directory" FORCE)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH "Library directory" FORCE)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH "Archive directory" FORCE)

FOREACH(CONFIG ${CMAKE_CONFIGURATION_TYPES} )
    STRING(TOUPPER ${CONFIG} UPPER_CONFIG)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${UPPER_CONFIG} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG} CACHE PATH "Runtime directory" FORCE)
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${UPPER_CONFIG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CONFIG} CACHE PATH "Library directory" FORCE)
    SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${UPPER_CONFIG} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${CONFIG} CACHE PATH "Archive directory" FORCE)
ENDFOREACH()

FIND_PACKAGE(OpenSceneGraph 3.0.0 REQUIRED COMPONENTS osgDB osgGA osgText osgUtil osgViewer)
FIND_PACKAGE(Boost 1.45.0 REQUIRED)
#FIND_PACKAGE(Boost 1.45.0 REQUIRED COMPONENTS program_options)

OPTION(OSGKALEIDO_ENABLE_EXCEPTIONS "Use exceptions." ON)

if(OSGKALEIDO_ENABLE_EXCEPTIONS)
	ADD_DEFINITIONS(-DOSGKALEIDO_EXCEPTIONS_ENABLED)
endif(OSGKALEIDO_ENABLE_EXCEPTIONS)

SET(KALEIDO_SRC_FILES
	ext/kaleido/kaleido.c
	ext/kaleido/uniform.c
)

SET(KALEIDO_HEADER_FILES
	ext/kaleido/kaleido.h
)

SET(SRC_FILES
	src/DynamicGeometry.cpp
	src/Polyhedron.cpp
	src/PolyhedronGeometry.cpp
	src/Util.cpp
)

SET(PRIVATE_HEADER_FILES
	include/osgKaleido/Conversion
	include/osgKaleido/Util
)

SET(PUBLIC_HEADER_FILES
	include/osgKaleido/DynamicGeometry
	include/osgKaleido/Export
	include/osgKaleido/Polyhedron
	include/osgKaleido/PolyhedronGeometry
)

SET(HEADER_FILES
	${PRIVATE_HEADER_FILES}
	${PUBLIC_HEADER_FILES}
)

INCLUDE_DIRECTORIES(
	include
	src
	ext/kaleido
	ext/wildcard
	${OSG_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
)

ADD_LIBRARY(kaleido STATIC ${KALEIDO_HEADER_FILES} ${KALEIDO_SRC_FILES})

SOURCE_GROUP("Header Files" FILES ${KALEIDO_HEADER_FILES})
SOURCE_GROUP("Source Files" FILES ${KALEIDO_SRC_FILES})

ADD_LIBRARY(osgKaleido SHARED ${HEADER_FILES} ${SRC_FILES})

SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
SOURCE_GROUP("Source Files" FILES ${SRC_FILES})

SET_TARGET_PROPERTIES(osgKaleido PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADER_FILES}")

TARGET_LINK_LIBRARIES(osgKaleido
	debug     ${OPENTHREADS_LIBRARY_DEBUG}
	optimized ${OPENTHREADS_LIBRARY}
	debug     ${OSG_LIBRARY_DEBUG}
	optimized ${OSG_LIBRARY}
	debug     ${OSGDB_LIBRARY_DEBUG}
	optimized ${OSGDB_LIBRARY}
	debug     ${OSGUTIL_LIBRARY_DEBUG}
	optimized ${OSGUTIL_LIBRARY}
	kaleido
)

ADD_SUBDIRECTORY(examples/osgkaleidosimple)
ADD_SUBDIRECTORY(examples/osgkaleidosphere)
ADD_SUBDIRECTORY(examples/osgkaleidoviewer)

INSTALL(TARGETS osgKaleido
	ARCHIVE       DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	LIBRARY       DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	RUNTIME       DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include/osgKaleido
)