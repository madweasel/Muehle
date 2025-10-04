######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################
cmake_minimum_required (VERSION 3.15)

# check if the Developer Command Prompt for Visual Studio (x64) is used
if(NOT DEFINED ENV{VSCMD_VER} OR NOT DEFINED ENV{VSCMD_ARG_TGT_ARCH} OR NOT "$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x64")
    message(FATAL_ERROR "Please run VS Code inside the 'x64 Native Tools Command Prompt for VS 2022'!\n\
        1) Press 'WINDOWS-KEY' and type 'x64 Native Tools Command Prompt for VS 2022'.\n\
        2) Type 'code' and press 'ENTER'.")
endif()

# extract compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Muehle")

# paths
set(PATH_WEASEL_LIBRARY "${CMAKE_SOURCE_DIR}/ext/WeaselLibrary")
set(PATH_MUEHLE_SRC     "${CMAKE_SOURCE_DIR}/src/Muehle")
set(PATH_TICTACTOE_SRC  "${CMAKE_SOURCE_DIR}/src/TicTacToe")

# Print paths
message("PATH_WEASEL_LIBRARY:  ${PATH_WEASEL_LIBRARY}")
message("PATH_MUEHLE_SRC:      ${PATH_MUEHLE_SRC}")
message("PATH_TICTACTOE_SRC:   ${PATH_TICTACTOE_SRC}")
message("CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

# Specify the compiler and settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Unicode
add_definitions(-DUNICODE -D_UNICODE)

# Set 64-bit architecture
set(CMAKE_GENERATOR_PLATFORM x64)

# Set compiler flags for different build types
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /Zi /O2")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Zi")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/MD")
else()
    set(CMAKE_CXX_FLAGS "--coverage")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")
endif()

# Specify the generator
set(CMAKE_GENERATOR "Ninja")

# Add packages
enable_testing()
find_package(GTest CONFIG REQUIRED)
find_package(directxtk CONFIG REQUIRED)

# Add subdirectories
add_subdirectory("ext/WeaselLibrary")
link_directories("ext/WeaselLibrary")

# Include sub-projects.
add_subdirectory("src/ticTacToe")
add_subdirectory("tst/ticTacToeTest")
add_subdirectory("tst/MuehleTest")
add_subdirectory("src/DatabaseTransformer")
add_subdirectory("src/Muehle")

# Set the folder structure
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(CompressorLib weaselEssentialsLib miniMaxLib pgsLib PROPERTIES FOLDER WeaselLibrary)
set_target_properties(CompressorTest GenericTest muehleTest pgsTest TicTacToeTest MiniMaxTest PROPERTIES FOLDER Test)
set_target_properties(fieldStructTest minMaxAITest perfectAITest stateAddressingTest threadSpecificTest PROPERTIES FOLDER Test)
set_target_properties(TicTacToe DatabaseTransformer PROPERTIES FOLDER Games)
if(MSVC)
    set_target_properties(perfectAITest PROPERTIES LINK_FLAGS "/PROFILE")
    set_target_properties(MuehleCmd PROPERTIES LINK_FLAGS "/PROFILE")
    set_target_properties(wildWeaselLib PROPERTIES FOLDER WeaselLibrary)
    set_target_properties(MuehleWin PROPERTIES FOLDER Games)
    set_target_properties(MuehleCmd PROPERTIES FOLDER Games)
endif()

