#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <utility>
#include "BitBuffer.h"
#include "VCode.h"

using std::int8_t;
using std::uint8_t;
using std::size_t;
using std::vector;


namespace vcodegen {

	static int crc32;
	static int checksum_16;
	static int bitlength;

	VCode VCode::encodeBinary(const vector<uint8_t> &data) {
		vector<VcSegment> segs{ VcSegment::makeBytes(data) };
		return encodeSegments(segs);
	}


	VCode VCode::encodeSegments(const vector<VcSegment> &segs, int mask) {

		
		int dataUsedBits = VcSegment::getTotalBits(segs);
		if (dataUsedBits == -1)
			throw "Assertion error";
		size_t dataCapacityBits = getNumDataCodewords() * 8;
		BitBuffer bb;
		for (const VcSegment &seg : segs) {
			bb.insert(bb.end(), seg.getData().begin(), seg.getData().end());
		}
		bitlength = bb.size();
		crc32 = CRC32(bb.getBytes());
		bb.appendBits(0, std::min<size_t>(4, dataCapacityBits - bb.size()));
		bb.appendBits(0, (8 - bb.size() % 8) % 8);

		for (uint8_t padByte = 0xEC; bb.size() < dataCapacityBits; padByte ^= 0xEC ^ 0x11)
			bb.appendBits(padByte, 8);
		if (bb.size() % 8 != 0)
			throw "Assertion error";

		return VCode(bb.getBytes(), mask);
	}


	VCode::VCode(const vector<uint8_t> &dataCodewords, int mask) :
		modules(size, vector<bool>(size)), 
		isFunction(size, vector<bool>(size)) {

		drawFunctionPatterns();
		drawCodewords(dataCodewords);
		this->mask = handleConstructorMasking(mask);
	}


	int VCode::getSize() const {
		return size;
	}

	int VCode::getMask() const {
		return mask;
	}


	bool VCode::getModule(int x, int y) const {
		return 0 <= x && x < size && 0 <= y && y < size && module(x, y);
	}

	void VCode::drawFunctionPatterns() {

		drawFinderPattern(3, 3);

		drawFormatBits(0);  
	}


	void VCode::drawFormatBits(int mask) {

		// Draw mask bit
		setFunctionModule(8, 0, (mask & 4) != 0);
		setFunctionModule(9, 0, (mask & 2) != 0);
		setFunctionModule(10, 0, (mask & 1) != 0);

		// Draw bitlenght
		for (int i = 0; i<13; i++)
		{
			setFunctionModule(11 + i, 0, ((bitlength >> (12 - i)) & 1) != 0);
		}

		//Draw CRC32
		int EDC = crc32;
		for (int i = 0; i<32; i++)
		{
			setFunctionModule(24 + i, 0, ((EDC >> (31 - i)) & 1) != 0);
		}

		char* MasktoByte = intToByteArray(mask);
		char* lengthtoByte = intToByteArray(bitlength);
		char* CRCtoByte = intToByteArray(crc32);

		char *total = new char[12];
		for (int i = 0; i<12; i++)
		{
			if (i <= 3)
				total[i] = MasktoByte[i];
			else if (i>3 && i <= 7)
				total[i] = lengthtoByte[i - 4];
			else if (i>7 && i<12)
				total[i] = CRCtoByte[i - 8];
		}
		long checksum_16 = calculateChecksum(total);

		//Draw format checksum
		for (int i = 0; i<16; i++)
		{
			setFunctionModule(8 + i, 1, ((checksum_16 >> (15 - i)) & 1) != 0);
		}
	}

	void VCode::drawFinderPattern(int x, int y) {
		for (int i = -4; i <= 4; i++) {
			for (int j = -4; j <= 4; j++) {
				int dist = std::max(std::abs(i), std::abs(j));  // Chebyshev/infinity norm
				int xx = x + j, yy = y + i;
				if (0 <= xx && xx < size && 0 <= yy && yy < size)
					setFunctionModule(xx, yy, dist != 2 && dist != 4);
			}
		}
	}

	void VCode::setFunctionModule(int x, int y, bool isBlack) {
		modules.at(y).at(x) = isBlack;
		isFunction.at(y).at(x) = true;
	}


	bool VCode::module(int x, int y) const {
		return modules.at(y).at(x);
	}

	void VCode::drawCodewords(const vector<uint8_t> &data) {
		if (data.size() != static_cast<unsigned int>(getNumRawDataModules() / 8))
			throw "Invalid argument";

		int i = 0;  // Bit index into the data
		int x, y = 0;
		for (y = 1; y<size; y++)
		{
			if (y == 1)
				x = 24;
			else if (y < 8)
				x = 8;
			else
				x = 0;
			for (x = x; x<size; x++)
			{

				if (!isFunction[y][x] && i < data.size() * 8) {
					modules[y][x] = ((data[i >> 3] >> (7 - (i & 7))) & 1) != 0;
					i++;
				}
				// If there are any remainder bits (0 to 7), they are already
				// set to 0/false/white when the grid of modules was initialized
			}
		}
		if (static_cast<unsigned int>(i) != data.size() * 8)
			throw "Assertion error";
	}


	void VCode::applyMask(int mask) {
		if (mask < 0 || mask > 7)
			throw "Mask value out of range";
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				bool invert;
				switch (mask) {
				case 0:  invert = (x + y) % 2 == 0;                    break;
				case 1:  invert = y % 2 == 0;                          break;
				case 2:  invert = x % 3 == 0;                          break;
				case 3:  invert = (x + y) % 3 == 0;                    break;
				case 4:  invert = (x / 3 + y / 2) % 2 == 0;            break;
				case 5:  invert = x * y % 2 + x * y % 3 == 0;          break;
				case 6:  invert = (x * y % 2 + x * y % 3) % 2 == 0;    break;
				case 7:  invert = ((x + y) % 2 + x * y % 3) % 2 == 0;  break;
				default:  throw "Assertion error";
				}
				modules.at(y).at(x) = modules.at(y).at(x) ^ (invert & !isFunction.at(y).at(x));
			}
		}
	}


	int VCode::handleConstructorMasking(int mask) {
		if (mask == -1) {  // Automatically choose best mask
			long minPenalty = LONG_MAX;
			for (int i = 0; i < 8; i++) {
				drawFormatBits(i);
				applyMask(i);
				long penalty = getPenaltyScore();
				if (penalty < minPenalty) {
					mask = i;
					minPenalty = penalty;
				}
				applyMask(i);  // Undoes the mask due to XOR
			}
		}
		if (mask < 0 || mask > 7)
			throw "Assertion error";
		drawFormatBits(mask);  // Overwrite old format bits
		applyMask(mask);  // Apply the final choice of mask
		return mask;  // The caller shall assign this value to the final-declared field
	}


	long VCode::getPenaltyScore() const {
		long result = 0;

		// Adjacent modules in row having same color
		for (int y = 0; y < size; y++) {
			bool colorX = false;
			for (int x = 0, runX = -1; x < size; x++) {
				if (x == 0 || module(x, y) != colorX) {
					colorX = module(x, y);
					runX = 1;
				}
				else {
					runX++;
					if (runX == 5)
						result += PENALTY_N1;
					else if (runX > 5)
						result++;
				}
			}
		}
		// Adjacent modules in column having same color
		for (int x = 0; x < size; x++) {
			bool colorY = false;
			for (int y = 0, runY = -1; y < size; y++) {
				if (y == 0 || module(x, y) != colorY) {
					colorY = module(x, y);
					runY = 1;
				}
				else {
					runY++;
					if (runY == 5)
						result += PENALTY_N1;
					else if (runY > 5)
						result++;
				}
			}
		}

		// 2*2 blocks of modules having same color
		for (int y = 0; y < size - 1; y++) {
			for (int x = 0; x < size - 1; x++) {
				bool  color = module(x, y);
				if (color == module(x + 1, y) &&
					color == module(x, y + 1) &&
					color == module(x + 1, y + 1))
					result += PENALTY_N2;
			}
		}

		// Finder-like pattern in rows
		for (int y = 0; y < size; y++) {
			for (int x = 0, bits = 0; x < size; x++) {
				bits = ((bits << 1) & 0x7FF) | (module(x, y) ? 1 : 0);
				if (x >= 10 && (bits == 0x05D || bits == 0x5D0))  // Needs 11 bits accumulated
					result += PENALTY_N3;
			}
		}
		// Finder-like pattern in columns
		for (int x = 0; x < size; x++) {
			for (int y = 0, bits = 0; y < size; y++) {
				bits = ((bits << 1) & 0x7FF) | (module(x, y) ? 1 : 0);
				if (y >= 10 && (bits == 0x05D || bits == 0x5D0))  // Needs 11 bits accumulated
					result += PENALTY_N3;
			}
		}

		// Balance of black and white modules
		int black = 0;
		for (const vector<bool> &row : modules) {
			for (bool color : row) {
				if (color)
					black++;
			}
		}
		int total = size * size;
		// Find smallest k such that (45-5k)% <= dark/total <= (55+5k)%
		for (int k = 0; black * 20L < (9L - k)*total || black * 20L > (11L + k)*total; k++)
			result += PENALTY_N4;
		return result;
	}

	int VCode::getNumRawDataModules() {
		int result = size * size;
		result -= 64;
		result -= 64;
		return result;
	}


	int VCode::getNumDataCodewords() {
		return getNumRawDataModules() / 8;
	}

	int VCode::CRC32(const std::vector<std::uint8_t> &data)
	{
		int table[] = {
			0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
			0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
			0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
			0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
			0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
			0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
			0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
			0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
			0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
			0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
			0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
			0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
			0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
			0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
			0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
			0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
			0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
			0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
			0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
			0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
			0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
			0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
			0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
			0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
			0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
			0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
			0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
			0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
			0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
			0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
			0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
			0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
		};
		unsigned int crc = 0xffffffff;
		for (int i = 0; i < data.size(); i++)
		{
			crc = (crc >> 8) ^ table[(crc^data.at(i)) & 0xff];
		}
		crc = crc ^ 0xffffffff;
		return crc;
	}

	long VCode::calculateChecksum(char buf[])
	{
		int length =12;
		int i = 0;

		long sum = 0;
		long data;

		while (length > 1) {
			data = (((buf[i] << 8) & 0xff00) | ((buf[i + 1]) & 0xff));
			sum += data;
			if ((sum & 0xffff0000) > 0) {
				sum = sum & 0xffff;
				sum += 1;
			}
			i += 2;
			length -= 2;
		}

		if (length>0) {
			sum += (buf[i] << 8 & 0xff00);
			if ((sum & 0xffff0000)>0) {
				sum = sum & 0xffff;
				sum += 1;
			}
		}
		sum = ~sum;
		sum = sum & 0xffff;
		return sum;
	}

	char * VCode::intToByteArray(int value)
	{
		char* byteArray = new char[4];
		byteArray[0] = (char)(value >> 24);
		byteArray[1] = (char)(value >> 16);
		byteArray[2] = (char)(value >> 8);
		byteArray[3] = (char)(value);

		return byteArray;
	}

	/*---- Tables of constants ----*/

	const int VCode::PENALTY_N1 = 3;
	const int VCode::PENALTY_N2 = 3;
	const int VCode::PENALTY_N3 = 40;
	const int VCode::PENALTY_N4 = 10;

}
