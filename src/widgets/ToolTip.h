#ifndef BACKYARDBRAINS_WIDGETS_TOOLTIP_H
#define BACKYARDBRAINS_WIDGETS_TOOLTIP_H

#include "Widget.h"
#include <string>
#include <stdint.h>

namespace BackyardBrains {

namespace Widgets {

class ToolTip : public Widget {
public:
	ToolTip(const char *text, int liveTime);
private:
	std::string _text;
	uint32_t _creationTime;
	int _lifeTime;

	void paintEvent();
	void advance();
};

}

}

#endif
