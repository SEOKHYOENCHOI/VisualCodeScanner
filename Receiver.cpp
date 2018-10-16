#include "Receiver.h"


Point Receiver::rtxRequest()
{
	vector<uchar, allocator<uchar>> s;
	Point *before = &beforertx;
	int n = 0;
	int l = 0;
	for (int i = 0; i <= seqLength; i++) {
		if (seqCheck[i] == false) {
			n++;
			l = i;
			s.push_back(i);
		}
	}
	rt_timer.setStartTime();
	
	if (before->x != n ) {
		Point rtx(n, l);
	//	VCode rtxVCode = VCode::encodeBinary(s);
	//	toVCImage(rtxVCode, 6, 10);
		cout << "----- Sequence ";
		for (int i = 0; i < n; i++)
			cout << (int)s.at(i) << " ";
		cout << " was not received" << endl;
		before->x = n;
		before->y = l;
		return rtx;
	}
	else
		return *before;
}

void Receiver::init(string d)
{
	rt_timer = vc_timer();
	exe_timer = vc_timer();
	seqLength = d.at(1);
	
	frameBuffer = new char*[seqLength];
	frameBufferSize = new int[seqLength];
	filename = new char[d.length() - 1];
	seqCheck = (bool*)calloc(seqLength + 1, sizeof(bool));
	//	cout << symbol->get_data().substr(3, headerLength) << endl;
	rtxnFrame.x = seqLength - 1;
	rtxnFrame.y = seqLength - 1;
	fileNameLength = d.length() - 2;
	sprintf(filename, "%s", d.substr(2, d.length()).c_str());
	cout << "File Name : " << filename << endl;
	cout << "Sequence Length : " << seqLength << endl << endl;

//	cv::imshow("rtx", rtxMat);
}

void Receiver::recv(string d)
{
	seq = d.at(0);
	if (seqCheck[seq] == false) {
		if (seq == 0) {
			init(d);
			seqCheck[seq] = true;
			//printState();
		}
		else if (seq <= seqLength) {
			int dlen = d.length();
			char* tempBuff = new char[dlen - 1];
			frameBufferSize[seq - 1] = dlen - 1;// remvBSandGetLen(d, tempBuff, dlen);//buffer size 
			for (int i = 1; i < dlen; i++) {
				tempBuff[i - 1] = d.at(i);
			}
			frameBuffer[seq - 1] = new char[frameBufferSize[seq - 1]];
			memcpy(frameBuffer[seq - 1], tempBuff, frameBufferSize[seq - 1]);
			delete[] tempBuff;
			seqCheck[seq] = true;
			//printState();
			cout << "Sequence " << seq << " was received" << endl;
		}
	}
	
	rt_timer.setEndTime();
}

int Receiver::remvBSandGetLen(string d, char * buffer, int len)
{
	int bSize = len;
	for (int i = 0, j = 1; j < len + 1; i++, j++) {
		if (d.at(j) == -61) {
			buffer[i] = d.at(++j) + 64;
			bSize--;
		}
		else if (d.at(j) == -62) {
			buffer[i] = d.at(++j);
			bSize--;
		}
		else
			buffer[i] = d.at(j);
	}
	return bSize;
}

void Receiver::destroy()
{
	delete[] filename;
	free(seqCheck);
	for (int i = 0; i < seqLength; i++)
		delete[] frameBuffer[i];
	delete[] frameBuffer;
	delete[] frameBufferSize;
}

bool Receiver::sequenceCheck()
{
	int i = 0;
	for (; i < seqLength + 1; i++) {
		if (seqCheck[i] == true)
			continue;
		else {
			beforertx = rtxnFrame;
			rtxnFrame = rtxRequest();
			rt_timer.setStartTime();
			break;
		}
	}
	if (i == seqLength + 1 && seqLength > 0)
		return true;
	else
		return false;
}

void Receiver::toVCImage(const VCode vc, int scale, int border)
{
	int vcSize = (56 + 5 * 2) * 7;
	Mat rtx(vcSize, vcSize, CV_8UC1);
	uchar *ptr = (uchar*)rtx.data;
	bool val = 0;
	int x1 = 0, x2 = 0;
	for (int y = 0; y < rtx.rows; y++) {
		for (int x = 0; x < rtx.cols; x++) {
			if (val == 0 && val != vc.getModule(x / scale - border, y / scale - border)) { 
				x1 = x;
				val = 1;
			}
			else if (val == 1 && val != vc.getModule(x / scale - border, y / scale - border)) {
				x2 = x;
				break;
			}
		}
		if (x2 != 0)break;
	}

	int blockSize = (x2 - x1) / 7;
	for (int y = 0; y < rtx.rows; y++) {
		for (int x = 0; x < rtx.cols; x++) {
			if (y <= ((x1 - blockSize)) || x <= ((x1 - blockSize))|| y > ((x1 + blockSize*57)) || x > ((x1 + blockSize*57))) {
				ptr[y*rtx.cols + x] = 0x00;
				continue;
			}
			bool val = vc.getModule(x / scale - border, y / scale - border);
			val ? ptr[y*rtx.cols + x] = 0x00 : ptr[y*rtx.cols + x] = 0xff;
		}
	}
	cv::imshow("rtx", rtx);
}

void Receiver::end()
{
	vector<uchar, allocator<uchar>> s;
	s.push_back(seqLength + 1);
	VCode endVCode = VCode::encodeBinary(s);
	toVCImage(endVCode, 6, 10);
	waitKey(10000);
}
