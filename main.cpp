#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <fstream>
#include "Receiver.h"
#include "VC_Scanner.h"

using namespace cv;
using namespace std;

int main(int, char**)
{
	srand((unsigned)time(NULL));
	VideoCapture cap;
	ofstream fout;
	//Receiver rcvr = Receiver(fout);
	cap.open(0);
	if (!cap.isOpened()) {
		cerr << "ERROR! Unable to open camera\n";
		return -1;
	}
	cout << "Input Visual Code" << endl;
	Receiver rcvr = Receiver(fout);
	double beforeBlockSize = 0;
	double beforeOffset = 0;
	while(1)
	{
		Mat frame;	
		cap.read(frame);
		if (frame.empty()) {
			cerr << "ERROR! blank frame grabbed\n";
			break;
		}
		VC_Scanner scanner = VC_Scanner(frame, beforeBlockSize, beforeOffset);
		string vc_data;
		if ((vc_data= scanner.scan()).length() != 0) {
			rcvr.recv(vc_data);
			beforeBlockSize = scanner.getBlockSize();
			beforeOffset = scanner.getOffset();
		}
		imshow("frame", frame);
		if (waitKey(1)==27)
			break;
		if (rcvr.timeCheck() && rcvr.sequenceCheck()) {
			rcvr.fWrite();
			char* exe =(char*)malloc(7+rcvr.getFileNameLength());
			memcpy(exe, "start ", 6);
			memcpy(&exe[6], rcvr.getFileName(), rcvr.getFileNameLength());
			exe[6 + rcvr.getFileNameLength()] = '\0';
			printf("Open file...\n");
			system(exe);
			cout << endl;
			rcvr.destroy();
			free(exe);
			//rcvr.end();
			break;
		}
	}
	return 0;
}