/*
	프로그램 : 환율 예측 프로그램
	작성 날짜 : 2024-04-10
	작성자 : 김종완, 김원호, 김민석
	프로그램 설명 :
	환율은 최근에 경제적으로 많은 관심을 가지고 있으며 1800까지 올라간다는 관측이 있다.
	이런 변동성이 심한 현재 환율을 HMM 모델의 알고리즘을 이용하여 보이지 않는
	현재 상태(증가, 감소, 변동없음)을 분석함으로 미래 환율의 변동을 예측해보자.
*/

#include "serialHMM.h"
#include "ParallelHMM.h"
#include "DataLoader.h"

int main(void) {

	DataLoader loader;
	
	// 파일 읽는 부분
	//loader.loadCSV();
	loader.normalize();		// 데이터 정규화, 변화량 계산 + min-max 정규화

	// states : 5, Observation : 금리차3, 환율3, 달러3 3x3x3 = 27
	HMM hmm(5, 27);
	hmm.initialize();		// 데이터 읽기, 파라미터 초기화, 관측 데이터 생성

}