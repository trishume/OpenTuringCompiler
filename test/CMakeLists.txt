set(TEST_FILES
	test.t
	test13.t
	teststdlib.t
	testmath.t
	testplanetbench.t
	heapsort.t
	sierpinskidraw.t
	3DSphere.t
	testGetAndPut.t
	icon.jpg)

enable_testing()
add_test (Compiler13Test src/compiler test.t)

# install
set(TEST_INSTALL_DIR ${INSTALL_DIR}/test)
install(FILES ${TEST_FILES} DESTINATION ${TEST_INSTALL_DIR})