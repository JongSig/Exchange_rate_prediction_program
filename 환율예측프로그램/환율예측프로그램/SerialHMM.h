#pragma once
#include "HMM.h"

class SerialHMM : public HMM {
private:
	double** alpha = nullptr;		// forward
	double** beta = nullptr;		// backward
	double** gamma = nullptr;		// »óÅÂ È®·ü
	double** xi = nullptr;		// ÀüÀÌ È®·ü

public:
	SerialHMM(int states, int obs_dim);
	~SerialHMM();

	double forward() override;
	void backward() override;

	void computeGamma();
	void computeXi();

	void baumWelch(); 

	void predictNextState(double* next_state);
};