cmake_minimum_required(VERSION 3.7)
project(behrens_converter)

# .\cleanup.bat & cmake CMakeLists.txt -G "MinGW Makefile" & mingw32-make -j5

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17") 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_REQUIRED_FLAGS -std=c++17)
include(CheckCXXSymbolExists)


if(WIN32)
  set(BISON_EXECUTABLE "C:/ProgramData/chocolatey/lib/winflexbison3/tools/win_bison.exe")
  set(FLEX_EXECUTABLE "C:/ProgramData/chocolatey/lib/winflexbison3/tools/win_flex.exe")
endif()



if(WIN32)
    set(PYTHON_VERSION "3.7")
    set(Python_ADDITIONAL_VERSIONS 3.7)
    FIND_PACKAGE(PythonInterp 3.7)
    FIND_PACKAGE(PythonLibs 3.7 REQUIRED)
    include_directories(C:/devs/converter/pymlcls/fastlib/pybind11/include)
else()
    set(PYTHON_VERSION "3.7")
    set(Python_ADDITIONAL_VERSIONS 3.7)
    FIND_PACKAGE(PythonInterp 3.7)
    FIND_PACKAGE(PythonLibs 3.7 REQUIRED)
    include_directories(./pybind11/include)
endif()


FIND_PACKAGE(PythonInterp)


message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")


LINK_LIBRARIES(${Boost_LIBRARIES} ${PYTHON_LIBRARIES}) # Deprecated but so convenient!

set(BISON_COMPILE_FLAGS  "-d -t -g")
set(FLEX_COMPILE_FLAGS  "")


if (UNIX)
    include_directories("/usr/include/python${PYTHON_VERSION}")
endif (UNIX)


if (WIN32)
    include_directories("C:/Users/Win10/AppData/Local/Programs/Python/Python37/include")
endif (WIN32)


include_directories(".")

find_package(BISON)
BISON_TARGET(gen_flp_parser src/compiler/compiler.y ${CMAKE_CURRENT_BINARY_DIR}/gen_flp_parser.cpp
             COMPILE_FLAGS ${BISON_COMPILE_FLAGS}
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/gen_flp_parser.h
)

find_package(FLEX)
FLEX_TARGET(gen_flp_parser src/compiler/compiler.l ${CMAKE_CURRENT_BINARY_DIR}/gen_flp_lexer.cpp
            COMPILE_FLAGS ${FLEX_COMPILE_FLAGS}
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/gen_flp_lexer.h
)


set(SOURCES
    src/main.cpp 
    src/compiler/token_table.cpp 
    src/compiler/python_module.cpp
)


add_library(
    behrens_gparser SHARED
    ${BISON_gen_flp_parser_OUTPUTS}
    ${FLEX_gen_flp_parser_OUTPUTS}
    ${SOURCES} 
)


if (UNIX)
  target_link_libraries(behrens_gparser -lpython${PYTHON_VERSION})
  add_custom_target(Copy
     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/libbehrens_gparser.so ${CMAKE_BINARY_DIR}/../behrens_converter/behrens_gparser.so
  )
endif (UNIX)

if (WIN32)
  add_custom_target(Copy
     COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/libbehrens_gparser.dll ${CMAKE_BINARY_DIR}/../behrens_converter/behrens_gparser.pyd
  )
endif (WIN32)


# target_include_directories(lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

