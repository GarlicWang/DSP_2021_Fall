#include <fstream>
#include "File.h"
#include "Ngram.h"
#include "Vocab.h"
#include "VocabMap.h"
using namespace std;

int main(int argc, char* argv[]){
    if(argc != 5){
        printf("wrong length");
        return 0;
    }
    int ngram_order = 2;
    Vocab voc;
    Ngram lm(voc, ngram_order);
    File lmFile(argv[3], "r");
    lm.read(lmFile);
    lmFile.close();

    fstream fout;
    fout.open(argv[4],ios::out);

    File fmap(argv[2], "r");
    char* map[35];              // 35 ZhuYin
    int num[35];                // 有多少個對應的國字
    char zhu[35][2];            // 所有的注音
    char** zhu_map[35];         // 所有對應的國字
    for(int i = 0;i<35;i++){
        map[i] = fmap.getline();
        zhu[i][0] = map[i][0];
        zhu[i][1] = map[i][1];
        num[i] = (strlen(map[i])-4)/3;
        zhu_map[i] = new char* [num[i]];
        for(int j = 0;j<num[i];j++){
            zhu_map[i][j] = new char [2];
        }
        for(int j = 3;j<(strlen(map[i]) - 1); j+=3){
            zhu_map[i][(j/3)-1][0] = map[i][j];
            zhu_map[i][(j/3)-1][1] = map[i][j+1];
        }
    }
    fmap.close();

    File test_data(argv[1], "r");
    char* text;

    for(int set = 0; set<50; set++){         // 50 sentences!!
        text = test_data.getline();
        char sentence[64][2];
        int count=0;
        for(int i = 0; i < strlen(text)-1;i++){
            if(text[i]!=' '){
                sentence[count][0] = text[i];
                i++;
                sentence[count][1] = text[i];
                count++;
            }
        }
        int big_num = 10000;

        VocabIndex** sentence_index;
        double** sentence_prob;
        int** previous_index;
        int* choice_num;
        sentence_index = new VocabIndex* [count+1];
        sentence_prob = new double* [count+1];
        previous_index = new int* [count+1];
        choice_num = new int [count];
        for(int i = 0;i<count;i++){
            choice_num[i] = 0;
        }
       
        VocabIndex wid;
    
        char tmp[2];

        // first iteration
        tmp[0] = sentence[0][0];
        tmp[1] = sentence[0][1];
        wid = voc.getIndex(tmp);
        VocabIndex start_id = voc.getIndex(Vocab_SentStart);
        VocabIndex end_id = voc.getIndex(Vocab_SentEnd);
        VocabIndex unknown_id = voc.getIndex(Vocab_Unknown);
        VocabIndex context[] = {start_id, Vocab_None};
        int zhu_idx = -1;
        if(wid == Vocab_None){      // 注音or沒看過
        ///////
            sentence_index[0] = new VocabIndex [big_num];
            sentence_prob[0] = new double [big_num];
            previous_index[0] = new int [big_num];
            for(int j = 0;j<big_num;j++){
                sentence_index[0][j] = Vocab_None;
                sentence_prob[0][j] = 0;
                previous_index[0][j] = -1;
            }
        /////////

            for(int z=0;z<35;z++){
                if(tmp[0] == zhu[z][0] && tmp[1] == zhu[z][1]){
                    zhu_idx = z;
                }
            }
            if(zhu_idx == -1){      // 沒看過的字
                wid = voc.addWord(tmp);
                sentence_index[0][0] = wid; 
                sentence_prob[0][0] = 0;
                previous_index[0][0] = 0;
                choice_num[0] = 1;
            }
            else{
                int tmp_idx = 0;
                for(int j = 0;j<num[zhu_idx];j++){
                    tmp[0] = zhu_map[zhu_idx][j][0];
                    tmp[1] = zhu_map[zhu_idx][j][1];
                    wid = voc.getIndex(tmp);
                    if(wid != Vocab_None){
                        sentence_index[0][tmp_idx] = wid;
                        sentence_prob[0][tmp_idx] = lm.wordProb(wid, context);
                        tmp_idx++;
                    }
                }
                choice_num[0] = tmp_idx;
            }
        }
        else{          // 國字
            ///////
            sentence_index[0] = new VocabIndex [1];
            sentence_prob[0] = new double [1];
            previous_index[0] = new int [1];
            sentence_index[0][0] = Vocab_None;
            sentence_prob[0][0] = 0;
            previous_index[0][0] = -1;
            ///////// 

            sentence_index[0][0] = wid;
            sentence_prob[0][0] = lm.wordProb(wid, context);
            choice_num[0] = 1;
        }

        for(int i = 1;i<count ;i++){            // count!!
            tmp[0] = sentence[i][0];
            tmp[1] = sentence[i][1];
            wid = voc.getIndex(tmp);
            zhu_idx=-1;

            if(wid == Vocab_None){      // 注音or沒看過
                ///////
                sentence_index[i] = new VocabIndex [big_num];
                sentence_prob[i] = new double [big_num];
                previous_index[i] = new int [big_num];
                for(int j = 0;j<big_num;j++){
                    sentence_index[i][j] = Vocab_None;
                    sentence_prob[i][j] = 0;
                    previous_index[i][j] = -1;
                }
                /////////
                for(int z=0;z<35;z++){
                    if(tmp[0] == zhu[z][0] && tmp[1] == zhu[z][1]){
                        zhu_idx = z;
                    }
                }
                if(zhu_idx == -1){      // 沒看過的字
                    wid = voc.addWord(tmp);
                    sentence_index[i][0] = wid; 
                    double tmp_max = -10000;
                    int tmp_max_index = 0;  
                    for(int last = 0;last < choice_num[i-1];last++){
                        if(sentence_prob[i-1][last] > tmp_max){
                            tmp_max = sentence_prob[i-1][last];
                            tmp_max_index = last;
                        }
                    }
                    sentence_prob[i][0] = tmp_max;
                    previous_index[i][0] = tmp_max_index;
                    choice_num[i] = 1;
                }
                else{
                    int tmp_idx = 0;
                    for(int j = 0;j<num[zhu_idx];j++){
                        tmp[0] = zhu_map[zhu_idx][j][0];
                        tmp[1] = zhu_map[zhu_idx][j][1];
                        wid = voc.getIndex(tmp);
                        if(wid != Vocab_None){      
                            double tmp_max = -10000;
                            int tmp_max_index = 0;
                            for(int last = 0;last < choice_num[i-1];last++){
                                VocabIndex context[] = {sentence_index[i-1][last], Vocab_None};
                                if(lm.wordProb(wid, context) + sentence_prob[i-1][last] > tmp_max){
                                    tmp_max = lm.wordProb(wid, context) + sentence_prob[i-1][last];
                                    tmp_max_index = last;
                                }
                            }
                            sentence_index[i][tmp_idx] = wid;
                            sentence_prob[i][tmp_idx] = tmp_max;
                            previous_index[i][tmp_idx] = tmp_max_index;
                            tmp_idx++;
                        }
                    }
                    choice_num[i] = tmp_idx;
                }   
            }
            else{          // 國字
                ///////
                sentence_index[i] = new VocabIndex [1];
                sentence_prob[i] = new double [1];
                previous_index[i] = new int [1];
                sentence_index[i][0] = Vocab_None;
                sentence_prob[i][0] = 0;
                previous_index[i][0] = -1;
                ///////// 
                sentence_index[i][0] = wid;
                double tmp_max = -10000;
                int tmp_max_index = 0;
                for(int last = 0;last < choice_num[i-1];last++){
                    VocabIndex context[] = {sentence_index[i-1][last], Vocab_None};
                    if(lm.wordProb(wid, context) + sentence_prob[i-1][last] > tmp_max){
                        tmp_max = lm.wordProb(wid, context) + sentence_prob[i-1][last];
                        tmp_max_index = last;
                    }
                }
                sentence_prob[i][0] = tmp_max;
                previous_index[i][0] = tmp_max_index;
                choice_num[i] = 1;
            }
        }
        ///////
        sentence_index[count] = new VocabIndex [1];
        sentence_prob[count] = new double [1];
        previous_index[count] = new int [1];
        sentence_index[count][0] = Vocab_None;
        sentence_prob[count][0] = 0;
        previous_index[count][0] = -1;
        ///////// 
        double tmp_max = -10000;
        int tmp_max_index = 0;
        for(int last = 0;last < choice_num[count-1];last++){
            VocabIndex context[] = {sentence_index[count-1][last], Vocab_None};
            if(lm.wordProb(end_id, context) + sentence_prob[count-1][last] > tmp_max){
                tmp_max = lm.wordProb(end_id, context) + sentence_prob[count-1][last];
                tmp_max_index = last;
            }
        }

        sentence_prob[count][0] = tmp_max;
        previous_index[count][0] = tmp_max_index;

        int *all_index;
        all_index = new int [count];
        for(int i = 0;i<count;i++){
            all_index[i] = 0;
        }
        all_index[count-1] = previous_index[count][0];
        for(int rev = count - 1 ;rev>0;rev--){
            all_index[rev-1] = previous_index[rev][all_index[rev]];
        }
        fout << voc.getWord(start_id) << " ";
        for(int i = 0;i<count;i++){
            fout << voc.getWord(sentence_index[i][all_index[i]]) << " ";
        }
        fout << voc.getWord(end_id) << endl;
    }    
}