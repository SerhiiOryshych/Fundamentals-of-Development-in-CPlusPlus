#include "sphere_serialize.h"

using namespace std;

namespace Sphere {

    void SerializePoint(Point point, SphereProto::Coords &proto) {
        proto.set_lat(point.latitude);
        proto.set_lon(point.longitude);
    }

    Point DeserializePoint(const SphereProto::Coords &proto) {
        return {proto.lat(), proto.lon()};
    }

}
