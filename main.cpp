#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include "alarm.h" // 알림 헤더파일 추가

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


// 현재 시간을 나타내는 함수
const string currentDateTime() {

	time_t     now = time(NULL);
	struct tm  t;
	char       buf[80];

	localtime_s(&t, &now);
	strftime(buf, sizeof(buf), OUTPUTTIME, &t);

	return buf;
}

int main() {

	bool record = false;	// 녹화 상태
	bool cap = false;	// 캡쳐 상태
	bool check = false;	// 확인 상태

	VideoCapture capture(0);	// 카메라 연결 번호

	// 카메라 연결에 문제가 생겼을 때
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
	Mat frame, next_frame;                                      // 영상 촬영 및 움직임 감지를 위한 프레임 객체 선언
	Mat grayImage1, grayImage2;
	Mat diffImage1, resultImage1;
	Mat prev_frame, post_frame;                                 // 물체의 변화 감지를 위한 프레임 객체 선언
	Mat prev_grayframe, post_grayframe;
	Mat diffImage2, resultImage2;
	Mat temp;

	capture.read(prev_frame);	// 배경이 될 프레임을 카메라카 켜지면 바로 저장

	namedWindow("frame", WINDOW_AUTOSIZE);

	int timer = 0;	// 타이머 초기화
	int end_timer = 0;	// 종료 타이머 초기화
	int fourcc = VideoWriter::fourcc(EXTEN);

	VideoWriter outputVideo;

	while (1) {

		string text = currentDateTime();

		capture >> frame;
		capture >> next_frame;

		if (frame.empty())
			break;

		// 움직임 감지를 확인하기 위한 영상처리 과정
		cvtColor(frame, grayImage1, COLOR_BGR2GRAY);                          // 그레이스케일로 변환
		cvtColor(next_frame, grayImage2, COLOR_BGR2GRAY);                     // 그레이스케일로 변환
		GaussianBlur(grayImage1, grayImage1, Size(5, 5), 0.5);                // 가우시안블러 처리
		absdiff(grayImage1, grayImage2, diffImage1);                          // 그레이이미지들 간의 차이를 저장
		threshold(diffImage1, resultImage1, nThreshold, 255, THRESH_BINARY);	// 임계값에 의한 의진화 처리

		diffImage1.setTo(0);

		int move_cnt = countNonZero(resultImage1);	// 변화하는 픽셀 수 체크

		//imshow("mask", mask);
		putText(frame, text, Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 20), FONT_HERSHEY_DUPLEX, 0.6, Scalar(255, 0, 0));

		// 움직임이 감지되었을 때
		if (move_cnt >= 1000 && record == false) {

			record = !record;	// 녹화 상태 변환
			//cout << movecnt << endl;
			//outputVideo.open(text + VIDEOFOEMAT, fourcc, 24, size, true);
			cout << text << " 움직임 감지" << endl;
			cout << "move_cnt : " << move_cnt << endl;
		}

		// 녹화 상태일 때
		if (record) {

			timer++;	// 녹화를 시작하면서 타이머 시작

			// 지정한 시간 이상동안 움직임이 감지되었을 때
			if (timer >= SENSING && cap == false) {

				cap = !cap;	// 캡쳐 상태 변환
				//imwrite(text + " 거수자 감지.png", frame);

				// 추가 부분 //
				// 캡쳐 프레임에 이벤트명 삽입
				//ASCII 문자만 사용 가능하여 한글 사용 불가
				putText(frame, "Suspicious person detected", Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 40), FONT_HERSHEY_DUPLEX, 0.6, Scalar(255, 0, 0));
				imshow("move", frame);
				// 자동 종료되는 타이머 알림 설정
				MessageBoxTimeout(NULL, _T("거수자 감지."), _T("알림"), MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL | MB_ICONINFORMATION, 0, 1200);
				// 추가 끝 //

				cout << text << " 거수자 감지" << endl;
			}

			// 움직임이 없을 때
			if (move_cnt < 100) {

				end_timer++;	// 움직임이 없으면 타이머 시작

				// 지정한 시간 이상동안 움직임이 없을 때
				if (end_timer >= ENDTIME) {

					record = !record;  // 녹화 상태 변환
					cap = !cap;     // 캡쳐 상태 변환
					check = !check;   // 확인 상태 변환
					timer = 0;        // 타이머 초기화
					end_timer = 0;        // 종료 타이머 초기화

					capture.read(post_frame);	// 전경이 될 프레임을 움직임이 없으면 저장
					//outputVideo.release();
					cout << text << " 움직임 없음" << endl;
					cout << "move_cnt : " << move_cnt << endl;
				}

			}

			//outputVideo << frame;
		}

		// 움직임이 감지되고 끝났을 때
		if (check) {

			check = !check;

			// 물체의 변화를 확인하기 위한 영상처리 과정
			cvtColor(prev_frame, prev_grayframe, COLOR_BGR2GRAY);
			cvtColor(post_frame, post_grayframe, COLOR_BGR2GRAY);
			GaussianBlur(prev_grayframe, prev_grayframe, Size(5, 5), 0.5);
			absdiff(prev_grayframe, post_grayframe, diffImage2);
			threshold(diffImage2, resultImage2, nThreshold, 255, THRESH_BINARY);

			diffImage2.setTo(0);

			int check_cnt = countNonZero(resultImage2);

			// 물건의 유무를 확인
			// 변화가 생겼을 때
			if (check_cnt > 20000) {
				
				// 추가 부분 //
			    putText(frame, "Change of things", Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 40), FONT_HERSHEY_DUPLEX, 0.6, Scalar(255, 0, 0));
				//putText(frame, (LPCWSTR)L"물건 변화", Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 40), FONT_HERSHEY_DUPLEX, 0.6, Scalar(255, 0, 0));
				imshow("move", frame);
			    MessageBoxTimeout(NULL, _T("물건 변화."), _T("알림"), MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL | MB_ICONINFORMATION, 0, 1200);
				// 추가 끝 //

				cout << "물건 변화" << endl;
				cout << "check_cnt : " << check_cnt << endl;
				cout << endl;

				//imshow("prev_frame", prev_frame);
				//imshow("post_frame", post_frame);
				// 물체의 변화가 생겼으면 전경을 후경으로 교체
				temp = prev_frame;
				prev_frame = post_frame;
				post_frame = temp;
			}

			// 변화가 없을 때
			else {

				// 추가 부분 //
				putText(frame, "No change in things", Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 40), FONT_HERSHEY_DUPLEX, 0.6, Scalar(255, 0, 0));
				//putText(frame, (LPCWSTR)L"물건 변화 없음", Point(20, capture.get(CAP_PROP_FRAME_HEIGHT) - 40), FONT_HERSHEY_DUPLEX, 0.6, CV_RGB(255, 0, 0));
				imshow("move", frame);
			    MessageBoxTimeout(NULL, _T("물건 변화 없음."), _T("알림"), MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL | MB_ICONINFORMATION, 0, 1200);
				// 추가 끝 //

				cout << "물건 변화" << endl;
				cout << "물건 변화 없음" << endl;
				cout << "check_cnt : " << check_cnt << endl;
				cout << endl;
			}
		}

		imshow("frame", frame);

		// ESC 키를 누르면 종료
		if (waitKey(DELAY) == 27)
			break;
	}

	return 0;
}
