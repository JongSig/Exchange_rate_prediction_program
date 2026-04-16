/*
	프로그램 : 환율 예측 프로그램
	작성 날짜 : 2024-04-10
	작성자 : 김종완, 김원호, 김민석
	프로그램 설명 :
	환율은 최근에 경제적으로 많은 관심을 가지고 있으며 1800까지 올라간다는 관측이 있다.
	이런 변동성이 심한 현재 환율을 HMM 모델의 알고리즘을 이용하여 보이지 않는
	현재 상태(증가, 감소, 변동없음)을 분석함으로 미래 환율의 변동을 예측해보자.
*/

#include "DataLoader.h"
#include "SerialHMM.h"
#include "ParallelHMM.h"
#include <iostream>
using namespace std;

int main(void) {
	DataLoader loader;

	// 파일 읽는 부분
	if (!loader.loadCSV("sequence_data.csv")) {
		cout << "파일 로드 실패" << endl;
		return 0;
	}

	// 정규화 함수
	loader.normalize();

	// 직렬, 병렬 컴포넌트 생성
	SerialHMM serial(loader.getData(), loader.getSize(), 5, 27);
	ParallelHMM parallel(loader.getData(), loader.getSize(), 5, 27);

	// 초기화
	serial.initialize();


	// parallel.initialize();

	// 학습
	for (int i = 0; i < 10; i++) {
		serial.baumWelch();
		parallel.baumWelch();
	}

	// 결과 비교
	double s_prob = serial.forward();
	double p_prob = parallel.forward();

	// 출력
	cout << "Serial: " << s_prob << endl;
	cout << "Parallel: " << p_prob << endl;

	return 0;
}