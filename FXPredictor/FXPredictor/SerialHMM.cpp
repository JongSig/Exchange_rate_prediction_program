#include "SerialHMM.h"
#include <iostream>
using namespace std;

SerialHMM::SerialHMM(const DataPoint* data, int seq_len, int states, int obs_dim)
	: HMM(states, obs_dim)
{
	this->data = data;
	this->seq_len = seq_len;

	// 동적할당
	alpha = new double* [seq_len];
	beta = new double* [seq_len];
	gamma = new double* [seq_len];
	xi = new double* [seq_len - 1];

	obs = new int[seq_len];

	for (int t = 0; t < seq_len; t++) {
		alpha[t] = new double[states];
		beta[t] = new double[states];
		gamma[t] = new double[states];
	}

	for (int t = 0; t < seq_len - 1; t++) {
		xi[t] = new double[states * states];
	}

}



void SerialHMM::initialize()
{
	initObservations();
	initStateTransProb();
	initEmission();
}



void SerialHMM::initObservations()
{
	for (int t = 0; t < seq_len; t++) {

		int ex = discretize3(data[t].exchange_rate);
		int dxy = discretize3(data[t].dxy);
		int ir = discretize3(data[t].diffUstoKr);

		obs[t] = ex * 9 + dxy * 3 + ir;
	}
}



double SerialHMM::forward()
{
	double* c = new double[seq_len]; // scaling factor

	// 1. init
	for (int i = 0; i < states; i++) {
		alpha[0][i] = pi[i] * B[i][obs[0]];
	}

	// scale t=0
	c[0] = 0.0;
	for (int i = 0; i < states; i++) c[0] += alpha[0][i];

	if (c[0] == 0) c[0] = 1e-12;
	c[0] = 1.0 / c[0];

	for (int i = 0; i < states; i++)
		alpha[0][i] *= c[0];

	// 2. induction
	for (int t = 1; t < seq_len; t++) {

		c[t] = 0.0;

		for (int j = 0; j < states; j++) {

			alpha[t][j] = 0.0;

			for (int i = 0; i < states; i++) {
				alpha[t][j] += alpha[t - 1][i] * A[i][j];
			}

			alpha[t][j] *= B[j][obs[t]];
			c[t] += alpha[t][j];
		}

		if (c[t] == 0) c[t] = 1e-12;
		c[t] = 1.0 / c[t];

		for (int j = 0; j < states; j++)
			alpha[t][j] *= c[t];
	}

	// 3. log likelihood
	double logProb = 0.0;
	for (int t = 0; t < seq_len; t++) {
		logProb += log(c[t]);
	}

	delete[] c;
	return logProb;
}


void SerialHMM::backward()
{
	double* c = new double[seq_len];

	// forward에서 scaling 사용했으면 동일 c 필요
	// (간단 버전: backward도 normalize)

	for (int i = 0; i < states; i++) {
		beta[seq_len - 1][i] = 1.0;
	}

	for (int t = seq_len - 2; t >= 0; t--) {

		for (int i = 0; i < states; i++) {
			beta[t][i] = 0.0;

			for (int j = 0; j < states; j++) {

				beta[t][i] +=
					A[i][j] *
					B[j][obs[t + 1]] *
					beta[t + 1][j];
			}
		}

		// normalization (핵심)
		double sum = 0.0;
		for (int i = 0; i < states; i++) sum += beta[t][i];

		if (sum == 0) sum = 1e-12;

		for (int i = 0; i < states; i++)
			beta[t][i] /= sum;
	}

	delete[] c;
}



void SerialHMM::baumWelch()
{
	// E-step
	forward();
	backward();
	computeGamma();
	computeXi();

	// M-step
	// pi update
	for (int i = 0; i < states; i++) {
		pi[i] = gamma[0][i];
	}

	// A update
	for (int i = 0; i < states; i++) {
		for (int j = 0; j < states; j++) {
			double numer = 0.0;
			double denom = 0.0;

			for (int t = 0; t < seq_len - 1; t++) {
				numer += xi[t][i * states + j];
				denom += gamma[t][i];
			}

			if (denom == 0) {
				A[i][j] = 0;
			}
			else {
				A[i][j] = numer / denom;
			}
		}
	}

	// B update
	for (int j = 0; j < states; j++) {
		for (int k = 0; k < obs_dim; k++) {
			double numer = 0.0;
			double denom = 0.0;

			for (int t = 0; t < seq_len; t++) {
				if (obs[t] == k) {
					numer += gamma[t][j];
				}

				denom += gamma[t][j];
			}

			if (denom == 0) {
				B[j][k] = 0;
			}
			else {
				B[j][k] = numer / denom;
			}
		}
	}
}



void SerialHMM::computeGamma()
{
	for (int t = 0; t < seq_len; t++) {

		double sum = 0.0;

		for (int i = 0; i < states; i++) {
			gamma[t][i] = alpha[t][i] * beta[t][i];
			sum += gamma[t][i];
		}

		if (sum == 0) {
			for (int i = 0; i < states; i++) {
				gamma[t][i] = 0;
			}
		}
		else {
			for (int i = 0; i < states; i++) {
				gamma[t][i] /= sum;
			}
		}
	}
}



void SerialHMM::computeXi()
{
	int N = states;
	int T = seq_len;

	for (int t = 0; t < T - 1; t++) {

		double sum = 0.0;

		// 1. xi 계산
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

		// 2. normalize
		if (sum == 0) {
			for (int i = 0; i < N * N; i++) {
				xi[t][i] = 0;
			}
		}
		else {
			for (int i = 0; i < N * N; i++) {
				xi[t][i] /= sum;
			}
		}
	}
}


SerialHMM::~SerialHMM() {

	// alpha
	for (int t = 0; t < seq_len; t++) {
		delete[] alpha[t];
	}
	delete[] alpha;

	// beta
	for (int t = 0; t < seq_len; t++) {
		delete[] beta[t];
	}
	delete[] beta;

	// gamma
	for (int t = 0; t < seq_len; t++) {
		delete[] gamma[t];
	}
	delete[] gamma;

	// obs
	delete[] obs;

	// xi
	for (int t = 0; t < seq_len - 1; t++) {
		delete[] xi[t];
	}
	delete[] xi;
}
