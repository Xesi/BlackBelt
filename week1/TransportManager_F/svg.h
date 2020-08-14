
#include <vector>
#include <string>
#include <iostream>
#include <variant>
#include <optional>
#include <memory>

namespace Svg {

	struct Rgb {
		uint8_t red = 0, green = 0, blue = 0;
		Rgb() = default;
		Rgb(uint8_t red, uint8_t green, uint8_t blue) :
			red(red), green(green), blue(blue) {}
	};


	struct Point {
		double x = 0.0, y = 0.0;
		Point() = default;
		Point(double x, double y) : x(x), y(y) {};
	};


	using Color = std::variant<std::monostate, std::string, Rgb>;
	const Color NoneColor{};

	void Render(std::ostream& out, std::monostate) {
		out << "none";
	}
	void Render(std::ostream& out, const std::string& value) {
		out << value;
	}
	void Render(std::ostream& out, Rgb rgb) {
		out << "rgb("
			<< static_cast<int>(rgb.red) << ','
			<< static_cast<int>(rgb.green) << ','
			<< static_cast<int>(rgb.blue) << ')';
	}
	void RenderColor(std::ostream& out, const Color& color) {
		visit([&out](const auto& value) {
			Render(out, value);
		}, color);
	}

	class Object {
	public:
		virtual void Render(std::ostream& out) const = 0;
		virtual ~Object() = default;
	};


	template<typename Child>
	class Figure {
	public:
		template<typename T>
		Child& SetFillColor(const T& color) {
			fillColor = Color(color);
			return AsOwner();
		}
		template<typename T>
		Child& SetStrokeColor(const T& color) {
			strokeColor = Color(color);
			return AsOwner();
		}
		Child& SetStrokeWidth(double width) {
			strokeWidth = width;
			return AsOwner();
		}
		Child& SetStrokeLineCap(const std::string& line) {
			lineCap = line;
			return AsOwner();
		}
		Child& SetStrokeLineJoin(const std::string& line) {
			lineJoin = line;
			return AsOwner();
		}
		void RenderAttrs(std::ostream& out) const {
			out << " fill=\"";
			RenderColor(out, fillColor);
			out << "\" stroke=\"";
			RenderColor(out, strokeColor);
			out << "\" stroke-width=\"" << strokeWidth << '\"';
			if (lineCap) {
				out << " stroke-linecap=\"" << *lineCap << '\"';
			}
			if (lineJoin) {
				out << " stroke-linejoin=\"" << *lineJoin << '\"';
			}
		}

	private:
		Color fillColor{ NoneColor };
		Color strokeColor{ NoneColor };
		double strokeWidth{ 1.0 };
		std::optional<std::string> lineCap{ std::nullopt };
		std::optional<std::string> lineJoin{ std::nullopt };
		Child& AsOwner() {
			return static_cast<Child&>(*this);
		}
	};


	class Circle : public Object, public Figure<Circle> {
	public:
		Circle() : cx(0.0), cy(0.0), r(0.0) {};
		Circle& SetCenter(Point pt) {
			cx = pt.x;
			cy = pt.y;
			return *this;
		}
		Circle& SetRadius(double radius) {
			r = radius;
			return *this;
		}
		void Render(std::ostream& out) const override {
			out << "<circle cx=\"" << cx << "\" cy=\""
				<< cy << "\" r=\"" << r << '\"';
			RenderAttrs(out);
			out << "/>";
		}
	private:
		double cx = 0, cy = 0, r = 0;
	};


	class Polyline : public Object, public Figure<Polyline> {
	public:
		Polyline() = default;
		Polyline& AddPoint(Point pt) {
			points.push_back(pt);
			return *this;
		}
		void Render(std::ostream& out) const override {
			out << "<polyline points=\"";
			for (const Point& p : points) {
				out << p.x << ',' << p.y << " ";
			}
			out << '\"';
			RenderAttrs(out);
			out << "/>";
		}
	private:
		std::vector<Point> points;
	};


	class Text : public Object, public Figure<Text> {
	public:
		Text() = default;
		Text& SetPoint(Point pt) {
			x = pt.x;
			y = pt.y;
			return *this;
		}
		Text& SetOffset(Point pt) {
			dx = pt.x;
			dy = pt.y;
			return *this;
		}
		Text& SetFontSize(uint32_t value) {
			fontSize = value;
			return *this;
		}
		Text& SetFontFamily(const std::string& str) {
			fontFamily = str;
			return *this;
		}
		Text& SetData(const std::string& str) {
			data = str;
			return *this;
		}
		void Render(std::ostream& out) const override {
			out << "<text x=\"" << x << "\" y=\"" << y
				<< "\" dx=\"" << dx << "\" dy=\"" << dy << '\"'
				<< " font-size=\"" << fontSize << '\"';
			if (fontFamily) {
				out << " font-family=\"" << *fontFamily << '\"';
			}
			RenderAttrs(out);
			out << ">" << data;
			out << "</text>";
		}
	private:
		double x = 0.0, y = 0.0;
		double dx = 0.0, dy = 0.0;
		uint32_t fontSize = 1;
		std::optional<std::string> fontFamily{ std::nullopt };
		std::string data;
	};


	class Document {
	public:
		Document() = default;

		template <typename ObjectType>
		void Add(ObjectType obj) {
			figures.push_back(std::make_unique<ObjectType>(std::move(obj)));
		}

		void Render(std::ostream& out) {
			out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
				<< "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">";
			for (const auto& figure : figures) {
				figure->Render(out);
			}
			out << "</svg>";
		}
	private:
		std::vector<std::unique_ptr<Object>> figures;
	};


}
