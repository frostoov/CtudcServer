#include "nettools.hpp"

#include <cstdint>
#include <cstring>
#include <trekdata/structs.hpp>

using trekdata::NevodPackage;


bool verifyNevodPackage (char* data, size_t size) {
	if (size != NevodPackage::getSize() )
	{ return false; }
	int8_t   keyword[6];
	membuf tmpBuffer (data, size);
	std::istream stream (&tmpBuffer);
	::deserialize (stream, keyword, sizeof (keyword) );
	if (memcmp (keyword, "TRACK ", sizeof (keyword) ) )
	{ return false; }
	return true;
}
