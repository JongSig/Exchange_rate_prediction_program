/*
   병렬 코드 :
*/
#include "ParallelHMM.h"
#include <omp.h>
#include <thread>

ParallelHMM::ParallelHMM(const DataPoint* data, int seq_len, int states, int obs_dim)
    : HMM(states, obs_dim) 
{
    // 데이터 동적 할당
    // 초기화 및 할당 병렬화는 오버헤드만 증가할 것 같음
    /*DataLoader loader;

    // CSV 로딩
    if (!loader.loadCSV("hmm_financial_data.csv")) {
        printf("데이터 로딩 실패!\n");
        return;
    }*/

    // 정규화 수행
    //loader.normalize();

    this->data = data;
    this->seq_len = seq_len;
    this->states = states;
    this->obs_dim = obs_dim;

    // 3. 관측 데이터 변환
    obs = new int[seq_len];
    for (int t = 0; t < seq_len; t++) {
        // 정규화된 0.0~1.0 값을 3구간으로 인코딩
        int r = (data[t].exchange_rate < 0.33) ? 0 : (data[t].exchange_rate < 0.66) ? 1 : 2;
        int d = (data[t].diffUstoKr < 0.33) ? 0 : (data[t].diffUstoKr < 0.66) ? 1 : 2;
        int x = (data[t].dxy < 0.33) ? 0 : (data[t].dxy < 0.66) ? 1 : 2;

        // 27차원 인덱스 생성 (환율/금리차/달러인덱스 조합)
        obs[t] = r * 9 + d * 3 + x;
    }

    pi = new double[states];

    A = new double* [states];
    B = new double* [states];

    for (int i = 0; i < states; i++) {
        A[i] = new double[states];
        B[i] = new double[obs_dim];
    }

    alpha = new double* [seq_len];
    beta = new double* [seq_len];
    gamma = new double* [seq_len];

    for (int t = 0; t < seq_len; t++) {
        alpha[t] = new double[states];
        beta[t] = new double[states];
        gamma[t] = new double[states];

        memset(alpha[t], 0, sizeof(double) * states);
        memset(beta[t], 0, sizeof(double) * states);
        memset(gamma[t], 0, sizeof(double) * states);
    }

    xi = new double* [seq_len - 1];

    #pragma omp parallel for schedule(dynamic, chunk_size)
    for (int t = 0; t < seq_len - 1; t++) {
        xi[t] = new double[states * states];

        memset(xi[t], 0, sizeof(double) * states * states);
    }
    num_procs = std::thread::hardware_concurrency();
}

ParallelHMM::~ParallelHMM() {
    // 메모리 해제: 해제 병렬화는 오버헤드만 증가할 것 같음 
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
        delete[] alpha[i];
        delete[] beta[i];
        delete[] gamma[i];
    }
    delete[] alpha;
    delete[] beta;
    delete[] gamma;

    for (int i = 0; i < seq_len - 1; i++) {
        delete[] xi[i];
    }
    delete[] xi;
}

double ParallelHMM::forward() {
    double prob = 0;
    #pragma omp parallel
    {
        // 초기화
        #pragma omp for
        for (int i = 0; i < states; i++) {
            alpha[0][i] = pi[i] * B[i][obs[0]];
        }

        // 재귀
        for (int t = 1; t < seq_len; t++) {
            #pragma omp for 
            for (int j = 0; j < states; j++) {
                alpha[t][j] = 0;

                for (int i = 0; i < states; i++) {
                    alpha[t][j] += alpha[t - 1][i] * A[i][j];
                }

                alpha[t][j] *= B[j][obs[t]];
            }
        }

        // likelihood 반환
        #pragma omp for reduction(+:prob)
        for (int i = 0; i < states; i++) {
            prob += alpha[seq_len - 1][i];
        }
    }
    return prob;
}

void ParallelHMM::backward() {
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < states; i++) {
            beta[seq_len - 1][i] = 1;
        }

        for (int t = seq_len - 2; t >= 0; t--) {
            #pragma omp for
            for (int i = 0; i < states; i++) {
                beta[t][i] = 0;
                for (int j = 0; j < states; j++) {
                    beta[t][i] += A[i][j] * B[j][obs[t + 1]] * beta[t + 1][j];
                }
            }
        }
    }
}

void ParallelHMM::computeGammaAndXi() {
    #pragma omp parallel for nowait
    for (int t = 0; t < seq_len; t++) {
        double sum = 1e-18;
        for (int i = 0; i < states; i++) {
            gamma[t][i] = alpha[t][i] * beta[t][i];
            sum += gamma[t][i];
        }
        for (int i = 0; i < states; i++) {
            gamma[t][i] /= sum;
        }
    }
    #pragma omp parallel for
    for (int t = 0; t < seq_len - 1; t++) {
        double sum = 1e-18;

        for (int i = 0; i < states; i++) {
            for (int j = 0; j < states; j++) {
                xi[t][i * states + j] =
                    alpha[t][i] *
                    A[i][j] *
                    B[j][obs[t + 1]] *
                    beta[t + 1][j];

                sum += xi[t][i * states + j];
            }
        }

        for (int i = 0; i < states * states; i++) {
            xi[t][i] /= sum;
        }
    }
}

void ParallelHMM::baumWelch() {
    forward();
    backward();
    computeGammaAndXi();

    int d_per_line = l1_line_size / sizeof(double);
    int a_off = ((states * states + d_per_line - 1) / d_per_line) * d_per_line;
    int b_off = ((states * obs_dim + d_per_line - 1) / d_per_line) * d_per_line;
    int d_off = ((states + d_per_line - 1) / d_per_line) * d_per_line;

    double* shared_A_numer = new double[num_procs * a_off] {0};
    double* shared_B_numer = new double[num_procs * b_off] {0};
    double* shared_A_denom = new double[num_procs * d_off] {0};
    double* shared_B_denom = new double[num_procs * d_off] {0};

    #pragma omp parallel num_threads(num_procs)
    {
        int tid = omp_get_thread_num();
        // pi : 상태 초기 확률 (N)
        #pragma omp for
        for (int i = 0; i < states; i++) {
            pi[i] = gamma[0][i];
        }

        double* my_A_numer = &shared_A_numer[tid * a_off];
        double* my_B_numer = &shared_B_numer[tid * b_off];
        double* my_A_denom = &shared_A_denom[tid * d_off];
        double* my_B_denom = &shared_B_denom[tid * d_off];

        #pragma omp for
        for (int t = 0; t < seq_len - 1; t++) {
            for (int i = 0; i < states; i++) {
                my_A_denom[i] += gamma[t][i];
                my_B_denom[i] += gamma[t][i];

                // A : 상태 전의 확률 (N x N)
                for (int j = 0; j < states; j++) {
                    my_A_numer[i * states + j] += xi[t][i * states + j];
                }

                // B : 관측 확률 (N x M)
                int k = obs[t];
                my_B_numer[i * obs_dim + k] += gamma[t][i];
            }
        }

        // reduction (취합)
        for (int j = 1; j < num_procs; j *= 2) {
            #pragma omp barrier
            if (tid % (2 * j) == 0 && (tid + j) < num_procs) {
                for (int i = 0; i < states * states; i++) {
                    my_A_numer[i] += shared_A_numer[(tid + j) * a_off + i];

                }
                for (int i = 0; i < states * obs_dim; i++) {
                    my_B_numer[i] += shared_B_numer[(tid + j) * b_off + i];
                }
                for (int i = 0; i < states; i++) {
                    my_A_denom[i] += shared_A_denom[(tid + j) * d_off + i];
                    my_B_denom[i] += shared_B_denom[(tid + j) * d_off + i];
                }
            }
        }

        #pragma omp for
        for (int i = 0; i < states; i++) {
            // A 행렬 업데이트
            double den_a = shared_A_denom[0 * d_off + i];
            if (den_a > 1e-18) {
                for (int j = 0; j < states; j++) {
                    A[i][j] = shared_A_numer[0 * a_off + i * states + j] / den_a;
                }
            }

            // B 행렬 업데이트
            double den_b = shared_B_denom[0 * d_off + i];
            if (den_b > 1e-18) {
                for (int k = 0; k < obs_dim; k++) {
                    B[i][k] = shared_B_numer[0 * b_off + i * obs_dim + k] / den_b;
                }
            }
        }
    }

    delete[] shared_A_numer;
    delete[] shared_B_numer;
    delete[] shared_A_denom;
    delete[] shared_B_denom;
}

// 다음날 상태를 예측 함수
void ParallelHMM::predictNextState(double* next_state) {
#pragma omp parallel for if(num_procs <= states)
    for (int j = 0; j < states; j++) {
        next_state[j] = 0;

        for (int i = 0; i < states; i++) {
            next_state[j] += alpha[seq_len - 1][i] * A[i][j];
        }
    }
}