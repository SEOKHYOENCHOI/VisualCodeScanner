#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "VcSegment.h"


namespace vcodegen {

	class VCode final {

	public: static VCode encodeBinary(const std::vector<std::uint8_t> &data);
	public: static VCode encodeSegments(const std::vector<VcSegment> &segs, int mask = -1); 
	private: static const int size = 56;
	private: int mask;
	private: std::vector<std::vector<bool> > modules;    
	private: std::vector<std::vector<bool> > isFunction; 
	public: VCode(const std::vector<std::uint8_t> &dataCodewords, int mask);
	public: int getSize() const;
	public: int getMask() const;
	public: bool getModule(int x, int y) const;
	private: void drawFunctionPatterns();
	private: void drawFormatBits(int mask);
	private: void drawFinderPattern(int x, int y);
	private: void setFunctionModule(int x, int y, bool isBlack);
	private: bool module(int x, int y) const;
	private: void drawCodewords(const std::vector<std::uint8_t> &data);
	private: void applyMask(int mask);
	private: int handleConstructorMasking(int mask);
	private: long getPenaltyScore() const;
	private: static int getNumRawDataModules();
	private: static int getNumDataCodewords();
	private: static const int PENALTY_N1;
	private: static const int PENALTY_N2;
	private: static const int PENALTY_N3;
	private: static const int PENALTY_N4;
	public: static int CRC32(const std::vector<std::uint8_t> &data);
	public: long calculateChecksum(char data[]);
	public: char* intToByteArray(int value);

	};
}
