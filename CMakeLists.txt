
cmake_minimum_required(VERSION 3.0)

project(refalloc C)

option(CMAKE_DISABLE_TESTING "Disable testing.")
option(REFALLOC_TEST_C11 "Test with C 2011")
option(BUILD_SHARED_LIBS "Build a shared (not static) library")

add_library(refalloc "refalloc.c" "refalloc.h")

if (BUILD_SHARED_LIBS)
  if (WIN32)
    target_compile_definitions(refalloc PUBLIC "REFALLOC_WIN32_DLL")
  endif (WIN32)
endif (BUILD_SHARED_LIBS)

if (NOT CMAKE_DISABLE_TESTING)
  # simple (single-thread) test
  add_executable(test_refalloc_simple "tests/refalloc_simple.c")
  target_link_libraries(test_refalloc_simple refalloc)

  # POSIX threads test
  if (UNIX)
    add_executable(test_refalloc_posix "tests/refalloc_posix.c")
    target_link_libraries(test_refalloc_posix refalloc pthread)
  endif(UNIX)

  # Win32 threads test
  if (WIN32)
    add_executable(test_refalloc_win32 "tests/refalloc_win32.c")
    target_link_libraries(test_refalloc_win32 refalloc)
  endif (WIN32)

  # C11 threads test
  if (REFALLOC_TEST_C11)
    add_executable(test_refalloc_c11 "tests/refalloc_c11.c")
    target_link_libraries(test_refalloc_c11 refalloc)
  endif(REFALLOC_TEST_C11)
endif (NOT CMAKE_DISABLE_TESTING)
