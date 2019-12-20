find_package(Threads REQUIRED)

add_executable(player player.c)

target_link_libraries(player
	aal
	${CMAKE_THREAD_LIBS_INIT}
)

add_executable(recorder recorder.c)

target_link_libraries(recorder
	aal
	${CMAKE_THREAD_LIBS_INIT}
)