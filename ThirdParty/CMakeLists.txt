file(GLOB dot_cpp *.cpp)
file(GLOB dot_h *.h)

file(GLOB_RECURSE glm "glm/*.hpp" "glm/*.cpp" "glm/*.inl")

source_group("glm" FILES ${glm})
add_library(ThirdParty STATIC ${dot_cpp} ${dot_h} ${glm})
target_link_libraries(ThirdParty SDL2 libfbxsdk-md)




