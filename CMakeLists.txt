cmake_minimum_required(VERSION 3.19)
project(tp3a_speedmm C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER "/usr/local/bin/gcc-10")
set(OpenMP_C_FLAGS "-fopenmp")

# build into bin dir, not current dir
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#find_package(OpenMP)
#if (OPENMP_FOUND)
#    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
#endif()
if (${APPLE})
    message ("MAC OS X")
    set(OpenMP_C_FLAGS "-fopenmp")
endif()

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")

add_executable(matrix_1 src/matrix.c src/matrix_simple_impl.c src/csvhelper.c)
add_executable(matrix_2 src/matrix.c src/matrix_block_impl.c src/csvhelper.c)
add_executable(matrix_3 src/matrix.c src/matrix_vector_impl.c src/csvhelper.c)
add_executable(matrix_4 src/matrix.c src/matrix_omp_impl.c src/csvhelper.c)
add_executable(heap_matrix_test test/heap_matrix_test.c)

