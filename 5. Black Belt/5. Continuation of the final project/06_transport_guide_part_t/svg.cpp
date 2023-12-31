#include "svg.h"

template<typename cur_prim>
XML::config Svg::Primitive<cur_prim>::MakeXml() const {
    XML::config xml;

    xml.data.emplace_back("fill", (!fill_color.IsNone() ? static_cast<std::string>(fill_color) : "none"));
    xml.data.emplace_back("stroke", (!stroke_color.IsNone() ? static_cast<std::string>(stroke_color) : "none"));
    xml.data.emplace_back("stroke-width", stroke_width);

    if (!type_of_stroke_linecap.empty())
        xml.data.emplace_back("stroke-linecap", type_of_stroke_linecap);
    if (!type_of_stroke_linejoin.empty())
        xml.data.emplace_back("stroke-linejoin", type_of_stroke_linejoin);

    return xml;
}


Svg::Polyline &Svg::Polyline::AddPoint(Svg::Point pnt) &{
    points.emplace_back(pnt);
    return *this;
}

Svg::Polyline &&Svg::Polyline::AddPoint(Svg::Point pnt) &&{
    points.emplace_back(pnt);
    return std::move(*this);
}

XML::xml Svg::Polyline::MakeXml() const {
    auto xml = Primitive<Polyline>::MakeXml();

    xml.open = "polyline";
    xml.close = "/";
    std::stringstream ss_to_str;
    for (auto &point: points) {
        ss_to_str << point.x << "," << point.y << " ";
    }
    xml.data.emplace_back("points", ss_to_str.str());

    return xml;
}

Svg::Circle &Svg::Circle::SetCenter(Svg::Point pnt) &{
    point = pnt;
    return *this;
}

Svg::Circle &Svg::Circle::SetRadius(double rad) &{
    radius = rad;
    return *this;
}

Svg::Circle &&Svg::Circle::SetCenter(Svg::Point pnt) &&{
    point = pnt;
    return std::move(*this);
}

Svg::Circle &&Svg::Circle::SetRadius(double rad) &&{
    radius = rad;
    return std::move(*this);
}

XML::xml Svg::Circle::MakeXml() const {
    auto xml = Primitive<Circle>::MakeXml();

    xml.open = "circle";
    xml.close = "/";
    xml.data.emplace_back("cx", point.x);
    xml.data.emplace_back("cy", point.y);
    xml.data.emplace_back("r", radius);

    return xml;
}

Svg::Rect &Svg::Rect::SetPoint(Svg::Point pnt) &{
    Left_Up_Corner = pnt;
    return *this;
}

Svg::Rect &Svg::Rect::SetHeight(double height) &{
    h = height;
    return *this;
}

Svg::Rect &Svg::Rect::SetWidth(double width) &{
    w = width;
    return *this;
}

Svg::Rect &&Svg::Rect::SetPoint(Svg::Point pnt) &&{
    Left_Up_Corner = pnt;
    return std::move(*this);
}

Svg::Rect &&Svg::Rect::SetHeight(double height) &&{
    h = height;
    return std::move(*this);
}

Svg::Rect &&Svg::Rect::SetWidth(double width) &&{
    w = width;
    return std::move(*this);
}

XML::xml Svg::Rect::MakeXml() const {
    auto xml = Primitive<Rect>::MakeXml();

    xml.open = "rect";
    xml.close = "/";
    xml.data.emplace_back("x", Left_Up_Corner.x);
    xml.data.emplace_back("y", Left_Up_Corner.y);
    xml.data.emplace_back("width", w);
    xml.data.emplace_back("height", h);

    return xml;
}


Svg::Text &Svg::Text::SetPoint(Svg::Point pnt) &{
    point = pnt;
    return *this;
}

Svg::Text &Svg::Text::SetOffset(Svg::Point pnt) &{
    offset = pnt;
    return *this;
}

Svg::Text &Svg::Text::SetFontSize(uint32_t size) &{
    font_size = size;
    return *this;
}

Svg::Text &Svg::Text::SetFontFamily(const std::string &font) &{
    font_family = font;
    return *this;
}

Svg::Text &Svg::Text::SetData(const std::string &data) &{
    text = XML::text(data);
    return *this;
}

Svg::Text &Svg::Text::SetFontWeight(const std::string &f_weight) &{
    font_weight = f_weight;
    return *this;
}

Svg::Text &&Svg::Text::SetPoint(Svg::Point pnt) &&{
    point = pnt;
    return std::move(*this);
}

Svg::Text &&Svg::Text::SetOffset(Svg::Point pnt) &&{
    offset = pnt;
    return std::move(*this);
}

Svg::Text &&Svg::Text::SetFontSize(uint32_t size) &&{
    font_size = size;
    return std::move(*this);
}

Svg::Text &&Svg::Text::SetFontFamily(const std::string &font) &&{
    font_family = font;
    return std::move(*this);
}

Svg::Text &&Svg::Text::SetData(const std::string &data) &&{
    text = XML::text(data);
    return std::move(*this);
}

Svg::Text &&Svg::Text::SetFontWeight(const std::string &f_weight) &&{
    font_weight = f_weight;
    return std::move(*this);
}


XML::xml Svg::Text::MakeXml() const {
    auto xml = Primitive<Text>::MakeXml();
    auto xml_par = XML::config_with_parameters();
    xml_par.data = std::move(xml.data);

    xml_par.open = "text";
    xml_par.close = "/text";
    xml_par.data.emplace_back("x", point.x);
    xml_par.data.emplace_back("y", point.y);
    xml_par.data.emplace_back("dx", offset.x);
    xml_par.data.emplace_back("dy", offset.y);
    xml_par.data.emplace_back("font-size", font_size);
    if (!font_family.empty())
        xml_par.data.emplace_back("font-family", font_family);
    if (!font_weight.empty())
        xml_par.data.emplace_back("font-weight", font_weight);
    xml_par.parameters->emplace<XML::text>(text);

    return xml_par;
}

void Svg::Document::Render(std::ostream &out) {
    XML::config_with_parameters conf;
    conf.open = "svg";
    conf.close = "/svg";
    conf.data.emplace_back("version", 1.1);
    conf.data.emplace_back("xmlns", "http://www.w3.org/2000/svg");

    std::vector<XML::xml> pod_xml;
    auto visitor = [&pod_xml](const auto &prim_node) {
        pod_xml.emplace_back(prim_node.MakeXml());
    };
    for (auto &el: primitives) {
        std::visit(visitor, el);
    }
    conf.parameters->emplace<std::vector<XML::xml>>(pod_xml);
    xml_s.emplace_back(conf);

    XML::Serializer::Serialize(xml_s, out);
}

Svg::Document::Document() {
    XML::config xml_;
    XML::Array arr;
    arr.emplace_back("version", std::to_string(1.0));
    arr.emplace_back("encoding", "UTF-8");
    xml_.open = "?xml";
    xml_.close = "?";
    xml_.data = std::move(arr);
    xml_s.emplace_back(xml_);
}

void Svg::Document::SimpleRender() {
    XML::config_with_parameters conf;
    conf.open = "svg";
    conf.close = "/svg";
    conf.data.emplace_back("version", 1.1);
    conf.data.emplace_back("xmlns", "http://www.w3.org/2000/svg");

    std::vector<XML::xml> pod_xml;
    auto visitor = [&pod_xml](const auto &prim_node) {
        pod_xml.emplace_back(prim_node.MakeXml());
    };
    for (auto &el: primitives) {
        std::visit(visitor, el);
    }
    conf.parameters->emplace<std::vector<XML::xml>>(pod_xml);
    xml_s.emplace_back(conf);
}

XML::xml Svg::Document::Get() const {
    return xml_s;
}

Svg::Document::Document(const Svg::Document &other) {
    primitives = other.primitives;
    xml_s = other.xml_s;
    while (xml_s.size() >= 2)
        xml_s.pop_back();
}

Svg::Document::Document(Svg::Document &&other) {
    primitives = std::move(other.primitives);
    xml_s = std::move(other.xml_s);
    while (xml_s.size() >= 2)
        xml_s.pop_back();
}
