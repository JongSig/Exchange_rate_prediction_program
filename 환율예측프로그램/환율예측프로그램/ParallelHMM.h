#pragma once
#include "HMM.h"

class ParallelHMM : public HMM {
public:
	ParallHMM(int states, int obs_dim);

	// 병렬 forward
	double P_Forward(DataPoint* data, int T);

	// 병렬 backward
	void P_Backward();

	// 병렬 Baum-welch
	void P_Baum_Welch(DataPoint* data, int T, int iterations);
};