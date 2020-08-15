#include "svg.h"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

namespace Svg {
	void RenderColor(ostream& out, monostate) {
		out << "none";
	}
	void RenderColor(ostream& out, const string& value) {
		out << value;
	}
	void RenderColor(ostream& out, Rgb rgb) {
		out << "rgb("
			<< static_cast<int>(rgb.red) << ','
			<< static_cast<int>(rgb.green) << ','
			<< static_cast<int>(rgb.blue) << ')';
	}
	void RenderColor(ostream& out, Rgba rgba) {
		out << "rgba("
			<< static_cast<int>(rgba.red) << ','
			<< static_cast<int>(rgba.green) << ','
			<< static_cast<int>(rgba.blue) << ','
			<< fixed << setprecision(8)
			<< static_cast<double>(rgba.alpha) << ')';
	}
	void RenderColor(ostream& out, const Color& color) {
		visit([&out](const auto& value) {
			RenderColor(out, value);
		}, color);
	}

	Circle& Circle::SetCenter(Point p) {
		cx = p.x;
		cy = p.y;
		return *this;
	}
	Circle& Circle::SetRadius(double new_r) {
		r = new_r;
		return *this;
	}
	void Circle::Render(ostream& out) const {
		out << "<circle cx=\"" << cx << "\" cy=\"" << cy << "\" r=\"" << r << '\"';
		PathProps::RenderAttrs(out);
		out << " />";
	}

	Polyline& Polyline::AddPoint(Point p) {
		points.push_back(move(p));
		return *this;
	}
	void Polyline::Render(ostream& out) const {
		out << "<polyline points=\"";
		for (const Point& p : points) {
			out << p.x << ',' << p.y << " ";
		}
		out << '\"';
		PathProps::RenderAttrs(out);
		out << " />";
	}

	Text& Text::SetPoint(Point p) {
		x = p.x;
		y = p.y;
		return *this;
	}
	Text& Text::SetOffset(Point p) {
		dx = p.x;
		dy = p.y;
		return *this;
	}
	Text& Text::SetFontSize(uint32_t fz) {
		fontSize = fz;
		return *this;
	}
	Text& Text::SetFontFamily(const string& ff) {
		fontFamily = ff;
		return *this;
	}
	Text& Text::SetData(const string& s) {
		text = s;
		return *this;
	}
	void Text::Render(ostream& out) const {
		out << "<text x=\"" << x << "\" y=\"" << y << "\" dx=\"" << dx << "\" dy=\"" << dy << '\"'
			<< " font-size=\"" << fontSize << '\"';
		if (fontFamily) {
			out << " font-family=\"" << *fontFamily << '\"';
		}
		PathProps::RenderAttrs(out);
		out << " >" << text << "</text>";
	}

	void Document::Render(ostream& out) const {
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
			<< "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">";
		for (const auto& obj : objects) {
			obj->Render(out);
		}
		out << "</svg>";
	}
}