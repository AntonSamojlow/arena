include_guard()

function(enable_sanitizers
  target_name
  ENABLE_SANITIZER_ADDRESS
  ENABLE_SANITIZER_LEAK
  ENABLE_SANITIZER_UNDEFINED_BEHAVIOR
  ENABLE_SANITIZER_THREAD
  ENABLE_SANITIZER_MEMORY)

  # collect all enabled sanitizers
  set(SANITIZERS "")
  if(MSVC)
    if(ENABLE_SANITIZER_LEAK
       OR ENABLE_SANITIZER_UNDEFINED_BEHAVIOR
       OR ENABLE_SANITIZER_THREAD
       OR ENABLE_SANITIZER_MEMORY)
       message(SEND_ERROR "MSVC only supports address sanitizer")
    endif()
    if(ENABLE_SANITIZER_ADDRESS)
       list(APPEND SANITIZERS "address")
    endif()
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(ENABLE_SANITIZER_ADDRESS)
      list(APPEND SANITIZERS "address")
    endif()

    if(ENABLE_SANITIZER_LEAK)
      list(APPEND SANITIZERS "leak")
    endif()

    if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
      list(APPEND SANITIZERS "undefined")
    endif()

    if(ENABLE_SANITIZER_THREAD)
      if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
        message(WARNING "Thread sanitizer does not work with Address and Leak sanitizer enabled")
      else()
        list(APPEND SANITIZERS "thread")
      endif()
    endif()
    if(ENABLE_SANITIZER_MEMORY AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      message(
        WARNING
          "Memory sanitizer requires all the code (including libc++) to be MSan-instrumented otherwise it reports false positives"
      )
      if("address" IN_LIST SANITIZERS
         OR "thread" IN_LIST SANITIZERS
         OR "leak" IN_LIST SANITIZERS)
        message(SEND_ERROR 
          "Memory sanitizer does not work with Address, Thread and Leak sanitizer enabled")
      else()
        list(APPEND SANITIZERS "memory")
      endif()
    endif()
  endif()

  list(
    JOIN
    SANITIZERS
    ","
    LIST_OF_SANITIZERS)

  # set build directives accordingly
  if(NOT LIST_OF_SANITIZERS OR "${LIST_OF_SANITIZERS}" STREQUAL "")
    message(STATUS "no SANITIZERS enabled for '${target_name}'")
  else()
    message(STATUS "SANITIZERS enabled for '${target_name}': ${LIST_OF_SANITIZERS}")
    if(NOT MSVC)
      message(STATUS "(compiler is not MSVC-like)")
      target_compile_options(${target_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
      target_link_options(${target_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
    else()
      message(STATUS "compiler is MSVC-like")
      
      if("${index_of_vs_install_dir}" STREQUAL "-1")
        message(
        SEND_ERROR
            "Using MSVC sanitizers requires setting the MSVC environment before building the project. Please manually open the MSVC command prompt and rebuild the project."
        )
      endif()
      
      # apply temporary fix for https://github.com/llvm/llvm-project/issues/56300 (and related solution https://stackoverflow.com/q/74186326)
      add_compile_definitions(_DISABLE_STRING_ANNOTATION=1 _DISABLE_VECTOR_ANNOTATION=1)
      
      target_compile_options(${target_name} PUBLIC /fsanitize=${LIST_OF_SANITIZERS} /Zi /INCREMENTAL:NO)
      target_link_options(${target_name} INTERFACE /fsanitize=${LIST_OF_SANITIZERS} /INCREMENTAL:NO)
      
      if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        # detected MSVC+Clang => compiler is (clang-cl) => need to link the clang_rt.asan* libs
        # NOTE: we asuume a 64bit OS and that PATH is set up for these libs:
        message(STATUS "Clang-Cl detected - linking clang_rt.asan* libraries, assuming x64 => ensure they exist in PATH")
        if(CMAKE_BUILD_TYPE STREQUAL Release)
			    target_link_libraries(${target_name} PRIVATE clang_rt.asan_dynamic-x86_64 clang_rt.asan_dynamic_runtime_thunk-x86_64)
			    target_link_options(${target_name} PRIVATE /wholearchive:clang_rt.asan_dynamic_runtime_thunk-x86_64.lib)	
		    elseif(CMAKE_BUILD_TYPE STREQUAL Debug)
			    target_link_libraries(${target_name} PRIVATE clang_rt.asan_dbg_dynamic-x86_64 clang_rt.asan_dbg_dynamic_runtime_thunk-x86_64)
			    target_link_options(${target_name} PRIVATE /wholearchive:clang_rt.asan_dbg_dynamic_runtime_thunk-x86_64.lib)
		    endif()	
        
        # INFO to set up PATH / Whre to find libs:
        # location of clang_rt.asan* dlls for x64:
	      # target_link_directories(main PRIVATE "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.34.31933/bin/Hostx64/x64")
        # location asan_dbg_dynamic_runtime_thunk for x64:
	      # target_link_directories(main PRIVATE "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/lib/clang/15.0.1/lib/windows")
      endif()

    endif()
  endif()

endfunction()
