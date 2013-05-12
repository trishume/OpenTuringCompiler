include_directories (src ../)

find_package(SFML REQUIRED system window graphics network audio)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${CommonLibIncludeDir})

set(StdLibSFMLSources 
	src/Main.cpp
    src/Time.cpp
    src/Draw.cpp
    src/View.cpp
    src/Font.cpp
    src/RGB.cpp
    src/Pic.cpp
    src/Input.cpp
    src/Window.cpp
    src/WindowManager.cpp
	)

set(StdLibSFMLHeaders
	src/Main.h
  src/Font.h
    src/WindowManager.h
    src/RGB.h
	)

# here so they can be edited in the xcode project
set(StdLibSFMLIncludes
	includes/stdlib-sfml.t
	includes/Time.t
	includes/Draw.t
  includes/Input.t
  includes/Font.t
	includes/View.t
	includes/Pic.t
  includes/Window.t
	)

set(StdLibSFMLFonts
  fonts/Arial.ttf
  fonts/Courier\ New.ttf
  fonts/Times\ New\ Roman.ttf
  )

# module builds a plugin-like library
add_library(StdlibSFML MODULE ${StdLibSFMLSources} ${StdLibSFMLHeaders} ${StdLibSFMLIncludes}
										${StdLibCommonHeaders} ${StdLibCommonSources})
target_link_libraries(StdlibSFML OpenTuringCommonLib ${SFML_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

# install
set(STDLIB_SFML_INSTALL_DIR ${INSTALL_DIR}/lib/stdlib-sfml)
install(FILES ${StdLibSFMLIncludes} DESTINATION ${STDLIB_SFML_INSTALL_DIR})
install(TARGETS StdlibSFML DESTINATION ${STDLIB_SFML_INSTALL_DIR})

# install fonts
set(STDLIB_FONTS_INSTALL_DIR ${INSTALL_DIR}/lib/fonts)
install(FILES ${StdLibSFMLFonts} DESTINATION ${STDLIB_FONTS_INSTALL_DIR})
