find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

add_executable(player player.cpp)

target_include_directories(player
    PRIVATE
		${GTEST_INCLUDE_DIRS}
)

target_link_libraries(player
	PRIVATE
		aal
		${CMAKE_THREAD_LIBS_INIT}
		${GTEST_BOTH_LIBRARIES}
)

add_executable(recorder recorder.cpp)

target_link_libraries(recorder
	aal
	${CMAKE_THREAD_LIBS_INIT}
)

install(
	TARGETS player recorder
	DESTINATION bin
)