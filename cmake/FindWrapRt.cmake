include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

find_library(LIBRT rt)

cmake_push_check_state()
if(LIBRT_FOUND)
    list(APPEND CMAKE_REQUIRED_LIBRARIES "${LIBRT}")
endif()

check_cxx_source_compiles("
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[]) {
    timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
}" HAVE_GETTIME)

cmake_pop_check_state()

add_library(WrapRt INTERFACE)
if (LIBRT_FOUND)
    target_link_libraries(WrapRt INTERFACE "${LIBRT}")
endif()

set(WrapRt_FOUND "${HAVE_GETTIME}")

