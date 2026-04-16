/*
	HMM : Hidden Markov Model

	Parameters
	A[][]: State transition probability
	B[][]: Emisstion probability
	pi[] : Initial state probability

	Create function
	forward()
	Backward()
	viterbi()
	Baum-Welch()

*/
#include "DataLoader.h"

#pragma once
class HMM {
protected:
	int states;
	int obs_dim;
	int seq_len;

	double* pi;
	double** A;
	double** B;

public:
	HMM(int states, int obs_dim);

	// 초기화
	virtual void initialize() = 0;

	// 공유하는 함수
	void initObservations();
	void initStateTransProb();
	void initEmission();
	int discretize3(double x);

	// virtual function
	virtual double forward() = 0;
	virtual void backward() = 0;
	virtual void baumWelch() = 0;
	virtual void viterbi() = 0;

	// debugging ㅠㅠ
	// int getObs(int t) const;

	virtual ~HMM();
};
