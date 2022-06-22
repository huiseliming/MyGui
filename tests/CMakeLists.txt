#file(GLOB_RECURSE PrivateFiles *.h *.cpp)

add_executable(Tests 
	Catch2Main.cpp
	QueueTaskQueueTest.cpp
	QueueTest.cpp
)

target_link_libraries(Tests PRIVATE EasyGui)

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(Tests PRIVATE Catch2::Catch2)
set_property(TARGET Tests PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

add_executable(CoreApplicationTest CoreApplicationTest.cpp)
target_link_libraries(CoreApplicationTest PUBLIC EasyGui)
set_property(TARGET CoreApplicationTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

add_executable(VulkanGuiApplicationTest VulkanGuiApplicationTest.cpp)
target_link_libraries(VulkanGuiApplicationTest PUBLIC EasyGui)
set_property(TARGET VulkanGuiApplicationTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")