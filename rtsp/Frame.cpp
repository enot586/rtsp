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


void Frame::ToVector(std::vector<uint8_t>& v)
{
	v.resize(s);
	
	for (size_t i = 0; i < s; ++i)
		v[i] = p[i];
}

std::vector<uint8_t>& Frame::ToVector()
{
	return v;
}

void Frame::SetJpeg(uint8_t* p, size_t size)
{
	if ( size > v.size() )
		v.resize(size);

	for (size_t i = 0; i < size; ++i)
		v[i] = p[i];
}