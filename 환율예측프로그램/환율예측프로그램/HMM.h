#pragma once

#define MAX_N 10
#define MAX_M 10
#define MAX_T 100

struct HMM {
	int N;		// stay 개수
	int M;		// observation 개수 

	double A[MAX_N][MAX_N];	// 전이 행렬
	double B[MAX_N][MAX_M];	// 관측 확률
	double pi[MAX_N];		// 초기 상태 확률
};

// Forward / Backward
void forward(HMM& hmm,
			 int O[], int T,
			 double alpha[][MAX_N]);	// O : 현재 시간의 관측(observation) 데이터

void backward(const HMM& hmm,
			  int O[], int T,
			  double beta[][MAX_N]);


// Baum-Welch 구성 요소
void computeGamma();
void computeXi();
void updateParameters();


// 학습 & 예측
void trainHMM();
void predictHMM();