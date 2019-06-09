#pragma once
#include "header.h"



class handpos
{
private :

	//opencv 이용해서 손 영역 검출하는 함수들 모음

	//변수
	Mat frame;

	vector <vector<Point>> Contours;

	vector<vector<Point>>_hull;
	vector<vector<int> > _hullsI;
	vector<vector<Vec4i>> _defects;
	//제일 긴 컨투어 인덱스 
	int LongestContour;

	//골라진 컨투어/defect
	vector<Point> _selectContours;
	vector<Vec4i> _selectdefects;
	vector<Point>_selecthull;

	Point centerPoint; //손바닥 중심 위치 
	double maxdist;
	int FmaxdistIndex;

	vector<Point> _Fars;
	Point hull_center;
	Point FarCenter;
	Point SECenter;
	Point SECenter_adv;
	int whatline[200];
	
	handPosition result;


	//이미지 구멍 메꾸기
	void cvFillHoles(Mat& input)
	{
		cv::Mat image = input;

		cv::Mat image_thresh;
		cv::threshold(image, image_thresh, 125, 255, cv::THRESH_BINARY);

		// Loop through the border pixels and if they're black, floodFill from there
		cv::Mat mask;
		image_thresh.copyTo(mask);
		for (int i = 0; i < mask.cols; i++) {
			if (mask.at<char>(0, i) == 0) {
				cv::floodFill(mask, cv::Point(i, 0), 255, 0, 10, 10);
			}
			if (mask.at<char>(mask.rows - 1, i) == 0) {
				cv::floodFill(mask, cv::Point(i, mask.rows - 1), 255, 0, 10, 10);
			}
		}
		for (int i = 0; i < mask.rows; i++) {
			if (mask.at<char>(i, 0) == 0) {
				cv::floodFill(mask, cv::Point(0, i), 255, 0, 10, 10);
			}
			if (mask.at<char>(i, mask.cols - 1) == 0) {
				cv::floodFill(mask, cv::Point(mask.cols - 1, i), 255, 0, 10, 10);
			}
		}


		// Compare mask with original.
		cv::Mat newImage;
		image.copyTo(newImage);
		for (int row = 0; row < mask.rows; ++row) {
			for (int col = 0; col < mask.cols; ++col) {
				if (mask.at<char>(row, col) == 0) {
					newImage.at<char>(row, col) = 255;
				}
			}
		}
		//cv::imshow("filled image", mask);
		input = newImage;
	}

	bool findConvex(const Mat & mask)
	{
		findContours(mask, Contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
		if (Contours.size() == 0)
		{
			return false;
		}
			
		vector<vector<Point>>hull(Contours.size());
		vector<vector<int> > hullsI(Contours.size()); // Indices to contour points
		vector<vector<Vec4i>> defects(Contours.size());

		LongestContour = 0;
		int countmaxsize = 0;
		int countover3 = 0;
		//컨벡스 쓰기
		for (int i = 0; i < Contours.size(); i++)
		{
			//제일 긴 컨투어 라인 찾기 
			convexHull(Contours[i], hull[i], false);
			convexHull(Contours[i], hullsI[i], false);
			if (hullsI[i].size() > 3) // You need more than 3 indices          
			{
				convexityDefects(Contours[i], hullsI[i], defects[i]);

				if (countmaxsize < Contours[i].size())
				{
					countmaxsize = Contours[i].size();
					LongestContour = i;
				}
				countover3++;
			}
		}
		if (countover3 == 0)
		{
			return false;
		}

		//전역변수화
		_hull = hull;
		_hullsI = hullsI;
		_defects = defects;

		_selectContours = Contours[LongestContour];
		_selectdefects = defects[LongestContour];
		_selecthull = hull[LongestContour];

		return true;
	}

	bool getRealcenterPoint()
	{
		int sumFarCenterX = 0, sumFarCenterY = 0;
		int sumSECenterX = 0, sumSECenterY = 0;

		int count = 0;
		Point selectFars[20] = {Point(0,0)};


		for (int i = 0; i < _selectdefects.size(); i++)
		{

			const Vec4i& v = _selectdefects[i];
			float depth = v[3];
			if (depth > 1500 && count < 20) //  filter defects by depth, e.g more than 10
			{
				int startidx = v[0]; Point ptStart(_selectContours[startidx]);
				int endidx = v[1]; Point ptEnd(_selectContours[endidx]);
				int faridx = v[2]; Point ptFar(_selectContours[faridx]);

				selectFars[count] = ptFar;
				sumFarCenterX += ptFar.x;
				sumFarCenterY += ptFar.y;

				sumSECenterX += (ptStart.x + ptEnd.x);
				sumSECenterY += (ptStart.y + ptEnd.y);

				count++;
			}
		}
		if (count < 5)
		{
			cout << "깊이 모자람";
			return false;
		}
		if (count > 15)
		{
			cout << "많아;";
			return false;
		}
		FarCenter.x = sumFarCenterX / count;
		FarCenter.y = sumFarCenterY / count;

		SECenter.x = sumSECenterX / (count * 2);
		SECenter.y = sumSECenterY / (count * 2);

		vector<Point> Fars(count);

		maxdist = 0;
		int Fdistance = 0;
		int FmaxdistIndex = 0;

		for (int i = 0; i < count; i++)
		{
			Fars[i] = selectFars[i];

			//하는김에 평균과 가장 먼 거리의 점 구하기
			Fdistance = sqrt(pow(Fars[i].x - FarCenter.x, 2) + pow(Fars[i].y - FarCenter.y, 2));
			if (Fdistance > maxdist)
			{
				FmaxdistIndex = i;
				maxdist = Fdistance;
			}
		}
		_Fars = Fars;
	}

	handPosition GetRealConvexhull(Mat & input)
	{
		int x = 0, y = 0;

		float scale = 1.2; // 교차점이 0개면 늘려야 함

		cout << _selecthull.size();

		for (int i = 0; i < _selecthull.size(); i++)
		{
			x += _selecthull[i].x;
			y += _selecthull[i].y;
		}

		hull_center.x = x / _selecthull.size();
		hull_center.y = y / _selecthull.size();


		//교차점 구하기 
		vector<Point> crossPoints(200);


		int rimit = 10;//무한방지용 상수 - 손목 없을 수도 있으니까

		int crosscount = checkcross(scale, crossPoints);
		int over120 = 0; //120도 넘는게 2개는 있어야 통과

		while (crosscount < 2)
		{
			scale += 0.3;
			crosscount = checkcross(scale, crossPoints);
			rimit--;
			if (rimit == 0) break;
		}
		if (rimit != 0)
		{
			rimit = 10; //한계상수 초기화
			for (int i = 0; i < crossPoints.size(); i++)
			{
				if (findangle(crossPoints[i], hull_center, FarCenter) > 120) over120++;
			}
			//만족할때까지 scale 늘리기
			while (over120 < 2)
			{
				scale += 0.3;
				over120 = 0;

				crossPoints.resize(2000);
				checkcross(scale, crossPoints);
				for (int i = 0; i < crossPoints.size(); i++)
				{
					if (findangle(crossPoints[i], hull_center, FarCenter) > 120) over120++;
				}

				rimit--;
				if (rimit == 0) {
					break;
				}
			}
		}


		vector<Point> cut(_selecthull.size() + 1);
		RotatedRect rect;
		if (rimit != 0)
		{
			cutcunvexhull(crossPoints, cut);
			rect = minAreaRect(cut);
		}
		else
		{
			rect = minAreaRect(_selecthull);
		}
		Point2f vertices[4];
		rect.points(vertices);

		handPosition rst;
		rst.center = FarCenter; //이거 나중에~
		rst.rotateXY = rect.angle;
		for (int i = 0; i < 4 ; i++)
		{
			rst.vertices[i] = vertices[i];
		}

		//사이즈 지정
		double dst1 =
			sqrt((vertices[0].x - vertices[1].x * vertices[0].x - vertices[1].x) + (vertices[0].y - vertices[1].y * vertices[0].y - vertices[1].y));
		double dst2 =
			sqrt((vertices[1].x - vertices[2].x * vertices[1].x - vertices[2].x) + (vertices[1].y - vertices[2].y * vertices[1].y - vertices[2].y));

		if (dst1 >= dst2)
		{
			rst.Llength = dst1;
			rst.Slength = dst2;
		}
		else
		{
			rst.Llength = dst2;
			rst.Slength = dst1;
		}

		return rst;
	}
	

	int checkcross(float scale, vector<Point> & crossPoints)
	{
		Point prevPoint;
		Point nowPoint;

		int countcross = 0;

		for (int j = 0; j < _selecthull.size(); j++)
		{
			if (j == 0)
			{
				prevPoint = _selecthull[_selecthull.size() - 1];
				nowPoint = _selecthull[j];
			}
			else
			{
				prevPoint = _selecthull[j - 1];
				nowPoint = _selecthull[j];
			}

			Point rst[2];
			int noa;
			bool chk = findcrossPoint(prevPoint, nowPoint, FarCenter, maxdist * scale, noa, rst);
			if (chk)
			{
				if (noa == 1)
				{
					whatline[countcross] = j; //어느 점 전인지 적어두기...
					crossPoints[countcross] = rst[0];
					countcross++;
				}
				if (noa == 2)
				{
					crossPoints[countcross] = rst[0];
					whatline[countcross] = j; //어느 점 전인지 적어두기...

					crossPoints[countcross + 1] = rst[1];
					whatline[countcross + 1] = j; //어느 점 전인지 적어두기...

					countcross = countcross + 2;
				}
			}
		}
		crossPoints.resize(countcross);
		return countcross;
	}

	//원과의 교차점 구하기
	bool findcrossPoint(Point start, Point end, Point center, float radius, int& numOfAns, Point rst[2])
	{
		//a와 b로 이루어진 선분과 center이 중심이고 radius 가 반지름인 원과의 교차점 구하기
		numOfAns = 0;

		float degree = float((end.y - start.y)) / float((end.x - start.x));			//기울기

		float constv = start.y - degree * start.x; // 상수

		float a = 1 + degree * degree;
		float b = -2 * (center.x + degree * (center.y - constv));
		float c = center.x * center.x + (center.y - constv) * (center.y - constv) - radius * radius;

		if ((b * b - 4 * a * c) < 0) // 허근
		{
			return false;
		}

		float ans1 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
		float ans2 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);

		//접선일 때
		if ((b * b - 4 * a * c) == 0) // 중근
		{
			numOfAns = 1;
			rst[0].x = ans1;
			rst[0].y = degree * ans1 + constv;
			return true;
		}

		//근이 두 개일 때
		//근이 진짜인지 판별
		//교점이어야하므로 x좌표가 두 점 사이에 있어야함

		if (((start.x - ans1) * (end.x - ans1)) < 0)
		{
			rst[0].x = ans1;
			rst[0].y = degree * ans1 + constv;
			numOfAns++;
		}
		if (((start.x - ans2) * (end.x - ans2)) < 0)
		{
			rst[numOfAns].x = ans2;
			rst[numOfAns].y = degree * ans2 + constv;
			numOfAns++;
		}
		if (numOfAns == 0)
		{
			return false; // 원 안 or 원 밖의 점
		}

		return true;
	}

	double findangle(Point a, Point b, Point center)
	{
		Point newA(a.x - center.x, a.y - center.y);
		Point newB(b.x - center.x, b.y - center.y);

		double angleA = atan(newA.y / double(newA.x));
		double angleB = atan(newB.y / double(newB.x));

		angleA = angleA * 180 / M_PI;
		angleB = angleB * 180 / M_PI;

		//4분면 나누기
		if (newA.x < 0) angleA = 180 + angleA;
		else if (newA.y < 0) angleA = 360 + angleA;

		if (newB.x < 0) angleB = 180 + angleB;
		else if (newB.y < 0) angleB = 360 + angleB;

		//차 구하기
		double rst = abs(angleA - angleB);
		if (rst > 180) rst = 360 - rst;

		return rst;
	}

	void cutcunvexhull(vector<Point>crossPoints, vector<Point> & rstList)
	{

		for (int i = 0; i < _selecthull.size(); i++)
		{
			rstList[i] = _selecthull[i];
		}

		//각도(120 이상)제일 큰 거 두개 고르기 (없음말기)
		//if 140 이상인게 세 개라면 그중 양 끝거로 <- 나중에
		//직선방정식 세우기
		//점 체킹 -> 뺄 거 빼기
		//자른 점 집어넣기
		int over120 = 0;
		int maxAngleindex[2] = { 0,0 }; // 제일 큰 거 두개 
		double maxAnglevalue[2] = { 0,0 };

		double angle;
		for (int i = 0; i < crossPoints.size(); i++)
		{
			angle = findangle(crossPoints[i], hull_center, FarCenter);
			if (angle > 120)
			{
				if (angle > maxAnglevalue[0])
				{
					maxAnglevalue[1] = maxAnglevalue[0];
					maxAnglevalue[0] = angle;
					maxAngleindex[1] = maxAngleindex[0];
					maxAngleindex[0] = i;
				}
				else if (angle > maxAnglevalue[1])
				{
					maxAngleindex[1] = i;
					maxAnglevalue[1] = angle;
				}
				over120++;
			}
		}
		if (over120 < 2)
		{
			rstList.resize(_selecthull.size());
			return;
		}


		//직선방정식 세우기 
		Point max1 = crossPoints[maxAngleindex[0]];
		Point max2 = crossPoints[maxAngleindex[1]];

		double degree = (max2.y - max1.y) / double((max2.x - max1.x));
		double constv = max1.y - degree * max1.x;

		//자르는 점의 행렬상 위치 파악 -만약 여기서 같은 선 위에 있다면 그냥 스킵해버린다
		int nextindex1 = whatline[maxAngleindex[0]];
		int nextindex2 = whatline[maxAngleindex[1]];
		if (nextindex1 == nextindex2) return;

		//센터의 위치 파악
		double up = hull_center.y - (hull_center.x * degree + constv); // 양수면 위에 있음
		double isup;
		int listcount = 0;
		for (int i = 0; i < _selecthull.size(); i++)
		{
			//새 점 넣기
			if (i == nextindex1)
			{
				rstList[listcount] = max1;
				listcount++;
			}
			else if (i == nextindex2)
			{
				rstList[listcount] = max2;
				listcount++;
			}

			//기존 점 고르기
			isup = _selecthull[i].y - (_selecthull[i].x * degree + constv);
			if (up * isup > 0) // 같은 방향
			{
				rstList[listcount] = _selecthull[i];
				listcount++;
			}
		}
		rstList.resize(listcount);
	}


public:
	bool getHandPosition(Mat frame, handPosition &result)
		{
			Mat YCrCbframe;
			vector<Mat> planes;
			Mat usemask;

			Mat eroded;
			Mat closed;

			//업그레이드 가능한 부분... 상수값으로 안하고 조정하기
			int minCr = 133; //128 
			int maxCr = 175;
			int minCb = 78; //73
			int maxCb = 134;

			//opencv를 이용해서 중점과 사각형을 구한다
			cvtColor(frame, YCrCbframe, COLOR_RGB2YCrCb);
			split(YCrCbframe, planes);
			usemask = (minCr < planes[1]) & (planes[1] < maxCr) & (minCb < planes[2]) & (planes[2] < maxCb);

			//모폴로지 연산 클로즈 > 이로드 
			morphologyEx(usemask, closed, MORPH_CLOSE, Mat(5, 5, CV_8U, Scalar(1)));
			erode(closed, eroded, Mat(3, 3, CV_8U, Scalar(1)), Point(-1, -1), 2); //침식

			//구멍 채우기
			cvFillHoles(eroded);

			imshow("check", eroded);


			//컨벡스 설정 
			if (!findConvex(eroded)) 
				return false;

			
			//defect의 중점 / hull의 중점 구하기
			if (!getRealcenterPoint())
				return false;

			
			//도형 자르고 사각형 구하기
		
			result = GetRealConvexhull(eroded);

			return true;

		}
};

// 쓸 객체
handpos Handpos;