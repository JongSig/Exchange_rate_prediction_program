/*
	Hidden Markov Model, 정의
*/

#include "HMM.h"

HMM::HMM(int states, int obs_dim) {
	this->states = states;
	this->obs_dim = obs_dim;
}

// 모델 파라미터 초기화
// 초기화할 파라미터 : pi, A
// pi : 초기 확률(Initial_state_probability)
// A : 상태 전이 확률 (State_transition_probability)
// pi는 N, 1차원 행렬, A는 N x N 2차원 행렬
// N는 상태 개수이다.
// 
// 중요 사항 : pi와 A은 행의 합은 1이 되어야 한다.
//
void HMM::initStateTransProb() {
	int N = states; // 상태 개수

	for (int i = 0; i < N; i++) {
		pi[i] = 1.0 / N;

		for (int j = 0; j < N; j++) {
			A[i][j] = 1.0 / N;
		}
	}
}

// 관측 확률 초기화
// B
void HMM::initEmission() {

	int N = states;
	int M = obs_dim;

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			B[i][j] = 1.0 / M;
		}
	}
}

// 0, 1, 2로 이산화
int HMM::discretize3(double x) {
	if (x < 0.33) return 0;
	else if (x < 0.66) return 1;
	else return 2;
}

// 현재 관측 데이터 초기화
void HMM::initObservations() {

	for (int t = 0; t < seq_len; t++) {

		int ex = discretize3(data[t].exchange_rate);
		int dxy = discretize3(data[t].dxy);
		int ir = discretize3(data[t].diffUstoKr);

		// 3 x 3 x 3 = 27 개의 관측값
		obs[t] = ex * 9 + dxy * 3 + ir;
	}
}


// 초기화 함수
void HMM::initialize() {
	initObservations();
	initStateTransProb();
	initEmission();
}