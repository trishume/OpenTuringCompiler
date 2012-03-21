include_directories (src includes ../ ${CommonLibIncludeDir})

set(StdLibSources 
    src/Math.cpp
    src/Time.cpp
    src/Typeconv.cpp
	)

set(StdLibHeaders
	)

# here so they can be edited in the xcode project
set(StdLibIncludes
	includes/stdlib.t
	includes/Math.t
	includes/Time.t
	includes/Typeconv.t
	)

# module builds a plugin-like library
add_library(Stdlib MODULE ${StdLibSources} ${StdLibHeaders} ${StdLibIncludes}
									${StdLibCommonHeaders} ${StdLibCommonSources})
target_link_libraries(Stdlib OpenTuringCommonLib)

# install
set(STDLIB_INSTALL_DIR ${INSTALL_DIR}/lib/stdlib)
install(FILES ${StdLibIncludes} DESTINATION ${STDLIB_INSTALL_DIR})
install(TARGETS Stdlib DESTINATION ${STDLIB_INSTALL_DIR})