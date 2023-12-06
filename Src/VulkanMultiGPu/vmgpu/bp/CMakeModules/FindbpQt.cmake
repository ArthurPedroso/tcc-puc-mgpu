# Try to find bpQt library
# Once done this will define
# BP_QT_FOUND
# BP_QT_INCLUDE_DIRS
# BP_QT_LIBRARIES

find_package(bp REQUIRED)

set(CMAKE_AUTOMOC ON)
find_package(Qt6Core REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Widgets REQUIRED)

if (Qt6Core_VERSION VERSION_LESS 5.10.0 OR
    Qt6Gui_VERSION VERSION_LESS 5.10.0 OR
    Qt6Widgets_VERSION VERSION_LESS 5.10.0)
	message(FATAL_ERROR "Minimum supported Qt version is 5.10.0")
endif()

if (Qt6_POSITION_INDEPENDENT_CODE)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

find_path(BP_QT_INCLUDE_DIR bpQt/Window.h
	  HINTS ${BP_QT_ROOT_DIR}/include ${BP_ROOT_DIR}/bpQt/include ${BP_QT_INCLUDE_DIR}
	  PATHS ../bp/bpQt/include)

find_library(BP_QT_LIBRARY_RELEASE NAMES bpQt
	     HINTS
	     	${BP_QT_ROOT_DIR}/lib
	     	${BP_QT_ROOT_DIR}/cmake-build-release
	     	${BP_ROOT_DIR}/cmake-build-release
	     PATHS
	     	${BP_QT_LIBRARY_DIR}
	     	${BP_LIBRARY_DIR}
	     	../bp/lib
	     	../bp/cmake-build-release)
find_library(BP_QT_LIBRARY_DEBUG NAMES bpQtd
	     HINTS
	     	${BP_QT_ROOT_DIR}/lib
	     	${BP_QT_ROOT_DIR}/cmake-build-debug
	     	${BP_ROOT_DIR}/cmake-build-debug
	     PATHS
	     	${BP_QT_LIBRARY_DIR}
	     	${BP_LIBRARY_DIR}
	     	../bp/lib
	     	../bp/cmake-build-debug)

if(BP_QT_LIBRARY_RELEASE AND BP_QT_LIBRARY_DEBUG)
	set(BP_QT_LIBRARY optimized ${BP_QT_LIBRARY_RELEASE} debug ${BP_QT_LIBRARY_DEBUG})
elseif(BP_QT_LIBRARY_RELEASE)
	set(BP_QT_LIBRARY ${BP_QT_LIBRARY_RELEASE})
elseif(BP_QT_LIBRARY_DEBUG)
	set(BP_QT_LIBRARY ${BP_QT_LIBRARY_DEBUG})
else()
	message(SEND_ERROR "bpQt library not found.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BP_QT DEFAULT_MSG
				  BP_QT_LIBRARY BP_QT_INCLUDE_DIR)
mark_as_advanced(BP_QT_INCLUDE_DIR BP_QT_LIBRARY)

set(BP_QT_INCLUDE_DIRS ${BP_QT_INCLUDE_DIR} ${BP_INCLUDE_DIRS} ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Gui_INCLUDE_DIRS} ${Qt6Widgets_INCLUDE_DIRS})
set(BP_QT_LIBRARIES ${BP_QT_LIBRARY} ${BP_LIBRARIES} Qt6::Core Qt6::Gui Qt6::Widgets)
