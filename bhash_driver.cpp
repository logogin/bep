#include <iostream>
#include <string>
#include <fstream>
#include <map>
//#include <tr1/unordered_map>
#include "bhash.hpp"
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

  map<string, int> keys_map;
  vector<string> keys;
  unsigned int num  = 0;
  {
    ifstream ifs(argv[1]);
    if (!ifs){
      fprintf(stderr, "cannot open %s\n", argv[1]);
      return -1;
    }
    string line;

    vector<pair<string, int> > keyVals;
    while (getline(ifs ,line)){
      keys.push_back(line);
      keyVals.push_back(make_pair(line, num));
      keys_map[line] = num;
      num++;
    }

    bep_tool::bhash bh;
    if (bh.build(keys) == -1){
      return -1;
    }
    
    if (bh.save(argv[2]) == -1){
      return -1;
    }
  }


  {
    bep_tool::bhash bh;
    if (bh.load(argv[2]) == -1){
      return -1;
    }

    vector<unsigned int> counter(num+1);
    {
      double start = gettimeofday_sec();    
      for (size_t i = 0; i < keys.size(); i++){
	const bep_tool::ub4 id = bh.lookup_wocheck(keys[i]);
	counter[id]++;
      }
      double time = gettimeofday_sec() - start;
      printf("lookup_wo %f key:%f usec\n", time, time * 1000. * 1000. / keys.size());
    }

    {
      double start = gettimeofday_sec();    
      for (size_t i = 0; i < keys.size(); i++){
	const bep_tool::ub4 id = bh.lookup(keys[i]);
	if (id == bep_tool::NOTFOUND){
	  counter[num]++;
	} else {
	  counter[id]++;
	}
      }
      double time = gettimeofday_sec() - start;
      printf("lookup    %f key:%f usec\n", time, time * 1000. * 1000. / keys.size());
    }
  
    {
      double start = gettimeofday_sec();    
      for (size_t i = 0; i < keys.size(); i++){
	map<string,int>::const_iterator it = keys_map.find(keys[i]);
	if (it == keys_map.end()){
	  counter[num]++;
	} else {
	  counter[it->second]++;
	}
      }
      double time = gettimeofday_sec() - start;
      printf("tr1umap   %f key:%f usec\n", time, time * 1000. * 1000. / keys.size());
    }

    size_t sum = 0;
    for (size_t i = 0; i < keys.size(); i++){
      sum += counter[i];
    }
    if (sum == 0){
      printf(" \n"); // disallow optimization
    }

  }
  return 0;
}
