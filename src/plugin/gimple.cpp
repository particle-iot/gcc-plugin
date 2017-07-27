#include "gimple.h"

particle::Location particle::location(const_gimple g) {
    return gimple_location_safe(g);
}
