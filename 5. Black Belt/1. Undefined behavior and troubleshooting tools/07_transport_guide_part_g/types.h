#pragma once

#include <cmath>
#include <iomanip>
#include <sstream>

struct Rgb {
    int red = 0;
    int green = 0;
    int blue = 0;

    Rgb() : red(0), green(0), blue(0) {}

    Rgb(int newRed, int newGreen, int newBlue)
            : red(newRed), green(newGreen), blue(newBlue) {}
};

struct Rgba : public Rgb {
    double alpha = 0.0;

    Rgba() : Rgb(), alpha(0.0) {}

    Rgba(int newRed, int newGreen, int newBlue, double newAlpha)
            : Rgb(newRed, newGreen, newBlue), alpha(newAlpha) {}
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
const Color NoneColor{};

std::string GetString(const Color &color) {
    if (std::holds_alternative<std::monostate>(color)) {
        return "none";
    } else if (std::holds_alternative<std::string>(color)) {
        return std::get<std::string>(color);
    } else if (std::holds_alternative<Rgb>(color)) {
        const auto rgb = std::get<Rgb>(color);
        return "rgb(" + std::to_string(rgb.red) + "," + std::to_string(rgb.green) +
               "," + std::to_string(rgb.blue) + ")";
    } else {
        const auto rgba = std::get<Rgba>(color);
        std::stringstream doubleStr;
        doubleStr << std::fixed << std::setprecision(6) << rgba.alpha;
        return "rgba(" + std::to_string(rgba.red) + "," +
               std::to_string(rgba.green) + "," + std::to_string(rgba.blue) + "," +
               doubleStr.str() + ")";
    }
}

struct Point {
    double x = 0.0;
    double y = 0.0;

    Point() : x(0.0), y(0.0) {}

    Point(double newX, double newY) : x(newX), y(newY) {}
};

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