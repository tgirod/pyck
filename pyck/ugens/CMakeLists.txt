FIND_PACKAGE(PythonLibs)
OPTION(BUILD_SHARED_LIBS "turn OFF for .a libs" ON)

add_library (osc osc.cpp)
target_link_libraries (osc boost_python core)
