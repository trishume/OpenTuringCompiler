set(TEST_FILES
	test.t
	test13.t
	stdlibtest.t
	tonymathtest.t
	testplanetbench.t
	tonymathtest.t
	heapsort.t
	sierpinskidraw.t)

enable_testing()
add_test (Compiler13Test src/compiler test.t)

# install
set(TEST_INSTALL_DIR ${INSTALL_DIR}/test)
install(FILES ${TEST_FILES} DESTINATION ${TEST_INSTALL_DIR})