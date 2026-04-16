#include "HMM.h"

HMM::HMM(int states, int obs_dim) 
{
	this->states = states;
	this->obs_dim = obs_dim;

	pi = new double[states];
	
	A = new double* [states];
	for (int i = 0; i < states; i++)
		A[i] = new double[states];

	B = new double* [states];
	for (int i = 0; i < states; i++)
		B[i] = new double[obs_dim];
}



void HMM::initStateTransProb()
{
	int N = states;

	// pi 초기화
	for (int i = 0; i < N; i++) {
		pi[i] = 1.0 / N;
	}

	// A 초기화
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			A[i][j] = 1.0 / N;
		}
	}
}



void HMM::initEmission() 
{
	int N = states;
	int M = obs_dim;

	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			B[i][j] = 1.0 / M;
		}
	}
}



int HMM::discretize3(double x)
{
	if (x < 0.33) return 0;
	else if (x < 0.66) return 1;
	else return 2;
}




/*int HMM::getObs(int t) const {
	return obs[t];
}*/



HMM::~HMM()
{
	delete[] pi;

	for (int i = 0; i < states; i++) {
		delete[] A[i];
	}
	delete[] A;

	for (int i = 0; i < states; i++) {
		delete[] B[i];
	}
	delete[] B;
}