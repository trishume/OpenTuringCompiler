include_directories (src includes ../ ${CommonLibIncludeDir})

set(StdLibSources 
    src/Math.cpp
    src/Time.cpp
	)

set(StdLibHeaders
	)

# here so they can be edited in the xcode project
set(StdLibIncludes
	includes/stdlib.t
	includes/Math.t
	includes/Time.t
	)

# module builds a plugin-like library
add_library(OpenTuringStdlib MODULE ${StdLibSources} ${StdLibHeaders} ${StdLibIncludes}
									${StdLibCommonHeaders} ${StdLibCommonSources})
target_link_libraries(OpenTuringStdlib OpenTuringCommonLib)

# install
set(STDLIB_INSTALL_DIR ${INSTALL_DIR}/lib/stdlib)
install(FILES ${StdLibIncludes} DESTINATION ${STDLIB_INSTALL_DIR})
install(TARGETS OpenTuringStdlib DESTINATION ${STDLIB_INSTALL_DIR})