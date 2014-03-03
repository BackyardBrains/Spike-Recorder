#include "qDebug.h"

#include <iostream>

namespace BackyardBrains {

namespace Widgets {

qDebug::qDebug()
{
	std::cerr << std::endl;
}

qDebug::~qDebug()
{
	std::cerr << std::endl;
}

} // namespace Widgets

} // namespace BackyardBrains
