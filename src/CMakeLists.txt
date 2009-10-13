SET(LINK_TYPE SHARED)
#SET(LINK_TYPE STATIC)

# Location of library include files
INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIRS} ${GTK2_INCLUDE_DIRS} ${GCONF2_INCLUDE_DIRS} )

# Library location for the linker
LINK_DIRECTORIES(${GLIB2_LIBRARY_DIRS} ${GTK2_LIBRARY_DIRS} ${GCONF2_LIBRARY_DIRS})

#==================================================================
# Function check
#
INCLUDE (CheckSymbolExists)
# CHECK_SYMBOL_EXISTS(gtk_widget_set_tooltip_text "gtk/gtk.h" HAVE_GTK_WIDGET_SET_TOOLTIP_TEXT)
# CHECK_LIBRARY_EXISTS(gtkx11-2.0 gtk_widget_set_tooltip_text "" HAVE_GTK_WIDGET_SET_TOOLTIP_TEXT)
INCLUDE (CheckLibraryExists)
CHECK_LIBRARY_EXISTS(glib-2.0 g_dpgettext2 "" HAVE_G_DPGETTEXT2)
CHECK_LIBRARY_EXISTS(glib-2.0 g_once_init_enter "" HAVE_G_ONCE_INIT_ENTER)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

#==================================================================
# Sources
#
SET(MAKER_DIALOG_BASE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialog.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialog.h
${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogProperty.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogProperty.h)

SET(GOB_GENERATED
    ${CMAKE_CURRENT_SOURCE_DIR}/maker-dialog-gtk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/maker-dialog-gtk.h
    ${CMAKE_CURRENT_SOURCE_DIR}/maker-dialog-gtk-private.h
    )

ADD_CUSTOM_COMMAND(OUTPUT ${GOB_GENERATED}
    COMMAND ${GOB2} ${CMAKE_CURRENT_SOURCE_DIR}/MakerDialogGtk.gob -o ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS MakerDialogGtk.gob
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

SET_SOURCE_FILES_PROPERTIES(${GOB_GENERATED}
    PROPERTIES GENERATED TRUE)

# library
ADD_LIBRARY(MakerDialog ${MAKER_DIALOG_BASE_SRC})
ADD_LIBRARY(MakerDialogGtk ${GOB_GENERATED})


# executable built from some source
ADD_EXECUTABLE(md-example
   md-example.c
)

#ADD_EXECUTABLE(dialogTest dialogTest.c
#   maker-dialog.c
#   )

# Link the executable to the library.

TARGET_LINK_LIBRARIES(MakerDialog ${GLIB2_LIBRARIES})

TARGET_LINK_LIBRARIES(MakerDialogGtk MakerDialog ${GTK2_LIBRARIES})

TARGET_LINK_LIBRARIES(md-example MakerDialog MakerDialogGtk)

#TARGET_LINK_LIBRARIES(generate-gconf-schemas ${GTK2_LIBRARIES})

#TARGET_LINK_LIBRARIES(dialogTest ${GTK2_LIBRARIES} ${IBUS_LIBRARIES}
#    ${CHEWING_LIBRARIES} ${GCONF2_LIBRARIES} ${X11_LIBRARIES} ${XTST_LIBRARIES})


IF(NOT DEFINED LIB_INSTALL_DIR)
    IF( $ENV{MACHTYPE} MATCHES "64")
	SET(LIB_INSTALL_DIR lib64)
	ADD_DEFINITIONS(-DLIB_INSTALL_DIR='"${LIB_INSTALL_DIR}"')
    ELSE()
	SET(LIB_INSTALL_DIR lib)
	ADD_DEFINITIONS(-DLIB_INSTALL_DIR='"${LIB_INSTALL_DIR}"')
    ENDIF()
ENDIF()

#ADD_CUSTOM_COMMAND(TARGET generate-gconf-schemas POST_BUILD
#    COMMAND ${CMAKE_BINARY_DIR}/bin/generate-gconf-schemas -v 3 -l
#   "C;${TRANSLATED}" ${CMAKE_BINARY_DIR}/data/ibus-chewing.schemas
#   COMMENT "Generating gconf-schemas"
#   VERBATIM
#   )

INSTALL(TARGETS
    RUNTIME DESTINATION bin
     LIBRARY DESTINATION ${LIB_INSTALL_DIR}
)

