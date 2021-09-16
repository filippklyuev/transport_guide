#pragma once

#include <cmath>

namespace geo {

struct Coordinates {
    double lat = 0.0;
    double lng = 0.0;
};

double ComputeDistance(Coordinates from, Coordinates to);

bool operator==(const Coordinates& lhs, const Coordinates& rhs);

bool operator!=(const Coordinates& lhs, const Coordinates& rhs);

} // namespace geo
