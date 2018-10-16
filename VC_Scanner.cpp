#include <Math.h>
#include <stdlib.h>
#include "VC_Scanner.h"

typedef struct {
	int x;
	int y;
}edge_pos;

void grassFire_8(uchar* edgeData, uchar* lData, vector<edge_pos>* v, int label, int w, int h) {
	edge_pos pos = v->back();
	if (pos.x > 0 && pos.x < w - 1 && pos.y>0 && pos.y < h - 1 && v->size()<4000) {
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				if (edgeData[(pos.y + i)*w + pos.x + j] == 0 && lData[(pos.y + i)*w + pos.x + j] == 0) {
					lData[(pos.y + i)*w + pos.x + j] = label;
					edge_pos temp;
					temp.x = pos.x + j;
					temp.y = pos.y + i;
					//label_info info_temp;
					//info_temp.start = l_info.start;
					//info_temp.label = l_info.label;
					v->push_back(temp);
					grassFire_8(edgeData, lData, v, label, w, h);
				}
			}
		}
	}
}

__inline uchar VC_Scanner::edgeProcess(uchar around[3][3], char mask[3][3]) {
	int masked = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			masked += around[i][j] * mask[i][j];
		}
	}
	if (abs(masked) > 255) {
		return 0;
	}
	else
		return 0xff;
}
__inline bool processLU2RU(Mat bin, Point srt, double epsilon, int minSize) {}
__inline bool processRU2RD(Mat bin, Point srt, double epsilon, int minSize) {}
__inline bool processRD2LD(Mat bin, Point srt, double epsilon, int minSize){}
__inline bool processRD2LU(Mat bin, Point srt, double epsilon, int minSize){}

vector<Point> VC_Scanner::findSquare(Mat bin, double epsilon, int minSize)
{
	//clock-wise
	int w = bin.cols;
	int h = bin.rows;
	uchar* rData = (uchar*)bin.data;
	int windowSize = CODE_SIZE / 6;
	Point ulv, ruv, ldv, rdv = Point(0, 0);
	Point reg;
	uchar p1, p2, p3, p4, p5;
	char laplacianMask[3][3] = { {-1,-1,-1}
							     ,{-1, 8,-1 }
							     ,{-1,-1,-1 } };
	int thresh = 255 * 3;
	vector<vector<Point>> trackingLines = vector<vector<Point>>();
	uchar* temp = (uchar*)calloc(w*h, 1);
	for (int i = 1; i < h - 1; i++) {
		for (int j = 1; j < w - 1; j++) {
			uchar around[3][3] = {
				{ rData[(i - 1)*w + j - 1],rData[(i - 1)*w + j],rData[(i - 1)*w + j + 1] },
			{ rData[i*w + j - 1],      rData[i*w + j],      rData[i*w + j + 1] },
			{ rData[(i + 1)*w + j - 1],rData[(i + 1)*w + j],rData[(i + 1)*w + j + 1] } };

			temp[i*w + j] = edgeProcess(around, laplacianMask);
		}
	}
	bin.data = temp;
	vector<Point>;

	return vector<Point>();
}

vector<Point> VC_Scanner::pointCorrecting(vector<Point> srt)
{
	vector<Point> temp(4);
	for (int i = 0; i < 4; i++)temp[i] = srt[i];
	Point t(0, 0);
	for (int i = 0; i < 4; i++) {
		int s1 = 0, s2 = 0;
		for (int j = i + 1; j < 4; j++) {
			s1 = temp[i].x + temp[i].y;
			s2 = temp[j].x + temp[j].y;
			if (s1 >= s2) {
				t = temp[i];
				temp[i] = temp[j];
				temp[j] = t;
			}
		}
	}
	if (temp[1].x < temp[2].x) {
		t = temp[1];
		temp[1] = temp[2];
		temp[2] = t;
	}
	return temp;
}

Mat VC_Scanner::imageWarp()
{
	vector<Point> pos(4);
	pos = findCntr(frame);

	if (pos[1].x != 0) {

		vector<Point2f> beforeWarp(4);
		
		beforeWarp[0].x = pos[0].x ;
		beforeWarp[0].y = pos[0].y;

		beforeWarp[1].x = pos[1].x ;
		beforeWarp[1].y = pos[1].y ;

		beforeWarp[2].x = pos[2].x ;
		beforeWarp[2].y = pos[2].y ;

		beforeWarp[3].x = pos[3].x ;
		beforeWarp[3].y = pos[3].y ;
		
		Size warpSize(480, 480);
		Mat warpImg(warpSize, frame.type());

		vector<Point2f> afterWarp(4);
		afterWarp[0] = Point2f(0, 0);
		afterWarp[1] = Point2f(warpImg.cols, 0);
		afterWarp[2] = Point2f(0, warpImg.rows);
		afterWarp[3] = Point2f(warpImg.cols, warpImg.rows);
		
		Mat warp = getPerspectiveTransform(beforeWarp, afterWarp);
		warpPerspective(frame, warpImg, warp, warpSize);
		return warpImg;
	}
	else 
		return Mat();
}

void VC_Scanner::binarizeAndBlurring(cv::InputArray srt, cv::OutputArray dst)
{
	Mat gray, blur;
	cvtColor(srt, gray, CV_BGR2GRAY);
	//GaussianBlur(gray, blur, Size(5, 5),0.0);
	//bilateralFilter(gray, blur, 5, 30, 30);
	//threshold(blur, dst, 250, 255,THRESH_OTSU);
//	imshow("blur", blur);
	cv::adaptiveThreshold(gray, dst, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 17, 1);
	//ColorConversionCodes
}

vector<Point> VC_Scanner::findCntr(Mat srt)
{
	Mat bin;
	vector<Point> cntr_point(4);
	binarizeAndBlurring(srt, bin);
	
	vector<Vec4i> hierarchy;
	vector<vector<Point>> contour;
	findContours(bin, contour, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<vector<Point>> contour_poly(contour.size());
	vector<Rect> boundRect(contour.size());
	
	for (size_t i = 0; i < contour.size(); i++) {
		approxPolyDP(Mat(contour[i]), contour_poly[i], 55, true);
		
		boundRect[i] = boundingRect(Mat(contour_poly[i]));
		if (
			boundRect[i].size().height > CODE_SIZE &&
			boundRect[i].size().width > CODE_SIZE &&
			abs(boundRect[i].size().width - boundRect[i].height) < 15 &&
			contour_poly[i].size() == 4
			)
		{
			for (int j = 0; j < 4; j++)cntr_point[j] = contour_poly[i][j];
			drawContours(srt, contour_poly, (int)i, Scalar(0, 0, 255), 1, 8, vector<Vec4i>(), 0, Point());
			break;
		}
	}
	cntr_point = pointCorrecting(cntr_point);

	return 
		cntr_point;
}

bool VC_Scanner::ContourAreaSortPredicate(const vector<Point> c1, const vector<Point> c2) {
	return contourArea(c1) > contourArea(c2);
}

bool VC_Scanner::find_Finder(uchar* data, double blockSize, double _offset, double center, int locate)
{
	
	if (blockSize == 0) 
		return false; 
	

	int ii = 0, jj = 0;
	switch (locate) 
	{
	case LEFTUP:
	{
		int ii = 0, jj = 0;
		for (double i = center + _offset; i < blockSize * 7 + _offset; i += blockSize) {
			for (double j = center + _offset; j < blockSize * 7 + _offset; j += blockSize) {
				if (data[(int)round(round(i)*width + j)] != Finder[ii][jj])
					break;
				if (ii == 6 && jj == 6)
					return true;
				jj++;
			}
			if (jj < 7)
				return false;
			ii++;
			jj = 0;
		}
		break;
	}
	case LEFTDOWN: {
		int ii = 0, jj = 0;
		for (double i = width - blockSize * 7 - _offset + center; i < width - _offset; i += blockSize) {
			for (double j = center + _offset; j < blockSize * 7 + _offset + center; j += blockSize) {
				if (data[(int)round(round(i)*width + j)] != Finder[ii][jj])
					break;
				if (ii == 6 && jj == 6)
					return true;
				jj++;
			}
			if (jj < 7)
				return false;
			ii++;
			jj = 0;
		}
		break;
	}
	case RIGHTDOWN: {
		int ii = 0, jj = 0;
		for (double i = width - blockSize * 7 - _offset + center; i < width - _offset-1; i += blockSize) {
			for (double j = width - blockSize * 7 - _offset + center; j <width - _offset-1; j += blockSize) {
				if (data[(int)round(round(i)*width + j)] != Finder[ii][jj])
					break;
				if (ii == 6 && jj == 6)
					return true;
				jj++;
			}
			if (jj < 7) {
				return false;
			}
			ii++;
			jj = 0;
		}
		
		break;
	}
	case RIGHTUP: {
		int ii = 0, jj = 0;
		for (double i = center + _offset; i < blockSize * 7 + _offset + center; i += blockSize) {
			for (double j = width - blockSize * 7 - _offset + center; j < width - _offset; j += blockSize) {
				if (data[(int)round(round(i)*width + j)] != Finder[ii][jj])
					break;
				if (ii == 6 && jj == 6)
					return true;
				jj++;
			}
			if (jj < 7)
				return false;
			jj = 0;
			ii++;
		}
		break;
	}
	}
	return false;
}
double VC_Scanner::calOffset(uchar * data, double _offset)
{
	
	int n = 0;
	double offset_temp = 0;
	for (int i = (int)ceil(_offset); i < _offset * 7; i++) {
		int comp = 0;
		int j = 0;
		if (data[i*(int)width] == ZERO_CROSSING(comp)) {
			comp = !comp;
			for (; data[i*(int)width + j] != ZERO_CROSSING(comp); j++);
		}
		comp = 0;
	
		for (; data[i*(int)width + j] != ZERO_CROSSING(comp); j++);
		
		if (j >= _offset*2 || j <= _offset*0.5)continue;

		offset_temp += j;
		n++;
		
	}
	for (int j = (int)ceil(_offset); j < _offset * 7; j++) {
		int comp = 0;
		int i = 0;
		if (data[i*(int)width + j] == ZERO_CROSSING(comp)) {
			comp = !comp;
			for (; data[i*(int)width + j] != ZERO_CROSSING(comp); i++);
		}
		comp = 0;

		for (; data[i*(int)width + j] != ZERO_CROSSING(comp); i++);

		if (i >= _offset * 2 || i <= _offset * 0.5)continue;

		offset_temp += i;
		n++;

	}
	offset_temp /= n;
	
	return offset_temp;
}
double VC_Scanner::calBlockSize(uchar* data, double width)
{
	Point2i finderPos[7];
	
	int comp = 1;
	int count = 0;
	for (int i = 0; i < width / 2 ; i++) {
		if (data[i*(int)width + i] == ZERO_CROSSING(comp)) {
			comp = !comp;
			finderPos[count++] = Point2i(i,i);
		}
		if (count == 7) {
			double size = processDiagonal(finderPos);
			double _offset = sqrt(pow(abs(finderPos[0].x - finderPos[1].x), 2) + pow(abs(finderPos[0].y - finderPos[1].y), 2)) / sqrt(2)*(((double)rand() / RAND_MAX)*0.05 + 0.975);
			//_offset *= (((double)rand() / RAND_MAX)*0.05 + 0.975);
			
			if (find_Finder(data, size, _offset, size / 2, LEFTUP)) {
				offset = _offset;
				finderLocate = LEFTUP;
				return size;
			}
			else {
				break;
			}
		}
	}
	comp = 1;
	count = 0;
	for (int i = (int)width, j = 0; i > width / 2 ; i--, j++) {
		if (data[i*(int)width + j] == ZERO_CROSSING(comp)) {
			comp = !comp;
			finderPos[count++] = Point2i(i,j);
		}
		if (count == 7) {
			double size = processDiagonal(finderPos);
			double _offset = sqrt(pow(abs(finderPos[0].x - finderPos[1].x), 2) + pow(abs(finderPos[0].y - finderPos[1].y), 2)) / sqrt(2);
			//_offset *= (((double)rand() / RAND_MAX)*0.05 + 0.975);
			if (find_Finder(data, size, _offset, size / 2, LEFTDOWN)) {
				offset = _offset;
				finderLocate = LEFTDOWN;
				return size;
			}
			else
				break;
		}
	}
	comp = 1;
	count = 0;
	for (int i = 0, j = (int)width; i < width / 2 ; i++, j--) {
		if (data[i*(int)width + j] == ZERO_CROSSING(comp)) {
			comp = !comp;
			finderPos[count++] = Point2i(i, j);
		}
		if (count == 7) {
			double size = processDiagonal(finderPos);
			double _offset = sqrt(pow(abs(finderPos[0].x - finderPos[1].x), 2) + pow(abs(finderPos[0].y - finderPos[1].y), 2)) / sqrt(2);
			//_offset *= (((double)rand() / RAND_MAX)*0.05 + 0.975);
			if (find_Finder(data, size, _offset, size / 2 ,RIGHTUP)) {
				offset = _offset;
				finderLocate = RIGHTUP;
				return size;
			}
			else
				break;
		}
	}
	comp = 1;
	count = 0;
	for (int i = (int)width; i > width / 2 ; i--) {
		if (data[i*(int)width + i] == ZERO_CROSSING(comp)) {
			comp = !comp;
			finderPos[count++] = Point2i(i, i);
		}
		if (count == 7) {
			double size = processDiagonal(finderPos);
			double _offset = sqrt(pow(abs(finderPos[0].x - finderPos[1].x), 2) + pow(abs(finderPos[0].y - finderPos[1].y), 2)) / sqrt(2);
			//_offset *= (((double)rand() / RAND_MAX)*0.05 + 0.975);

			if (find_Finder(data, size, _offset, size / 2 ,RIGHTDOWN)) {
				offset = _offset;
				finderLocate = RIGHTDOWN;
				return size;
			}
			else {
				return 0;
			}
		}
	}
	return 0;
}



double VC_Scanner::processDiagonal(Point2i* finderPos)
{
	double lines[6], len = 0;
	lines[0] = sqrt(pow(abs(finderPos[1].x - finderPos[2].x), 2) + pow(abs(finderPos[1].y - finderPos[2].y), 2));
	lines[1] = sqrt(pow(abs(finderPos[2].x - finderPos[3].x), 2) + pow(abs(finderPos[2].y - finderPos[3].y), 2));
	lines[2] = sqrt(pow(abs(finderPos[3].x - finderPos[4].x), 2) + pow(abs(finderPos[3].y - finderPos[4].y), 2)) / 3;
	lines[3] = sqrt(pow(abs(finderPos[4].x - finderPos[5].x), 2) + pow(abs(finderPos[4].y - finderPos[5].y), 2));
	lines[4] = sqrt(pow(abs(finderPos[5].x - finderPos[6].x), 2) + pow(abs(finderPos[5].y - finderPos[6].y), 2));
	lines[5] = sqrt(pow(abs(finderPos[1].x - finderPos[6].x), 2) + pow(abs(finderPos[1].y - finderPos[6].y), 2)) / 7; 

	for (int i = 0; i < 6; i++)
		len += lines[i];
	return ((len / 6) / sqrt(2));
}

double VC_Scanner::blockSizeCorrecting(uchar* data, double blockSize, double _offset, double width)
{
	
	int comp;
	int nblock = 0;
	
	double pos = _offset ; 
	double *blockSizeData = (double*)malloc(sizeof(double));
	double currentX = _offset;
	
	for (double i = _offset+ blockSize; i < width - _offset- blockSize; i += blockSize) {
		data[(int)round(round(i)*width + round(_offset))] == 255 ? comp = 0 : comp = 1;
		for (double j = _offset + blockSize; j < width - _offset- blockSize; j++) {
			if (data[(int)round(round(i)*width + round(j))] == ZERO_CROSSING(comp)) {
				double n = round((abs(currentX - j)) / blockSize);
				if (n != 0 && abs(currentX - j) / n <= blockSize * 1.4&&abs(currentX - j) / n >= blockSize * 0.6) {
					blockSizeData[nblock++] = (abs(currentX - j) / n)*(((double)rand() / RAND_MAX)*0.125 + 0.9375);
					blockSizeData = (double*)realloc(blockSizeData, sizeof(double)*(nblock + 2));
					currentX = j;
					comp = !comp;
				}
			}
		}
		currentX = _offset;
	}

	//data[(int)round(round(pos)*width + round(_offset))] == 255 ? comp = 1 : comp = 0;
	//for (double i = _offset; i < width - _offset; i++) {
	//	if (data[(int)round(round(pos)*width + round(i))] == ZERO_CROSSING(comp)) {
	//		double n = round((abs(currentPos - i)) / blockSize);
	//		if (n != 0 && abs(currentPos - i) / n > blockSize * 4 / 5) {
	//			//printf("+%.4lf\n", abs(currentPos - i) / n);
	//			blockSizeData[nblock] = abs(currentPos - i) / n;
	//			nblock++;
	//			currentPos = i;
	//			comp = !comp;
	//		}
	//	}
	//}

	//currentPos = _offset;
	//data[(int)round(round(width-pos)*width + round(_offset))] == 255 ? comp = 1 : comp = 0;
	//for (double i = _offset; i < width - _offset; i++) {
	//	if (data[(int)round(round(width - pos)*width + round(i))] == ZERO_CROSSING(comp)) {
	//		double n = round((abs(currentPos - i)) / blockSize);			
	//		if (n != 0 && abs(currentPos - i) / n > blockSize * 4 / 5) {
	//			blockSizeData[nblock] = abs(currentPos - i) / n;
	//			nblock++;
	//			currentPos = i;
	//			comp = !comp;
	//		}
	//	}
	//}

	//currentPos = _offset;
	//data[(int)round(round(_offset)*width + round(pos))] == 255 ? comp = 1 : comp = 0;
	//for (double i = _offset; i < width - _offset; i++) {
	//	if (data[(int)round(round(i)*width + round(pos))] == ZERO_CROSSING(comp)) {
	//		double n = round((abs(currentPos - i)) / blockSize);		
	//		if (n != 0 && abs(currentPos - i) / n > blockSize * 4 / 5) {
	//			blockSizeData[nblock] = abs(currentPos - i) / n;
	//			nblock++;
	//			currentPos = i;
	//			comp = !comp;
	//		}
	//	}
	//}

	//currentPos = _offset;
	//data[(int)round(round(_offset)*width + round(width-pos))] == 255 ? comp = 1 : comp = 0;
	//for (double i = _offset; i < width - _offset; i++) {
	//	if (data[(int)round(round(i)*width + round(width-pos))] == ZERO_CROSSING(comp)) {
	//		double n = round((abs(currentPos - i)) / blockSize);			
	//		if (n != 0 && abs(currentPos - i) / n > blockSize * 4 / 5) {
	//			blockSizeData[nblock] = abs(currentPos - i) / n;
	//			nblock++;
	//			currentPos = i;
	//			comp = !comp;
	//		}
	//	}
	//}
	blockSizeData[nblock] = blockSize;
	double newBlockSize = blockSize_RANSAC(blockSizeData, nblock);
	free(blockSizeData);
	if (newBlockSize != 0) {
		offset = newBlockSize;
		return newBlockSize;
	}
	else 
		return blockSize;
}

double VC_Scanner::blockSize_RANSAC(double * blockSizeData, int nBlocks)
{
	int best_Inliers = 0;
	double corrected_BlockSize = 0;
	int r0 = 0;
	int r1 = 0;
	double variance = 0;
	double average = 0;
	double sigma = 0;
	
	for (int i = 0; i < nBlocks; i++)
		average += blockSizeData[i];
	average /= nBlocks;

	for (int i = 0; i < nBlocks; i++) 
		sigma += (blockSizeData[i] - average) * (blockSizeData[i] - average);

	variance = sqrt(sigma / nBlocks);
	//printf("average : %.4lf\nvariance : %.4lf\n", average, variance);

	// log(p - 1) / log(1 - inlier percentage ^ samples) p = 0.999, inlier = 50%, samples = 2
	int round = 25;
	for (int i = 0; i < round; i++) {

		r0 = isaac_next_uint(&isaac, nBlocks);
		do {
			r1 = isaac_next_uint(&isaac, nBlocks);
		} while (r0 == r1);
		/*r0 = (int)((double)rand() / RAND_MAX * nBlocks);
		do {
			r1 = (int)((double)rand() / RAND_MAX * nBlocks);
		}while (r0 == r1);
		*/
		double average_Temp = (blockSizeData[r0] + blockSizeData[r1]) / 2;
		int inliers = 0;
		for (int j = 0; j < nBlocks; j++) {
			//if (j + 1 == nBlocks)break;
			double blockSizeTemp = blockSizeData[j] ;
			double residual = abs(average_Temp - blockSizeTemp) / average_Temp;
			if (residual * 100 < 3 * variance) {
				inliers++;
			}
		}
		if (inliers > best_Inliers) {
			best_Inliers = inliers;
			corrected_BlockSize = average_Temp;
		}
	}
	//printf("best Inliers : %d\ncorrectedBlockSize:%.4lf\n", best_Inliers, corrected_BlockSize);
	return corrected_BlockSize;
}

__inline uchar VC_Scanner::getBlock(uchar * data, double x, double y, double center, double width)
{
	uchar p1, p2, p3, p4, p5, p6, p7, p8, p9;

	int block;
	double _offset = center / 3;

	p1 = data[(int)round(round(y - _offset)*width + round(x - _offset))];
	p2 = data[(int)round(round(y - _offset)*width + round(x))];
	p3 = data[(int)round(round(y - _offset)*width + round(x + _offset))];

	p4 = data[(int)round(round(y)*width + round(x - _offset))];
	p5 = data[(int)round(round(y)*width + round(x))];
	p6 = data[(int)round(round(y)*width + round(x + _offset))];

	p7 = data[(int)round(round(y + _offset)*width + round(x - _offset))];
	p8 = data[(int)round(round(y + _offset)*width + round(x))];
	p9 = data[(int)round(round(y + _offset)*width + round(x + _offset))];

	block = p1 + p2 + p3 + p4 + p6 + p7 + p8 + p9;
	if (block >= 255 * 5 && p5 == 255)
		return 0;
	else 
		return 1;
}

__inline uchar VC_Scanner::demasking(int i, int j, unsigned char mask)
{
	switch (mask) {
	case 0:return (((i + j) % 2) == 0);
	case 1:return ((j % 2) == 0);
	case 2:return ((i % 3) == 0);
	case 3:return (((i + j) % 3) == 0);
	case 4:return (((j / 2 + i / 3) % 2) == 0);
	case 5:return (((i*j) % 2 + (i*j) % 3) == 0);
	case 6:return ((((i*j) % 3 + i * j) % 2) == 0);
	case 7:return ((((i + j) % 2 + i * j%3) % 2) == 0);
	default:return 0;
	}
}

bool VC_Scanner::config(Mat warp,int mode)
{
	Mat gray, bin;
	cvtColor(warp, gray, CV_BGR2GRAY);
	if (mode == 0)
		threshold(gray, bin, 255, 255, THRESH_OTSU);
	else 
		cv::adaptiveThreshold(gray, bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 31, 1);
	
	if ((blockSize = calBlockSize((uchar*)bin.data, bin.rows)) > 0) {

		blockSize = blockSizeCorrecting((uchar*)bin.data, blockSize, offset, width);
		if ((centerLocate = blockSize / 2) > 0) {
			switch (finderLocate) {

			case LEFTUP: {
				Point2f pt = Point2f(bin.rows / 2 - 1, bin.cols / 2 - 1);
				Mat rotate = cv::getRotationMatrix2D(pt, 0, 1);
				Mat rot;
				cv::warpAffine(bin, rot, rotate, bin.size());
				warpFrame = rot;
				return true;
			}

			case LEFTDOWN: {
				Point2f pt = Point2f(bin.rows / 2 - 1, bin.cols / 2 - 1);
				Mat rotate = cv::getRotationMatrix2D(pt, -90, 1);
				Mat rot;
				cv::warpAffine(bin, rot, rotate, bin.size());
				warpFrame = rot;
				return true;
			}

			case RIGHTUP: {
				Point2f pt = Point2f(bin.rows / 2 - 1, bin.cols / 2 - 1);
				Mat rotate = cv::getRotationMatrix2D(pt, 90, 1);
				Mat rot;
				cv::warpAffine(bin, rot, rotate, bin.size());
				warpFrame = rot;

				return true;
			}

			case RIGHTDOWN: {
				Point2f pt = Point2f(bin.rows / 2 - 1, bin.cols / 2 - 1);
				Mat rotate = cv::getRotationMatrix2D(pt, 180, 1);
				Mat rot;
				cv::warpAffine(bin, rot, rotate, bin.size());
				warpFrame = rot;

				return true;
			}

			case UNDEFINED: {
				return false;
			}

			default:
				return false;
			}
		}
		else 
			return false;

	}
	else
		return false;
}

void VC_Scanner::decodeData(VC_Data* vcData, uchar * d, Point2i endPoint)
{
	int dataLength = vcData->getDataLength();
	uchar* vc_data = (uchar*)malloc(sizeof(uchar)*dataLength / 8);
	for (int i = 0; i < dataLength / 8; i++)vc_data[i] = 0;
	unsigned int *dataBuffer = (unsigned int*)malloc(sizeof(int)*dataLength);
	for (int i = 0; i < dataLength; i++)dataBuffer[i] = 0;
	int bitCount = 0;
	int x = 0, y = 0;
	int mask = vcData->getMask();
	for (double i = centerLocate + offset; i < width - offset; i += blockSize, y++) {
		for (double j = centerLocate + offset; j < width - offset; j += blockSize, x++) {
			if (y > 7 || ((y<8) && ((y >= endPoint.y&&x > endPoint.x) || (y >= 2 && x >= 8))))
				dataBuffer[bitCount++] = ((getBlock(d, j, i, centerLocate, width) ^ demasking(x, y, mask)));
			if (bitCount == dataLength) break;
		}
		if (bitCount == dataLength) break;
		x = 0;
	}
	for (int i = 0; i < bitCount / 8; i++) {
		vc_data[i] = 0;
		int bit = 7;
		for (int j = 0; j < 8; j++, bit--) {
			vc_data[i] += dataBuffer[i * 8 + j] * (int)pow(2, bit);
		}
	}
	unsigned int crc = 0xffffffff;
	for (int i = 0; i < bitCount / 8; i++) {
		crc = (crc >> 8) ^ crc_Table[(crc^vc_data[i]) & 0xff];
	}
	crc = crc ^ 0xffffffff;
	if (crc == vcData->getHeader().crc_32) {
		string temp;
		for (int i = 0; i < dataLength / 8; i++)temp += (char)vc_data[i];
		vcData->setData(temp);
	}
	
	free(vc_data);
	free(dataBuffer);
}

Point2i VC_Scanner::decodeHeader(VC_Data* vcData, uchar* data)
{
	int bitCount = 0;
	int x = 0, y = 0;
	int headerLength = 3 + 13 + 32 + 16;	//mask + dataLength + crc_32 + checksum
	int *headerBuffer = (int*)calloc(headerLength,sizeof(int));
	Point2i endPoint = Point2i(0, 0);
	double head = centerLocate + offset + blockSize * 8;
	for (double i = centerLocate + offset; i < width - offset; i += blockSize, y++) {
		for (double j = centerLocate + offset; j < width - offset; j += blockSize, x++) {
			if (j > head)
				headerBuffer[bitCount++] = getBlock(data, j, i, centerLocate, width);
			
			if (bitCount >= headerLength) {
				endPoint.x = x;
				endPoint.y = y;
				break;
			}
		}
		if (bitCount >= headerLength)break;
		x = 0;
	}
	int i = 0;
	int bit = 3 - 1;
	int mask = 0;
	for (; i < 3; i++, bit--)
		mask += headerBuffer[i] * (int)pow(2, bit);

	int dataLength = 0;
	bit = 13 - 1;
	for (; i < 16; i++, bit--)
		dataLength += headerBuffer[i] * (int)pow(2, bit);

	unsigned int crc_32 = 0;
	bit = 32 - 1;
	for (; i < 48; i++, bit--) 
		crc_32 += headerBuffer[i] * (int)pow(2, bit);
	
	int header_checkSum = 0;
	bit = 16 - 1;
	for (; i < 64; i++, bit--) 
		header_checkSum += headerBuffer[i] * (int)pow(2, bit);
	
	long checksum = vcData->checkSum(mask,dataLength,crc_32);

	free(headerBuffer);
	if (checksum == header_checkSum) {
		vcData->setHeader(mask, dataLength, crc_32);
		return endPoint;
	}
	else
		return Point(0, 0);
}

string VC_Scanner::scan()
{
	Mat warp;
	int c = 0;
	if (!(warp = imageWarp()).empty() && config(warp, 0)) {
		
		rawData = (uchar*)warpFrame.data;
		vcData = VC_Data();
		//offset = blockSize ;
		Point2i end;
		if (beforeBlockSize > 0) {
			c++;
			rawData = (uchar*)warpFrame.data;
			vcData = VC_Data();
			blockSize = beforeBlockSize;
			offset = beforeOffset;
			if ((end = decodeHeader(&vcData, rawData)) == Point(0, 0))goto next1;
			else {
				decodeData(&vcData, rawData, end);
next1:			if (vcData.getData().length() == 0 && config(warp, 1)) {
				rawData = (uchar*)warpFrame.data;
				vcData = VC_Data();
				c++;
				//offset = blockSize ;
				if ((end = decodeHeader(&vcData, rawData)) == Point(0, 0) && config(warp, 0)) {
					c++;
					rawData = (uchar*)warpFrame.data;
					vcData = VC_Data();
					goto next2;
				}
}
				return vcData.getData();
				
			}
		}
		else {
next2:		if ((end = decodeHeader(&vcData, rawData)) == Point(0, 0)) {
			goto next3;
}
			else {
				decodeData(&vcData, rawData, end);
next3:			if (vcData.getData().length() == 0 && config(warp, 1)) {
					rawData = (uchar*)warpFrame.data;
					vcData = VC_Data();
					//offset = blockSize ;
					if ((end = decodeHeader(&vcData, rawData)) == Point(0, 0))
						return string();
					else
						decodeData(&vcData, rawData, end);
				}
				return vcData.getData();
			}
		}
	}
	else
		return string();
}