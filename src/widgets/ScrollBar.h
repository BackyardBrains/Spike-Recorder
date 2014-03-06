#ifndef BACKYARDBRAINS_WIDGETS_SCROLLBAR_H
#define BACKYARDBRAINS_WIDGETS_SCROLLBAR_H

#include "Widget.h"
#include "global.h"

#include <string>

namespace BackyardBrains {

namespace Widgets {

class ScrollBar : public Widget
{
public:
	ScrollBar(Orientation orientation = Horizontal, Widget *parent = NULL);
	~ScrollBar();

	int minimum() const;
	void setMinimum(int minVal);
	int maximum() const;
	void setMaximum(int maxVal);
	void setRange(int minVal, int maxVal);
	int value() const;
	void setValue(int val);
	int pageStep() const;
	void setPageStep(int val);

	void updateValue(int val); // don't emit a signal
	sigslot::signal1<int> valueChanged;
	// sigslot::signal1<int> rangeChanged;

private:
	void paintEvent();
	void mousePressEvent(MouseEvent *event);
	void mouseMotionEvent(MouseEvent *event);
	void mouseReleaseEvent(MouseEvent *event);

	enum ClickArea
	{
		CLICKED_NONE,
		CLICKED_BEFORE_SLIDER,
		CLICKED_AFTER_SLIDER,
		CLICKED_SLIDER
	};

	int _GutterLength() const;
	int _SliderLength() const;
	Rect _GutterRect() const;
	Rect _SliderRect() const;
	ClickArea _DetermineArea(const Point &p) const;
	int _ValueToSliderOffset(int val) const;

	Orientation _orientation;
	int _minimum;
	int _maximum;
	int _value;
	int _pageStep;
	ClickArea _clickState;
	int _sliderClickedPixelOffset;
	int _draggingSliderOffset;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
