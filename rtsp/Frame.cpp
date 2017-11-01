#include "stdafx.h"
#include "Frame.h"
#include <fstream>  



Frame::~Frame()
{

}

void Frame::ToFile()
{
	std::ofstream ofs;
	ofs.open("out.jpg", std::ofstream::out | std::ofstream::binary);
	
	ofs.write( (char*)p.get(), s);

	ofs.close();
}
