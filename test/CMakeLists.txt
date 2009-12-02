# Location of library include files
INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIRS}  ${GOBJECT2_INCLUDE_DIRS}  ${GTK2_INCLUDE_DIRS}
    ${GCONF2_INCLUDE_DIRS} ../src .)

# Library location for the linker
LINK_DIRECTORIES(${GLIB2_LIBRARY_DIRS} ${GOBJECT2_LIBRARY_DIRS} ${GTK2_LIBRARY_DIRS}
    ${GCONF2_LIBRARY_DIRS} ../bin)

# Source
SET(check_functions_SRCS
    check_functions.c check_functions.h
)

# Test programs
ADD_EXECUTABLE(check_types.exe check_types.c
    ${check_functions_SRCS})
TARGET_LINK_LIBRARIES(check_types.exe MakerDialog)

ADD_EXECUTABLE(check_util.exe check_util.c
    ${check_functions_SRCS})
TARGET_LINK_LIBRARIES(check_util.exe MakerDialog)

