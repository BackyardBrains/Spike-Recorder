#include "ScrollBar.h"
#include "Painter.h"

#include <iostream>
#include <cassert>

namespace BackyardBrains {

namespace Widgets {

static const int SCROLL_BUTTON_SIZE = 16;
static const int SLIDER_MIN_SIZE = 8;

ScrollBar::ScrollBar(Orientation orientation, Widget *parent) : Widget(parent), _orientation(orientation), _minimum(0), _maximum(0), _value(0), _singleStep(1), _pageStep(1), _clickState(CLICKED_NONE), _sliderClickedPixelOffset(0), _draggingSliderOffset(0)
{
	setSizePolicy((_orientation == Horizontal) ? SizePolicy(SizePolicy::Expanding, SizePolicy::Fixed) : SizePolicy(SizePolicy::Fixed, SizePolicy::Expanding));
	if (_orientation == Vertical)
		setSizeHint(Size(SCROLL_BUTTON_SIZE, SCROLL_BUTTON_SIZE*2));
	else
		setSizeHint(Size(SCROLL_BUTTON_SIZE*2, SCROLL_BUTTON_SIZE));
}

ScrollBar::~ScrollBar()
{
}

int ScrollBar::minimum() const
{
	return _minimum;
}

void ScrollBar::setMinimum(int minVal)
{
	_minimum = minVal;
}

int ScrollBar::maximum() const
{
	return _maximum;
}

void ScrollBar::setMaximum(int maxVal)
{
	_maximum = maxVal;
}

void ScrollBar::setRange(int minVal, int maxVal)
{
	_minimum = minVal;
	_maximum = maxVal;
}

int ScrollBar::value() const
{
	return _value;
}

void ScrollBar::setValue(int val)
{
	const int oldValue = _value;
	_value = std::max(_minimum, std::min(val, _maximum));
	if (_value != oldValue)
		valueChanged.emit(val);
}

void ScrollBar::updateValue(int val) {
	_value = std::max(_minimum, std::min(val, _maximum));
}

int ScrollBar::singleStep() const
{
	return _singleStep;
}

void ScrollBar::setSingleStep(int val)
{
	_singleStep = val;
}

int ScrollBar::pageStep() const
{
	return _pageStep;
}

void ScrollBar::setPageStep(int val)
{
	_pageStep = val;
}

void ScrollBar::paintEvent()
{
	Painter::setColor(Colors::gray);
	// draw the buttons
// 	Painter::drawRect(_LessButtonRect());
// 	Painter::drawRect(_MoreButtonRect());
	// draw the gutter
	Painter::setColor(Color(80,80,80));
	Painter::drawRect(_GutterRect());
	// draw the slider
	const Rect sliderRect = _SliderRect();
	Painter::setColor(Colors::gray);
	Painter::drawRect(sliderRect);
}

void ScrollBar::mousePressEvent(MouseEvent *event)
{
	if (event->button() == LeftButton)
	{
		event->accept();

		_clickState = _DetermineArea(event->pos());
		switch (_clickState)
		{
			case CLICKED_LESS_BUTTON:
			setValue(value()-singleStep());
			break;

			case CLICKED_MORE_BUTTER:
			setValue(value()+singleStep());
			break;

			case CLICKED_BEFORE_SLIDER:
			case CLICKED_AFTER_SLIDER:
			_clickState = CLICKED_SLIDER;
			_sliderClickedPixelOffset = (_orientation == Horizontal) ? (_SliderRect().width()/2) : _SliderRect().height()/2;
			_draggingSliderOffset = 0;
			mouseMotionEvent(event);
			break;
			case CLICKED_SLIDER:
			_sliderClickedPixelOffset = (_orientation == Horizontal) ? (event->pos().x - _SliderRect().x) : (event->pos().y - _SliderRect().y);
			_draggingSliderOffset = _ValueToSliderOffset(_value);
			break;

			default:
			break;
		}
	}
	else
		Widget::mousePressEvent(event);
}

void ScrollBar::mouseMotionEvent(MouseEvent *event)
{
	if (event->buttons() == LeftButton)
	{
		switch (_clickState)
		{
			case CLICKED_SLIDER:
			{
				const int newSliderOffset = (_orientation == Horizontal) ? (event->pos().x - _sliderClickedPixelOffset - _GutterRect().x) : (event->pos().y - _sliderClickedPixelOffset - _GutterRect().y);
				const int scrollRange = std::max(0, _GutterLength() - _SliderLength()+1); // TODO is the +1 right?
				_draggingSliderOffset = std::max(0, std::min(scrollRange-1, newSliderOffset)); // TODO is the -1 right?

				const int valueInterval = _maximum - _minimum;
				const int calculatedRelativeValAfter = scrollRange ? ((valueInterval*_draggingSliderOffset+scrollRange/2)/scrollRange) : 0;

				setValue(_minimum + calculatedRelativeValAfter);


			}
			break;
			default:
			break;
		}
	}
}

void ScrollBar::mouseReleaseEvent(MouseEvent *event)
{
	if (event->button() == LeftButton)
	{
		_clickState = CLICKED_NONE;
	}
}

int ScrollBar::_GutterLength() const
{
	return std::max(0, ((_orientation == Horizontal) ? width() : height()) - SCROLL_BUTTON_SIZE*2);
}

int ScrollBar::_SliderLength() const
{
	const int valueInterval = _maximum - _minimum;
	const int gutterLength = _GutterLength();
	return std::max(SLIDER_MIN_SIZE, (valueInterval ? (gutterLength*std::min(_pageStep, valueInterval)/valueInterval) : gutterLength));
}

Rect ScrollBar::_GutterRect() const
{
	if (_orientation == Horizontal)
		return Rect(SCROLL_BUTTON_SIZE, 0, _GutterLength(), height());
	else // if (_orientation == Vertical)
		return Rect(0, SCROLL_BUTTON_SIZE, width(), _GutterLength());
}

Rect ScrollBar::_SliderRect() const
{
	const int sliderLength = _SliderLength();
	const int sliderOffset = _ValueToSliderOffset(_value);
	// const int sliderOffset = _ValueToSliderOffset(_value);
	if (_orientation == Horizontal)
		return Rect(SCROLL_BUTTON_SIZE + sliderOffset, 0, sliderLength, height());
	else // if (_orientation == Vertical)
		return Rect(0, SCROLL_BUTTON_SIZE + sliderOffset, width(), sliderLength);
}

Rect ScrollBar::_LessButtonRect() const
{
	return Rect(0, 0, SCROLL_BUTTON_SIZE, SCROLL_BUTTON_SIZE);
}

Rect ScrollBar::_MoreButtonRect() const
{
	if (_orientation == Horizontal)
		return Rect(width()-SCROLL_BUTTON_SIZE, 0, SCROLL_BUTTON_SIZE, SCROLL_BUTTON_SIZE);
	else // if (_orientation == Vertical)
		return Rect(0, height()-SCROLL_BUTTON_SIZE, SCROLL_BUTTON_SIZE, SCROLL_BUTTON_SIZE);
}

ScrollBar::ClickArea ScrollBar::_DetermineArea(const Point &p) const
{
	if (_LessButtonRect().contains(p))
		return CLICKED_LESS_BUTTON;
	else if (_MoreButtonRect().contains(p))
		return CLICKED_MORE_BUTTER;
	else if (_SliderRect().contains(p))
		return CLICKED_SLIDER;
	else if (_GutterRect().contains(p))
	{
		if ((_orientation == Horizontal) ? (p.x < _SliderRect().x) : (p.y < _SliderRect().y))
			return CLICKED_BEFORE_SLIDER;
		else
			return CLICKED_AFTER_SLIDER;
	}
	return CLICKED_NONE;
}

int ScrollBar::_ValueToSliderOffset(int val) const
{
	const int relativeVal = val - _minimum;
	const int valueInterval = _maximum - _minimum;
	const int sliderOffset = (valueInterval ? (std::max(0, _GutterLength() - _SliderLength())*relativeVal/valueInterval) : 0);
	return sliderOffset;
}

} // namespace Widgets

} // namespace BackyardBrains
