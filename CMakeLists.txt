cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(a1a_gen LANGUAGES C)

find_library(LIBM_LIBRARIES m)

file(GLOB SRC
	main.c
	output.c
	player.c
	table.c
)

set(A1AGEN_CFLAGS -O2 -Wall -fdata-sections -ffunction-sections)
set(A1AGEN_LDFLAGS -Wl,--gc-sections)

add_executable(${PROJECT_NAME} ${SRC})
target_compile_options(${PROJECT_NAME} PRIVATE ${A1AGEN_CFLAGS})
target_link_options(${PROJECT_NAME} PRIVATE ${A1AGEN_LDFLAGS})
target_link_libraries(${PROJECT_NAME} ${LIBM_LIBRARIES})
