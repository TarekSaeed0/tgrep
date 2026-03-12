cmake_minimum_required(VERSION 4.2.3)
project(
	tgrep
	VERSION 1.0
	LANGUAGES C
	DESCRIPTION "A project written in c"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(tgrep src/job_queue.c src/job_executor.c src/main.c)
target_compile_options(
	tgrep
	PRIVATE -Werror
			-Wall
			-Wextra
			-pedantic
			-Wfloat-equal
			-Wundef
			-Wshadow
			-Wpointer-arith
			-Wcast-align
			-Wstrict-prototypes
			-Wstrict-overflow=5
			-Wwrite-strings
			-Wcast-qual
			-Wconversion
)
target_include_directories(tgrep PUBLIC include)
target_link_libraries(
	tgrep
	PRIVATE pthread
)
