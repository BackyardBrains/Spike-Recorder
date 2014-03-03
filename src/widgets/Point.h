#ifndef BACKYARDBRAINS_WIDGETS_POINT_H
#define BACKYARDBRAINS_WIDGETS_POINT_H

namespace BackyardBrains {

namespace Widgets {

template<typename T>
struct PointBase
{
	PointBase() : x(0), y(0) {}
	PointBase(T xx, T yy) : x(xx), y(yy) {}
	PointBase(const PointBase &other) : x(other.x), y(other.y) {}

	bool operator==(const PointBase &other) const
	{
		return x == other.x && y == other.y;
	}
	bool operator!=(const PointBase &other) const
	{
		return x != other.x || y != other.y;
	}

	PointBase & operator=(const PointBase &other)
	{
		x = other.x;
		y = other.y;
		return *this;
	}
	PointBase & operator-=(const PointBase &other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
	PointBase & operator+=(const PointBase &other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}
	PointBase & operator/=(T scalar)
	{
		x /= scalar;
		y /= scalar;
		return *this;
	}
	PointBase & operator*=(T scalar)
	{
		x *= scalar;
		y *= scalar;
		return *this;
	}
	PointBase operator-(const PointBase &other) const
	{
		PointBase difference = *this;
		difference -= other;
		return difference;
	}
	PointBase operator+(const PointBase &other) const
	{
		PointBase sum = *this;
		sum += other;
		return sum;
	}
	PointBase operator/(T scalar) const
	{
		PointBase scaled = *this;
		scaled /= scalar;
		return scaled;
	}
	PointBase operator*(T scalar) const
	{
		PointBase scaled = *this;
		scaled *= scalar;
		return scaled;
	}
	PointBase operator-() const
	{
		return PointBase(-x, -y);
	}

	T x;
	T y;
};

typedef PointBase<int> Point;
typedef PointBase<float> PointF;

} // namespace Widgets

} // namespace BackyardBrains

#endif
