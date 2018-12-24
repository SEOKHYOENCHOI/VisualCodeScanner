
#include <climits>
#include <cstring>
#include <utility>
#include "VcSegment.h"

using std::uint8_t;
using std::vector;


namespace vcodegen {

	VcSegment::Mode::Mode(int mode, int cc0, int cc1, int cc2) :
		modeBits(mode) {
		numBitsCharCount[0] = cc0;
		numBitsCharCount[1] = cc1;
		numBitsCharCount[2] = cc2;
	}


	int VcSegment::Mode::getModeBits() const {
		return modeBits;
	}


	int VcSegment::Mode::numCharCountBits() const {
		return numBitsCharCount[0];

	}

	const VcSegment::Mode VcSegment::Mode::BYTE(0x4, 8, 16, 16);

	VcSegment VcSegment::makeBytes(const vector<uint8_t> &data) {
		if (data.size() > INT_MAX)
			throw "Data too long";
		BitBuffer bb;
	
		for (uint8_t b : data)
			bb.appendBits(b, 8);
		return VcSegment(Mode::BYTE, static_cast<int>(data.size()), std::move(bb));
	}

	VcSegment::VcSegment(Mode md, int numCh, const std::vector<bool> &dt) :
		mode(md),
		numChars(numCh),
		data(dt) {
		if (numCh < 0)
			throw "Invalid value";
	}


	VcSegment::VcSegment(Mode md, int numCh, std::vector<bool> &&dt) :
		mode(md),
		numChars(numCh),
		data(std::move(dt)) {
		if (numCh < 0)
			throw "Invalid value";
	}


	int VcSegment::getTotalBits(const vector<VcSegment> &segs) {
		int result = 0;
		for (const VcSegment &seg : segs) {
			int ccbits = seg.mode.numCharCountBits();
			// Fail if segment length value doesn't fit in the length field's bit-width
			if (seg.numChars >= (1L << ccbits))
				return -1;
			if (4 + ccbits > INT_MAX - result)
				return -1;
			result += 4 + ccbits;
			if (seg.data.size() > static_cast<unsigned int>(INT_MAX - result))
				return -1;
			result += static_cast<int>(seg.data.size());
		}
		return result;
	}


	VcSegment::Mode VcSegment::getMode() const {
		return mode;
	}


	int VcSegment::getNumChars() const {
		return numChars;
	}

	const std::vector<bool> &VcSegment::getData() const {
		return data;
	}

}
