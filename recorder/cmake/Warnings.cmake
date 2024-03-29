﻿include_guard()

# Set the compiler warnings
# References:
# - https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4000-c5999
# - https://clang.llvm.org/docs/DiagnosticsReference.html
# - https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md

set(MSVC_DEFAULT_WARNINGS
  # DISABLED:
  /wd4068
  # ENABLED:
  /permissive- # enforces standards conformance of MSVC

  /W4 # All reasonable warnings
  /w14242 # 'identifier': conversion from 'type1' to 'type1', possible loss of data
  /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
  /w14263 # 'function': member function does not override any base class virtual member function
  /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not
          # be destructed correctly
  /w14287 # 'operator': unsigned/negative constant mismatch
  /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside
          # the for-loop scope
  /w14296 # 'operator': expression is always 'boolean_value'
  /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
  /w14545 # expression before comma evaluates to a function which is missing an argument list
  /w14546 # function call before comma missing argument list
  /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
  /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
  /w14555 # expression has no effect; expected expression with side- effect
  /w14619 # pragma warning: there is no warning number 'number'
  /w14640 # Enable warning on thread un-safe static member initialization
  /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
  /w14905 # wide string literal cast to 'LPSTR'
  /w14906 # string literal cast to 'LPWSTR'
  /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
)

set(GCC_AND_CLANG_DEFAULT_WARNINGS
  # DISABLED:
  -Wno-c++98-compat-pedantic # disabled, as we do not aim to be c++98 compatible
  -Wno-zero-as-null-pointer-constant # disabled due to https://github.com/llvm/llvm-project/issues/43670

  # ENABLED:
  -Wall # standard
  -Wextra # reasonable and standard
  -Wshadow # warn the user if a variable declaration shadows one from a parent context
  -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor.
  -Wpedantic # warn if non-standard C++ is used

  -Wold-style-cast # warn for c-style casts
  -Wcast-align # warn for potential performance problem casts
  -Wunused # warn on anything being unused
  -Woverloaded-virtual # warn if you overload (not override) a virtual function
  -Wconversion # warn on type conversions that may lose data
  -Wsign-conversion # warn on sign conversions
  -Wnull-dereference # warn if a null dereference is detected
  -Wdouble-promotion # warn if float is implicit promoted to double
  -Wformat=2 # warn on security issues around functions that format output (ie printf)
  -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation

  # -Wlifetime # (only special branch of Clang currently) shows object lifetime issues
  -Wextra-semi # warn about extra semicolon
)

set(CLANG_DEFAULT_WARNINGS
  ${GCC_AND_CLANG_DEFAULT_WARNINGS}
  -Wextra-semi-stmt # warn about extra semicolon making empty statement
  -Wno-float-equal # generated from using <=>, where we on purposed accept unstable float default comparison
  -Wno-unsafe-buffer-usage
)

if(MSVC)
  # disable ctad warning for clang with MSVC - seems to create false positives?
  list(APPEND CLANG_DEFAULT_WARNINGS -Wno-ctad-maybe-unsupported )
  message(STATUS "MSVC deteced: disabling clang warning 'ctad-maybe-unsupported'")
endif()

set(GCC_DEFAULT_WARNINGS
  ${GCC_AND_CLANG_DEFAULT_WARNINGS}
  # DISABLED:
  -Wno-null-dereference # too many false positives (flagging effectively dead branches)

  -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
  -Wduplicated-cond # warn if if / else chain has duplicated conditions
  -Wduplicated-branches # warn if if / else branches have duplicated code
  -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
  -Wuseless-cast # warn if you perform a cast to the same type
)

function(set_specific_warnings
  target_name
  WARNINGS_AS_ERRORS
  MSVC_WARNINGS
  CLANG_WARNINGS
  GCC_WARNINGS)
  message(TRACE "[${target_name}] - function 'set_specific_warnings'")

  if("${MSVC_WARNINGS}" STREQUAL "")
    message(SEND_ERROR "[${target_name}] No warnings specified for MSVC")
  endif()

  if("${CLANG_WARNINGS}" STREQUAL "")
    message(SEND_ERROR "[${target_name}] No warnings specified for CLANG")
  endif()

  if("${GCC_WARNINGS}" STREQUAL "")
    message(SEND_ERROR "[${target_name}] No warnings specified for GCC")
  endif()

  if(WARNINGS_AS_ERRORS)
    message(STATUS "[${target_name}] warnings are treated as errors")
    list(APPEND CLANG_WARNINGS -Werror)
    list(APPEND GCC_WARNINGS -Werror)
    list(APPEND MSVC_WARNINGS /WX)
  endif()

  # test first for clang (i.p. before msvc) to set CLANG_WARNINGS for clang-cl
  if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(ENABLED_WARNINGS ${CLANG_WARNINGS})
  elseif(MSVC)
    set(ENABLED_WARNINGS ${MSVC_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(ENABLED_WARNINGS ${GCC_WARNINGS})
  else()
    message(SEND_ERROR "[${target_name}] No compiler warnings set for CXX compiler: '${CMAKE_CXX_COMPILER_ID}'")
  endif()

  message(STATUS "[${target_name}] warnings enabled: ${ENABLED_WARNINGS}")
  target_compile_options(${target_name} PRIVATE ${ENABLED_WARNINGS})
endfunction()

# function to set the default warnings
function(set_default_warnings
  target_name)
  message(TRACE "[${target_name}] - function 'set_default_warnings'")
  set_specific_warnings(
    ${target_name}
    "${WARNINGS_AS_ERRORS}"
    "${MSVC_DEFAULT_WARNINGS}"
    "${CLANG_DEFAULT_WARNINGS}"
    "${GCC_DEFAULT_WARNINGS}")
endfunction()

# function to set default warnings - excluding warnings that conflict with the test framework
function(set_test_warnings
  target_name)
  message(TRACE "[${target_name}] - function 'set_test_warnings'")

  set(CLANG_TEST_WARNINGS ${CLANG_DEFAULT_WARNINGS})
  set(MSVC_TEST_WARNINGS ${MSVC_DEFAULT_WARNINGS})
  set(GCC_TEST_WARNINGS ${GCC_DEFAULT_WARNINGS})
  if(MSVC)
    list(APPEND CLANG_TEST_WARNINGS -Wno-disabled-macro-expansion)
  endif()

  set_specific_warnings(
    ${target_name}
    "${WARNINGS_AS_ERRORS}"
    "${MSVC_TEST_WARNINGS}"
    "${CLANG_TEST_WARNINGS}"
    "${GCC_TEST_WARNINGS}")
endfunction()