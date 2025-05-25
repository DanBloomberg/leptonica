# cmake/LeptonicaFunc.cmake

# ##############################################################################
# FUNCTION find_and_handle_library
# ##############################################################################
function(find_and_handle_library LIB_NAME ENABLE_OPTION PKG_CONFIG_NAME)
  if(${ENABLE_OPTION})
    find_package(${PKG_CONFIG_NAME})
    if(${PKG_CONFIG_NAME}_FOUND)
      set(pkgs_private
          "${pkgs_private} ${PKG_CONFIG_NAME}"
          PARENT_SCOPE)
      # Propagate found variables to parent scope
      set(${PKG_CONFIG_NAME}_FOUND
          ${${PKG_CONFIG_NAME}_FOUND}
          PARENT_SCOPE)
      if(DEFINED ${PKG_CONFIG_NAME}_LIBRARIES)
        set(${PKG_CONFIG_NAME}_LIBRARIES
            ${${PKG_CONFIG_NAME}_LIBRARIES}
            PARENT_SCOPE)
      endif()
      if(DEFINED ${PKG_CONFIG_NAME}_INCLUDE_DIRS)
        set(${PKG_CONFIG_NAME}_INCLUDE_DIRS
            ${${PKG_CONFIG_NAME}_INCLUDE_DIRS}
            PARENT_SCOPE)
      endif()
    else()
      if(STRICT_CONF)
        message(
          FATAL_ERROR
            "Could not find ${PKG_CONFIG_NAME} libs. Use -D${ENABLE_OPTION}=OFF to disable ${PKG_CONFIG_NAME} support."
        )
      endif()
    endif()
  endif()
endfunction()
