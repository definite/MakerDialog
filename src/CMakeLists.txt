SET(LINK_TYPE SHARED)
SET(BUILD_SHARED_LIBS TRUE)
#SET(LINK_TYPE STATIC)

# Location of library include files
INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIRS}
    ${GTK2_INCLUDE_DIRS} ${GCONF2_INCLUDE_DIRS} . )

# Library location for the linker
LINK_DIRECTORIES(${GLIB2_LIBRARY_DIRS} ${GTK2_LIBRARY_DIRS} ${GCONF2_LIBRARY_DIRS})

#==================================================================
# Function check
#
INCLUDE (CheckLibraryExists)
CHECK_LIBRARY_EXISTS(gtk-x11-2.0 gtk_widget_set_tooltip_text "" HAVE_GTK_WIDGET_SET_TOOLTIP_TEXT)
CHECK_LIBRARY_EXISTS(glib-2.0 g_dpgettext2 "" HAVE_G_DPGETTEXT2)
CHECK_LIBRARY_EXISTS(glib-2.0 g_once_init_enter "" HAVE_G_ONCE_INIT_ENTER)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

#==================================================================
# Sources
#
SET(MAKER_DIALOG_BASE_SRC_C
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialog.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogConfig.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogConfigFile.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogConfigSet.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogConfigKeyFile.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogModule.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogPage.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogProperty.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogSpecParser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogTypes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogUi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogUtil.c
    )

SET(MAKER_DIALOG_BASE_SRC_H
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogConfigDef.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogConfigFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogConfigSet.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogConfigKeyFile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogModule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogPage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogProperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogSpecParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogUi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogUtil.h
    )

SET(MAKER_DIALOG_BASE_SRC
    ${MAKER_DIALOG_BASE_SRC_C} ${MAKER_DIALOG_BASE_SRC_H}
    )

SET(GOB_GENERATED
    ${CMAKE_CURRENT_SOURCE_DIR}/gtk/maker-dialog-gtk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gtk/maker-dialog-gtk.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gtk/maker-dialog-gtk-private.h
    )

ADD_CUSTOM_COMMAND(OUTPUT ${GOB_GENERATED}
    COMMAND ${GOB2} --no-touch ${CMAKE_CURRENT_SOURCE_DIR}/gtk/MakerDialogGtk.gob -o ${CMAKE_CURRENT_SOURCE_DIR}/gtk
    DEPENDS gtk/MakerDialogGtk.gob
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gtk
)

SET_SOURCE_FILES_PROPERTIES(${GOB_GENERATED}
    PROPERTIES GENERATED TRUE)

# Output executable or library
ADD_LIBRARY(MakerDialog ${MAKER_DIALOG_BASE_SRC})
TARGET_LINK_LIBRARIES(MakerDialog dl ${GLIB2_LIBRARIES})

ADD_LIBRARY(MakerDialogGConf2 MakerDialogConfigGConf.h MakerDialogConfigGConf.c)
TARGET_LINK_LIBRARIES(MakerDialogGConf2 MakerDialog ${GCONF2_LIBRARIES})

ADD_LIBRARY(MakerDialogGKeyFile
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogConfigKeyFile.c ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogConfigKeyFile.h)
TARGET_LINK_LIBRARIES(MakerDialogGKeyFile MakerDialog)

ADD_LIBRARY(MakerDialogGtk2 MakerDialogUiGtk.h ${GOB_GENERATED})
TARGET_LINK_LIBRARIES(MakerDialogGtk2 MakerDialog ${GTK2_LIBRARIES})

ADD_EXECUTABLE(MakerDialogGConfSchemas MakerDialogConfigGConfSchemas.c)
TARGET_LINK_LIBRARIES(MakerDialogGConfSchemas MakerDialog MakerDialogGConf2)

SET_TARGET_PROPERTIES(MakerDialog MakerDialogGKeyFile MakerDialogGtk2 MakerDialogGConf2
    PROPERTIES SOVERSION "${SO_VER_MAJOR}"
    VERSION "${SO_VER_MAJOR}.${SO_VER_MINOR}")


#ADD_CUSTOM_COMMAND(TARGET generate-gconf-schemas POST_BUILD
#    COMMAND ${CMAKE_BINARY_DIR}/bin/generate-gconf-schemas -v 3 -l
#   "C;${TRANSLATED}" ${CMAKE_BINARY_DIR}/data/ibus-chewing.schemas
#   COMMENT "Generating gconf-schemas"
#   VERBATIM
#   )

INSTALL(TARGETS MakerDialog MakerDialogGConf2 MakerDialogGKeyFile MakerDialogGtk2
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}/static
)

INSTALL(FILES ${MAKER_DIALOG_BASE_SRC_H}
    MakerDialogUiGtk.h
    MakerDialogConfigGConf.h
    DESTINATION include/${PROJECT_NAME}
    )

INSTALL(FILES  ${CMAKE_CURRENT_SOURCE_DIR}/gtk/maker-dialog-gtk.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gtk/maker-dialog-gtk-private.h
    DESTINATION include/${PROJECT_NAME}/gtk
    )

