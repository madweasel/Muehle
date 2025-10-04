######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(COMMON_SOURCE_FILES
    ${PATH_MUEHLE_SRC}/ai/minMaxAI.cpp
    ${PATH_MUEHLE_SRC}/ai/randomAI.cpp
    ${PATH_MUEHLE_SRC}/muehle.cpp
    ${PATH_MUEHLE_SRC}/fieldStruct.cpp
    ${PATH_MUEHLE_SRC}/ai/perfectAI.cpp
    ${PATH_MUEHLE_SRC}/ai/stateAddressing.cpp
    ${PATH_MUEHLE_SRC}/ai/threadSpecific.cpp
)

# Define header files
set(HEADER_FILES
    ${PATH_MUEHLE_SRC}/ai/minMaxAI.h
    ${PATH_MUEHLE_SRC}/ai/randomAI.h
    ${PATH_MUEHLE_SRC}/muehle.h
    ${PATH_MUEHLE_SRC}/fieldStruct.h
    ${PATH_MUEHLE_SRC}/ai/perfectAI.h
)

# List of test source files
set(TEST_SOURCE_FILES
    minMaxAITest.cpp
    muehleTest.cpp
    fieldStructTest.cpp
    threadSpecificTest.cpp
    perfectAITest.cpp
    stateAddressingTest.cpp
)

# Loop through test source files and create executables
foreach(TEST_SOURCE_FILE ${TEST_SOURCE_FILES})
    # Get the test name by removing the file extension
    get_filename_component(TEST_NAME ${TEST_SOURCE_FILE} NAME_WE)

    # Add executable
    add_executable(${TEST_NAME} ${COMMON_SOURCE_FILES} ${HEADER_FILES} ${TEST_SOURCE_FILE})

    # Include directories
    target_include_directories(${TEST_NAME} PRIVATE
        ${PATH_WEASEL_LIBRARY}
        ${PATH_MUEHLE_SRC}
    )

    # Compiler options
    target_compile_definitions(${TEST_NAME} PRIVATE _CONSOLE X64 GTEST_HAS_STD_TUPLE_ GTEST_HAS_TR1_TUPLE=0)

    # Add Google Test
    target_link_libraries(${TEST_NAME} PRIVATE 
        GTest::gtest GTest::gtest_main
        Cabinet.lib Shlwapi.lib
        CompressorLib miniMaxLib weaselEssentialsLib
    )

    # Add test
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
