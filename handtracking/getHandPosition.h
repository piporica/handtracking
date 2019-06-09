#pragma once
#include "header.h"



class handpos
{
private :

	//opencv �̿��ؼ� �� ���� �����ϴ� �Լ��� ����

	//����
	Mat frame;

	vector <vector<Point>> Contours;

	vector<vector<Point>>_hull;
	vector<vector<int> > _hullsI;
	vector<vector<Vec4i>> _defects;
	//���� �� ������ �ε��� 
	int LongestContour;

	//����� ������/defect
	vector<Point> _selectContours;
	vector<Vec4i> _selectdefects;
	vector<Point>_selecthull;

	Point centerPoint; //�չٴ� �߽� ��ġ 
	double maxdist;
	int FmaxdistIndex;

	vector<Point> _Fars;
	Point hull_center;
	Point FarCenter;
	Point SECenter;
	Point SECenter_adv;
	int whatline[200];
	
	handPosition result;


	//�̹��� ���� �޲ٱ�
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
		//������ ����
		for (int i = 0; i < Contours.size(); i++)
		{
			//���� �� ������ ���� ã�� 
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

		//��������ȭ
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
			cout << "���� ���ڶ�";
			return false;
		}
		if (count > 15)
		{
			cout << "����;";
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

			//�ϴ±迡 ��հ� ���� �� �Ÿ��� �� ���ϱ�
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

		float scale = 1.2; // �������� 0���� �÷��� ��

		cout << _selecthull.size();

		for (int i = 0; i < _selecthull.size(); i++)
		{
			x += _selecthull[i].x;
			y += _selecthull[i].y;
		}

		hull_center.x = x / _selecthull.size();
		hull_center.y = y / _selecthull.size();


		//������ ���ϱ� 
		vector<Point> crossPoints(200);


		int rimit = 10;//���ѹ����� ��� - �ո� ���� ���� �����ϱ�

		int crosscount = checkcross(scale, crossPoints);
		int over120 = 0; //120�� �Ѵ°� 2���� �־�� ���

		while (crosscount < 2)
		{
			scale += 0.3;
			crosscount = checkcross(scale, crossPoints);
			rimit--;
			if (rimit == 0) break;
		}
		if (rimit != 0)
		{
			rimit = 10; //�Ѱ��� �ʱ�ȭ
			for (int i = 0; i < crossPoints.size(); i++)
			{
				if (findangle(crossPoints[i], hull_center, FarCenter) > 120) over120++;
			}
			//�����Ҷ����� scale �ø���
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
		rst.center = FarCenter; //�̰� ���߿�~
		rst.rotateXY = rect.angle;
		for (int i = 0; i < 4 ; i++)
		{
			rst.vertices[i] = vertices[i];
		}

		//������ ����
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
					whatline[countcross] = j; //��� �� ������ ����α�...
					crossPoints[countcross] = rst[0];
					countcross++;
				}
				if (noa == 2)
				{
					crossPoints[countcross] = rst[0];
					whatline[countcross] = j; //��� �� ������ ����α�...

					crossPoints[countcross + 1] = rst[1];
					whatline[countcross + 1] = j; //��� �� ������ ����α�...

					countcross = countcross + 2;
				}
			}
		}
		crossPoints.resize(countcross);
		return countcross;
	}

	//������ ������ ���ϱ�
	bool findcrossPoint(Point start, Point end, Point center, float radius, int& numOfAns, Point rst[2])
	{
		//a�� b�� �̷���� ���а� center�� �߽��̰� radius �� �������� ������ ������ ���ϱ�
		numOfAns = 0;

		float degree = float((end.y - start.y)) / float((end.x - start.x));			//����

		float constv = start.y - degree * start.x; // ���

		float a = 1 + degree * degree;
		float b = -2 * (center.x + degree * (center.y - constv));
		float c = center.x * center.x + (center.y - constv) * (center.y - constv) - radius * radius;

		if ((b * b - 4 * a * c) < 0) // ���
		{
			return false;
		}

		float ans1 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
		float ans2 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);

		//������ ��
		if ((b * b - 4 * a * c) == 0) // �߱�
		{
			numOfAns = 1;
			rst[0].x = ans1;
			rst[0].y = degree * ans1 + constv;
			return true;
		}

		//���� �� ���� ��
		//���� ��¥���� �Ǻ�
		//�����̾���ϹǷ� x��ǥ�� �� �� ���̿� �־����

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
			return false; // �� �� or �� ���� ��
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

		//4�и� ������
		if (newA.x < 0) angleA = 180 + angleA;
		else if (newA.y < 0) angleA = 360 + angleA;

		if (newB.x < 0) angleB = 180 + angleB;
		else if (newB.y < 0) angleB = 360 + angleB;

		//�� ���ϱ�
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

		//����(120 �̻�)���� ū �� �ΰ� ���� (��������)
		//if 140 �̻��ΰ� �� ����� ���� �� ���ŷ� <- ���߿�
		//���������� �����
		//�� üŷ -> �� �� ����
		//�ڸ� �� ����ֱ�
		int over120 = 0;
		int maxAngleindex[2] = { 0,0 }; // ���� ū �� �ΰ� 
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


		//���������� ����� 
		Point max1 = crossPoints[maxAngleindex[0]];
		Point max2 = crossPoints[maxAngleindex[1]];

		double degree = (max2.y - max1.y) / double((max2.x - max1.x));
		double constv = max1.y - degree * max1.x;

		//�ڸ��� ���� ��Ļ� ��ġ �ľ� -���� ���⼭ ���� �� ���� �ִٸ� �׳� ��ŵ�ع�����
		int nextindex1 = whatline[maxAngleindex[0]];
		int nextindex2 = whatline[maxAngleindex[1]];
		if (nextindex1 == nextindex2) return;

		//������ ��ġ �ľ�
		double up = hull_center.y - (hull_center.x * degree + constv); // ����� ���� ����
		double isup;
		int listcount = 0;
		for (int i = 0; i < _selecthull.size(); i++)
		{
			//�� �� �ֱ�
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

			//���� �� ����
			isup = _selecthull[i].y - (_selecthull[i].x * degree + constv);
			if (up * isup > 0) // ���� ����
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

			//���׷��̵� ������ �κ�... ��������� ���ϰ� �����ϱ�
			int minCr = 133; //128 
			int maxCr = 175;
			int minCb = 78; //73
			int maxCb = 134;

			//opencv�� �̿��ؼ� ������ �簢���� ���Ѵ�
			cvtColor(frame, YCrCbframe, COLOR_RGB2YCrCb);
			split(YCrCbframe, planes);
			usemask = (minCr < planes[1]) & (planes[1] < maxCr) & (minCb < planes[2]) & (planes[2] < maxCb);

			//�������� ���� Ŭ���� > �̷ε� 
			morphologyEx(usemask, closed, MORPH_CLOSE, Mat(5, 5, CV_8U, Scalar(1)));
			erode(closed, eroded, Mat(3, 3, CV_8U, Scalar(1)), Point(-1, -1), 2); //ħ��

			//���� ä���
			cvFillHoles(eroded);

			imshow("check", eroded);


			//������ ���� 
			if (!findConvex(eroded)) 
				return false;

			
			//defect�� ���� / hull�� ���� ���ϱ�
			if (!getRealcenterPoint())
				return false;

			
			//���� �ڸ��� �簢�� ���ϱ�
		
			result = GetRealConvexhull(eroded);

			return true;

		}
};

// �� ��ü
handpos Handpos;