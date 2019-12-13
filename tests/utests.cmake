enable_testing()
find_package(gtest REQUIRED)
if(${GTEST_FOUND})
    include_directories(${GTEST_INCLUDE_DIRS})
    message("FOUND GTESTS")
    message(${GTEST_INCLUDE_DIRS})
    add_executable(atomic_operators_utest tests/atomic_operators_utest.cpp)
    target_link_libraries(atomic_operators_utest 
        ${GTEST_LIBRARY} ${PROJECT_NAME})
    include(GoogleTest)
    gtest_add_tests(TARGET atomic_operators_utest)
else()
   message("Not compiling tests")
endif()
