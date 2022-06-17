
file(GLOB_RECURSE PrivateFiles *.h *.cpp)

add_executable(Tests ${PrivateFiles})

target_link_libraries(Tests PRIVATE EasyGui)

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(Tests PRIVATE Catch2::Catch2)
