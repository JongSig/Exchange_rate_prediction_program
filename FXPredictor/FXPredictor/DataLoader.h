#pragma once
#include <string>
using namespace std;

// 하나의 시점 데이터
struct DataPoint {
	string date;			// 관측 날짜.

	double exchange_rate;	// 환율
	double diffUstoKr;		// 금리차 (us_rate - kr_rate)
	double dxy;				// 달러 인덱스
	double kospi;			// 코스피 지수
	double oil;				// 원유 가격

	// double kr_rate;			// 한국 금리
	// double us_rate;			// 미국 금리
};

// 클래스
class DataLoader {
private:
	DataPoint* data;	// 동적 배열
	int size;			// 시퀀스 데이터 크기

public:
	DataLoader();		// 생성자
	~DataLoader();		// 소멸자

	// 파일 읽기 함수
	bool loadCSV(const string& filename);

	// 데이터 정규화
	void normalize();

	// 데이터 반환 함수
	DataPoint* getData();
	int getSize();
};