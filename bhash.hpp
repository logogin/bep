#ifndef __BHASH_HPP__
#define __BHASH_HPP__

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <math.h>
#include <stdlib.h>

namespace bep_tool{

typedef unsigned long long int ub8; // unsigned 8-byte
typedef unsigned int  ub4; // unsigned 4-byte
typedef unsigned char ub;  // unsigned 1-byte

using namespace std;

struct hg_edge{
public:
  hg_edge(const ub4* _v){
    v[0] = _v[0]; 
    v[1] = _v[1]; 
    v[2] = _v[2];
  }

  hg_edge(const ub8* _v){
    v[0] = (ub4)_v[0]; 
    v[1] = (ub4)_v[1]; 
    v[2] = (ub4)_v[2];
  }

  int operator < (const hg_edge& a) const {
    return 
      v[0] != a.v[0] ? v[0] < a.v[0] :
      v[1] != a.v[1] ? v[1] < a.v[1] :
      v[2] <  a.v[2];
  }

  int operator == (const hg_edge & a) const {
    return 
      (v[0] == a.v[0]) &&
      (v[1] == a.v[1]) &&
      (v[2] == a.v[2]);
  }
  ub4 v[3];
};

static const double C_R = 1.3;
static const ub4 NOTFOUND = -1;
static const ub4 BYTEBLOCK = 8;
static const ub4 INTBLOCK  = 32;
static const char* KEYEXT = ".key";

class bhash{
public:
  bhash();
  ~bhash();
  int build(const vector<string>& keys); // build perfect hash functions for keys
  int save(const char* fileName) const;  // save current perfect hash function to fileName
  int load(const char* fileName);        // load fileName to current perfect hash function
  ub4 lookup(const string& key) const;   // lookup key and return ID or NOTFOUND
  ub4 lookup_wocheck(const string& key) const;   // lookup key and return ID without checking unknown keys
  bool lookupKey(const size_t i, string& key) const; // return key which retruns i as ID

  ub4 size() const; // return n

private:
  void bob_mix(ub4& a, ub4& b, ub4& c) const;
  void bob_hash(const ub* str, const ub4 length, const ub4 init, ub4& a, ub4& b, ub4& c) const;
  void bob_hash64(const ub* str, const ub4 length, const ub4 init, ub8& a, ub8& b, ub8& c) const;

  void bob_mix64(ub8& a, ub8& b, ub8& c) const;

  void clear();
  ub4 lookupGVAL(const uint i) const;

  ub4 seed; // seed for hash functions

  ub4 n;    // number of keys
  ub4 bn;   // number of total buckets (output of NON-minimal perfect hashing)
  ub4 bn_m; // number of buckets for each hash function

  // rank tables
  ub4 rank(const uint i) const;
  ub4 popCount(const uint i) const;
  ub4* B;      // store the assigned verticies
  ub4* levelA; // store every 256-th rank result
  ub*  levelB; // store every  32-th rank result
  ub*  gtable; // store g values in compressed representations

  ub8 keysLen;  // total length of keys
  ub* strTable; // store original keys in raw format
  ub8* strOffsets; // store key's offsets
};


} // bep_tool

#endif // __BHASH_HPP__
