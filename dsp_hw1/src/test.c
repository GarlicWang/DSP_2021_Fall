#include "hmm.h"
#include <math.h>

// the command will be like "./test modellist.txt data/test_seq.txt result.txt"
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("wrong argument length");
        return 0;
    }
    HMM hmms[5];
    load_models(argv[1], hmms, 5);
    // dump_models(hmms, 5);

    // load test seq
    FILE *fin = fopen(argv[2], "r");
    if (fin == NULL)
    {
        printf("open file error!!\n");
    }

    // make result file
    FILE *fout = fopen(argv[3], "w+t");
    for (int seq = 0; seq < 2500; seq++)
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

        double prob[5];
        for (int i = 0; i < 5; i++)
        {
            prob[i] = 0;
        }

        for (int model_index = 0; model_index < 5; model_index++)
        {
            double delta[MAX_SEQ][MAX_STATE];
            for (int time = 0; time < MAX_SEQ; time++)
            {
                for (int state = 0; state < MAX_STATE; state++)
                {
                    delta[time][state] = 0;
                }
            }
            for (int state = 0; state < hmms[model_index].state_num; state++)
            {
                delta[0][state] = hmms[model_index].initial[state] * hmms[model_index].observation[input2[0]][state];
            }
            for (int time = 1; time < sequence_num; time++)
            {
                for (int state = 0; state < hmms[model_index].state_num; state++)
                {
                    double max_delta = 0;
                    for (int substate = 0; substate < hmms[model_index].state_num; substate++)
                    {
                        double multi = delta[time - 1][substate] * hmms[model_index].transition[substate][state] * hmms[model_index].observation[input2[time]][state];
                        if (multi > max_delta)
                        {
                            max_delta = multi;
                        }
                    }
                    delta[time][state] = max_delta;
                }
            }
            double final_max = 0;
            for (int state = 0; state < hmms[model_index].state_num; state++)
            {
                if (delta[sequence_num - 1][state] > final_max)
                {
                    final_max = delta[sequence_num - 1][state];
                }
            }
            prob[model_index] = final_max;
            // printf("%e\t", prob[model_index]);
        }
        // printf("\n");
        int max_model_index = 0;
        double max_prob = 0;
        for (int model_index = 0; model_index < 5; model_index++)
        {
            if (prob[model_index] > max_prob)
            {
                max_model_index = model_index;
                max_prob = prob[model_index];
            }
        }
        // printf("max model : %d\t", max_model_index);
        // printf("max prob : %e\n", max_prob);
        fprintf(fout, "model_0%d.txt %e\n", max_model_index + 1, max_prob); //要記得+1!!!XDDD
    }
    return 0;
}