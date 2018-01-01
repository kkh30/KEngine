file(GLOB dot_cpp *.cpp)
file(GLOB dot_h *.h)

add_library(ThirdParty STATIC ${dot_cpp} ${dot_h})
target_link_libraries(ThirdParty SDL2)




