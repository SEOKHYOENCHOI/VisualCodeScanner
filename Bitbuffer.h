#pragma once

#include <cstdint>
#include <vector>


namespace vcodegen {

	class BitBuffer final : public std::vector<bool> {

	public: BitBuffer();
	public: int bitLenght = 0;
	public: std::vector<std::uint8_t> getBytes() const;
	public: void appendBits(std::uint32_t val, int len);
	public: int bitLength();

	};

}
