include_directories("${PROJECT_SOURCE_DIR}/ThirdParty")


file(GLOB_RECURSE KEWindow "KEWindow.cpp" "KEWindow.h")
file(GLOB KELog "KELog.h" "KELog.cpp")
source_group("KELog" FILES ${KELog})
source_group("KEWindow" FILES ${KEWindow})
add_library(EngineCore STATIC ${KEWindow} ${KELog})
target_link_libraries(EngineCore SDL2 vulkan-1)




