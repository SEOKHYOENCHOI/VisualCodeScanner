#pragma once
#include <string>
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include <windows.h>
#include <fstream>
#include <Math.h>
#include "VCode.h"

#define TIME_WAIT 500
#define THRESHOLD(nFrame) (TIME_WAIT * nFrame + 3000)
#define MAX_TIME_WAIT TIME_WAIT * 500

//using namespace qrcodegen;
using namespace std;
using namespace cv;
using namespace vcodegen;
class vc_timer {

public:
	vc_timer() {
		QueryPerformanceFrequency(&f);
		QueryPerformanceCounter(&s);
	}
	~vc_timer() {}
	double __inline getmsInterval() {
		return ((double)(e.QuadPart - s.QuadPart) / ((double)f.QuadPart / 1000));
	}
	int getsInterval() {

		return (int)floor(((double)(e.QuadPart - s.QuadPart) / (double)f.QuadPart));
	}
	void setStartTime() {
		QueryPerformanceCounter(&s);
	}
	void setStartTime(LARGE_INTEGER start) {
		s = start;
	}
	LARGE_INTEGER getEndTime() { return e; }

	void __inline setEndTime() {
		QueryPerformanceCounter(&e);
	}
	void __inline setEndTime(LARGE_INTEGER end) {
		e = end;
	}

private:
	LARGE_INTEGER s, e, f;
};

class Receiver {
	friend ostream &operator<<(ofstream&, const Receiver&) {

	}
public:
	Receiver(ofstream &fos) :fout(fos) {
		vector<uchar, std::allocator<uchar>> s;
		seqCheck = new bool[1];
		seqCheck[0] = false;
		seq = -1;
		seqLength = -1;
		rtxnFrame = Point(-1, -1);
		beforertx = Point(-1, -1);
	}
	//Re-Transmission Request
	Point rtxRequest();
	void init(string d);
	void recv(string d);
	int remvBSandGetLen(string d, char* buffer, int len);
	void destroy();
	void printState() {
		system("cls");
		printf("File Name : %s\n\n", filename);
		int fileSize = 0;
		for (int i = 1; i < seqLength + 1; i++) {
			printf("Sequence %3d - ", i);
			if (seqCheck[i] == true&& getFrameBuffSize()[i]>0) {
				printf("£Ï"); 
				fileSize += getFrameBuffSize()[i];
			}
			else printf("£Ø");
			printf("\n");
		}
		exe_timer.setEndTime();
		double bps = 0;
		bps = (double)fileSize * 8 / (exe_timer.getmsInterval() / 1000);
		printf("Bit Rate : %.3lf kbps\n", bps / 1000);
	}
	void fWrite()
	{
		int fileSize = 0;
		exe_timer.setEndTime();

		fout.open(getFileName(), ios::binary);
		for (int i = 0; i < getFrameLength(); i++) {
			fileSize += getFrameBuffSize()[i];
			fout.write(getBuffer()[i], getFrameBuffSize()[i]);
		}
		fout.close();
		double bps = (double)fileSize * 10 / (exe_timer.getmsInterval() / 1000);
		system("cls");
		cout << endl << "Complete!" << endl;
		cout << "File Name : " << filename << endl;
		printf("File Size : %d B\n", fileSize);
		printf("Transmission Time : %.3lfs\n", exe_timer.getmsInterval() / 1000);
		printf("Bit Rate : %.3lf kbps\n", bps / 1000);
	}
	bool __inline timeCheck() {
		return ((seq -1== rtxnFrame.y || rt_timer.getmsInterval() > THRESHOLD(rtxnFrame.x)) && seqCheck[0] == true);
	}
	bool sequenceCheck();
	static void toVCImage(const VCode vc, int scale, int border);
	void end();
	char** getBuffer() { return frameBuffer; }
	int getFrameLength() { return seqLength; }
	char* getFileName() { return filename; }
	int* getFrameBuffSize() { return frameBufferSize; }
	vc_timer getTimer() { return rt_timer; }
	int getFileNameLength() { return fileNameLength; }
private:
	bool *seqCheck;
	int seq;
	int seqLength;
	string d;
	//for retransmission request ( x = number of frames to receive, y = last frame sequence number to receive)
	Point rtxnFrame;
	Point beforertx;
	//
	ofstream &fout;
	int fileNameLength;
	//
	vc_timer exe_timer;
	vc_timer rt_timer;
	char* filename;
	char** frameBuffer;
	int* frameBufferSize;
};