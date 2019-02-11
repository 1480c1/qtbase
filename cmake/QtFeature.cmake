function(qt_feature_module_begin)
    qt_parse_all_arguments(arg "qt_feature_module_begin"
        "" "LIBRARY;PRIVATE_FILE;PUBLIC_FILE" "PUBLIC_DEPENDENCIES;PRIVATE_DEPENDENCIES" ${ARGN})

    if ("${arg_LIBRARY}" STREQUAL "")
        message(FATAL_ERROR "qt_feature_begin_module needs a LIBRARY name!")
    endif()
    if ("${arg_PUBLIC_FILE}" STREQUAL "")
        message(FATAL_ERROR "qt_feature_begin_module needs a PUBLIC_FILE name!")
    endif()
    if ("${arg_PRIVATE_FILE}" STREQUAL "")
        message(FATAL_ERROR "qt_feature_begin_module needs a PRIVATE_FILE name!")
    endif()

    set(__QtFeature_library "${arg_LIBRARY}" PARENT_SCOPE)
    set(__QtFeature_private_features "" PARENT_SCOPE)
    set(__QtFeature_public_features "" PARENT_SCOPE)

    set(__QtFeature_private_file "${arg_PRIVATE_FILE}" PARENT_SCOPE)
    set(__QtFeature_public_file "${arg_PUBLIC_FILE}" PARENT_SCOPE)

    set(__QtFeature_private_extra "" PARENT_SCOPE)
    set(__QtFeature_public_extra "" PARENT_SCOPE)
endfunction()

function(qt_feature feature)
    qt_parse_all_arguments(arg "qt_feature"
        "PRIVATE;PUBLIC"
        "LABEL;PURPOSE;SECTION;" "AUTODETECT;CONDITION;ENABLE;DISABLE;EMIT_IF" ${ARGN})

    set(_QT_FEATURE_DEFINITION_${feature} ${ARGN} PARENT_SCOPE)

    # Register feature for future use:
    if (arg_PUBLIC)
        list(APPEND __QtFeature_public_features "${feature}")
    endif()
    if (arg_PRIVATE)
        list(APPEND __QtFeature_private_features "${feature}")
    endif()

    set(__QtFeature_public_features ${__QtFeature_public_features} PARENT_SCOPE)
    set(__QtFeature_private_features ${__QtFeature_private_features} PARENT_SCOPE)
endfunction()

function(qt_evaluate_to_boolean expressionVar)
    if(${${expressionVar}})
        set(${expressionVar} ON PARENT_SCOPE)
    else()
        set(${expressionVar} OFF PARENT_SCOPE)
    endif()
endfunction()

function(qt_evaluate_config_expression resultVar)
    set(result "")
    set(nestingLevel 0)
    set(skipNext OFF)
    set(expression "${ARGN}")
    list(LENGTH expression length)

    math(EXPR length "${length}-1")
    foreach(memberIdx RANGE ${length})
        if(${skipNext})
            set(skipNext OFF)
            continue()
        endif()

        list(GET expression ${memberIdx} member)

        if("${member}" STREQUAL "(")
            if(${nestingLevel} GREATER 0)
                list(APPEND result ${member})
            endif()
            math(EXPR nestingLevel "${nestingLevel} + 1")
            continue()
        elseif("${member}" STREQUAL ")")
            math(EXPR nestingLevel "${nestingLevel} - 1")
            if(nestingLevel LESS 0)
                break()
            endif()
            if(${nestingLevel} EQUAL 0)
                qt_evaluate_config_expression(result ${result})
            else()
                list(APPEND result ${member})
            endif()
            continue()
        elseif(${nestingLevel} GREATER 0)
            list(APPEND result ${member})
            continue()
        elseif("${member}" STREQUAL "NOT")
            list(APPEND result ${member})
            continue()
        elseif("${member}" STREQUAL "AND")
            qt_evaluate_to_boolean(result)
            if(NOT ${result})
                break()
            endif()
            set(result "")
        elseif("${member}" STREQUAL "OR")
            qt_evaluate_to_boolean(result)
            if(${result})
                break()
            endif()
            set(result "")
        elseif("${member}" STREQUAL "STREQUAL" AND memberIdx LESS ${length})
            # Unfortunately the semantics for STREQUAL in if() are broken when the
            # RHS is an empty string and the parameters to if are coming through a variable.
            # So we expect people to write the empty string with single quotes and then we
            # do the comparison manually here.
            list(LENGTH result lhsIndex)
            math(EXPR lhsIndex "${lhsIndex}-1")
            list(GET result ${lhsIndex} lhs)
            list(REMOVE_AT result ${lhsIndex})
            set(lhs "${${lhs}}")

            math(EXPR rhsIndex "${memberIdx}+1")
            set(skipNext ON)

            list(GET expression ${rhsIndex} rhs)
            # We can't pass through an empty string with double quotes through various
            # stages of substitution, so instead it is represented using single quotes
            # and resolve here.
            string(REGEX REPLACE "'(.*)'" "\\1" rhs "${rhs}")

            string(COMPARE EQUAL "${lhs}" "${rhs}" stringCompareResult)
            list(APPEND result ${stringCompareResult})
        else()
            string(FIND "${member}" "QT_FEATURE_" idx)
            if(idx EQUAL 0)
                # Remove the QT_FEATURE_ prefix
                string(SUBSTRING "${member}" 11 -1 feature)
                qt_evaluate_feature(${feature})
            endif()

            list(APPEND result ${member})
        endif()
    endforeach()

    if("${result}" STREQUAL "")
        set(result ON)
    else()
        qt_evaluate_to_boolean(result)
    endif()

    set(${resultVar} ${result} PARENT_SCOPE)
endfunction()

function(qt_feature_set_cache_value resultVar feature emit_if calculated label)
    if (DEFINED "FEATURE_${feature}")
        # Must set up the cache
        if (NOT (emit_if))
            message(FATAL_ERROR "Sanity check failed: FEATURE_${feature} that was not emitted was found in the CMakeCache.")
        endif()

        # Revisit value:
        set(cache "${FEATURE_${feature}}")
        if ((cache STREQUAL "ON") OR (cache STREQUAL "OFF"))
            set(result "${cache}")
        else()
            message(FATAL_ERROR "Sanity check failed: FEATURE_${feature} has invalid value \"${cache}\"!")
        endif()
        # Fix-up user-provided values
        set("FEATURE_${feature}" "${cache}" CACHE BOOL "${label}")
    else()
        # Initial setup:
        if (emit_if)
            set("FEATURE_${feature}" "${calculated}" CACHE BOOL "${label}")
            set(result "${calculated}")
        else()
            set(result OFF)
        endif()
    endif()

    set("${resultVar}" "${result}" PARENT_SCOPE)
endfunction()

macro(qt_feature_set_value feature cache emit_if condition label)
    set(result "${cache}")

    if (NOT (condition) AND (cache))
        message(SEND_ERROR "Feature \"${feature}\": Forcing to \"${cache}\" breaks its condition.")
    endif()

    if (DEFINED "QT_FEATURE_${feature}")
        message(FATAL_ERROR "Feature ${feature} is already defined when evaluating configure.cmake features for ${target}.")
    endif()
    set(QT_FEATURE_${feature} "${result}" CACHE INTERNAL "Qt feature: ${feature}")
endmacro()

function(qt_evaluate_feature feature)
    # If the feature was set explicitly by the user to be on or off, in the cache, then
    # there's nothing for us to do.
    if(DEFINED "QT_FEATURE_${feature}")
        return()
    endif()

    if(NOT DEFINED _QT_FEATURE_DEFINITION_${feature})
        qt_debug_print_variables(DEDUP MATCH "^QT_FEATURE")
        message(FATAL_ERROR "Attempting to evaluate feature ${feature} but its definition is missing. Either the feature does not exist or a dependency to the module that defines it is missing")
    endif()

    cmake_parse_arguments(arg
        "PRIVATE;PUBLIC"
        "LABEL;PURPOSE;SECTION;" "AUTODETECT;CONDITION;ENABLE;DISABLE;EMIT_IF" ${_QT_FEATURE_DEFINITION_${feature}})

    if(DEFINED QT_FEATURE_${feature})
        return()
    endif()

    if("${arg_ENABLE}" STREQUAL "")
        set(arg_ENABLE OFF)
    endif()

    if("${arg_DISABLE}" STREQUAL "")
        set(arg_DISABLE OFF)
    endif()

    if("${arg_AUTODETECT}" STREQUAL "")
        set(arg_AUTODETECT ON)
    endif()

    if("${arg_CONDITION}" STREQUAL "")
        set(condition ON)
    else()
        qt_evaluate_config_expression(condition ${arg_CONDITION})
    endif()

    if(${arg_DISABLE})
        set(result OFF)
    elseif((${arg_ENABLE}) OR (${arg_AUTODETECT}))
        set(result ${condition})
    else()
        # feature not auto-detected and not explicitly enabled
        set(result OFF)
    endif()

    if("${arg_EMIT_IF}" STREQUAL "")
        set(emit_if ON)
    else()
        qt_evaluate_config_expression(emit_if ${arg_EMIT_IF})
    endif()

    if (NOT (condition) AND (calculated))
        message(FATAL_ERROR "Sanity check failed: Feature ${feature} is enabled but condition does not hold true.")
    endif()

    qt_feature_set_cache_value(cache "${feature}" "${emit_if}" "${result}" "${arg_LABEL}")
    qt_feature_set_value("${feature}" "${cache}" "${emit_if}" "${condition}" "${arg_LABEL}")
endfunction()

function(qt_feature_definition feature name)
    qt_parse_all_arguments(arg "qt_feature_definition" "NEGATE" "VALUE" "" ${ARGN})

    # Generate code:
    set(expected 1)
    if (arg_NEGATE)
        set(expected -1)
    endif()
    set(msg "\n#if defined(QT_FEATURE_${feature}) && QT_FEATURE_${feature} == ${expected}\n")
    if (arg_VALUE)
        string(APPEND msg "#  define ${name} ${arg_VALUE}\n")
    else()
        string(APPEND msg "#  define ${name}\n")
    endif()
    string(APPEND msg "#endif\n")

    # Store for later use:
    list(FIND __QtFeature_public_features "${feature}" public_index)
    if (public_index GREATER -1)
        string(APPEND __QtFeature_public_extra "${msg}")
    endif()
    list(FIND __QtFeature_private_features "${feature}" private_index)
    if (private_index GREATER -1)
        string(APPEND __QtFeature_private_extra "${msg}")
    endif()

    set(__QtFeature_public_extra ${__QtFeature_public_extra} PARENT_SCOPE)
    set(__QtFeature_private_extra ${__QtFeature_private_extra} PARENT_SCOPE)
endfunction()

function(qt_extra_definition name value)
    qt_parse_all_arguments(arg "qt_extra_definition" "PUBLIC;PRIVATE" "" "" ${ARGN})

    if (arg_PUBLIC)
        string(APPEND __QtFeature_public_extra "\n#define ${name} ${value}\n")
    elseif(arg_PRIVATE)
        string(APPEND __QtFeature_private_extra "\n#define ${name} ${value}\n")
    endif()

    set(__QtFeature_public_extra ${__QtFeature_public_extra} PARENT_SCOPE)
    set(__QtFeature_private_extra ${__QtFeature_private_extra} PARENT_SCOPE)
endfunction()

function(qt_internal_generate_feature_line line feature)
    if (QT_FEATURE_${feature} STREQUAL "ON")
        set(line "#define QT_FEATURE_${feature} 1\n\n" PARENT_SCOPE)
    elseif(QT_FEATURE_${feature} STREQUAL "OFF")
        set(line "#define QT_FEATURE_${feature} -1\n\n" PARENT_SCOPE)
    elseif(QT_FEATURE_${feature} STREQUAL "UNSET")
        set(line "#define QT_FEATURE_${feature} 0\n\n" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "${feature} has unexpected value \"${QT_FEATURE_${feature}}\"!")
    endif()
endfunction()

function(qt_internal_feature_write_file file features extra)
    message("Generating file ${file}.")
    set(contents "")
    foreach(it ${features})
        qt_internal_generate_feature_line(line "${it}")
        string(APPEND contents "${line}")
    endforeach()
    string(APPEND contents "${extra}")

    file(GENERATE OUTPUT "${file}" CONTENT "${contents}")
endfunction()

function(qt_feature_module_end target)
    set(all_features ${__QtFeature_public_features} ${__QtFeature_private_features})
    list(REMOVE_DUPLICATES all_features)

    foreach(feature ${all_features})
        qt_evaluate_feature(${feature})
    endforeach()

    set(enabled_public_features "")
    set(disabled_public_features "")
    set(enabled_private_features "")
    set(disabled_private_features "")

    foreach(feature ${__QtFeature_public_features})
       if(QT_FEATURE_${feature})
          list(APPEND enabled_public_features ${feature})
       else()
          list(APPEND disabled_public_features ${feature})
       endif()
    endforeach()

    foreach(feature ${__QtFeature_private_features})
       if(QT_FEATURE_${feature})
          list(APPEND enabled_private_features ${feature})
       else()
          list(APPEND disabled_private_features ${feature})
       endif()
    endforeach()

    foreach(feature ${all_features})
        unset(_QT_FEATURE_DEFINITION_${feature} PARENT_SCOPE)
    endforeach()

    qt_internal_feature_write_file("${CMAKE_CURRENT_BINARY_DIR}/${__QtFeature_private_file}"
        "${__QtFeature_private_features}" "${__QtFeature_private_extra}"
    )
    qt_generate_forwarding_headers("${__QtFeature_library}" SOURCE "${__QtFeature_private_file}" PRIVATE)

    qt_internal_feature_write_file("${CMAKE_CURRENT_BINARY_DIR}/${__QtFeature_public_file}"
        "${__QtFeature_public_features}" "${__QtFeature_public_extra}"
    )
    qt_generate_forwarding_headers("${__QtFeature_library}" SOURCE "${__QtFeature_public_file}")

    get_target_property(targetType "${target}" TYPE)
    if("${targetType}" STREQUAL "INTERFACE_LIBRARY")
        set(propertyPrefix "INTERFACE_")
    else()
        set(propertyPrefix "")
        set_target_properties("${target}" PROPERTIES EXPORT_PROPERTIES "QT_ENABLED_PUBLIC_FEATURES;QT_DISABLED_PUBLIC_FEATURES;QT_ENABLED_PRIVATE_FEATURES;QT_DISABLED_PRIVATE_FEATURES")
    endif()
    foreach(visibility public private)
        string(TOUPPER "${visibility}" capitalVisibility)
        foreach(state enabled disabled)
            string(TOUPPER "${state}" capitalState)

            set_property(TARGET "${target}" PROPERTY ${propertyPrefix}QT_${capitalState}_${capitalVisibility}_FEATURES "${${state}_${visibility}_features}")
        endforeach()
    endforeach()

    unset(__QtFeature_library PARENT_SCOPE)
    unset(__QtFeature_private_features PARENT_SCOPE)
    unset(__QtFeature_public_features PARENT_SCOPE)

    unset(__QtFeature_private_file PARENT_SCOPE)
    unset(__QtFeature_public_file PARENT_SCOPE)

    unset(__QtFeature_private_extra PARENT_SCOPE)
    unset(__QtFeature_public_extra PARENT_SCOPE)
endfunction()

function(qt_config_compile_test name)
    cmake_parse_arguments(arg "" "LABEL" "LIBRARIES;CODE" ${ARGN})

    set(_save_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
    set(CMAKE_REQUIRED_LIBRARIES "${arg_LIBRARIES}")
    check_cxx_source_compiles("${arg_UNPARSED_ARGUMENTS} ${arg_CODE}" HAVE_${name})
    set(CMAKE_REQUIRED_LIBRARIES "${_save_CMAKE_REQUIRED_LIBRARIES}")
    set(TEST_${name} "${HAVE_${name}}" CACHE INTERNAL "${arg_LABEL}")
endfunction()

function(qt_config_compile_test_x86simd extension label)
    string(TOUPPER ${extension} extension_uppercase)
    if (DEFINED TEST_X86SIMD_${extension})
        return()
    endif()

    try_compile(TEST_X86SIMD_${extension} "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/config.tests/x86_simd/main.cpp"
        COMPILE_DEFINITIONS -DQT_COMPILER_SUPPORTS_${extension_uppercase}
        OUTPUT_VARIABLE foo
    )
    set(TEST_subarch_${extension} "${TEST_X86SIMD_${extension}}" CACHE INTERNAL "${label}" )
endfunction()

function(qt_make_features_available target)
    if(NOT "${target}" MATCHES "^Qt::[a-zA-z][a-zA-Z0-9_]*$")
        message(FATAL_ERROR "${target} does not match Qt::[a-zA-z][a-zA-Z0-9_]*. INVALID NAME.")
    endif()
    if(NOT TARGET ${target})
        message(FATAL_ERROR "${target} not found.")
    endif()

    get_target_property(target_type "${target}" TYPE)
    if("${target_type}" STREQUAL "INTERFACE_LIBRARY")
        set(property_prefix "INTERFACE_")
    else()
        set(property_prefix "")
    endif()
    foreach(visibility IN ITEMS PUBLIC PRIVATE)
        set(value ON)
        foreach(state IN ITEMS ENABLED DISABLED)
            get_target_property(features "${target}" ${property_prefix}QT_${state}_${visibility}_FEATURES)
            if("${features}" STREQUAL "features-NOTFOUND")
                continue()
            endif()
            foreach(feature IN ITEMS ${features})
                if (DEFINED "QT_FEATURE_${feature}" AND NOT "${QT_FEATURE_${feature}}" STREQUAL "${value}")
                    message(FATAL_ERROR "Feature ${feature} is already defined and has a different value when importing features from ${target}.")
                endif()
                set(QT_FEATURE_${feature} "${value}" CACHE INTERNAL "Qt feature: ${feature} (from target ${target})")
            endforeach()
            set(value OFF)
        endforeach()
    endforeach()
endfunction()

