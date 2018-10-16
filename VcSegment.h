#pragma once

#include <cstdint>
#include <vector>
#include "BitBuffer.h"


namespace vcodegen {

	class VcSegment final {

	public: class Mode final {

	public: static const Mode BYTE;
	private: int modeBits;
	private: int numBitsCharCount[3];
	private: Mode(int mode, int cc0, int cc1, int cc2);
	public: int getModeBits() const;
	public: int numCharCountBits() const;

	};
	public: static VcSegment makeBytes(const std::vector<std::uint8_t> &data);
	private: Mode mode;
	private: int numChars;
	private: std::vector<bool> data;
	public: VcSegment(Mode md, int numCh, const std::vector<bool> &dt);
	public: VcSegment(Mode md, int numCh, std::vector<bool> &&dt);
	public: Mode getMode() const;
	public: int getNumChars() const;
	public: const std::vector<bool> &getData() const;
	public: static int getTotalBits(const std::vector<VcSegment> &segs);

	};

}
