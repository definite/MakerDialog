SET(LINK_TYPE SHARED)
#SET(LINK_TYPE STATIC)

# Location of library include files
INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIRS} ${GTK2_INCLUDE_DIRS}
    ${GCONF2_INCLUDE_DIRS} ../src .)

# Library location for the linker
LINK_DIRECTORIES(${GLIB2_LIBRARY_DIRS} ${GTK2_LIBRARY_DIRS}
    ${GCONF2_LIBRARY_DIRS} ../bin)

ADD_EXECUTABLE(md-example-gtk-keyfile.exe
   md-example-gtk-keyfile.c
)
TARGET_LINK_LIBRARIES(md-example-gtk-keyfile.exe MakerDialog MakerDialogGtk)

ADD_EXECUTABLE(md-example-gtk-gconf.exe
    md-example-gtk-gconf.c
)
TARGET_LINK_LIBRARIES(md-example-gtk-gconf.exe MakerDialog MakerDialogGtk  MakerDialogGConf)



INSTALL(TARGETS
    RUNTIME DESTINATION bin
)

