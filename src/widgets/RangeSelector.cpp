#include "RangeSelector.h"
#include "Painter.h"
#include <sstream>
#include "widgets/BitmapFontGL.h"
#include "widgets/Application.h"
#include <iostream>
#include <math.h>
#define WIDTH_OF_ZERO 20
namespace BackyardBrains {

namespace Widgets {
static const int SCROLL_BUTTON_SIZE = 14;
static const int SLIDER_MIN_SIZE = 8;


RangeSelector::RangeSelector(Widget *parent, int heightOfComponent) : Widget(parent)
{
    _minimum = 0;
    _maximum = 0;
    _lowValue = 0;
    _highValue = 0;

    if(heightOfComponent<2*baseHeight)
    {
        heightOfComponent = 2*baseHeight;
    }
    baseHeight = heightOfComponent/2;
    sliderMinSize = heightOfComponent/2;
    setSizePolicy(SizePolicy(SizePolicy::Expanding, SizePolicy::Fixed));

    setSizeHint(Size(sliderMinSize, 2*baseHeight));

}

RangeSelector::~RangeSelector()
{
}


int RangeSelector::minimum() const
{
    return _minimum;
}

void RangeSelector::setMinimum(int minVal)
{
    _minimum = minVal;
    checkMinMaxValues();
}


int RangeSelector::maximum() const
{
    return _maximum;
}


void RangeSelector::setMaximum(int maxVal)
{
    _maximum = maxVal;
    checkMinMaxValues();
}

void RangeSelector::checkMinMaxValues()
{
    if(_minimum> _lowValue)
    {
        _lowValue = _minimum;
    }
    if(_maximum< _lowValue)
    {
        _lowValue = _maximum;
    }
    if(_minimum> _highValue)
    {
        _highValue = _minimum;
    }
    if(_maximum< _highValue)
    {
        _highValue = _maximum;
    }
}

void RangeSelector::setRange(int minVal, int maxVal)
{
    if(minVal>maxVal)
    {
        minVal = maxVal;
    }

    setMinimum(minVal);
    setMaximum(maxVal);
}


int RangeSelector::lowValue() const
{
    return _lowValue;
}

int RangeSelector::highValue() const
{
    return _highValue;
}

int RangeSelector::getLowValue()
{
    return _lowValue;
}

int RangeSelector::getHighValue()
{
     if(_highValue < 1)
    {
        return 0;
    }
    return _highValue;
}

void RangeSelector::initHighAndLow(int newHigh, int newLow)
{
    _lowValue = std::max(_minimum, std::min(newLow, _maximum));
    _highValue = std::max(_minimum, std::min(newHigh, _maximum));
}

void RangeSelector::setLowValue(int val)
{
    const int oldValue = _lowValue;
    _lowValue = std::max(_minimum, std::min(val, _maximum));

    //if we want to move low value over high value push high value
    if(_lowValue>_highValue)
    {
        setHighValue(_lowValue);
    }

    if (_lowValue != oldValue)
        lowValueChanged.emit(_lowValue);
}

void RangeSelector::setHighValue(int val)
{
     const int oldValue = _highValue;
	_highValue = std::max(_minimum, std::min(val, _maximum));

	//if we want to move high value below low value push low value
	if(_lowValue>_highValue)
    {
        setLowValue(_highValue);
    }

	if (_highValue != oldValue)
		highValueChanged.emit(_highValue);
}


void RangeSelector::paintEvent()
{
    if(width() == 0 && height() == 0)
		return;


    //----- draw log10 scale --------------
    double maximumOnLogScale = log10((double)_maximum);
    double withOfZero = WIDTH_OF_ZERO;
    double widthWithoutZero =(double)width() - withOfZero;
    //strechCoeficient converts log number to pixels
    double strechCoeficient = widthWithoutZero/maximumOnLogScale;
    for(int i=0;i<((int)maximumOnLogScale)+1;i++)
    {
        for(int k=1;k<10;k++)
        {
            double frequencyMark = k*pow(10,i);

            int positionInPixels =withOfZero+strechCoeficient*log10(frequencyMark);
            if(positionInPixels>width())
            {
                break;
            }
            if(k==1)
            {
                std::stringstream scaleMarkFreq;

                /*if(k==1 && i==0)
                {
                    scaleMarkFreq <<""<< 0<<"";
                }
                else
                {*/
                    scaleMarkFreq <<""<< frequencyMark<<"";
               // }
                Widgets::Painter::setColor(Widgets::Colors::white);
                Widgets::Application::font()->draw(scaleMarkFreq.str().c_str(),positionInPixels, baseHeight+6, AlignHCenter);
            }
            const Rect markRect = Rect(positionInPixels,baseHeight, 1,4);
            Painter::setColor(Colors::white);
            Painter::drawRect(markRect);
        }
    }

    const Rect gutRect = _GutterRect();
	Painter::setColor(Colors::widgetbg);
	Painter::drawRect(gutRect);

	const Rect gutGreenRect = Rect(_ValueToSliderOffset(_lowValue), 0, _ValueToSliderOffset(_highValue)-_ValueToSliderOffset(_lowValue), baseHeight);
	Painter::setColor(Colors::darkgreen);
	Painter::drawRect(gutGreenRect);

	const Rect lowSliderRect = _LowSliderRect();
	Painter::setColor(Colors::button);
	Painter::drawRect(lowSliderRect);

    const Rect highSliderRect = _HighSliderRect();
	Painter::setColor(Colors::button);
	Painter::drawRect(highSliderRect);
}

void RangeSelector::mousePressEvent(MouseEvent *event)
{
    if (event->button() == LeftButton)
	{
		event->accept();

        if(_HighSliderRect().contains(event->pos()))
        {
            _clickState = CLICKED_HIGH_SLIDER;
        }
        else if(_LowSliderRect().contains(event->pos()))
        {
            _clickState = CLICKED_LOW_SLIDER;
        }

	}
}

void RangeSelector::mouseMotionEvent(MouseEvent *event)
{
if (event->buttons() == LeftButton)
	{
        double position = (double)event->pos().x;
        double withOfZero = WIDTH_OF_ZERO;
        double widthWithoutZero =(double)width() - withOfZero;
        double maxLog = log10(_maximum);
        double logValue = maxLog*((position-WIDTH_OF_ZERO)/(widthWithoutZero));
		switch (_clickState)
		{
			case CLICKED_HIGH_SLIDER:
			{
                setHighValue(pow(10.0, logValue));
			}
			break;
			case CLICKED_LOW_SLIDER:
			{
                setLowValue(pow(10.0, logValue));
			}
			break;
			default:
			break;
		}
	}
}

void RangeSelector::mouseReleaseEvent(MouseEvent *event)
{
    if (event->button() == LeftButton)
	{
		_clickState = CLICKED_NONE;
	}
}

int RangeSelector::_GutterLength() const
{
    return width();
}

int RangeSelector::_SliderLength() const
{
    const int valueInterval = _maximum - _minimum;
	const int gutterLength = _GutterLength();
	return std::max(sliderMinSize, (valueInterval ? (gutterLength/valueInterval) : gutterLength));
}

Rect RangeSelector::_GutterRect() const
{
    return   Rect(0, 0, width(), baseHeight);
}

Rect RangeSelector::_HighSliderRect() const
{
    const int sliderLength = _SliderLength();
	const int sliderOffset = _ValueToSliderOffset(_highValue);

    return Rect(sliderOffset, 0, sliderLength, baseHeight);
}

Rect RangeSelector::_LowSliderRect() const
{
    const int sliderLength = _SliderLength();
	const int sliderOffset = _ValueToSliderOffset(_lowValue);

    return Rect(sliderOffset, 0, sliderLength, baseHeight);
}

int RangeSelector::_ValueToSliderOffset(int val) const
{
    double withOfZero = WIDTH_OF_ZERO;
    double widthWithoutZero =(double)width() - withOfZero- _SliderLength();
    const double relativeVal = val?log10((double)val):0;
	const double valueInterval =log10((double)_maximum);
	 int sliderOffset = valueInterval ? std::max(0, WIDTH_OF_ZERO+(int)(widthWithoutZero*(relativeVal/valueInterval))) : 0;
    if(val<1)
    {
        sliderOffset = 0;

    }

	return sliderOffset;
}



} // namespace Widgets

} // namespace BackyardBrains
