cmake_minimum_required(VERSION 2.4)
SET(CMAKE_C_FLAGS "-Wall")
####################################################################
# Required
####################################################################
FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(GLIB2 REQUIRED glib-2.0)
PKG_CHECK_MODULES(GOBJECT2 REQUIRED gobject-2.0)
PKG_CHECK_MODULES(GTK2 REQUIRED gtk+-2.0>=2.10)
PKG_CHECK_MODULES(GCONF2 REQUIRED gconf-2.0)
PKG_CHECK_MODULES(MAKERDIALOG MakerDialog)

FIND_PROGRAM(GOB2 gob2)
IF(${GOB2} STREQUAL "GOB2-NOTFOUND")
    MESSAGE(FATAL_ERROR "gob2 not found, install gob2 please.")
ENDIF()

IF(MAKERDIALOG_FOUND)
ELSE()
    SET(MAKERDIALOG_INCLUDE_DIRS ../src)
    SET(MAKERDIALOG_LIBRARY_DIRS ../bin .)
ENDIF()

# Location of library include files
INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIRS} ${GTK2_INCLUDE_DIRS}
    ${GCONF2_INCLUDE_DIRS} ${MAKERDIALOG_INCLUDE_DIRS} .)

# Library location for the linker
LINK_DIRECTORIES(${GLIB2_LIBRARY_DIRS} ${GTK2_LIBRARY_DIRS}
    ${GCONF2_LIBRARY_DIRS} ${MAKERDIALOG_LIBRARY_DIRS} .)

SET(COMMON_LIBS  ${GLIB2_LIBRARIES} ${GOBJECT2_LIBRARIES} ${GTK2_LIBRARIES}
    MakerDialog MakerDialogGtk2)

#ADD_EXECUTABLE(md-example-gtk-keyfile.exe
#   md-example-gtk-keyfile.c
#)
#TARGET_LINK_LIBRARIES(md-example-gtk-keyfile.exe ${COMMON_LIBS})

#ADD_EXECUTABLE(md-example-gtk-keyfile2.exe
#    md-example-gtk-keyfile2.c
#    )
#TARGET_LINK_LIBRARIES(md-example-gtk-keyfile2.exe ${COMMON_LIBS})

ADD_EXECUTABLE(md-example-gtk-keyfile3.exe
    md-example-gtk-keyfile3.c
    )
TARGET_LINK_LIBRARIES(md-example-gtk-keyfile3.exe ${COMMON_LIBS}
    MakerDialogGKeyFile)

#ADD_EXECUTABLE(md-example-gtk-gconf.exe
#    md-example-gtk-gconf.c
#)
#TARGET_LINK_LIBRARIES(md-example-gtk-gconf.exe MakerDialogGConf
#    ${COMMON_LIBS} ${GCONF2_LIBRARIES})

#INSTALL(TARGETS
#    RUNTIME DESTINATION bin
#)

