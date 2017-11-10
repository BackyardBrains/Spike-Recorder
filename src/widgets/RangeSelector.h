#ifndef RANGESELECTOR_H
#define RANGESELECTOR_H
#include "TextInput.h"
#include "Widget.h"

namespace BackyardBrains {

namespace Widgets  {

        class RangeSelector : public Widget
        {
            public:
                RangeSelector( Widget *parent = NULL, int heightOfComponent = 40);
                ~RangeSelector();

                int minimum() const;
                void setMinimum(int minVal);
                int maximum() const;
                void setMaximum(int maxVal);
                void setRange(int minVal, int maxVal);
                void setLowValue(int val);
                void setHighValue(int val);
                int getLowValue();
                int getHighValue();
                sigslot::signal1<int> lowValueChanged;
                sigslot::signal1<int> highValueChanged;
            void initHighAndLow(int newHigh, int newLow);
            protected:
            private:


                int lowValue() const;
                int highValue() const;
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
                int baseHeight = 20;//half the height of component (height of slider)
                int sliderMinSize = 8;//width of dragging pins (depends on heightOfComponent)

        };

} // namespace Widgets

} // namespace BackyardBrains
#endif // RANGESELECTOR_H
