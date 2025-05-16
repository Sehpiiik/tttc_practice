cmake_minimum_required(VERSION 3.20)
project(UnusedVarDetector LANGUAGES C CXX)

find_package(LLVM  REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

set(CMAKE_CXX_STANDARD 17)
add_library(UnusedVarDetector SHARED UnusedVarDetector.cpp)
target_compile_options(UnusedVarDetector PRIVATE -fno-rtti -fno-exceptions)

target_include_directories(UnusedVarDetector PRIVATE
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

if(APPLE)
  set_target_properties(UnusedVarDetector PROPERTIES
    LINK_FLAGS "-Wl,-undefined,dynamic_lookup"
    SUFFIX ".dylib"   
    PREFIX ""          
  )
else() # Linux
  target_link_options(UnusedVarDetector PRIVATE
    "-Wl,--unresolved-symbols=ignore-in-shared-libs"
  )
endif()

set_target_properties(UnusedVarDetector PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugin
)
