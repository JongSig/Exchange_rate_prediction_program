#pragma once
#include "HMM.h"

class ParallelHMM : public HMM {
private:
	DataPoint* data;
	int* obs;


	double** alpha;      // forward
	double** beta;      // backward
	double** gamma;      // 상태 확률
	double** xi = nullptr;      // 전이 확률
	int num_procs = 1;         // 병렬 처리에 사용할 프로세스 수
	int l1_line_size = 64;      // L1 캐시 라인 크기 (바이트 단위)
	int chunk_size = 256;         // 병렬 처리 시 작업 단위 크기
	int states;
	int obs_dim;
public:
	ParallelHMM(const DataPoint* data, int seq_len, int states, int obs_dim);

	double forward() override;
	void backward() override;

	void computeGammaAndXi();

	void baumWelch();

	void predictNextState(double* next_state);
};