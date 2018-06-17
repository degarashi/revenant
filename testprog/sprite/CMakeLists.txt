cmake_minimum_required(VERSION 2.8)
project(revenant)
set(TEST_NAME sprite)

set(SRC )
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/.. SRC)
add_executable(${TEST_NAME} ${SRC})
target_link_libraries(${TEST_NAME} ${TESTEXE_LIB})
