#pragma once

#include "types.h"
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace Svg {

    class Object {
    public:
        virtual ~Object() = default;

        virtual void Render(std::ostream &out) const = 0;
    };

    using ObjectPtr = std::unique_ptr<Object>;

    template<typename Owner>
    class PathProps {
    public:
        Owner &SetFillColor(const Color &color) {
            fillColor = color;
            return AsOwner();
        }

        Owner &SetStrokeColor(const Color &color) {
            strokeColor = color;
            return AsOwner();
        }

        Owner &SetStrokeWidth(double width) {
            strokeWidth = width;
            return AsOwner();
        }

        Owner &SetStrokeLineCap(const std::string &lineCap) {
            strokeLineCap = lineCap;
            return AsOwner();
        }

        Owner &SetStrokeLineJoin(const std::string &lineJoin) {
            strokeLineJoin = lineJoin;
            return AsOwner();
        }

        virtual void Render(std::ostream &out) const {
            out << "fill=\\\"" << GetString(fillColor) << "\\\" "
                << "stroke=\\\"" << GetString(strokeColor) << "\\\" "
                << "stroke-width=\\\"" << std::fixed << std::setprecision(6)
                << strokeWidth << "\\\"";
            if (strokeLineCap.has_value()) {
                out << " stroke-linecap=\\\"" << strokeLineCap.value() << "\\\"";
            }
            if (strokeLineJoin.has_value()) {
                out << " stroke-linejoin=\\\"" << strokeLineJoin.value() << "\\\"";
            }
        }

    private:
        Color fillColor = NoneColor;
        Color strokeColor = NoneColor;
        double strokeWidth = 1.0;
        std::optional<std::string> strokeLineCap;
        std::optional<std::string> strokeLineJoin;

        Owner &AsOwner() { return static_cast<Owner &>(*this); }
    };

    class Circle : public Object, public PathProps<Circle> {
    public:
        Circle &SetCenter(Point newCenter) {
            center = newCenter;
            return *this;
        }

        Circle &SetRadius(double newRadius) {
            radius = newRadius;
            return *this;
        }

        void Render(std::ostream &out) const override {
            out << "<circle "
                << "cx=\\\"" << center.x << R"(\" cy=\")" << center.y << R"(\" r=\")"
                << std::fixed << std::setprecision(6) << radius << "\\\" ";
            PathProps::Render(out);
            out << " />";
        }

    private:
        Point center = Point();
        double radius = 1.0;
    };

    class Polyline : public Object, public PathProps<Polyline> {
    public:
        Polyline &AddPoint(Point point) {
            points.push_back(point);
            return *this;
        }

        void Render(std::ostream &out) const override {
            out << "<polyline "
                << "points=\\\"";
            for (const auto &p: points) {
                out << std::fixed << std::setprecision(6) << p.x << "," << p.y << " ";
            }
            out << "\\\" ";
            PathProps::Render(out);
            out << " />";
        }

    private:
        std::vector<Point> points;
    };

    class Text : public Object, public PathProps<Text> {
    public:
        Text &SetPoint(Point point) {
            textPoint = point;
            return *this;
        }

        Text &SetOffset(Point point) {
            offsetPoint = point;
            return *this;
        }

        Text &SetFontSize(uint32_t size) {
            fontSize = size;
            return *this;
        }

        Text &SetFontFamily(const std::string &family) {
            fontFamily = family;
            return *this;
        }

        Text &SetFontWeight(const std::string &weight) {
            fontWeight = weight;
            return *this;
        }

        Text &SetData(const std::string &text) {
            data = text;
            return *this;
        }

        void Render(std::ostream &out) const override {
            out << "<text "
                << "x=\\\"" << textPoint.x << R"(\" y=\")" << textPoint.y
                << R"(\" dx=\")" << offsetPoint.x << R"(\" dy=\")" << offsetPoint.y
                << R"(\" font-size=\")" << fontSize << "\\\" ";
            if (fontFamily.has_value()) {
                out << "font-family=\\\"" << fontFamily.value() << "\\\" ";
            }
            if (fontWeight.has_value()) {
                out << "font-weight=\\\"" << fontWeight.value() << "\\\" ";
            }
            PathProps::Render(out);
            out << " >" << data << "</text>";
        }

    private:
        Point textPoint;
        Point offsetPoint;
        uint32_t fontSize = 1;
        std::optional<std::string> fontFamily;
        std::optional<std::string> fontWeight;
        std::string data;
    };

    class Document : public Object {
    public:
        void Render(std::ostream &out) const override {
            out << R"("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>)"
                << R"(<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">)";

            for (const auto &object: objects) {
                object->Render(out);
            }

            out << R"(</svg>)"
                << "\"";
        }

        void Add(Circle object) {
            objects.push_back(std::make_unique<Circle>(std::move(object)));
        }

        void Add(Polyline object) {
            objects.push_back(std::make_unique<Polyline>(std::move(object)));
        }

        void Add(Text object) {
            objects.push_back(std::make_unique<Text>(std::move(object)));
        }

    private:
        std::vector<ObjectPtr> objects;
    };
} // namespace Svg

// int main() {
//   Svg::Document svg;
//
//   svg.Add(Svg::Polyline{}
//               .SetStrokeColor(Svg::Rgb{140, 198, 63}) // soft green
//               .SetStrokeWidth(16)
//               .SetStrokeLineCap("round")
//               .AddPoint({50, 50})
//               .AddPoint({250, 250}));
//
//   for (const auto point : {Svg::Point{50, 50}, Svg::Point{250, 250}}) {
//     svg.Add(Svg::Circle{}.SetFillColor("white").SetRadius(6).SetCenter(point));
//   }
//
//   svg.Add(Svg::Text{}
//               .SetPoint({50, 50})
//               .SetOffset({10, -10})
//               .SetFontSize(20)
//               .SetFontFamily("Verdana")
//               .SetFillColor("black")
//               .SetData("C"));
//   svg.Add(Svg::Text{}
//               .SetPoint({250, 250})
//               .SetOffset({10, -10})
//               .SetFontSize(20)
//               .SetFontFamily("Verdana")
//               .SetFillColor("black")
//               .SetData("C++"));
//
//   svg.Render(std::cout);
// }
