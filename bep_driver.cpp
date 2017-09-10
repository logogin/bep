#include <iostream>
#include <string>
#include <fstream>
#include <map>
//#include <tr1/unordered_map>
#include "bep.hpp"

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

double gettimeofday_sec(){
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

using namespace std;

int main(int argc, char* argv[]){
  if (argc != 3){
    fprintf(stderr, "%s keylist index\n", argv[0]);
    return -1;
  }

  bep_tool::bep<unsigned int> bp;
  vector<string> keys; // for check
  {
    ifstream ifs(argv[1]);
    if (!ifs){
      fprintf(stderr, "cannot open %s\n", argv[1]);
      return -1;
    }
    
    string line;
    unsigned int  num = 0;
    while (getline(ifs ,line)){
      bp[line] = num;
      keys.push_back(line);
      num++;
    }
  }

  // check
  for (size_t i = 0; i < keys.size(); i++){
    if (bp[keys[i]] != i){
      fprintf(stderr, "bep check error i:%u key:%s retVal:%u\n", (unsigned int)i, keys[i].c_str(), bp[keys[i]]);
      break;
    }
  }

  // save load check
  {
    // you need to save vals by yourself    
    unsigned int* vals = NULL;
    if (bp.save(argv[2], vals) == -1){
      fprintf(stderr, "save error\n");
      return -1;
    }

    bep_tool::bep<unsigned int> bp2;    
    if (bp2.load(argv[2], vals) == -1){
      fprintf(stderr, "load error\n");
    }

    for (size_t i = 0; i < keys.size(); i++){
      const string& key = keys[i];
      if (bp[key] != bp2[key]){
	fprintf(stderr, "save read error key:%s bp:%u bp2:%u\n", key.c_str(), bp[key], bp2[key]);
	return -1;
      }
    }
  }
     

  // time check 
  size_t sum = 0;
  double start = gettimeofday_sec();
  for (size_t i = 0; i < keys.size(); i++){
    sum += bp[keys[i]];
  }
  double time = gettimeofday_sec() - start;
  printf("%.4f %.4f usec\n",time, time/keys.size() * 1000 * 1000);

  if (sum == 0){
    printf(" \n"); // disallow optimization
  }

  return 0;
}
