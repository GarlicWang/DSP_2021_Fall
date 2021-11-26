#include "hmm.h"
#include <math.h>

// the command will be like "./train 100 model_init.txt data/train_seq_01.txt model_01.txt"
int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        printf("wrong argument length\n");
        return 0;
    }
    int iteration = atoi(argv[1]); // 100
    // printf("iter = %d\n",iteration);

    // load initial HMM model
    HMM hmm_model;
    loadHMM(&hmm_model, argv[2]);
    // dumpHMM(stderr, &hmm_model);
    FILE *fout = fopen(argv[4], "w+t");

    for (int iter = 0; iter < iteration; iter++)
    {
        // load input sequence
        FILE *fin = fopen(argv[3], "r");
        if (fin == NULL)
        {
            printf("open file error!!\n");
        }
        // make output file
        

        double gamma_initial_sum[MAX_STATE];
        double transition_numerator[MAX_STATE][MAX_STATE];
        double transition_denominator[MAX_STATE][MAX_STATE];
        double observation_numerator[MAX_OBSERV][MAX_STATE];
        double observation_denominator[MAX_OBSERV][MAX_STATE];
        for (int state = 0; state < MAX_STATE; state++)
        {
            gamma_initial_sum[state] = 0;
        }
        for (int state1 = 0; state1 < MAX_STATE; state1++)
        {
            for (int state2 = 0; state2 < MAX_STATE; state2++)
            {
                transition_numerator[state1][state2] = 0;
                transition_denominator[state1][state2] = 0;
            }
        }
        for (int obs = 0; obs < hmm_model.observ_num; obs++)
        {
            for (int state = 0; state < MAX_STATE; state++)
            {
                observation_numerator[obs][state] = 0;
                observation_denominator[obs][state] = 0;
            }
        }

        for (int seq = 0; seq < 10000; seq++)
        {
            char input[MAX_SEQ] = "";
            int input2[MAX_SEQ];
            int sequence_num = 0;
            for (int i = 0; i < MAX_SEQ; i++)
            {
                fscanf(fin, "%c", &input[i]);
                // printf("%c", input[i]);
                if (input[i] == '\n')
                {
                    // printf("end sequence, seq_num = %d\n", i);
                    sequence_num = i;
                    break;
                }
            }
            for (int i = 0; i < sequence_num; i++)
            {
                if (input[i] == 'A')
                {
                    input2[i] = 0;
                }
                else if (input[i] == 'B')
                {
                    input2[i] = 1;
                }
                else if (input[i] == 'C')
                {
                    input2[i] = 2;
                }
                else if (input[i] == 'D')
                {
                    input2[i] = 3;
                }
                else if (input[i] == 'E')
                {
                    input2[i] = 4;
                }
                else if (input[i] == 'F')
                {
                    input2[i] = 5;
                }
                // printf("%d", input2[i]);
            }
            // printf("\n");

            double alpha[MAX_SEQ][MAX_STATE];
            double beta[MAX_SEQ][MAX_STATE];
            double gamma[MAX_SEQ][MAX_STATE];
            double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE];

            // 可忽略
            for (int time = 0; time < MAX_SEQ; time++)
            {
                for (int state = 0; state < MAX_STATE; state++)
                {
                    alpha[time][state] = 0;
                    beta[time][state] = 0;
                    gamma[time][state] = 0;
                }
            }
            for (int time = 0; time < sequence_num - 1; time++)
            {

                for (int state1 = 0; state1 < hmm_model.state_num; state1++)
                {
                    for (int state2 = 0; state2 < hmm_model.state_num; state2++)
                    {
                        epsilon[time][state1][state2] = 0;
                    }
                }
            }
            //////
            // printf("alpha : \n");
            // Foward algorithm
            for (int state = 0; state < hmm_model.state_num; state++)
            {
                alpha[0][state] = hmm_model.initial[state] * hmm_model.observation[input2[0]][state];
                // printf("%e\t", alpha[0][state]);
            }
            // printf("\n");
            for (int time = 1; time < sequence_num; time++)
            {
                for (int state = 0; state < hmm_model.state_num; state++)
                {
                    for (int substate = 0; substate < hmm_model.state_num; substate++)
                    {
                        alpha[time][state] += alpha[time - 1][substate] * hmm_model.transition[substate][state] * hmm_model.observation[input2[time]][state];
                    }
                    // printf("%e\t", alpha[time][state]);
                }
                // printf("\n");
            }
            // printf("\nbeta : \n");
            // Backward
            for (int state = 0; state < hmm_model.state_num; state++)
            {
                beta[sequence_num - 1][state] = 1;
                // printf("%e\t", beta[sequence_num - 1][state]);
            }
            // printf("\n");
            for (int time = sequence_num - 2; time >= 0; time--)
            {
                for (int state = 0; state < hmm_model.state_num; state++)
                {
                    for (int substate = 0; substate < hmm_model.state_num; substate++)
                    {
                        beta[time][state] += beta[time + 1][substate] * hmm_model.transition[state][substate] * hmm_model.observation[input2[time + 1]][substate];
                    }
                    // printf("%e\t", beta[time][state]);
                }
                // printf("\n");
            }

            // printf("\ngamma : \n");
            // calculating gamma
            for (int time = 0; time < sequence_num; time++)
            {
                double denominator = 0;
                for (int state = 0; state < hmm_model.state_num; state++)
                {
                    denominator += alpha[time][state] * beta[time][state];
                }
                for (int state = 0; state < hmm_model.state_num; state++)
                {
                    gamma[time][state] = alpha[time][state] * beta[time][state] / denominator;
                    // printf("%f\t", gamma[time][state]);
                }
                // printf("\n");
            }
            // printf("\nepsilon : \n");
            // calculating epsilon
            for (int time = 0; time < sequence_num - 1; time++)
            {
                // printf("t = %d\n", time);
                double denominator = 0;
                for (int state1 = 0; state1 < hmm_model.state_num; state1++)
                {
                    for (int state2 = 0; state2 < hmm_model.state_num; state2++)
                    {
                        denominator += alpha[time][state1] * hmm_model.transition[state1][state2] * hmm_model.observation[input2[time + 1]][state2] * beta[time + 1][state2];
                    }
                }
                // printf("t = %d\n",time);
                for (int state1 = 0; state1 < hmm_model.state_num; state1++)
                {
                    for (int state2 = 0; state2 < hmm_model.state_num; state2++)
                    {
                        epsilon[time][state1][state2] = alpha[time][state1] * hmm_model.transition[state1][state2] * hmm_model.observation[input2[time + 1]][state2] * beta[time + 1][state2] / denominator;
                        // printf("%f\t", epsilon[time][state1][state2]);
                    }
                    // printf("\n");
                }
            }
            // adding gamma_initail_sum
            for (int state = 0; state < hmm_model.state_num; state++)
            {
                gamma_initial_sum[state] += gamma[0][state];
            }

            // adding transition sum
            for (int state1 = 0; state1 < hmm_model.state_num; state1++)
            {
                for (int state2 = 0; state2 < hmm_model.state_num; state2++)
                {
                    double numerator = 0;
                    double denominator = 0;
                    for (int time = 0; time < sequence_num - 1; time++)
                    {
                        numerator += epsilon[time][state1][state2];
                        denominator += gamma[time][state1];
                    }
                    transition_numerator[state1][state2] += numerator;
                    transition_denominator[state1][state2] += denominator;
                }
            }
            // adding observation sum
            for (int obs = 0; obs < hmm_model.observ_num; obs++)
            {
                for (int state = 0; state < hmm_model.state_num; state++)
                {
                    double numerator = 0;
                    double denominator = 0;
                    for (int time = 0; time < sequence_num; time++)
                    {
                        denominator += gamma[time][state];
                        if (input2[time] == obs)
                        {
                            numerator += gamma[time][state];
                        }
                    }
                    observation_numerator[obs][state] += numerator;
                    observation_denominator[obs][state] += denominator;
                }
            }
        }
        // updating parameters
        // updating pi
        // printf("\nnew pi : \n");
        for (int state = 0; state < hmm_model.state_num; state++)
        {
            hmm_model.initial[state] = gamma_initial_sum[state] / 10000;
            // printf("%f\t", hmm_model.initial[state]);
        }
        // printf("\n");

        // updating transition
        // printf("\nnew transition : \n");
        for (int state1 = 0; state1 < hmm_model.state_num; state1++)
        {
            for (int state2 = 0; state2 < hmm_model.state_num; state2++)
            {
                hmm_model.transition[state1][state2] = transition_numerator[state1][state2] / transition_denominator[state1][state2];
                // printf("%f\t",hmm_model.transition[state1][state2]);
            }
            // printf("\n");
        }

        // updating observation
        // printf("\nnew observation : \n");
        for (int obs = 0; obs < hmm_model.observ_num; obs++)
        {
            for (int state = 0; state < hmm_model.state_num; state++)
            {
                hmm_model.observation[obs][state] = observation_numerator[obs][state] / observation_denominator[obs][state];
                // printf("%f\t", hmm_model.observation[obs][state]);
            }
            // printf("\n");
        }
        // dumpHMM(stderr, &hmm_model);
    }
    dumpHMM(fout, &hmm_model);
    // fprintf(fout,"%f\n",&hmm_model);
    return 0;
}