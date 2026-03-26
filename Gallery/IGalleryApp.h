#pragma once

/*
	Base gallery for the album manager and the GUI manager
	the only pure method is run(), how the application starts is complitly up to the child class
*/

class IGalleryApp
{
public:
	virtual ~IGalleryApp() = default;
	virtual void run() = 0;
};
