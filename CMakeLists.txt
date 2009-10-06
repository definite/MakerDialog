cmake_minimum_required(VERSION 2.4)
####################################################################
# Init Definition
####################################################################
INCLUDE(init.CMakeLists.txt)

####################################################################
# Project basic information
####################################################################
PROJECT(MakerDialog)
SET(PROJECT_DESCRIPTION "MakerDialog provides a simple way to make a
    configuration dialog.")
SET(CMAKE_C_FLAGS "-Wall")

SET(RELEASE_FILE ${CMAKE_SOURCE_DIR}/RELEASE-NOTES.txt)
SET(RPM_RELEASE_FILE ${CMAKE_SOURCE_DIR}/SPECS/RPM-RELEASE-NOTES.txt)
SETTING_FILE_GET_ATTRIBUTE(PRJ_VER_PATTERN "PRJ_VER" ${RELEASE_FILE})
#MESSAGE("PRJ_VER_PATTERN=${PRJ_VER_PATTERN}")
SETTING_FILE_GET_ATTRIBUTE(PRJ_VER_RELEASE "RPM_RELEASE" ${RPM_RELEASE_FILE})

SETTING_FILE_GET_ATTRIBUTE(PRJ_VER_PATTERN "SO_VER_MAJOR" ${RELEASE_FILE})
SETTING_FILE_GET_ATTRIBUTE(PRJ_VER_PATTERN "SO_VER_MINOR" ${RELEASE_FILE})
SET(AUTHORS "Ding-Yi Chen")
SET(MAINTAINER "Ding-Yi Chen <dchen at redhat.com>")

# For CVS/SVN/GIT change message
SETTING_FILE_GET_ATTRIBUTE(CHANGE_SUMMARY "SUMMARY" ${RELEASE_FILE})
#MESSAGE("### CHANGE_SUMMARY=${CHANGE_SUMMARY}")

# Changelog items
COMMAND_OUTPUT_TO_VARIABLE(CHANGELOG_ITEMS tail -n +6 ${RELEASE_FILE})
#MESSAGE("### CHANGELOG_ITEMS=${CHANGELOG_ITEMS}|")

STRING(COMPARE GREATER "${PRJ_VER_RELEASE}" "1" NEED_RPM_RELEASE_INFO)
IF(${NEED_RPM_RELEASE_INFO})
    SETTING_FILE_GET_ATTRIBUTE(RPM_RELEASE_SUMMARY "RPM_RELEASE_SUMMARY" ${RPM_RELEASE_FILE})
    COMMAND_OUTPUT_TO_VARIABLE(RPM_CHANGELOG_ITEMS tail -n +5 ${RPM_RELEASE_FILE})
ELSE(${NEED_RPM_RELEASE_INFO})
    SET(RPM_RELEASE_SUMMARY ${CHANGE_SUMMARY})
    SET(RPM_CHANGELOG_ITEMS ${CHANGELOG_ITEMS})
ENDIF(${NEED_RPM_RELEASE_INFO})
#MESSAGE("### RPM_RELEASE_SUMMARY=${RPM_RELEASE_SUMMARY}")
#MESSAGE("### RPM_CHANGELOG_ITEMS=|${RPM_CHANGELOG_ITEMS}|")


#==================================================================
# Developer setting.
#
SET(CVS_DIST_TAGS F-10 F-11)
SET(HOSTING_SERVICE_PROVIDER "GitHub")
SET(DEVELOPER_SETTING_FILE "DEVELOPER_SETTING_NO_PACK")
#SET(DEVELOPER_DEPENDS ***USER_DEFINE***)
ADD_CUSTOM_TARGET(commit
    COMMAND git commit -a -m "${CHANGE_SUMMARY}"
    COMMAND git tag -a "${PRJ_VER}" -m "Ver ${PRJ_VER}"
    COMMENT "Commit and tag the changes"
    )

####################################################################
# Includes
####################################################################

#==================================================================
# Include common.CMakeLists.txt
#
SET(GENERATE_DOXYFILE TRUE)
INCLUDE(common.CMakeLists.txt)

####################################################################
# Post Include setting
####################################################################

####################################################################
# RPM configuration
####################################################################
FILE(READ ${CMAKE_SOURCE_DIR}/SPECS/RPM-ChangeLog RPM_CHANGELOG)
GENERATE_SPEC(SPECS/project.spec.in)

####################################################################
# Definitions
####################################################################
ADD_DEFINITIONS(-DPRJ_VERSION='"${PRJ_VER_FULL}"')
ADD_DEFINITIONS(-DPKGDATADIR='"${PROJECT_DATADIR}"')
ADD_DEFINITIONS(-DSYSTEMDIR='"${PROJECT_DATADIR}"')

####################################################################
# Required
####################################################################
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(GLIB2 REQUIRED glib-2.0)
PKG_CHECK_MODULES(GTK2 REQUIRED gtk+-2.0)
PKG_CHECK_MODULES(GCONF2 REQUIRED gconf-2.0)

FIND_PROGRAM(GOB2 gob2)
IF(${GOB2} STREQUAL "GOB2-NOTFOUND")
    MESSAGE(FATAL_ERROR "gob2 not found, install gob2 please.")
ENDIF()


####################################################################
# Sub directories
####################################################################
ADD_SUBDIRECTORY(src bin)

####################################################################
# Packing
####################################################################

#====================================================================
# Files to be install.
#
SET(MAIN_DOCS AUTHORS README ChangeLog COPYING)

#INSTALL(FILES ${MAIN_DOCS}
#    DESTINATION "${PROJECT_DOCDIR}")



#====================================================================
# CPack configuration
#

SET(CPACK_RESOURCE_FILE_LICENSE
    ${CMAKE_CURRENT_SOURCE_DIR}/COPYING)
SET(PRJ_COMMON_IGNORE_FILES
    "/docs/"
    "\\\\.cache$"  "\\\\.spec$"
    "messages.po$"
)

SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_SOURCE_IGNORE_FILES} ${PRJ_COMMON_IGNORE_FILES} "/bin/"
    "\\\\.xml$" "\\\\.schemas")
SET(CPACK_PACKAGE_IGNORE_FILES ${CPACK_BINARY_IGNORE_FILES} ${PRJ_COMMON_IGNORE_FILES} "/src/"
    "\\\\.spec$" "\\\\.in$" "\\\\.pot$")

INCLUDE(CPack)

ADD_DEPENDENCIES(pack_src pot_file)

