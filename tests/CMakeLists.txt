# Test executables. Included from backend/CMakeLists.txt via
#   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../tests ${CMAKE_BINARY_DIR}/tests)
# when BUILD_TESTS=ON. Each test is a standalone executable with its own main()
# that reports failure through assert() / a non-zero exit code, so no test
# framework is linked.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BACKEND_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../backend)

# include directories matching backend/CMakeLists.txt
include_directories(
    ${BACKEND_DIR}/src/platform
    ${BACKEND_DIR}/src/parsers
    ${BACKEND_DIR}/src/text
    ${BACKEND_DIR}/vendor
    ${BACKEND_DIR}/src/routes
    ${BACKEND_DIR}/src/core
)

# self-contained tests: standard library only, no backend sources required
add_executable(combined_pipeline_test combined_pipeline_test.cpp)
add_test(NAME combined_pipeline_test COMMAND combined_pipeline_test)

add_executable(json_parser_test json_parser_test.cpp)
add_test(NAME json_parser_test COMMAND json_parser_test)

add_executable(semi_structured_integration_test semi_structured_integration_test.cpp)
add_test(NAME semi_structured_integration_test COMMAND semi_structured_integration_test)

add_executable(unstructured_integration_test unstructured_integration_test.cpp)
add_test(NAME unstructured_integration_test COMMAND unstructured_integration_test)

# exercises the real RFC 4180 CSV parser, so that one backend source is
# compiled in.
# NOTE: this test references "../include/csv_parser.h" and parseCSVRFC4180(),
# neither of which exists anywhere in the tree (the parser API is parseCSV /
# parseCSVLine in backend/src/parsers/csv_parser.h), so this target does not
# currently compile. It is registered unchanged so CI surfaces the defect;
# fixing the test or providing the API is a separate change.
add_executable(test_csv_rfc4180 test_csv_rfc4180.cpp ${BACKEND_DIR}/src/parsers/csv_parser.cpp)
add_test(NAME test_csv_rfc4180 COMMAND test_csv_rfc4180)
