#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <optional>
#include <variant>

namespace Svg {
	struct Point {
		double x = 0.0, y = 0.0;
	};

	struct Rgb {
		uint8_t red = 0, green = 0, blue = 0;
	};
	struct Rgba : public Rgb {
		double alpha = 0;
	};

	using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
	const Color NoneColor{};

	void RenderColor(std::ostream& out, std::monostate);
	void RenderColor(std::ostream& out, const std::string&);
	void RenderColor(std::ostream& out, Rgb);
	void RenderColor(std::ostream& out, Rgba);
	void RenderColor(std::ostream& out, const Color&);

	class Object {
	public:
		virtual void Render(std::ostream& out) const = 0;
		virtual ~Object() = default;
	};
	using ObjectPtr = std::unique_ptr<Object>;

	template <typename Owner>
	class PathProps {
	public:
		Owner& SetFillColor(const Color& color) {
			fill = color;
			return AsOwner();
		}
		Owner& SetStrokeColor(const Color& color) {
			stroke = color;
			return AsOwner();
		}
		Owner& SetStrokeWidth(double width) {
			strokeWidth = width;
			return AsOwner();
		}
		Owner& SetStrokeLineCap(const std::string& lineCap) {
			strokeLineCap = lineCap;
			return AsOwner();
		}
		Owner& SetStrokeLineJoin(const std::string& lineJoin) {
			strokeLineJoin = lineJoin;
			return AsOwner();
		}
		void RenderAttrs(std::ostream& out) const {
			out << " fill=\"";
			RenderColor(out, fill);
			out << "\" stroke=\"";
			RenderColor(out, stroke);
			out << "\" stroke-width=\"" << strokeWidth << '\"';
			if (strokeLineCap) {
				out << " stroke-linecap=\"" << *strokeLineCap << '\"';
			}
			if (strokeLineJoin) {
				out << " stroke-linejoin=\"" << *strokeLineJoin << '\"';
			}
		}

	private:
		Color fill = NoneColor;
		Color stroke = NoneColor;
		double strokeWidth = 1.0;
		std::optional<std::string> strokeLineCap;
		std::optional<std::string> strokeLineJoin;

		Owner& AsOwner() {
			return static_cast<Owner&>(*this);
		}
	};

	class Circle : public Object, public PathProps<Circle> {
	public:
		Circle& SetCenter(Point);
		Circle& SetRadius(double);
		void Render(std::ostream&) const override;

	private:
		double cx = 0.0;
		double cy = 0.0;
		double r = 1.0;
	};

	class Polyline : public Object, public PathProps<Polyline> {
	public:
		Polyline& AddPoint(Point);
		void Render(std::ostream&) const override;
	private:
		std::vector<Point> points;
	};

	class Text : public Object, public PathProps<Text> {
	public:
		Text& SetPoint(Point);
		Text& SetOffset(Point);
		Text& SetFontSize(uint32_t);
		Text& SetFontFamily(const std::string&);
		Text& SetData(const std::string&);
		void Render(std::ostream&) const override;
	private:
		double x = 0.0;
		double y = 0.0;
		double dx = 0.0;
		double dy = 0.0;
		uint32_t fontSize = 1;
		std::optional<std::string> fontFamily;
		std::string text;
	};

	class Document : public Object {
	public:
		template <typename ObjectType>
		void Add(ObjectType obj) {
			objects.push_back(make_unique<ObjectType>(std::move(obj)));
		}

		void Render(std::ostream& out) const override;
	private:
		std::vector<ObjectPtr> objects;
	};
}