cmake_minimum_required(VERSION 2.6)
project(Notifications C)

set(CMAKE_INSTALL_PREFIX out)
set(CMAKE_C_STANDARD 99)

include(FindPkgConfig)
pkg_search_module(GLIB2 REQUIRED glib-2.0)
if (GLIB2_FOUND)
    message("glib2: found")
    include_directories(${GLIB2_INCLUDE_DIRS})
    link_libraries(${GLIB2_LIBRARIES})
endif (GLIB2_FOUND)

if (NOT GLIB2_FOUND)
    message(FATAL_ERROR "glib-2.0 not found")
endif (NOT GLIB2_FOUND)

pkg_search_module(LIBNOTIFY REQUIRED libnotify)
if (LIBNOTIFY_FOUND)
    message("libnotify: found")
    include_directories(${LIBNOTIFY_INCLUDE_DIRS})
    link_libraries(${LIBNOTIFY_LIBRARIES})
endif (LIBNOTIFY_FOUND)

if (NOT LIBNOTIFY_FOUND)
    message(FATAL_ERROR "libnotify not found")
endif (NOT LIBNOTIFY_FOUND)

add_subdirectory(src)

