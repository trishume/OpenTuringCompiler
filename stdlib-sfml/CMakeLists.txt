include_directories (src ../)

find_package(SFML REQUIRED system window graphics network audio)

set(StdLibSFMLSources 
    src/Time.cpp
    src/Draw.cpp
    src/WindowManager.cpp
	)

set(StdLibSFMLHeaders
    ../openTuringLibDefs.h
    ../openTuringRuntimeError.h
    src/WindowManager.h
	)

# here so they can be edited in the xcode project
set(StdLibSFMLIncludes
	includes/stdlib-sfml.t
	includes/stdlib-sfml-post.t
	includes/Time.t
	includes/Draw.t
	)

# module builds a plugin-like library
add_library(OpenTuringStdlibSFML MODULE ${StdLibSFMLSources} ${StdLibSFMLHeaders} ${StdLibSFMLIncludes})
target_link_libraries(OpenTuringStdlibSFML ${SFML_LIBRARIES})

# install
set(STDLIB_SFML_INSTALL_DIR ${INSTALL_DIR}/lib/stdlib-sfml)
install(FILES ${StdLibSFMLIncludes} DESTINATION ${STDLIB_SFML_INSTALL_DIR})
install(TARGETS OpenTuringStdlibSFML DESTINATION ${STDLIB_SFML_INSTALL_DIR})