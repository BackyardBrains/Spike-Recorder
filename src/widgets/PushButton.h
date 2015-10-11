#ifndef BACKYARDBRAINS_WIDGETS_PUSHBUTTON_H
#define BACKYARDBRAINS_WIDGETS_PUSHBUTTON_H

#include "Widget.h"

namespace BackyardBrains {

namespace Widgets {

class TextureGL;

class PushButton : public Widget
{
public:
	PushButton(Widget *parent = NULL);

	void setNormalTex(const TextureGL *tex);
	void setHoverTex(const TextureGL *tex);
// 	Size sizeHint() const;
    void setRightPadding(int padding);
	sigslot::signal0<> clicked;
private:
	void paintEvent();
	void mousePressEvent(MouseEvent *event);
	void mouseReleaseEvent(MouseEvent *event);
	void mouseMotionEvent(MouseEvent *event);
	void leaveEvent();
	void enterEvent();
	
	const TextureGL *_normaltex;
	const TextureGL *_hovertex;
    int _rightPadding = 0;

	bool _hover;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
