#ifndef BACKYARDBRAINS_WIDGETS_FLAGS_H
#define BACKYARDBRAINS_WIDGETS_FLAGS_H

#ifndef DECLARE_FLAGS
#define DECLARE_FLAGS(y, x) \
typedef BackyardBrains::Widgets::Flags<x> y;
#endif

#ifndef DECLARE_OPERATORS_FOR_FLAGS
#define DECLARE_OPERATORS_FOR_FLAGS(x)
// static x operator|(x::enum_type a, x::enum_type b) {return BackyardBrains::Widgets::Flags<x::enum_type>(a) | b;}
// static x operator|(x::enum_type a, BackyardBrains::Widgets::Flags<x::enum_type> b) {return b | a;}
#endif

//static x operator~(x::enum_type val) {return ~BackyardBrains::Widgets::Flags<x::enum_type>(val);}

namespace BackyardBrains {

namespace Widgets {

template <typename T>
class Flags
{
public:
	typedef T enum_type;
	Flags() : _value(0)
	{
	}
	Flags(T flag) : _value(flag)
	{
	}
	Flags(int flags) : _value(flags)
	{
	}
	Flags(const Flags &other) : _value(other._value)
	{
	}
	Flags & operator=(const Flags &other)
	{
		_value = other._value;
		return *this;
	}
	Flags & operator=(T value)
	{
		_value = value;
		return *this;
	}
	Flags & operator|=(const Flags &other) {_value |= other._value; return *this;}
	Flags & operator&=(const Flags &other) {_value &= other._value; return *this;}
	Flags & operator^=(const Flags &other) {_value ^= other._value; return *this;}
	Flags & operator|=(T mask) {_value |= mask; return *this;}
	Flags & operator&=(T mask) {_value &= mask; return *this;}
	Flags & operator^=(T mask) {_value ^= mask; return *this;}

	Flags operator|(const Flags &other) const {return Flags(static_cast<T>(_value | other._value));}
	Flags operator&(const Flags &other) const {return Flags(static_cast<T>(_value & other._value));}
	Flags operator^(const Flags &other) const {return Flags(static_cast<T>(_value ^ other._value));}
	Flags operator|(T val) const {return Flags(static_cast<T>(_value | val));}
	Flags operator&(T val) const {return Flags(static_cast<T>(_value & val));}
	Flags operator^(T val) const {return Flags(static_cast<T>(_value ^ val));}
	Flags operator~() const {return Flags(static_cast<T>(~_value));}
	operator int() const {return _value;}
	bool operator!() const {return !_value;}
	bool testFlag(T f) const {return (_value & f) == Int(f) && (f != 0 || _value == f);}
	void setFlag(T f) {_value |= f;}
	void unsetFlag(T f) {_value &= ~f;}
protected:
	int _value;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
