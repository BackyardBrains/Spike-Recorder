#ifndef RANGESELECTOR_H
#define RANGESELECTOR_H

#include "Widget.h"

namespace BackyardBrains {

namespace Widgets  {

        class RangeSelector : public Widget
        {
            public:
                RangeSelector( Widget *parent = NULL);
                ~RangeSelector();

                int minimum() const;
                void setMinimum(int minVal);
                int maximum() const;
                void setMaximum(int maxVal);
                void setRange(int minVal, int maxVal);
                int lowValue() const;
                void setLowValue(int val);
                int highValue() const;
                void setHighValue(int val);
                void updateLowLogValue(int val); // don't emit a signal
                void updateHighLogValue(int val); // don't emit a signal

                sigslot::signal1<int> lowValueChanged;
                sigslot::signal1<int> highValueChanged;
            protected:
            private:
                int _minimum;
                int _maximum;
                int _lowValue;
                int _highValue;


                enum ClickArea
                {
                    CLICKED_NONE,
                    CLICKED_LOW_SLIDER,
                    CLICKED_HIGH_SLIDER
                };

                int _GutterLength() const;
                int _SliderLength() const;
                Rect _GutterRect() const;
                Rect _HighSliderRect() const;
                Rect _LowSliderRect() const;

                int _highSliderClickedPixelOffset;
                int _highDraggingSliderOffset;
                int _lowSliderClickedPixelOffset;
                int _lowDraggingSliderOffset;

                void checkMinMaxValues();
                void paintEvent();
                void mousePressEvent(MouseEvent *event);
                void mouseMotionEvent(MouseEvent *event);
                void mouseReleaseEvent(MouseEvent *event);

                ClickArea _clickState;
                int _ValueToSliderOffset(int val) const;

        };

} // namespace Widgets

} // namespace BackyardBrains
#endif // RANGESELECTOR_H
