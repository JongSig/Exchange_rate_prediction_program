/*
	HMM의 구성 : 
	보이지 않는 상태(Hidden State), 관측하능한 결과(Observation)


	
	viterbi 라는 알고리즘도 있는데, 시간이 남으면 한번 구현
	해보는 것도 좋을 듯합니다.

	viterbi : "전체 경로 확률이 최대가 되는 상태 시퀀스"를 찾음
*/

#pragma once

#include "DataLoader.h"

class HMM {
protected:
	// DataLoader의 데이터를 가져옴
	DataPoint* data = nullptr;	// 정규화된 데이터

	// 상태, 관측 백터와 상태 전이, 관측 행렬 선언할 때 사용
	int states;		// 상태의 개수		(States)
	int obs_dim;	// 관측값의 개수	(Observation_dimension)
	int seq_len  = 0;	// 시퀀스 길이		(sequence)

	// DataLoader에서 최종적으로 반환되는 백터
	int* obs = nullptr;		// 관측값 (observed data)

	// N : states, M : obs_dim
	double* pi = nullptr;		// 초기 상태 확률 (N)	(Initial_state_probability)
	double** A = nullptr;		// 상태 전이 확률 (NxN)	(State_transition_probability)
	double** B = nullptr;		// 관측 확률 (NxM)		(emission_probability)


public:
	HMM(int states, int obs_dim);

	/* 데이터 초기화 */
	void initObservations();	// obs
	void initStateTransProb();	// pi, A
	void initEmission();		// B

	void initialize();

	// 이산화 0, 1, 2
	int discretize3(double x);

	/* forward Algorithm */
	// - 시간 T까지의 부분 관측 시퀀스를 고려,
	// 상태 Sj에 있을 누적 확률을 재귀적으로 계산
	virtual double forward() = 0;

	/* backward algorithm */
	// - Forward와 반대로, 현재 상태에서 남은 관측 시퀀스를 
	// 생성할 확률을 계산
	virtual void backward() = 0;

	// Baum-welch algorithm
	// - 반복적인 알고리즘으로, 현재 모델 파라미터 세타를 기준으로 
	// "상태 점유 확률"을 추정하는 E-step과 추정된 정보를 바탕으로 
	// 파라미터를 최적화 하는 M-step을 번갈아 수행
	virtual void baumWelch() = 0;

};