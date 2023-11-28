#include <memory>

namespace RAII {
    template<typename Provider>
    class Booking {
    private:
        using BookingId = typename Provider::BookingId;

        Provider *provider = nullptr;
        BookingId booking_id = 0;

    public:
        Booking(Provider *p, int counter) : provider(p), booking_id(counter) {}

        ~Booking() {
            if (provider) {
                provider->CancelOrComplete(*this);
            }
        }

        Booking(const Booking &) = delete;

        Booking(Booking &&other) : provider(other.provider), booking_id(other.booking_id) {
            if (this != &other) {
                other.provider = nullptr;
            }
        }

        Booking &operator=(const Booking &) = delete;

        Booking &operator=(Booking &&other) {
            if (this != &other) {
                provider = other.provider;
                booking_id = other.booking_id;
                other.provider = nullptr;
            }
            return *this;
        }

        BookingId GetId() const {
            return booking_id;
        }
    };
}