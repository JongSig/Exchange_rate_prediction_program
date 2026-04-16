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
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// 생성자: 메모리 생성
DataLoader::DataLoader() {

	size = 1636;
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
	ifstream file(filename);
	if (!file.is_open())
		return false;

	string line;
	getline(file, line);

	int index = 0;
	while (getline(file, line) && index < size) {
		// CSV 파일의 각 줄을 파싱하여 DataPoint 구조체에 저장하는 코드 작성
		// 예시: "2020-01-01, 1100.5, 1.5, 98.3, 2200.4, 60.5"
		// 위와 같은 형식으로 CSV 파일이 구성되어 있다고 가정

		// CSV 라인을 ','로 분리하여 각 필드를 추출하는 코드 작성
		// 예시: date, exchange_rate, diffUstoKr, dxy, kospi, oil

		stringstream ss(line);
		string value;
		// 0. 날짜 읽기
		getline(ss, data[index].date, ',');

		// 1. 환율
		getline(ss, value, ','); data[index].exchange_rate = stod(value);

		// 2. 한미 금리차
		getline(ss, value, ','); data[index].diffUstoKr = stod(value);

		// 3. 달러 인덱스 (DXY)
		getline(ss, value, ','); data[index].dxy = stod(value);

		// 4. 코스피 지수
		getline(ss, value, ','); data[index].kospi = stod(value);

		// 5. 원유 가격 (WTI)
		getline(ss, value, ','); data[index].oil = stod(value);

		index++;
	}

	size = index; // 실제로 읽어온 데이터 개수로 size 업데이트
	file.close();
	return true;

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
		// 한미 금리차의 경우, 0으로 나누는 상황이 발생할 수 있으므로 예외 처리
		if (data[i - 1].diffUstoKr != 0)
			data[i].diffUstoKr = (data[i].diffUstoKr - data[i - 1].diffUstoKr) / data[i - 1].diffUstoKr;
		else
			data[i].diffUstoKr = 0;
		// 달러 인덱스
		data[i].dxy =
			(data[i].dxy - data[i - 1].dxy) / data[i - 1].dxy;
		// 코스피 지수
		data[i].kospi =
			(data[i].kospi - data[i - 1].kospi) / data[i - 1].kospi;
		// 원유 가격
		data[i].oil =
			(data[i].oil - data[i - 1].oil) / data[i - 1].oil;

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

		// 금리차 min-max 구하기
		if (data[i].diffUstoKr < min[1]) min[1] = data[i].diffUstoKr;
		if (data[i].diffUstoKr > max[1]) max[1] = data[i].diffUstoKr;

		// 달러 인덱스 min-max 구하기
		if (data[i].dxy < min[2]) min[2] = data[i].dxy;
		if (data[i].dxy > max[2]) max[2] = data[i].dxy;

		// 코스피 지수 min-max 구하기
		if (data[i].kospi < min[3]) min[3] = data[i].kospi;
		if (data[i].kospi > max[3]) max[3] = data[i].kospi;

		// 원유 가격 min-max 구하기
		if (data[i].oil < min[4]) min[4] = data[i].oil;
		if (data[i].oil > max[4]) max[4] = data[i].oil;


	}

	// 정규화
	for (int i = 0; i < size; i++) {
		// ex) 환율 정규화
		data[i].exchange_rate = (data[i].exchange_rate - min[0]) / (max[0] - min[0]);
		// 금리차 정규화
		data[i].diffUstoKr = (data[i].diffUstoKr - min[1]) / (max[1] - min[1]);
		// 달러 인덱스 정규화
		data[i].dxy = (data[i].dxy - min[2]) / (max[2] - min[2]);
		// 코스피 지수 정규화
		data[i].kospi = (data[i].kospi - min[3]) / (max[3] - min[3]);
		// 원유 가격 정규화
		data[i].oil = (data[i].oil - min[4]) / (max[4] - min[4]);

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