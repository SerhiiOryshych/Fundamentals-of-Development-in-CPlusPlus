#pragma once

#include <cmath>

struct LatLng {
    double lat, lng;
};

bool operator==(const LatLng &p1, const LatLng &p2) {
    return p1.lat == p2.lat && p1.lng == p2.lng;
}

const double PI = 3.1415926535;
const double EARTH_RADIUS_M = 6371.0 * 1000;

double toRadians(double degrees) { return degrees * PI / 180.0; }

double calculateDistance(const LatLng &p1, const LatLng &p2) {
    double lat1 = p1.lat;
    double lon1 = p1.lng;
    double lat2 = p2.lat;
    double lon2 = p2.lng;

    double lat1Rad = toRadians(lat1);
    double lon1Rad = toRadians(lon1);
    double lat2Rad = toRadians(lat2);
    double lon2Rad = toRadians(lon2);

    double deltaLat = lat2Rad - lat1Rad;
    double deltaLon = lon2Rad - lon1Rad;

    double a = pow(sin(deltaLat / 2), 2) +
               cos(lat1Rad) * cos(lat2Rad) * pow(sin(deltaLon / 2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    double distanceKm = EARTH_RADIUS_M * c;
    return distanceKm;
}