SET(LINK_TYPE SHARED)
#SET(LINK_TYPE STATIC)

# Location of library include files
INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIRS} ${GTK2_INCLUDE_DIRS}
    ${GCONF2_INCLUDE_DIRS} ../src .)

# Library location for the linker
LINK_DIRECTORIES(${GLIB2_LIBRARY_DIRS} ${GTK2_LIBRARY_DIRS}
    ${GCONF2_LIBRARY_DIRS} ../bin)

# library
ADD_EXECUTABLE(md-example.exe
   md-example.c
)

TARGET_LINK_LIBRARIES(md-example.exe MakerDialog MakerDialogGtk)

INSTALL(TARGETS
    RUNTIME DESTINATION bin
)

