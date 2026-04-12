/*
	DataLoader : 시퀀스 데이터를 읽어오는 함수 정의
	
	읽어야 하는 시퀀스 데이터
	- 환율
	- 한국 금리
	- 미국 금리
	- 달러(DXY)
	- 코스피(KOSPI)
	- 원유(WTI)
*/

#include "DataLoader.h"
#include <stdio.h>

// 생성자: 메모리 생성
DataLoader::DataLoader() {
	
	size = 1000;
	// dataSize = 1,000 * (8 * 6 + 40) = 88,000 .. 88KB 할당, 메모리 충분
	data = new DataPoint[size];
}

// 소멸자: 메모리 해제
DataLoader::~DataLoader() {
	delete[] data;
}

// CSV 파일 [로딩]
/******구현해야 되는 부분******/
/* 
	주요 역할 : CSV 파일의 데이터를 읽어온다.
	CSV 파일을 하나로 합쳐도 될거 같고,
	파일들을 하나씩 읽어서 마지막에 병합하셔도 될거 같아요.
	편하신데로 부탁드립니다.
	편하게 쓰시는 함수 있으면 임이로 추가해서 쓰세요.
*/
bool DataLoader::loadCSV(const string& filename) {

}


// 전처리 : 데이터의 변화률 정규화 (0 ~ 1)
// 기존의 data에 변화율을 덮어쓰기하는 방식으로 해요.
// 이거 코드 완성해주세요.
// 변화율 = ((x_t - x_(t-1)) / x_(t-1))
void DataLoader::normalize() {

	// 데이터의 변화율 계산
	// 변화율 사용 이유
	// 1. 정확한 계산
	// 2. 정규화에 있어 이상치 최소화
	for (int i = 1; i < size; i++) {
		// 환율
		data[i].exchange_rate = 
			(data[i].exchange_rate - data[i - 1].exchange_rate) / data[i - 1].exchange_rate;
		// 금리차, 달러 인덱스 등등, 변화율 다 구해주세요. 위에 환율처럼 만들어주시면 됩니다.

	}

	// 정규화에 사용할 min, max 초기화
	double min[5], max[5];

	for (int i = 0; i < 5; i++) {
		min[i] = 1e9;
		max[i] = -1e9;
	}

	// ※변화율 정규화
	
	// min-max 구하기
	for (int i = 1; i < size; i++) {
		// ex) 환율 min-max 구하기
		if (data[i].exchange_rate < min[0]) min[0] = data[i].exchange_rate;
		if (data[i].exchange_rate > max[0]) max[0] = data[i].exchange_rate;
		
		// 아래에 나머지 구현해주세요. kr, us_rate 재외
	}

	// 정규화
	for (int i = 0; i < size; i++) {
		// ex) 환율 정규화
		data[i].exchange_rate = (data[i].exchange_rate - min[0]) / (max[0] - min[0]);
		
		// 나머지들도 구현해주세요.
	}

}
// 정규화 끝!

//
//
/******************************/


// 데이터 반환 함수
DataPoint* DataLoader::getData() {
	return data;
}

// 데이터 개수 반환 함수
int DataLoader::getSize() {
	return size;
}