
file(GLOB_RECURSE KEFbxImporter "fbximporter/*.h" "fbximporter/*.cxx")
file(GLOB_RECURSE KEWindow "KEWindow.cpp" "KEWindow.h")
file(GLOB KELog "KELog.h" "KELog.cpp")
source_group("KELog" FILES ${KELog})
source_group("KEWindow" FILES ${KEWindow})
source_group("KEFbxImporter" FILES ${KEFbxImporter})
add_library(EngineCore STATIC ${KEWindow} ${KELog} ${KEFbxImporter})
target_link_libraries(EngineCore SDL2 ThirdParty)




