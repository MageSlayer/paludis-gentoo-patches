
include(CheckCXXCompilerFlag)

function(append_if condition value)
  if (${condition})
    foreach(variable ${ARGN})
      set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    endforeach(variable)
  endif()
endfunction()

if(NOT $ENV{LET_ME_RICE})
  set(flags ${CMAKE_C_FLAGS};${CMAKE_CXX_FLAGS};${CMAKE_EXE_LINKER_FLAGS};${CMAKE_SHARED_LINKER_FLAGS})
  foreach(flag enforce-eh;fast-math;rtti;visibility;znow;std=;align-functions=;prefetch-loop-arrays;Ofast)
    list(FIND flags ${flag} have_flag)
    if(have_flag)
      message(SEND_ERROR "unsupported flag `${flag}` found")
    endif()
  endforeach()
endif()

check_cxx_compiler_flag("-Werror -Wall" CXX_SUPPORTS_WALL)
append_if(CXX_SUPPORTS_WALL "-Wall" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wextra" CXX_SUPPORTS_WEXTRA)
append_if(CXX_SUPPORTS_WEXTRA "-Wextra" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wold-style-cast" CXX_SUPPORTS_WOLD_STYLE_CAST)
append_if(CXX_SUPPORTS_WOLD_STYLE_CAST "-Wold-style-cast" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wredundant-decls" CXX_SUPPORTS_WREDUNDANT_DECLS)
append_if(CXX_SUPPORTS_WREDUNDANT_DECLS "-Wredundant-decls" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wstrict-null-sentinel" CXX_SUPPORTS_WSTRICT_NULL_SENTINEL)
append_if(CXX_SUPPORTS_WSTRICT_NULL_SENTINEL "-Wstrict-null-sentinel" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wmissing-noreturn" CXX_SUPPORTS_WMISSING_NORETURN)
append_if(CXX_SUPPORTS_WMISSING_NORETURN "-Wmissing-noreturn" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Woverloaded-virtual" CXX_SUPPORTS_WOVERLOADED_VIRTUAL)
append_if(CXX_SUPPORTS_WOVERLOADED_VIRTUAL "-Woverloaded-virtual" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Winit-self" CXX_SUPPORTS_WINIT_SELF)
append_if(CXX_SUPPORTS_WINIT_SELF "-Winit-self" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wunreachable-code" CXX_SUPPORTS_WUNREACHABLE_CODE)
append_if(CXX_SUPPORTS_WUNREACHABLE_CODE "-Wunreachable-code" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wunused" CXX_SUPPORTS_WUNUSED)
append_if(CXX_SUPPORTS_WUNUSED "-Wunused" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wshadow" CXX_SUPPORTS_WSHADOW)
append_if(CXX_SUPPORTS_WSHADOW "-Wshadow" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wwrite-strings" CXX_SUPPORTS_WWRITE_STRINGS)
append_if(CXX_SUPPORTS_WWRITE_STRINGS "-Wwrite-strings" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wsignature-shadow" CXX_SUPPORTS_WSIGNATURE_SHADOW)
append_if(CXX_SUPPORTS_WSIGNATURE_SHADOW "-Wsignature-shadow" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wfloat-equal" CXX_SUPPORTS_WFLOAT_EQUAL)
append_if(CXX_SUPPORTS_WFLOAT_EQUAL "-Wfloat-equal" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wno-ignored-qualifiers" CXX_SUPPORTS_WNO_IGNORED_QUALIFIERS)
append_if(CXX_SUPPORTS_WNO_IGNORED_QUALIFIERS "-Wno-ignored-qualifiers" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -Wno-non-virtrual-dtor" CXX_SUPPORTS_WNO_NON_VIRTUAL_DTOR)
append_if(CXX_SUPPORTS_WNO_NON_VIRTUAL_DTOR "-Wno-non-virtual-dtor" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -fvisibility=hidden" CXX_SUPPORTS_FVISIBILITY_HIDDEN)
append_if(CXX_SUPPORTS_FVISIBILITY_HIDDEN "-fvisibility=hidden" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -fvisibility-inlines-hidden" CXX_SUPPORTS_FVISIBILITY_INLINES_HIDDEN)
append_if(CXX_SUPPORTS_FVISIBILITY_INLINES_HIDDEN "-fvisibility-inlines-hidden" CMAKE_CXX_FLAGS)

check_cxx_compiler_flag("-Werror -fno-strict-aliasing" CXX_SUPPORTS_FNO_STRICT_ALIASING)
check_cxx_compiler_flag("-Werror -g0" CXX_SUPPORTS_G0)

check_cxx_compiler_flag("-Werror -fdeclspec" CXX_SUPPORTS_FDECLSPEC)

