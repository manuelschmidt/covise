MACRO(USE_BOOST)
  set(COMPONENTS
        chrono
        program_options
        system
        thread
        filesystem
        iostreams
        date_time
        serialization
        regex
        locale
        )

  IF(WIN32)
  add_definitions("-DBOOST_ALL_NO_LIB")
  add_definitions("-DBOOST_ALL_DYN_LINK")
  set(COMPONENTS ${COMPONENTS} zlib)
  ENDIF(WIN32)
  
  covise_find_package(Boost
    COMPONENTS
    ${COMPONENTS}
    QUIET
  )
  if (Boost_FOUND AND (NOT Boost_VERSION VERSION_LESS "105300"))
    set(COMPONENTS ${COMPONENTS} atomic)
    covise_find_package(Boost
        COMPONENTS
        ${COMPONENTS}
        QUIET
    )
  endif()

  IF ((NOT Boost_FOUND) AND (${ARGC} LESS 1))
    USING_MESSAGE("Skipping because of missing Boost")
    RETURN()
  ENDIF((NOT Boost_FOUND) AND (${ARGC} LESS 1))
  IF(NOT BOOST_USED AND Boost_FOUND)
    SET(BOOST_USED TRUE)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    SET(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_LIBRARIES})
  ENDIF()
ENDMACRO(USE_BOOST)
