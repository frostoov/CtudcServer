#include "nettools.hpp"

#include <cstdint>
#include <cstring>
#include <tdcdata/structs.hpp>

using tdcdata::DecorPackage;
using tdcdata::NevodPackage;

bool verifyDecorPackage(char* data, size_t size) {
	static size_t emptySize = sizeof(DecorPackage::keyword) + sizeof(DecorPackage::command) +
	                          sizeof(DecorPackage::numberOfPm) + sizeof(DecorPackage::numberOfPackage) +
	                          sizeof(DecorPackage::checksum) + sizeof(DecorPackage::length);
	if(size < emptySize)
		return false;
	membuf tmpBuffer(data, size);
	std::istream stream(&tmpBuffer);
	int8_t   keyword[6];
	uint8_t  command;
	int8_t   numberOfPm;
	uint32_t numberOfPackage;
	uint16_t checksum;
	int16_t  length;
	::deserialize(stream, keyword, sizeof(keyword));
	if( memcmp(keyword, "NVDDC", sizeof(keyword)) )
		return false;
	::deserialize(stream, command);
	::deserialize(stream, numberOfPm);
	::deserialize(stream, numberOfPackage);
	::deserialize(stream, checksum);
	::deserialize(stream, length);
	auto bufferSize = static_cast<int16_t>(size - emptySize);
	if(length != bufferSize)
		return false;
	return true;
}


bool verifyNevodPackage(char* data, size_t size) {
	if(size != NevodPackage::getSize())
		return false;
	int8_t   keyword[6];
	membuf tmpBuffer(data, size);
	std::istream stream(&tmpBuffer);
	::deserialize(stream, keyword, sizeof(keyword));
	if(memcmp(keyword, "TRACK ", sizeof(keyword)) )
		return false;
	return true;
}
