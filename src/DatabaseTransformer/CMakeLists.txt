######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    miniMaxDatabaseTransformer.cpp
)

# Define header files
set(HEADER_FILES
)

# Add executable
add_executable(DatabaseTransformer ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
target_include_directories(DatabaseTransformer PRIVATE
    ${PATH_WEASEL_LIBRARY}
)

# Compiler options
target_compile_definitions(DatabaseTransformer PRIVATE _CONSOLE X64)

# Linker options
target_link_libraries(DatabaseTransformer PRIVATE 
    Cabinet.lib Shlwapi.lib xmllite.lib 
    weaselEssentialsLib CompressorLib miniMaxLib
)

# Unicode
add_definitions(-DUNICODE -D_UNICODE)
