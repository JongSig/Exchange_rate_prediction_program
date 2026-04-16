#pragma once
#include "HMM.h"
#include "DataLoader.h"

class SerialHMM : public HMM {
private:
	const DataPoint* data;
	int* obs;
	int seq_len;

	double** alpha;
	double** beta;
	double** gamma;
	double** xi;

public:
	SerialHMM(const DataPoint* data, int seq_len, int states, int obs_dim);

	void initialize();
	void initObservations();

	// main Algorithm
	double forward() override;
	void backward() override;
	void baumWelch() override;
	void viterbi() override {}

	// compute
	void computeGamma();
	void computeXi();

	~SerialHMM();
};