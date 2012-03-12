cmake_minimum_required(VERSION 2.8)

include_directories ("/usr/include/python2.7")

include_directories ("${PROJECT_SOURCE_DIR}/pyck")    
add_subdirectory (pyck)
