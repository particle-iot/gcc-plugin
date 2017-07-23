#include "plugin/location.h"

particle::Location::Location(location_t loc) :
        Location() {
    const char* const file = LOCATION_FILE(loc);
    const int line = LOCATION_LINE(loc);
    const int col = LOCATION_LINE(loc);
    if (file && line > 0 && col > 0) {
        file_ = file;
        line_ = line;
        col_ = col;
    }
}
