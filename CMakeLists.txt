# CMakeList.txt : CMake project for VFFmpeg, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


project ("VFFmpeg" C CXX)
set(CMAKE_CXX_STANDARD 14)

if(WIN32)
	list(APPEND EXTRA_LIBS 
		"opengl32"
	)
	set(CMAKE_EXE_LINKER_FLAGS "-std=gnu99 -static -static-libgcc  -static-libstdc++ -mwindows")
elseif(LINUX)
	list(APPEND EXTRA_LIBS 
		"-lGL -lGLU -lX11"
	)
endif()
list(APPEND SOURCES 
	src/VFFmpeg.cpp src/VFFmpeg.h src/video.cpp
)
# Add source to this project's executable.
add_executable (VFFmpeg ${SOURCES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VFFmpeg PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
add_subdirectory(lib/glfw)
add_subdirectory(lib/ffmpeg)
target_link_libraries(VFFmpeg glfw ffmpeg ${EXTRA_LIBS})