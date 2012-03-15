include_directories (src include ../)

set(CommonLibSources 
    src/RuntimeError.cpp
    src/FileSystem.cpp
	)

set(CommonLibHeaders
	include/TuringCommon/IDManager.h
	include/TuringCommon/LibDefs.h
	include/TuringCommon/RuntimeError.h
	include/TuringCommon/FileSystem.h
	)

# module builds a plugin-like library
add_library(OpenTuringCommonLib STATIC ${CommonLibHeaders} ${CommonLibSources})