#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include "alarm.h" // �˸� ������� �߰�

using namespace std;
using namespace cv;

#define SENSING 80
#define ENDTIME 10
//#define FONTFACE FONT_HERSHEY_COMPLEX_SMALL
#define DELAY 30
#define OUTPUTTIME "%Y.%m.%d.%H.%M.%S"
#define VIDEOFOEMAT ".mp4v"
#define EXTEN 'M', 'P', 'E', 'G'
#define nThreshold 50
#define WIN32_LEAN_AND_MEAN


// ���� �ð��� ��Ÿ���� �Լ�
const string currentDateTime() {

	time_t     now = time(NULL);
	struct tm  t;
	char       buf[80];

	localtime_s(&t, &now);
	strftime(buf, sizeof(buf), OUTPUTTIME, &t);

	return buf;
}

int main() {

	bool record = false;	// ��ȭ ����
	bool cap = false;	// ĸ�� ����
	bool check = false;	// Ȯ�� ����

	VideoCapture capture(0);	// ī�޶� ���� ��ȣ

	// ī�޶� ���ῡ ������ ������ ��
	if (!capture.isOpened()) {

		cout << "Can not open capture!!" << endl;

		return 0;
	}

	Size size = Size((int)capture.get(CAP_PROP_FRAME_WIDTH) / 4, (int)capture.get(CAP_PROP_FRAME_HEIGHT) / 4);
	int fps = (int)(capture.get(CAP_PROP_FPS));

	cout << "Size = " << size << endl;
	cout << "fps = " << fps << endl;

	imshow("frame", NULL);
	waitKey(100);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5)); //MORPH_ELLIPSE
	Mat frame, next_frame;                                      // ���� �Կ� �� ������ ������ ���� ������ ��ü ����
	Mat grayImage1, grayImage2;
	Mat diffImage1, resultImage1;
	Mat prev_frame, post_frame;                                 // ��ü�� ��ȭ ������ ���� ������ ��ü ����
	Mat prev_grayframe, post_grayframe;
	Mat diffImage2, resultImage2;
	Mat temp;

	capture.read(prev_frame);	// ����� �� �������� ī�޶�ī ������ �ٷ� ����

	namedWindow("frame", WINDOW_AUTOSIZE);

	int timer = 0;	// Ÿ�̸� �ʱ�ȭ
	int end_timer = 0;	// ���� Ÿ�̸� �ʱ�ȭ
	int fourcc = VideoWriter::fourcc(EXTEN);

	VideoWriter outputVideo;

	while (1) {

		string text = currentDateTime();

		capture >> frame;
		capture >> next_frame;

		if (frame.empty())
			break;

		// ������ ������ Ȯ���ϱ� ���� ����ó�� ����
		cvtColor(frame, grayImage1, COLOR_BGR2GRAY);                          // �׷��̽����Ϸ� ��ȯ
		cvtColor(next_frame, grayImage2, COLOR_BGR2GRAY);                     // �׷��̽����Ϸ� ��ȯ
		GaussianBlur(grayImage1, grayImage1, Size(5, 5), 0.5);                // ����þȺ� ó��
		absdiff(grayImage1, grayImage2, diffImage1);                          // �׷����̹����� ���� ���̸� ����
		threshold(diffImage1, resultImage1, nThreshold, 255, THRESH_BINARY);	// �Ӱ谪�� ���� ����ȭ ó��

		diffImage1.setTo(0);

		int move_cnt = countNonZero(resultImage1);	// ��ȭ�ϴ� �ȼ� �� üũ

		//imshow("mask", mask);
		putText(frame, text, Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 20), FONT_HERSHEY_DUPLEX, 0.6, Scalar(255, 0, 0));

		// �������� �����Ǿ��� ��
		if (move_cnt >= 1000 && record == false) {

			record = !record;	// ��ȭ ���� ��ȯ
			//cout << movecnt << endl;
			//outputVideo.open(text + VIDEOFOEMAT, fourcc, 24, size, true);
			cout << text << " ������ ����" << endl;
			cout << "move_cnt : " << move_cnt << endl;
		}

		// ��ȭ ������ ��
		if (record) {

			timer++;	// ��ȭ�� �����ϸ鼭 Ÿ�̸� ����

			// ������ �ð� �̻󵿾� �������� �����Ǿ��� ��
			if (timer >= SENSING && cap == false) {

				cap = !cap;	// ĸ�� ���� ��ȯ
				//imwrite(text + " �ż��� ����.png", frame);

				// �߰� �κ� //
				// ĸ�� �����ӿ� �̺�Ʈ�� ����
				//ASCII ���ڸ� ��� �����Ͽ� �ѱ� ��� �Ұ�
				putText(frame, "Suspicious person detected", Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 40), FONT_HERSHEY_DUPLEX, 0.6, Scalar(255, 0, 0));
				imshow("move", frame);
				// �ڵ� ����Ǵ� Ÿ�̸� �˸� ����
				MessageBoxTimeout(NULL, _T("�ż��� ����."), _T("�˸�"), MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL | MB_ICONINFORMATION, 0, 1200);
				// �߰� �� //

				cout << text << " �ż��� ����" << endl;
			}

			// �������� ���� ��
			if (move_cnt < 100) {

				end_timer++;	// �������� ������ Ÿ�̸� ����

				// ������ �ð� �̻󵿾� �������� ���� ��
				if (end_timer >= ENDTIME) {

					record = !record;  // ��ȭ ���� ��ȯ
					cap = !cap;     // ĸ�� ���� ��ȯ
					check = !check;   // Ȯ�� ���� ��ȯ
					timer = 0;        // Ÿ�̸� �ʱ�ȭ
					end_timer = 0;        // ���� Ÿ�̸� �ʱ�ȭ

					capture.read(post_frame);	// ������ �� �������� �������� ������ ����
					//outputVideo.release();
					cout << text << " ������ ����" << endl;
					cout << "move_cnt : " << move_cnt << endl;
				}

			}

			//outputVideo << frame;
		}

		// �������� �����ǰ� ������ ��
		if (check) {

			check = !check;

			// ��ü�� ��ȭ�� Ȯ���ϱ� ���� ����ó�� ����
			cvtColor(prev_frame, prev_grayframe, COLOR_BGR2GRAY);
			cvtColor(post_frame, post_grayframe, COLOR_BGR2GRAY);
			GaussianBlur(prev_grayframe, prev_grayframe, Size(5, 5), 0.5);
			absdiff(prev_grayframe, post_grayframe, diffImage2);
			threshold(diffImage2, resultImage2, nThreshold, 255, THRESH_BINARY);

			diffImage2.setTo(0);

			int check_cnt = countNonZero(resultImage2);

			// ������ ������ Ȯ��
			// ��ȭ�� ������ ��
			if (check_cnt > 20000) {
				
				// �߰� �κ� //
			    putText(frame, "Change of things", Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 40), FONT_HERSHEY_DUPLEX, 0.6, Scalar(255, 0, 0));
				//putText(frame, (LPCWSTR)L"���� ��ȭ", Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 40), FONT_HERSHEY_DUPLEX, 0.6, Scalar(255, 0, 0));
				imshow("move", frame);
			    MessageBoxTimeout(NULL, _T("���� ��ȭ."), _T("�˸�"), MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL | MB_ICONINFORMATION, 0, 1200);
				// �߰� �� //

				cout << "���� ��ȭ" << endl;
				cout << "check_cnt : " << check_cnt << endl;
				cout << endl;

				//imshow("prev_frame", prev_frame);
				//imshow("post_frame", post_frame);
				// ��ü�� ��ȭ�� �������� ������ �İ����� ��ü
				temp = prev_frame;
				prev_frame = post_frame;
				post_frame = temp;
			}

			// ��ȭ�� ���� ��
			else {

				// �߰� �κ� //
				putText(frame, "No change in things", Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 40), FONT_HERSHEY_DUPLEX, 0.6, Scalar(255, 0, 0));
				//putText(frame, (LPCWSTR)L"���� ��ȭ ����", Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 40), FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0));
				imshow("move", frame);
			    MessageBoxTimeout(NULL, _T("���� ��ȭ ����."), _T("�˸�"), MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL | MB_ICONINFORMATION, 0, 1200);
				// �߰� �� //

				cout << "���� ��ȭ" << endl;
				cout << "���� ��ȭ ����" << endl;
				cout << "check_cnt : " << check_cnt << endl;
				cout << endl;
			}
		}

		imshow("frame", frame);

		// ESC Ű�� ������ ����
		if (waitKey(DELAY) == 27)
			break;
	}

	return 0;
}
