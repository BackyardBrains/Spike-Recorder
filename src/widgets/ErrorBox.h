#ifndef BACKYARDBRAINS_WIDGETS_ERRORBOX_H
#define BACKYARDBRAINS_WIDGETS_ERRORBOX_H

#include "Widget.h"
#include <string>

namespace BackyardBrains {

namespace Widgets {

class ErrorBox : public Widget {
public:
	ErrorBox(const char *text);
private:
	std::string text;

	void paintEvent();
	void mousePressEvent(MouseEvent *event);
};

}

}

#endif
