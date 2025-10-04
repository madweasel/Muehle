######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

set(CMAKE_GENERATOR_PLATFORM x64)

# Define source files
set(SOURCE_FILES
    TicTacToeTest.cpp
    ${PATH_TICTACTOE_SRC}/ticTacToe.cpp
)

# Define header files
set(HEADER_FILES
    ${PATH_TICTACTOE_SRC}/ticTacToe.h
)

# Add executable
add_executable(TicTacToeTest ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(TicTacToeTest PRIVATE
    ${PATH_WEASEL_LIBRARY}
    ${PATH_TICTACTOE_SRC}
)

# Compiler options
target_compile_definitions(TicTacToeTest PRIVATE _CONSOLE X64 GTEST_HAS_STD_TUPLE_ GTEST_HAS_TR1_TUPLE=0)

# Add Google Test
target_link_libraries(TicTacToeTest PRIVATE
    GTest::gtest GTest::gtest_main 
    Cabinet.lib
    CompressorLib miniMaxLib weaselEssentialsLib)

add_test(TicTacToeTests TicTacToeTest)
