#pragma once

#include "sphere.h"
#include "sphere.pb.h"

namespace Sphere {

    void SerializePoint(Point point, SphereProto::Coords &proto);

    Point DeserializePoint(const SphereProto::Coords &proto);

}
