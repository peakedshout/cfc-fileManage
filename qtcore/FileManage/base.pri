CONFIG(debug, debug|release) {
    BUILD_TYPE = debug
}

CONFIG(release, debug|release) {
    BUILD_TYPE = release
}

OBJECTS_DIR = $$BUILD_TYPE/obj
MOC_DIR     = $$BUILD_TYPE/moc
RCC_DIR     = $$BUILD_TYPE/rcc
UI_DIR      = $$BUILD_TYPE/ui
DESTDIR     = $$PWD/../bin_$${TARGET}_output/$$BUILD_TYPE

LIBDIR = $$DESTDIR

message("library path: $${LIBDIR}")
message("bin path:     $${DESTDIR}/$${TARGET}")
