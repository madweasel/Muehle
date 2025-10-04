######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    main.cpp
    ticTacToe.cpp
)

# Define header files
set(HEADER_FILES
    ticTacToe.h
)

# Add executable
add_executable(TicTacToe ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(TicTacToe PRIVATE
    ${PATH_WEASEL_LIBRARY}
)

# Compiler options
target_compile_definitions(TicTacToe PRIVATE _CONSOLE)

# Linker options
target_link_libraries(TicTacToe PRIVATE Cabinet.lib CompressorLib miniMaxLib weaselEssentialsLib)

# Unicode
add_definitions(-DUNICODE -D_UNICODE)
