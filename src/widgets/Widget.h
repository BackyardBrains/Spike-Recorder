#ifndef BACKYARDBRAINS_WIDGETS_WIDGET_H
#define BACKYARDBRAINS_WIDGETS_WIDGET_H

#include <vector>
#include <list>
#include <cstring>
#include <string>
#include <sigslot.h>

#include "Rect.h"
#include "Size.h"
#include "Event.h"
#include "SizePolicy.h"


namespace BackyardBrains {

namespace Widgets {

class Layout;
class BitmapFontGL;

class Widget : public sigslot::has_slots<>
{
public:
	typedef std::vector<Widget*> WidgetVector;
	Widget(Widget *parent = NULL);
	virtual ~Widget();

	static void run();

	Widget *parentWidget() const;
	void setParentWidget(Widget *w);

	Layout *layout();
	void setLayout(Layout *newLayout);

	void setGeometry(const Rect &newRect);
	Rect geometry() const;
	void setRect(const Rect &newRect);
	Rect rect() const;
	Point pos() const;
	void move(const Point &newPoint);
	void setSize(const Size &newSize); // TODO remove this? It should only be used by the layout?
	Size size() const;
	int width() const;
	int height() const;

	Size sizeHint() const;
	void setSizeHint(const Size &hint);

	SizePolicy sizePolicy() const;
	void setSizePolicy(const SizePolicy &newPolicy);

	Point mapToParent(const Point &point) const;
	Point mapToGlobal(const Point &point) const;

	WidgetVector &children();

	void close();
	void unclose();
	bool closed();
	void setVisible(bool visible);
	void setHidden(bool hidden);
	bool isVisible() const;
	bool isHidden() const;

	Widget *_GetWidgetAt(const Point &point);
	void setMouseTracking(bool enable);
	bool hasMouseTracking() const;

	void setDeleteOnClose(bool d);
	bool getDeleteOnClose() const;

	virtual void advance();
	virtual void resizeEvent(ResizeEvent *event);
	virtual void glResetEvent(); // happens when the GL context is reset. textures have to be reloaded in that case.
	virtual void paintEvent();
	virtual void enterEvent();
	virtual void leaveEvent();
	virtual void keyPressEvent(KeyboardEvent *event);
	virtual void keyReleaseEvent(KeyboardEvent *event);
	virtual void mousePressEvent(MouseEvent *event);
	virtual void mouseReleaseEvent(MouseEvent *event);
	virtual void mouseMotionEvent(MouseEvent *event);

	void _DoPaintEvents(const Point &offset, const Rect &clipRect);
	void _DoGlResetEvents();
	void _CallAdvance();
private:
	Widget *_parentWidget;
	Layout * _layout;
	WidgetVector _children;
	Rect _rect;
	SizePolicy _sizePolicy;
	Size _sizeHint;
	struct WidgetState
	{
		WidgetState() {memset(this, 0, sizeof(*this)); deleteOnClose = true;}
		bool closed : 1;
		bool hidden : 1;
		bool mouseTracking : 1;
		bool deleteOnClose : 1;
	} _state;


};

} // namespace Widgets

} // namespace BackyardBrains

#endif
