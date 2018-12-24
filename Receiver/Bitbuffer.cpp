
#include "BitBuffer.h"


namespace vcodegen {

	BitBuffer::BitBuffer()
		: std::vector<bool>() {}


	std::vector<std::uint8_t> BitBuffer::getBytes() const {
		std::vector<std::uint8_t> result(size() / 8 + (size() % 8 == 0 ? 0 : 1));
		for (std::size_t i = 0; i < size(); i++)
			result[i >> 3] |= (*this)[i] ? 1 << (7 - (i & 7)) : 0;
		return result;
	}


	void BitBuffer::appendBits(std::uint32_t val, int len) {
		if (len < 0 || len > 31 || val >> len != 0)
			throw "Value out of range";
		for (int i = len - 1; i >= 0; i--, bitLenght++)  // Append bit by bit
			this->push_back(((val >> i) & 1) != 0);
		
	}

	int BitBuffer::bitLength() {
		return bitLenght;
	}

}
