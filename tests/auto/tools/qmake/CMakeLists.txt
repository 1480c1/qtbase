# Generated from qmake.pro.

#####################################################################
## tst_qmake Test:
#####################################################################

# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    testdata/*)
list(APPEND test_data ${test_data_glob})

add_qt_test(tst_qmake
    SOURCES
        testcompiler.cpp testcompiler.h
        tst_qmake.cpp
    TESTDATA ${test_data}
)

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:qmake.pro:WIN32:
# testcase.timeout = "900"

extend_target(tst_qmake CONDITION CMAKE_CROSSCOMPILING
    DEFINES
        QMAKE_CROSS_COMPILED
)

# special case begin
# remove this because it's not needed
#extend_target(tst_qmake CONDITION CMAKE_BUILD_TYPE STREQUAL Debug AND debug_and_release
    #DEFINES
        #DEBUG_BUILD
#)

#extend_target(tst_qmake CONDITION debug_and_release AND NOT CMAKE_BUILD_TYPE STREQUAL Debug
    #DEFINES
        #RELEASE_BUILD
#)
# special case end
