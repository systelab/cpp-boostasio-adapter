enable_testing()

# Configure preprocessor definitions
add_compile_options(-DBOOST_ALL_DYN_LINK -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)

# Add project folder into includes
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find external dependencies
find_package(WebServerAdapterTestUtilities)

set(GTEST_PACKAGE_NAME gtest)
find_package(${GTEST_PACKAGE_NAME})
if (NOT TARGET ${GTEST_PACKAGE_NAME}::${GTEST_PACKAGE_NAME})
    set(GTEST_PACKAGE_NAME GTest)
    find_package(${GTEST_PACKAGE_NAME})
endif()

# Configure test project
set(BOOST_ASIO_WEB_SERVER_ADAPTER_TEST_PROJECT BoostAsioWebServerAdapterTest)
file(GLOB_RECURSE BOOST_ASIO_WEB_SERVER_ADAPTER_TEST_PROJECT_SRC "*.cpp")
file(GLOB_RECURSE BOOST_ASIO_WEB_SERVER_ADAPTER_TEST_PROJECT_HDR "*.h")
add_executable(${BOOST_ASIO_WEB_SERVER_ADAPTER_TEST_PROJECT} ${BOOST_ASIO_WEB_SERVER_ADAPTER_TEST_PROJECT_SRC} ${BOOST_ASIO_WEB_SERVER_ADAPTER_TEST_PROJECT_HDR})
target_link_libraries(${BOOST_ASIO_WEB_SERVER_ADAPTER_TEST_PROJECT}
					  BoostAsioWebServerAdapter
					  WebServerAdapterTestUtilities::WebServerAdapterTestUtilities
					  ${GTEST_PACKAGE_NAME}::${GTEST_PACKAGE_NAME})

#Configure source groups
foreach(FILE ${BOOST_ASIO_WEB_SERVER_ADAPTER_TEST_PROJECT_SRC} ${BOOST_ASIO_WEB_SERVER_ADAPTER_TEST_PROJECT_HDR}) 
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
    string(REPLACE "/" "\\" GROUP "${GROUP}")

    if ("${FILE}" MATCHES ".*\\.cpp")
       set(GROUP "Source Files${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.h")
       set(GROUP "Header Files${GROUP}")
    endif()

    source_group("${GROUP}" FILES "${FILE}")
endforeach()

# Register tests
add_test(NAME BoostAsioWebServerAdapterTest COMMAND ${BOOST_ASIO_WEB_SERVER_ADAPTER_TEST_PROJECT})

# Additional target for code coverage analysis
if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
	include(CodeCoverage)
	setup_target_for_coverage(BoostAsioWebServerAdapterTestCoverage BoostAsioWebServerAdapterTest coverage)
endif()
