#include "DataLoader.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// 생성자: 메모리 생성
DataLoader::DataLoader() {

    size = 2000;
    data = new DataPoint[size];
}

// 소멸자: 메모리 해제
DataLoader::~DataLoader() {
    delete[] data;
}

// CSV 파일 [로딩]
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

        if (line.empty()) continue;

        stringstream ss(line);
        string value;

        try {
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
        }
        catch (...) {
            cout << "파싱 실패 ta line" << index << endl;
            continue;
        }
        index++;
    }

    for (int i = 10; i < 20; i++) {
        cout << data[i].exchange_rate << endl;
    }

    size = index; // 실제로 읽어온 데이터 개수로 size 업데이트
    file.close();
    return true;
}


// 전처리 : 데이터의 변화률 정규화 (0 ~ 1)
// 이거 코드 완성해주세요.
// 변화율 = ((x_t - x_(t-1)) / x_(t-1))
void DataLoader::normalize() {
    // 원본 복사
    DataPoint* prev = new DataPoint[size];
    for (int i = 0; i < size; i++) {
        prev[i] = data[i];
    }

    // 변화율 계산
    for (int i = 1; i < size; i++) {

        // 환율
        if (prev[i - 1].exchange_rate != 0)
            data[i].exchange_rate =
            (prev[i].exchange_rate - prev[i - 1].exchange_rate) / prev[i - 1].exchange_rate;
        else
            data[i].exchange_rate = 0;

        // 금리차
        if (prev[i - 1].diffUstoKr != 0)
            data[i].diffUstoKr =
            (prev[i].diffUstoKr - prev[i - 1].diffUstoKr) / prev[i - 1].diffUstoKr;
        else
            data[i].diffUstoKr = 0;

        // DXY
        if (prev[i - 1].dxy != 0)
            data[i].dxy =
            (prev[i].dxy - prev[i - 1].dxy) / prev[i - 1].dxy;
        else
            data[i].dxy = 0;

        // 코스피
        if (prev[i - 1].kospi != 0)
            data[i].kospi =
            (prev[i].kospi - prev[i - 1].kospi) / prev[i - 1].kospi;
        else
            data[i].kospi = 0;

        // 원유
        if (prev[i - 1].oil != 0)
            data[i].oil =
            (prev[i].oil - prev[i - 1].oil) / prev[i - 1].oil;
        else
            data[i].oil = 0;
    }

    // 첫 값 초기화
    data[0].exchange_rate = 0;
    data[0].diffUstoKr = 0;
    data[0].dxy = 0;
    data[0].kospi = 0;
    data[0].oil = 0;

    delete[] prev;

    // min-max
    double min[5], max[5];
    for (int i = 0; i < 5; i++) {
        min[i] = 1e9;
        max[i] = -1e9;
    }

    for (int i = 1; i < size; i++) {
        if (data[i].exchange_rate < min[0]) min[0] = data[i].exchange_rate;
        if (data[i].exchange_rate > max[0]) max[0] = data[i].exchange_rate;

        if (data[i].diffUstoKr < min[1]) min[1] = data[i].diffUstoKr;
        if (data[i].diffUstoKr > max[1]) max[1] = data[i].diffUstoKr;

        if (data[i].dxy < min[2]) min[2] = data[i].dxy;
        if (data[i].dxy > max[2]) max[2] = data[i].dxy;

        if (data[i].kospi < min[3]) min[3] = data[i].kospi;
        if (data[i].kospi > max[3]) max[3] = data[i].kospi;

        if (data[i].oil < min[4]) min[4] = data[i].oil;
        if (data[i].oil > max[4]) max[4] = data[i].oil;
    }

    // 정규화 (0 나누기 방지 포함)
    for (int i = 1; i < size; i++) {
        double r0 = max[0] - min[0]; if (r0 == 0) r0 = 1e-12;
        double r1 = max[1] - min[1]; if (r1 == 0) r1 = 1e-12;
        double r2 = max[2] - min[2]; if (r2 == 0) r2 = 1e-12;
        double r3 = max[3] - min[3]; if (r3 == 0) r3 = 1e-12;
        double r4 = max[4] - min[4]; if (r4 == 0) r4 = 1e-12;

        data[i].exchange_rate = (data[i].exchange_rate - min[0]) / r0;
        data[i].diffUstoKr = (data[i].diffUstoKr - min[1]) / r1;
        data[i].dxy = (data[i].dxy - min[2]) / r2;
        data[i].kospi = (data[i].kospi - min[3]) / r3;
        data[i].oil = (data[i].oil - min[4]) / r4;
    }
}
 
// -- 정규화 끝나는 부분
/******************************/


// 데이터 반환 함수
DataPoint* DataLoader::getData() {
    return data;
}

// 데이터 개수 반환 함수
int DataLoader::getSize() {
    return size;
}