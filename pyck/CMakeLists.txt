FIND_PACKAGE(PythonLibs)
OPTION(BUILD_SHARED_LIBS "turn OFF for .a libs" ON)

add_library (core core.cpp)
target_link_libraries (core boost_python rtaudio)

include_directories ("${PROJECT_SOURCE_DIR}/pyck/ugens")    
add_subdirectory (ugens)
