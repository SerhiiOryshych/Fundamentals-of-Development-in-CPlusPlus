#include "Common.h"
#include <utility>

using namespace std;

class RectangleShape : public IShape {
public:
    RectangleShape() = default;

    RectangleShape(const Point &position, const Size &size,
                   shared_ptr<ITexture> texture)
            : position(position), size(size), texture(std::move(texture)) {}

    [[nodiscard]] unique_ptr<IShape> Clone() const override {
        return make_unique<RectangleShape>(this->position, this->size,
                                           this->texture);
    }

    void SetPosition(Point new_position) override { position = new_position; }

    [[nodiscard]] Point GetPosition() const override { return position; }

    void SetSize(Size new_size) override { size = new_size; }

    [[nodiscard]] Size GetSize() const override { return size; }

    void SetTexture(shared_ptr<ITexture> new_texture) override {
        texture = new_texture;
    }

    [[nodiscard]] ITexture *GetTexture() const override { return texture.get(); }

    void Draw(Image &image) const override {
        Size imageSize = Size({(int) image[0].size(), (int) image.size()});
        Size textureSize = texture ? texture->GetSize() : Size({0, 0});

        for (int x = 0; x < size.width; x++) {
            for (int y = 0; y < size.height; y++) {
                Point posInImage = Point({x + position.x, y + position.y});

                if (posInImage.x >= imageSize.width || posInImage.y >= imageSize.height) continue;

                if (x >= textureSize.width || y >= textureSize.height) {
                    image[posInImage.y][posInImage.x] = '.';
                } else {
                    image[posInImage.y][posInImage.x] = texture->GetImage()[y][x];
                }
            }
        }
    }

private:
    Point position;
    Size size;
    shared_ptr<ITexture> texture;
};

class EllipseShape : public IShape {
public:
    EllipseShape() = default;

    EllipseShape(const Point &position, const Size &size,
                 shared_ptr<ITexture> texture)
            : position(position), size(size), texture(std::move(texture)) {}

    [[nodiscard]] unique_ptr<IShape> Clone() const override {
        return make_unique<RectangleShape>(this->position, this->size,
                                           this->texture);
    }

    void SetPosition(Point new_position) override { position = new_position; }

    [[nodiscard]] Point GetPosition() const override { return position; }

    void SetSize(Size new_size) override { size = new_size; }

    [[nodiscard]] Size GetSize() const override { return size; }

    void SetTexture(shared_ptr<ITexture> new_texture) override {
        texture = new_texture;
    }

    [[nodiscard]] ITexture *GetTexture() const override { return texture.get(); }

    void Draw(Image &image) const override {
        Size imageSize = Size({(int) image[0].size(), (int) image.size()});
        Size textureSize = texture ? texture->GetSize() : Size({0, 0});

        for (int x = 0; x < size.width; x++) {
            for (int y = 0; y < size.height; y++) {
                if (!IsPointInEllipse({x, y}, size)) continue;

                Point posInImage = Point({x + position.x, y + position.y});

                if (posInImage.x >= imageSize.width || posInImage.y >= imageSize.height) continue;

                if (x >= textureSize.width || y >= textureSize.height) {
                    image[posInImage.y][posInImage.x] = '.';
                } else {
                    image[posInImage.y][posInImage.x] = texture->GetImage()[y][x];
                }
            }
        }
    }

private:
    Point position;
    Size size;
    shared_ptr<ITexture> texture;
};

// Напишите реализацию функции
unique_ptr<IShape> MakeShape(ShapeType shape_type) {
    switch (shape_type) {
        case ShapeType::Rectangle:
            return make_unique<RectangleShape>();
        case ShapeType::Ellipse:
            return make_unique<EllipseShape>();
    }
}