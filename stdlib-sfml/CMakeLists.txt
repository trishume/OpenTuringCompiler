include_directories (src ../)

find_package(SFML REQUIRED system window graphics network audio)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

set(StdLibSFMLSources 
    src/Time.cpp
    src/Draw.cpp
    src/View.cpp
    src/RGB.cpp
    src/WindowManager.cpp
	)

set(StdLibSFMLHeaders
    src/WindowManager.h
    src/RGB.h
	)

# here so they can be edited in the xcode project
set(StdLibSFMLIncludes
	includes/stdlib-sfml.t
	includes/stdlib-sfml-post.t
	includes/Time.t
	includes/Draw.t
	includes/View.t
	)

# module builds a plugin-like library
add_library(OpenTuringStdlibSFML MODULE ${StdLibSFMLSources} ${StdLibSFMLHeaders} ${StdLibSFMLIncludes}
										${StdLibCommonHeaders} ${StdLibCommonSources})
target_link_libraries(OpenTuringStdlibSFML ${SFML_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

# install
set(STDLIB_SFML_INSTALL_DIR ${INSTALL_DIR}/lib/stdlib-sfml)
install(FILES ${StdLibSFMLIncludes} DESTINATION ${STDLIB_SFML_INSTALL_DIR})
install(TARGETS OpenTuringStdlibSFML DESTINATION ${STDLIB_SFML_INSTALL_DIR})