#include "RangeSelector.h"
#include "Painter.h"
#include <sstream>
#include "widgets/BitmapFontGL.h"
#include "widgets/Application.h"
#include <iostream>

namespace BackyardBrains {

namespace Widgets {
static const int SCROLL_BUTTON_SIZE = 14;
static const int SLIDER_MIN_SIZE = 8;
static const int Y_OFFSET = 20;

RangeSelector::RangeSelector(Widget *parent) : Widget(parent)
{
    _minimum = 0;
    _maximum = 0;
    _lowValue = 0;
    _highValue = 0;

    setSizePolicy(SizePolicy(SizePolicy::Expanding, SizePolicy::Fixed));

    setSizeHint(Size(SLIDER_MIN_SIZE, 4*Y_OFFSET));
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

void RangeSelector::setLowValue(int val)
{
    const int oldValue = _lowValue;
	_lowValue = std::max(_minimum, std::min(val, _maximum));

	//if we want to move low value over high value push high value
	if(_lowValue>_highValue)
    {
        setHighValue(_lowValue);
    }

    int lowLogFreqValue = (int)pow(10,(((double)lowValue())/((double)_maximum)*log10((double)_maximum)));
	if(_lowValue==0)
    {
        lowLogFreqValue = 0;
    }

	if (_lowValue != oldValue)
		lowValueChanged.emit(lowLogFreqValue);
}

int RangeSelector::highValue() const
{
    return _highValue;
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


    int highLogFreqValue = (int)pow(10,(((double)highValue())/((double)_maximum)*log10((double)_maximum)));
	if(_highValue==0)
    {
        highLogFreqValue = 0;
    }

	if (_highValue != oldValue)
		highValueChanged.emit(highLogFreqValue);
}

void RangeSelector::updateLowLogValue(int val) // don't emit a signal
{
    val = _maximum*(log10((double)val)/log10((double)_maximum)) +1;
    _lowValue = std::max(_minimum, std::min(val, _maximum));

	//if we want to move low value over high value push high value
	if(_lowValue>_highValue)
    {
        setHighValue(_lowValue);
    }
}

void RangeSelector::updateHighLogValue(int val) // don't emit a signal
{

    val = _maximum*(log10((double)val)/log10((double)_maximum)) +1;

    _highValue = std::max(_minimum, std::min(val, _maximum));

	//if we want to move high value below low value push low value
	if(_lowValue>_highValue)
    {
        setLowValue(_highValue);
    }
}

void RangeSelector::paintEvent()
{
    if(width() == 0 && height() == 0)
		return;

    std::stringstream titles;
	 titles <<"Set band-pass filter cutoff frequencies";
    Widgets::Painter::setColor(Widgets::Colors::white);
    Widgets::Application::font()->draw(titles.str().c_str(),width()/2, 0, AlignHCenter);

	std::stringstream s;
	int lowLogFreqValue = (int)pow(10,(((double)lowValue())/((double)_maximum)*log10((double)_maximum)));
	if(_lowValue==0)
    {
        lowLogFreqValue = 0;
    }
	 s <<"Low: "<< lowLogFreqValue<<"Hz";
    //Widgets::Painter::setColor(bg);
    //drawtextbgbox(s.str(), 35, y-20, Widgets::AlignLeft);
    Widgets::Painter::setColor(Widgets::Colors::white);
    Widgets::Application::font()->draw(s.str().c_str(),width()/2-100, Y_OFFSET, AlignHCenter);

    std::stringstream s1;
    int highLogFreqValue = (int)pow(10,(((double)highValue())/((double)_maximum)*log10((double)_maximum)));
	if(_highValue==0)
    {
        highLogFreqValue = 0;
    }
    s1 <<" High: "<<highLogFreqValue<<"Hz";
    //Widgets::Painter::setColor(bg);
    // drawtextbgbox(s.str(), 35, y-20, Widgets::AlignLeft);
    Widgets::Painter::setColor(Widgets::Colors::white);
    Widgets::Application::font()->draw(s1.str().c_str(),width()/2+100, Y_OFFSET, AlignHCenter);

    //----- draw log10 scale --------------
    double maximumOnLogScale = log10((double)_maximum);
    for(int i=0;i<((int)maximumOnLogScale)+1;i++)
    {
        for(int k=1;k<10;k++)
        {
            double exponent = 10^i;
            double frequencyMark = k*pow(10,i);

            int positionInPixels = width()*(log10(frequencyMark)/maximumOnLogScale);
            if(positionInPixels>width())
            {
                break;
            }
            if(k==1)
            {
                std::stringstream scaleMarkFreq;
                //scaleMarkFreq <<""<< frequencyMark<<"";
                if(k==1 && i==0)
                {
                    scaleMarkFreq <<""<< 0<<"";
                }
                else
                {
                    scaleMarkFreq <<""<< frequencyMark<<"";
                }
                Widgets::Painter::setColor(Widgets::Colors::white);
                Widgets::Application::font()->draw(scaleMarkFreq.str().c_str(),positionInPixels, 3*Y_OFFSET+6, AlignHCenter);
            }
            const Rect markRect = Rect(positionInPixels,3*Y_OFFSET, 1,4);
            Painter::setColor(Colors::white);
            Painter::drawRect(markRect);
        }
    }

    const Rect gutRect = _GutterRect();
	Painter::setColor(Colors::widgetbg);
	Painter::drawRect(gutRect);

	const Rect gutGreenRect = Rect(_ValueToSliderOffset(_lowValue), 2*Y_OFFSET, _ValueToSliderOffset(_highValue)-_ValueToSliderOffset(_lowValue), Y_OFFSET);
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
            _highSliderClickedPixelOffset = event->pos().x - _HighSliderRect().x;
            _highDraggingSliderOffset = _ValueToSliderOffset(_highValue);
        }
        else if(_LowSliderRect().contains(event->pos()))
        {
            _clickState = CLICKED_LOW_SLIDER;
            _lowSliderClickedPixelOffset = event->pos().x - _LowSliderRect().x;
            _lowDraggingSliderOffset = _ValueToSliderOffset(_lowValue);
        }

	}
}

void RangeSelector::mouseMotionEvent(MouseEvent *event)
{
if (event->buttons() == LeftButton)
	{
		switch (_clickState)
		{
			case CLICKED_HIGH_SLIDER:
			{
			    //std::cout<<"Coef: "<<(1-(abs(event->pos().y-2*Y_OFFSET)/400.0))<<"\n";
				const int newSliderOffset = (event->pos().x - _highSliderClickedPixelOffset);
				const int scrollRange = std::max(0, _GutterLength() - _SliderLength()); // TODO is the +1 right?
				_highDraggingSliderOffset = std::max(0, std::min(scrollRange, newSliderOffset)); // TODO is the -1 right?

				const int valueInterval = _maximum - _minimum;
				const int calculatedRelativeValAfter = scrollRange ? ((valueInterval*_highDraggingSliderOffset+scrollRange/2)/scrollRange) : 0;

				setHighValue(_minimum + calculatedRelativeValAfter);
			}
			break;
			case CLICKED_LOW_SLIDER:
			{
                const int newSliderOffset = event->pos().x - _lowSliderClickedPixelOffset;
				const int scrollRange = std::max(0, _GutterLength() - _SliderLength()); // TODO is the +1 right?
				_lowDraggingSliderOffset = std::max(0, std::min(scrollRange, newSliderOffset)); // TODO is the -1 right?

				const int valueInterval = _maximum - _minimum;
				const int calculatedRelativeValAfter = scrollRange ? ((valueInterval*_lowDraggingSliderOffset+scrollRange/2)/scrollRange) : 0;

				setLowValue(_minimum + calculatedRelativeValAfter);
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
	return std::max(SLIDER_MIN_SIZE, (valueInterval ? (gutterLength/valueInterval) : gutterLength));
}

Rect RangeSelector::_GutterRect() const
{
    return   Rect(0, 2*Y_OFFSET, width(), Y_OFFSET);
}

Rect RangeSelector::_HighSliderRect() const
{
    const int sliderLength = _SliderLength();
	const int sliderOffset = _ValueToSliderOffset(_highValue);

    return Rect(sliderOffset, 2*Y_OFFSET, sliderLength, Y_OFFSET);
}

Rect RangeSelector::_LowSliderRect() const
{
    const int sliderLength = _SliderLength();
	const int sliderOffset = _ValueToSliderOffset(_lowValue);

    return Rect(sliderOffset, 2*Y_OFFSET, sliderLength, Y_OFFSET);
}

int RangeSelector::_ValueToSliderOffset(int val) const
{
	const int relativeVal = val - _minimum;
	const int valueInterval = _maximum - _minimum;
	const int sliderOffset = (valueInterval ? (std::max(0, _GutterLength() - _SliderLength())*relativeVal/valueInterval) : 0);
	return sliderOffset;
}



} // namespace Widgets

} // namespace BackyardBrains
