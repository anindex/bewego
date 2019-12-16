find_package(gtest QUIET)
enable_testing()
if(GTEST_LIBRARY)
    message(${GTEST_INCLUDE_DIRS})
    include_directories(${GTEST_INCLUDE_DIRS})
else()
    message("gtest library not found !!! setting it up at link target")
    set(GTEST_LIBRARY gtest)
endif()
add_executable(atomic_operators_utest tests/atomic_operators_utest.cpp)
target_link_libraries(atomic_operators_utest ${GTEST_LIBRARY} ${PROJECT_NAME})
include(GoogleTest)
gtest_add_tests(TARGET atomic_operators_utest)

find_package(ifopt)
if(ifopt_FOUND)
    # To install on mac 
    #  - https://github.com/ethz-adrl/ifopt
    #  - brew install ipopt
    # Formulate (ifopt:ifopt_core) and solve (ifopt::ifopt_ipopt) the problem
    add_executable(ifopt_test tests/ifopt_test.cpp)
    # Pull in include directories, libraries, ... 
    target_link_libraries(ifopt_test PUBLIC ifopt::ifopt_ipopt) 
endif() 
