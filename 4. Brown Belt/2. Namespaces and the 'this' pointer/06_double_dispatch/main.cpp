#include "geo2d.h"
#include "game_object.h"

#include "test_runner.h"

#include <vector>
#include <memory>

using namespace std;

// Define the classes Unit, Building, Tower, and Fence in such a way that they inherit from
// GameObject and implement its interface.

class Unit : public GameObject {
public:
    explicit Unit(geo2d::Point position);

    [[nodiscard]] geo2d::Point GetGeometry() const;

    [[nodiscard]] bool Collide(const GameObject &that) const override;

    [[nodiscard]] bool CollideWith(const Unit &that) const override;

    [[nodiscard]] bool CollideWith(const Building &that) const override;

    [[nodiscard]] bool CollideWith(const Tower &that) const override;

    [[nodiscard]] bool CollideWith(const Fence &that) const override;

private:
    geo2d::Point geometry;
};

class Building : public GameObject {
public:
    explicit Building(geo2d::Rectangle geometry);

    [[nodiscard]] geo2d::Rectangle GetGeometry() const;

    [[nodiscard]] bool Collide(const GameObject &that) const override;

    [[nodiscard]] bool CollideWith(const Unit &that) const override;

    [[nodiscard]] bool CollideWith(const Building &that) const override;

    [[nodiscard]] bool CollideWith(const Tower &that) const override;

    [[nodiscard]] bool CollideWith(const Fence &that) const override;

private:
    geo2d::Rectangle geometry;
};

class Tower : public GameObject {
public:
    explicit Tower(geo2d::Circle geometry);

    [[nodiscard]] geo2d::Circle GetGeometry() const;

    [[nodiscard]] bool Collide(const GameObject &that) const override;

    [[nodiscard]] bool CollideWith(const Unit &that) const override;

    [[nodiscard]] bool CollideWith(const Building &that) const override;

    [[nodiscard]] bool CollideWith(const Tower &that) const override;

    [[nodiscard]] bool CollideWith(const Fence &that) const override;

private:
    geo2d::Circle geometry;
};

class Fence : public GameObject {
public:
    explicit Fence(geo2d::Segment geometry);

    [[nodiscard]] geo2d::Segment GetGeometry() const;

    [[nodiscard]] bool Collide(const GameObject &that) const override;

    [[nodiscard]] bool CollideWith(const Unit &that) const override;

    [[nodiscard]] bool CollideWith(const Building &that) const override;

    [[nodiscard]] bool CollideWith(const Tower &that) const override;

    [[nodiscard]] bool CollideWith(const Fence &that) const override;

private:
    geo2d::Segment geometry;
};

Unit::Unit(geo2d::Point position) : geometry(position) {}

geo2d::Point Unit::GetGeometry() const {
    return geometry;
}

bool Unit::Collide(const GameObject &that) const {
    return that.CollideWith(*this);
}

bool Unit::CollideWith(const Unit &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Unit::CollideWith(const Building &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Unit::CollideWith(const Tower &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Unit::CollideWith(const Fence &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

Building::Building(geo2d::Rectangle geometry) : geometry(geometry) {}

geo2d::Rectangle Building::GetGeometry() const {
    return geometry;
}

bool Building::Collide(const GameObject &that) const {
    return that.CollideWith(*this);
}

bool Building::CollideWith(const Unit &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Building::CollideWith(const Building &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Building::CollideWith(const Tower &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Building::CollideWith(const Fence &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

Tower::Tower(geo2d::Circle geometry) : geometry(geometry) {}

geo2d::Circle Tower::GetGeometry() const {
    return geometry;
}

bool Tower::Collide(const GameObject &that) const {
    return that.CollideWith(*this);
}

bool Tower::CollideWith(const Unit &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Tower::CollideWith(const Building &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Tower::CollideWith(const Tower &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Tower::CollideWith(const Fence &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

Fence::Fence(geo2d::Segment geometry) : geometry(geometry) {}

geo2d::Segment Fence::GetGeometry() const {
    return geometry;
}

bool Fence::Collide(const GameObject &that) const {
    return that.CollideWith(*this);
}

bool Fence::CollideWith(const Unit &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Fence::CollideWith(const Building &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Fence::CollideWith(const Tower &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

bool Fence::CollideWith(const Fence &that) const {
    return geo2d::Collide(GetGeometry(), that.GetGeometry());
}

// Implement the function Collide from the GameObject.h file.
bool Collide(const GameObject &first, const GameObject &second) {
    return first.Collide(second);
}

void TestAddingNewObjectOnMap() {
    // The unit test simulates a situation where there are already some objects on the game map,
    // and we want to add a new one, for example, to build a new building or tower.
    // We can add it only if it does not intersect with any of the existing ones.
    using namespace geo2d;

    const vector<shared_ptr<GameObject>> game_map = {
            make_shared<Unit>(Point{3, 3}),
            make_shared<Unit>(Point{5, 5}),
            make_shared<Unit>(Point{3, 7}),
            make_shared<Fence>(Segment{{7, 3},
                                       {9, 8}}),
            make_shared<Tower>(Circle{Point{9, 4}, 1}),
            make_shared<Tower>(Circle{Point{10, 7}, 1}),
            make_shared<Building>(Rectangle{{11, 4},
                                            {14, 6}})
    };

    for (size_t i = 0; i < game_map.size(); ++i) {
        Assert(
                Collide(*game_map[i], *game_map[i]),
                "An object doesn't collide with itself: " + to_string(i)
        );

        for (size_t j = 0; j < i; ++j) {
            Assert(
                    !Collide(*game_map[i], *game_map[j]),
                    "Unexpected collision found " + to_string(i) + ' ' + to_string(j)
            );
        }
    }

    auto new_warehouse = make_shared<Building>(Rectangle{{4, 3},
                                                         {9, 6}});
    ASSERT(!Collide(*new_warehouse, *game_map[0]));
    ASSERT(Collide(*new_warehouse, *game_map[1]));
    ASSERT(!Collide(*new_warehouse, *game_map[2]));
    ASSERT(Collide(*new_warehouse, *game_map[3]));
    ASSERT(Collide(*new_warehouse, *game_map[4]));
    ASSERT(!Collide(*new_warehouse, *game_map[5]));
    ASSERT(!Collide(*new_warehouse, *game_map[6]));

    auto new_defense_tower = make_shared<Tower>(Circle{{8, 2}, 2});
    ASSERT(!Collide(*new_defense_tower, *game_map[0]));
    ASSERT(!Collide(*new_defense_tower, *game_map[1]));
    ASSERT(!Collide(*new_defense_tower, *game_map[2]));
    ASSERT(Collide(*new_defense_tower, *game_map[3]));
    ASSERT(Collide(*new_defense_tower, *game_map[4]));
    ASSERT(!Collide(*new_defense_tower, *game_map[5]));
    ASSERT(!Collide(*new_defense_tower, *game_map[6]));
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestAddingNewObjectOnMap);
    return 0;
}
