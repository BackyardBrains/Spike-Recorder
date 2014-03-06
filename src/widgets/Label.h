#ifndef BACKYARDBRAINS_WIDGETS_LABEL_H
#define BACKYARDBRAINS_WIDGETS_LABEL_H

#include "Widget.h"
#include <string>

namespace BackyardBrains {

namespace Widgets {

class Label : public Widget
{
public:
	static const int PADDING;

	Label(Widget *parent = NULL);

	void setText(const char *text);
	void setText(int num);
	void setText(float num);

	void updateSize();
private:
	void paintEvent();

	std::string _text;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
