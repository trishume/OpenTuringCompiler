include_directories (.)

add_custom_command(	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Turing.g.d_parser.cpp 
					COMMAND /usr/local/bin/make_dparser ${CMAKE_CURRENT_SOURCE_DIR}/Turing.g -v -Xcpp)

set(MySources 
    ASTSource.cpp
	BasicScopes.cpp 
	Executor.cpp 
	Message.cpp 
    Symbol.cpp 
	Scope.cpp 
	ScopeManager.cpp 
	StdLib.cpp 
	TuringType.cpp 
	TypeManager.cpp 
	Turing.g.d_parser.cpp
	ast.cpp 
	codegen.cpp 
	language.cpp 
	main.cpp
	)

set(MyHeaders
    ASTSource.h
	BasicScopes.h 
	Executor.h 
	Message.h 
	TuringValue.h 
    Symbol.h 
	Scope.h 
	ScopeManager.h 
	TuringType.h 
	TypeManager.h 
	VarDecl.h 
	ast.h 
	codegen.h 
	language.h
	)

# here so they can be edited in the xcode project
set(MyExtras
	Turing.g
	tokens.def
	)

set_source_files_properties( ${MySources}
                             PROPERTIES COMPILE_FLAGS ${LLVM_COMPILE_FLAGS}
                            )

add_executable(compiler ${MySources} ${MyHeaders} ${MyExtras} ${TEST_FILES})
set_target_properties(	compiler 
						PROPERTIES 
						LINK_FLAGS ${LLVM_LDFLAGS})
target_link_libraries(compiler ${LLVM_LIBS} -ldparse)

# install
set(COMPILER_INSTALL_DIR ${INSTALL_DIR})
install(TARGETS compiler DESTINATION ${COMPILER_INSTALL_DIR})
