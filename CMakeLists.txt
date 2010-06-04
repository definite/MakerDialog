cmake_minimum_required(VERSION 2.4)
####################################################################
# Init Definition
####################################################################
INCLUDE(init.CMakeLists.txt)

####################################################################
# Project basic information
####################################################################
PROJECT(MakerDialog)
SET(PROJECT_DESCRIPTION "Utility that conveniently handle configuration properties")
SET(CMAKE_C_FLAGS "-Wall -rdynamic")

SET(RELEASE_FILE ${CMAKE_SOURCE_DIR}/RELEASE-NOTES.txt)
SET(RPM_RELEASE_FILE ${CMAKE_SOURCE_DIR}/SPECS/RPM-RELEASE-NOTES.txt)
SETTING_FILE_GET_ATTRIBUTE(PRJ_VER_PATTERN "PRJ_VER" ${RELEASE_FILE})
SETTING_FILE_GET_ATTRIBUTE(PRJ_VER_RELEASE "RPM_RELEASE" ${RPM_RELEASE_FILE})

SETTING_FILE_GET_ATTRIBUTE(SO_VER_MAJOR "SO_VER_MAJOR" ${RELEASE_FILE})
SETTING_FILE_GET_ATTRIBUTE(SO_VER_MINOR "SO_VER_MINOR" ${RELEASE_FILE})
SET(AUTHORS "Ding-Yi Chen")
SET(MAINTAINER "Ding-Yi Chen <dchen at redhat.com>")

# For CVS/SVN/GIT change message
SETTING_FILE_GET_ATTRIBUTE(CHANGE_SUMMARY "SUMMARY" ${RELEASE_FILE})

# Changelog items
COMMAND_OUTPUT_TO_VARIABLE(CHANGELOG_ITEMS tail -n +6 ${RELEASE_FILE})

STRING(COMPARE GREATER "${PRJ_VER_RELEASE}" "1" NEED_RPM_RELEASE_INFO)
IF(${NEED_RPM_RELEASE_INFO})
    SETTING_FILE_GET_ATTRIBUTE(RPM_RELEASE_SUMMARY "RPM_RELEASE_SUMMARY" ${RPM_RELEASE_FILE})
    COMMAND_OUTPUT_TO_VARIABLE(RPM_CHANGELOG_ITEMS tail -n +5 ${RPM_RELEASE_FILE})
ELSE(${NEED_RPM_RELEASE_INFO})
    SET(RPM_RELEASE_SUMMARY ${CHANGE_SUMMARY})
    SET(RPM_CHANGELOG_ITEMS ${CHANGELOG_ITEMS})
ENDIF(${NEED_RPM_RELEASE_INFO})

#==================================================================
# Project specific setting
#

SET_ENV(PRJ_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib${IS_64}/${PROJECT_NAME})

#==================================================================
# Developer setting.
#
SET(HOSTING_SERVICE_PROVIDER "GitHub")
SET(DEVELOPER_SETTING_FILE "DEVELOPER_SETTING_NO_PACK")
#SET(DEVELOPER_DEPENDS ***USER_DEFINE***)
ADD_CUSTOM_TARGET(commit
    COMMAND git commit -a -m "${CHANGE_SUMMARY}"
    COMMAND git tag -a "${PRJ_VER}" -m "Ver ${PRJ_VER}" HEAD
    COMMENT "Commit and tag the changes"
    VERBATIM
    )


####################################################################
# Includes
####################################################################

#==================================================================
# Include common.CMakeLists.txt
#
SET(DOXYGEN_EXCLUDE "src/gtk")
SET(GENERATE_DOXYFILE TRUE)
INCLUDE(common.CMakeLists.txt)

####################################################################
# Post Include setting
####################################################################
SET(FEDORA_DIST_TAGS ${FEDORA_DIST_TAGS} EL-5)

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/project.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc" @ONLY)

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/project-gtk.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-gtk.pc" @ONLY)

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/project-gconf.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-gconf.pc" @ONLY)

####################################################################
# RPM configuration
####################################################################
FILE(READ ${CMAKE_SOURCE_DIR}/SPECS/RPM-ChangeLog RPM_CHANGELOG)
GENERATE_SPEC(SPECS/project.spec.in)

####################################################################
# Definitions
####################################################################

####################################################################
# Required
####################################################################
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(GLIB2 REQUIRED glib-2.0)
PKG_CHECK_MODULES(GOBJECT2 REQUIRED gobject-2.0)
PKG_CHECK_MODULES(GTK2 REQUIRED gtk+-2.0>=2.10)
PKG_CHECK_MODULES(GCONF2 REQUIRED gconf-2.0)

FIND_PROGRAM(GOB2 gob2)
IF(${GOB2} STREQUAL "GOB2-NOTFOUND")
    MESSAGE(FATAL_ERROR "gob2 not found, install gob2 please.")
ENDIF()


####################################################################
# Sub directories
####################################################################
ADD_SUBDIRECTORY(src bin)
ADD_SUBDIRECTORY(test)

ADD_CUSTOM_COMMAND(OUTPUT examples/Makefile
    COMMAND cmake -E chdir examples cmake .
    COMMENT "Build examples/Makefile"
    )

ADD_CUSTOM_TARGET(examples
    COMMAND cmake -E chdir examples make all
    DEPENDS examples/Makefile
    COMMENT "Making examples"
    )

ADD_CUSTOM_TARGET(clean_examples
    COMMAND cmake -E chdir examples make clean
    DEPENDS examples/Makefile
    COMMENT "Making examples clean"
    )

ADD_CUSTOM_TARGET(clean_all
    COMMAND make clean
    DEPENDS examples/Makefile
    COMMENT "Making all clean"
    )

ADD_DEPENDENCIES(clean_all clean_examples)


####################################################################
# Packing
####################################################################

#====================================================================
# Files to be install.
#
SET(MAIN_DOCS AUTHORS README ChangeLog COPYING COPYING.LESSER
    RELEASE-NOTES.txt)

#INSTALL(FILES ${MAIN_DOCS}
#    DESTINATION "${PROJECT_DOCDIR}")

# pkgconfig files.
INSTALL(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pc
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}-gtk.pc
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}-gconf.pc
    DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

#====================================================================
# CPack configuration
#

SET(CPACK_RESOURCE_FILE_LICENSE
    ${CMAKE_CURRENT_SOURCE_DIR}/COPYING)
SET(PRJ_COMMON_IGNORE_FILES
    "/docs/"
    "/user/"
    "\\\\.cache$"  "\\\\.spec$" "/doc/"
    "\\\\.exe$"
    "Doxyfile$"
    "MakerDialog.*\\\\.pc$"
    "messages.po$"
)

SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_SOURCE_IGNORE_FILES} ${PRJ_COMMON_IGNORE_FILES} "/bin/"
    "\\\\.xml$" "\\\\.schemas" "/doc/" )
SET(CPACK_PACKAGE_IGNORE_FILES ${CPACK_BINARY_IGNORE_FILES} ${PRJ_COMMON_IGNORE_FILES} "/src/"
    "\\\\.spec$" "\\\\.in$" "\\\\.pot$")

INCLUDE(CPack)

# Avoid gob2 bug.
ADD_DEPENDENCIES(pack_src all)

####################################################################
# Test Suites.
####################################################################
ENABLE_TESTING()

ADD_TEST("string_split_set"
    ${PROJECT_BINARY_DIR}/test/check_util.exe 0)
ADD_TEST("string_list_combine"
    ${PROJECT_BINARY_DIR}/test/check_util.exe 1)
ADD_TEST(cmp
    ${PROJECT_BINARY_DIR}/test/check_types.exe 0)
ADD_TEST(fromStr
    ${PROJECT_BINARY_DIR}/test/check_types.exe 1)

