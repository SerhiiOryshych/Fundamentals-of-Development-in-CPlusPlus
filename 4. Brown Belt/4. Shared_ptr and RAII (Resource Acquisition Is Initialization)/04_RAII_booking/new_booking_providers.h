#pragma once

// Your file with the definition of the Booking template class in the RAII namespace will be included here.
#include "booking.h"

#include <stdexcept>
#include <string>

using namespace std;

class FlightProvider {
public:
    using BookingId = int;
    using Booking = RAII::Booking<FlightProvider>;
    // We explicitly allow the functions of the Booking class to call private functions of our FlightProvider class.
    friend Booking;

    struct BookingData {
        string city_from;
        string city_to;
        string date;
    };

    Booking Book(const BookingData &data) {
        if (counter >= capacity) {
            throw runtime_error("Flight overbooking");
        }
        ++counter;
        return {this, counter};
    }

private:
    // We hide this function in private so that it can only be called by the corresponding friend class Booking.
    void CancelOrComplete(const Booking &booking) {
        --counter;
    }

public:
    static int capacity;
    static int counter;
};


class HotelProvider {
public:
    using BookingId = int;
    using Booking = RAII::Booking<HotelProvider>;
    friend Booking;

    struct BookingData {
        string city;
        string date_from;
        string date_to;
    };

    Booking Book(const BookingData &data) {
        if (counter >= capacity) {
            throw runtime_error("Hotel overbooking");
        }
        ++counter;
        return {this, counter};
    }

private:
    void CancelOrComplete(const Booking &booking) {
        --counter;
    }

public:
    static int capacity;
    static int counter;
};
