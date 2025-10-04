######################################################################
# CMakeLists.txt
# Copyright (c) Thomas Weber. All rights reserved.				
# Licensed under the MIT License.
# https://github.com/madweasel/madweasels-cpp
######################################################################

# Define source files
set(SOURCE_FILES
    ${PATH_MUEHLE_SRC}/ai/minMaxAI.cpp
    ${PATH_MUEHLE_SRC}/muehle.cpp
    ${PATH_MUEHLE_SRC}/fieldStruct.cpp
    ${PATH_MUEHLE_SRC}/ai/perfectAI.cpp
    ${PATH_MUEHLE_SRC}/ai/stateAddressing.cpp
    ${PATH_MUEHLE_SRC}/ai/threadSpecific.cpp
    ${PATH_MUEHLE_SRC}/ai/randomAI.cpp
)

# Define header files
set(HEADER_FILES
    ${PATH_MUEHLE_SRC}/ai/minMaxAI.h
    ${PATH_MUEHLE_SRC}/muehle.h
    ${PATH_MUEHLE_SRC}/fieldStruct.h
    ${PATH_MUEHLE_SRC}/ai/perfectAI.h
    ${PATH_MUEHLE_SRC}/ai/randomAI.h
    ${PATH_MUEHLE_SRC}/gui/millField2D.h
    ${PATH_MUEHLE_SRC}/gui/historyList.h
)

# Define GUI files
set(GUI_FILES
    ${PATH_MUEHLE_SRC}/gui/MuehleWin.h
    ${PATH_MUEHLE_SRC}/gui/resource.h
    ${PATH_MUEHLE_SRC}/gui/millField2D.cpp
    ${PATH_MUEHLE_SRC}/gui/historyList.cpp
    ${PATH_MUEHLE_SRC}/gui/MuehleWin.cpp
)

# Define CMD files
set(CMD_FILES
    ${PATH_MUEHLE_SRC}/cmd/MuehleCmd.cpp
    ${PATH_MUEHLE_SRC}/cmd/MuehleCmd.h
)

# Define resource files
set(RESOURCE_FILES
    ${PATH_MUEHLE_SRC}/gui/MuehleWin.rc
    ${CMAKE_SOURCE_DIR}/icons/MuehleWin.ico
    ${CMAKE_SOURCE_DIR}/icons/small.ico
)

# MuehleWin executable
if(MSVC)    # GUI is only available on MSVC due to DirectXTK
    add_executable(MuehleWin ${SOURCE_FILES} ${HEADER_FILES} ${GUI_FILES} ${RESOURCE_FILES})

    # Include directories
    target_include_directories(MuehleWin PRIVATE
        ${PATH_WEASEL_LIBRARY}
        ${PATH_WEASEL_LIBRARY}/wildWeasel/ssf_DXTK11
    )
    target_compile_definitions(MuehleWin PRIVATE WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS UNICODE _UNICODE)
    target_link_libraries(MuehleWin PRIVATE
        Microsoft::DirectXTK
        Msimg32.lib
        shlwapi.lib
        Shcore.lib
        XmlLite.lib
        d3d11.lib
        dxgi.lib
        dxguid.lib
        uuid.lib
        kernel32.lib
        user32.lib
        gdi32.lib
        winspool.lib
        comdlg32.lib
        advapi32.lib
        shell32.lib
        ole32.lib
        oleaut32.lib
        odbc32.lib
        odbccp32.lib
        runtimeobject.lib
        comctl32.lib
        Cabinet.lib
        CompressorLib
        miniMaxLib
        weaselEssentialsLib
        wildWeaselLib
    )    
    # So that function int APIENTRY _tWinMain() is used instead of int main()
    set_target_properties(MuehleWin PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    set_property(TARGET MuehleWin PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

# MuehlCmd executable
add_executable(MuehleCmd ${SOURCE_FILES} ${HEADER_FILES} ${CMD_FILES})
target_include_directories(MuehleCmd PRIVATE
    ${PATH_WEASEL_LIBRARY}
)
target_compile_definitions(MuehleCmd PRIVATE WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS UNICODE _UNICODE)
target_link_libraries(MuehleCmd PRIVATE
    CompressorLib
    miniMaxLib
    weaselEssentialsLib
)
set_property(TARGET MuehleCmd PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
