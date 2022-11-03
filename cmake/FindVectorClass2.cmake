set(VectorClass2_INCLUDE_DIR "" CACHE FILEPATH "")
string(LENGTH "${VectorClass2_INCLUDE_DIR}" length)
if(NOT ${length} GREATER 0)
    message(FATAL_ERROR "VectorClass2_INCLUDE_DIR can't be empty\n" "[VectorClass2] https://github.com/vectorclass/version2")
endif()
add_library(VectorClass2 INTERFACE)
target_include_directories(VectorClass2 INTERFACE ${VectorClass2_INCLUDE_DIR})
target_compile_definitions(VectorClass2 INTERFACE VCL_NAMESPACE=vcl)