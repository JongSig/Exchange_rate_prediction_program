/*
	직렬 코드입니다.
*/

#include "SerialHMM.h"

SerialHMM::SerialHMM(int states, int obs_dim) : HMM(states, obs_dim) {
	
	// 데이터 동적 할당
	DataLoader loader;
	data = loader.getData();
	seq_len = loader.getSize();

	obs = new int[seq_len];

	pi = new double[states];

	A = new double* [states];
	B = new double* [states];

	for (int i = 0; i < states; i++) {
		A[i] = new double[states];
		B[i] = new double[obs_dim];
	}

	//
	alpha = new double* [seq_len];
	beta = new double* [seq_len];
	gamma = new double* [seq_len];

	for (int t = 0; t < seq_len; t++) {
		alpha[t] = new double[states];
		beta[t] = new double[states];
		gamma[t] = new double[states];

		memset(alpha[t], 0, sizeof(double) * N);
        memset(beta[t], 0, sizeof(double) * N);
        memset(gamma[t], 0, sizeof(double) * N);
	}

	xi = new double* [seq_len - 1];
	for (int t = 0; t < seq_len - 1; t++) {
		xi[t] = new double[states * states];

		memset(xi[t], 0, sizeof(double) * N * N);
	}
}

SerialHMM::~SerialHMM() {
	// 메모리 해제'
	delete[] data;
	delete[] obs;
	delete[] pi;
	for (int i = 0; i < states; i++) {
		delete[] A[i];
		delete[] B[i];
	}
	delete[] A;
	delete[] B;

	for (int i = 0; i < seq_len; i++) {
		delete alpha[i];
		delete beta[i];
		delete gamma[i];
	}
	delete[] alpha;
	delete[] beta;
	delete[] gamma;

	for (int i = 0; i < seq_len - 1; i++) {
		delete xi[i];
	}
	delete[] xi;
}

double SerialHMM::forward() {
	int N = states;
	int T = seq_len;
	
	// 초기화
	for (int i = 0; i < N; i++) {
		alpha[0][i] = pi[i] * B[i][obs[0]];
	}

	// 재귀
	for (int t = 1; t < T; t++) {
		for (int j = 0; j < N; j++) {
			alpha[t][j] = 0;

			for (int i = 0; i < N; i++) {
				alpha[t][j] += alpha[t - 1][i] * A[i][j];
			}

			alpha[t][j] *= B[j][obs[t]];
		}
	}

	// likelihood 반환
	double prob = 0;
	for (int i = 0; i < N; i++) {
		prob += alpha[T - 1][i];
	}

	return prob;
}

void SerialHMM::backward() {
	int T = seq_len;
	int N = states;
	
	for (int i = 0; i < N; i++) {
		beta[T - 1][i] = 1;
	}

	for (int t = T - 2; t >= 0; t--) {
		for (int i = 0; i < N; i++) {
			beta[t][i] = 0;

			for (int j = 0; j < N; j++) {
				beta[t][i] += A[i][j] * B[j][obs[t + 1]] * beta[t + 1][j];
			}
		}
		k
	}
}

void SerialHMM::computeGamma() {
	int T = seq_len;
	int N = states;

	for (int t = 0; t < T; t++) {
		double sum = 0;

		for (int i = 0; i < N; i++) {
			gamma[t][i] = alpha[t][i] * beta[t][i];
			sum += gamma[t][i];
		}

		for (int i = 0; i < N; i++) {
			gamma[t][i] /= sum;
		}
	}
}

void SerialHMM::computeXi() {
	int T = seq_len;
	int N = states;

	for (int t = 0; t < T - 1; t++) {
		double sum = 0;

		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				xi[t][i * N + j] =
					alpha[t][i] *
					A[i][j] *
					B[j][obs[t + 1]] *
					beta[t + 1][j];

				sum += xi[t][i * N + j];
			}
		}

		for (int i = 0; i < N * N; i++) {
			xi[t][i] /= sum;
		}
	}
}

void SerialHMM::baumWelch() {
	int T = seq_len;
	int N = states;
	int M = obs_dim;

	forward();
	backward();
	computeGamma();
	computeXi();

	// pi : 상태 초기 확률 (N)
	for (int i = 0; i < N; i++) {
		pi[i] = gamma[0][i];
	}

	// A : 상태 전의 확률 (N x N)
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {

			double numer = 0;
			double denom = 0;

			for (int t = 0; t < T - 1; t++) {
				numer += xi[t][i * N + j];
				denom += gamma[t][i];
			}

			A[i][j] = numer / denom;
		}
	}

	// B : 관측 확률 (N x M)
	for (int j = 0; j < N; j++) {
		for (int k = 0; k < M; k++) {
			double numer = 0;
			double denom = 0;

			for (int t = 0; t < T; t++) {
				if (obs[t] == k)
					numer += gamma[t][j];
					
				denom += gamma[t][j];
			}

			B[j][k] = numer / denom;
		}
	}
}

// 다음날 상태를 예측 함수
void SerialHMM::predictNextState(double* next_state) {
	int T = seq_len;
	int N = states;

	for (int j = 0; j < N; j++) {
		next_state[j] = 0;

		for (int i = 0; i < N; i++) {
			next_state[j] += alpha[T - 1][i] * A[i][j];
		}
	}
}