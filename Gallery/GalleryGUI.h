#pragma once
#include "IGalleryApp.h"
#include "IDataAccess.h"

class GalleryGUI : public IGalleryApp
{
public:
	explicit GalleryGUI(IDataAccess& dataAccess);  // prevent the compiler from using that constructor for implicit conversions
	void run() override;
};