include_directories (src includes)

set(StdLibSources 
    src/Math.cpp
	)

set(StdLibHeaders
    src/openTuringLibDefs.h
	)

# here so they can be edited in the xcode project
set(StdLibIncludes
	includes/stdlib.t
	includes/Math.t
	)

SET (LIB_TYPE SHARED)
add_library(OpenTuringStdlib ${StdLibSources} ${StdLibHeaders} ${StdLibIncludes})

# install
set(STDLIB_INSTALL_DIR ${INSTALL_DIR}/lib/stdlib)
install(FILES ${StdLibIncludes} DESTINATION ${STDLIB_INSTALL_DIR})
install(TARGETS OpenTuringStdlib DESTINATION ${STDLIB_INSTALL_DIR})