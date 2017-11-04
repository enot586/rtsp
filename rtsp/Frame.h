#pragma once
#include <memory>
class Frame
{
	std::unique_ptr<uint8_t[]> p;
	std::vector<uint8_t> v;

	size_t s;
public:
	Frame() : p(nullptr), s(0)	{}

	Frame(uint8_t* buffer, size_t size) : p(buffer), s(size) 	{};

	void ToFile();

	std::vector<uint8_t>& ToVector();
	void ToVector(std::vector<uint8_t>& v);

	void SetJpeg(uint8_t* p, size_t size);

	~Frame();
};

