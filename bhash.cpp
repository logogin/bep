#include "bhash.hpp"

using namespace bep_tool;

bhash::bhash(): B(NULL), levelA(NULL), levelB(NULL), gtable(NULL), strTable(NULL), strOffsets(NULL), n(0) {}
bhash::~bhash() {
  clear();
}


void bhash::clear(){
  if (B) delete[] B;
  if (levelA) delete[] levelA;
  if (levelB) delete[] levelB;
  if (gtable) delete[] gtable;
  if (strTable) delete[] strTable;
  if (strOffsets) delete[] strOffsets;
}

ub4 bhash::size() const{
  return n;
}

ub4 bhash::lookupGVAL(const uint i) const{
  return (gtable[i/4] >> ((i&0x3)*2)) & 0x3;
}

ub4 bhash::popCount(const uint i) const {
  ub4 r = i;
  r = ((r & 0xAAAAAAAA) >> 1) + (r & 0x55555555);
  r = ((r & 0xCCCCCCCC) >> 2) + (r & 0x33333333);
  r = ((r >> 4) + r) & 0x0F0F0F0F;
  r = (r>>8) + r;
  return ((r>>16) + r) & 0x3F;
}

ub4 bhash::rank(const uint i) const {
  return levelA[i/256] + levelB[i/INTBLOCK] + popCount(B[i/INTBLOCK] & ((1U << (i%INTBLOCK)) - 1));;
}

ub4 bhash::lookup_wocheck(const string& key) const{
  ub8 v[3];
  bob_hash64((ub*)key.c_str(), key.size(), seed, v[0], v[1], v[2]);
  v[0] = (v[0] % bn_m);
  v[1] = (v[1] % bn_m) + bn_m;
  v[2] = (v[2] % bn_m) + bn_m * 2;

  unsigned int val = 0;
  val += lookupGVAL(v[0]);
  val += lookupGVAL(v[1]);
  val += lookupGVAL(v[2]);
  /*
  for (int j = 0; j < 3; j++){
    if ((B[v[j]/INTBLOCK] >> (v[j]%INTBLOCK)) & 1){
      v[j] = rank(v[j]);
      val += lookupGVAL(v[j]);
    }
  }
  */
  return rank(v[val%3]);
}

ub4 bhash::lookup(const string& key) const{
  const ub4 id = lookup_wocheck(key);
  const ub8 begin = strOffsets[id];
  const ub8 len = strOffsets[id+1]-begin;
  if (key.size() != len) return NOTFOUND;
  if (memcmp(key.c_str(), strTable+begin, len) != 0) return NOTFOUND;
  return id;
};   // lookup key and may return NOTFOUND

bool bhash::lookupKey(const size_t i, string& key) const{
  if (i >= n) return false;
  key.assign((char*)strTable + strOffsets[i], strOffsets[i+1] - strOffsets[i]);
  return true;
}

int bhash::build(const vector<string>& keys){
  clear();
  n    = keys.size();
  bn_m = (ub4)(n * C_R / 3.f);
  bn    = bn_m * 3;

  vector<ub4> deg(bn);
  vector<ub4> edges(n * 3);
  vector<ub4> offset(bn+1);
  vector<bool> visitedEdges(n);
  vector<bool> visitedVerticies(bn);
  vector<ub> g(bn); // vertex marker

  // main loop
  bool finished = false;
  for (int iter = 0; iter < 20; iter++){
    seed = rand();

    for (size_t i = 0; i < bn; i++) {
      deg[i] = 0;
    }
  
    // generate candidate edges
    vector<hg_edge> vs;
    for (size_t i = 0; i < keys.size(); i++){
      ub8 v[3];
      bob_hash64((ub*)keys[i].c_str(), keys[i].size(), seed, v[0], v[1], v[2]);
      //for (int j = 0; j < 3; j++){
      //  v[j] = bob_hash((ub*)keys[i].c_str(), keys[i].size(), seeds[j]) % bn_m;
      //}
      v[0] = (v[0] % bn_m);
      v[1] = (v[1] % bn_m) + bn_m;
      v[2] = (v[2] % bn_m) + bn_m * 2;

      vs.push_back(hg_edge(v));
      deg[v[0]]++;
      deg[v[1]]++;
      deg[v[2]]++;
    }

    // check whether identical edges exist or not
    sort(vs.begin(), vs.end());

    bool edgeSuccess = true;
    for (size_t i = 1; i < vs.size(); i++){
      if (vs[i-1] == vs[i]){
	printf("find identical edge\n");
	edgeSuccess = false;
	break;
      }
    }
    if (!edgeSuccess) continue;

    // build offset
    ub4 sum = 0;
    for (size_t i = 0; i < bn; i++){
      offset[i] = sum;
      sum += deg[i];
    }
    offset[bn] = sum;

    // build edges
    for (size_t i = 0; i < bn; i++){
      deg[i] = 0;
    }

    for (ub4 i = 0; i < n; i++){
      for (ub4 j = 0; j < 3; j++){
	edges[offset[vs[i].v[j]] + deg[vs[i].v[j]]++] = i;	
      }
    }

    // check validity
    for (ub4 i = 0; i < n; i++) {
      visitedEdges[i] = false;
    }
   
    // initialize
    queue<ub4> q;
    for (int i = 0; i < bn; i++){
      if (deg[i] == 1) q.push(edges[offset[i]]);
      visitedVerticies[i] = false;
      g[i] = 3; // unmarked
    }

    ub4 deleteNum = 0;
    printf("n:%u bn:%u\n", n, bn);

    vector<pair<ub4, ub4> > extractedEdges;

    while (!q.empty()){
      ub4 eID = q.front(); q.pop(); // edgeID
      if (visitedEdges[eID]) continue;
      deleteNum++;
      visitedEdges[eID] = true;
      
      const hg_edge& e = vs[eID];
      int choosed = -1;      
      for (int j = 0; j < 3; j++) {
	if (--deg[e.v[j]] == 1) {
	  for (ub4 i = offset[e.v[j]]; i < offset[e.v[j]+1]; i++){
	    if (!visitedEdges[edges[i]]){
	      q.push(edges[i]);
	      break;
	    }
	  }
	} else if (deg[e.v[j]] == 0){
	  choosed = j;
	}
      }

      if (choosed == -1){
	fprintf(stderr, "unexpected error: we cannot find free vertex\n");
	break;
      }
      extractedEdges.push_back(make_pair(eID, choosed));
      //printf("choose %u set %u\n", e.v[choosed], val%3);
    }

    if (deleteNum == n) {
      // Current candidate can generate perfect functions
      reverse(extractedEdges.begin(), extractedEdges.end());
      for (size_t i = 0; i < extractedEdges.size(); i++) {
	const hg_edge& e = vs[extractedEdges[i].first];	
	const ub4 choosed = extractedEdges[i].second;
	ub4 val = choosed + 30; // +30: large offsets for limiting positive value
	for (int j = 0; j < 3; j++){
	  if (!visitedVerticies[e.v[j]]) {
	    continue;
	  }
	  val -= g[e.v[j]];
	}
	g[e.v[choosed]] = val % 3; 
	visitedVerticies[e.v[choosed]] = true;
      }

      // set rank table and gtable
      B      = new ub4 [(bn + INTBLOCK - 1) / INTBLOCK];
      levelA = new ub4 [(bn + 256 - 1) / 256];
      levelB = new ub  [(bn + INTBLOCK - 1)  / INTBLOCK];
      gtable = new ub  [(bn + 4 - 1) / 4];
      memset(B, 0, sizeof(ub4)*((bn + INTBLOCK - 1) / INTBLOCK));
      memset(gtable, 0, sizeof(ub)*((bn + 4 - 1) / 4));

      ub4 r = 0;
      for (int i = 0; i < bn; i++){
	if (i % 256 == 0) levelA[i/256] = r;
	if (i %  INTBLOCK == 0) levelB[i/INTBLOCK]  = r - levelA[i/256];
	if (g[i] != 3) {
	  B[i/INTBLOCK] |= (1U << (i%INTBLOCK));
	  r++;
	}
	gtable[i/4] |= (g[i] << ((i%4)*2));
      }

      // set original keys 
      keysLen = 0;
      for (ub4 i = 0; i < n; i++){
	keysLen += keys[i].size();
      }
      strTable = new ub[keysLen];
      strOffsets = new ub8[n+1];

      vector<pair<ub4, ub4> > id2keys;
      for (ub4 i = 0; i < n; i++){
	id2keys.push_back(make_pair(lookup_wocheck(keys[i]), i));
      }

      sort (id2keys.begin(), id2keys.end());
      ub8 lenSum = 0;
      for (ub4 i = 0; i < n; i++){
	strOffsets[i] = lenSum;
	const string& str = keys[id2keys[i].second];
	memcpy(strTable+lenSum,str.c_str(),str.size());
	lenSum += str.size();
      }
      strOffsets[n] = lenSum;
      finished = true;
      break;
    }
  }

  if (!finished){
    fprintf(stderr, "cannot find perfect hash functions\n");
    return -1;
  }

  return 0;
}

int bhash::save(const char* fileName) const{
  FILE* fp = fopen(fileName, "wb");
  if (fp == NULL){
    fprintf(stderr, "cannot open %s\n", fileName);
    return -1;
  }

  if (fwrite(&n, sizeof(ub4), 1, fp) != 1){
    fprintf(stderr, "save error %s\n", fileName);
    return -1;
  }

  if (fwrite(&seed, sizeof(ub4), 1, fp) != 1){
    fprintf(stderr, "save error %s\n", fileName);
    return -1;
  }

  const size_t writeSizeINTBLOCK = (bn+INTBLOCK-1)/INTBLOCK;
  if (fwrite(B, sizeof(ub4), writeSizeINTBLOCK, fp) != writeSizeINTBLOCK){
    fprintf(stderr, "save error %s\n", fileName);
    return -1;
  }
  
  const size_t writeSize256 = (bn+256-1)/256;
  if (fwrite(levelA, sizeof(ub4), writeSize256, fp) != writeSize256){
    fprintf(stderr, "save error %s\n", fileName);
    return -1;
  }
  
  if (fwrite(levelB, sizeof(ub), writeSizeINTBLOCK, fp) != writeSizeINTBLOCK){
    fprintf(stderr, "save error %s\n", fileName);
    return -1;
  }

  const size_t writeSize4 = (bn+4-1)/4;
  if (fwrite(gtable, sizeof(ub), writeSize4, fp) != writeSize4){
    fprintf(stderr, "save error %s\n", fileName);
    return -1;
  }

  fclose(fp);

  string fileName_key(fileName);
  fileName_key += KEYEXT;
  FILE* fpkey = fopen(fileName_key.c_str(), "wb");
  if (fpkey == NULL){
    fprintf(stderr, "cannot open %s\n", fileName_key.c_str());
    return -1;
  }
  
  if (fwrite(&keysLen, sizeof(ub8), 1, fpkey) != 1){
    fprintf(stderr, "save error %s\n", fileName_key.c_str());
    return -1;
  }

  if (fwrite(strTable, sizeof(ub), keysLen, fpkey) != keysLen){
    fprintf(stderr, "save error %s\n", fileName_key.c_str());
    return -1;
  }

  if (fwrite(strOffsets, sizeof(ub8), n+1, fpkey) != n+1){
    fprintf(stderr, "save error %s\n", fileName_key.c_str());
    return -1;
  }

  fclose(fpkey);

  return 0;
}
 
int bhash::load(const char* fileName){
  FILE* fp = fopen(fileName, "rb");
  if (fp == NULL){
    fprintf(stderr, "cannot open %s\n", fileName);
    return -1;
  }

  clear();

  if (fread(&n, sizeof(ub4), 1, fp) != 1){
    fprintf(stderr, "load error %s\n", fileName);
    return -1;
  }

  if (fread(&seed, sizeof(ub4), 1, fp) != 1){
    fprintf(stderr, "load error %s\n", fileName);
    return -1;
  }

  bn_m = (ub4)(n * C_R / 3.f);
  bn = bn_m * 3;

  printf("read %d keys\n", n);

  const size_t writeSize32 = (bn+INTBLOCK-1)/INTBLOCK;
  B = new ub4 [writeSize32];
  if (fread(B, sizeof(ub4), writeSize32, fp) != writeSize32){
    fprintf(stderr, "load error %s 1\n", fileName);
    return -1;
  }
  
  const size_t writeSize256 = (bn+256-1)/256;
  levelA = new ub4 [writeSize32];
  if (fread(levelA, sizeof(ub4), writeSize256, fp) != writeSize256){
    fprintf(stderr, "load error %s 2\n", fileName);
    return -1;
  }
  
  levelB = new ub [writeSize32];
  if (fread(levelB, sizeof(ub), writeSize32, fp) != writeSize32){
    fprintf(stderr, "load error %s 3\n", fileName);
    return -1;
  }

  const size_t writeSize4 = (bn+4-1)/4;
  gtable = new ub [writeSize4];
  if (fread(gtable, sizeof(ub), writeSize4, fp) != writeSize4){
    fprintf(stderr, "load error %s 4\n", fileName);
    return -1;
  }
  fclose(fp);

  string fileName_key(fileName);
  fileName_key += KEYEXT;
  FILE* fpkey = fopen(fileName_key.c_str(), "rb");
  if (fpkey == NULL){
    fprintf(stderr, "cannot open %s\n", fileName_key.c_str());
    return -1;
  }

  if (fread(&keysLen, sizeof(ub8), 1, fpkey) != 1){
    fprintf(stderr, "load error1 %s\n", fileName_key.c_str());
    return -1;
  }  

  strTable = new ub [keysLen];
  if (fread(strTable, sizeof(ub), keysLen, fpkey) != keysLen){
    fprintf(stderr, "load error2 %s\n", fileName_key.c_str());
    return -1;
  }

  strOffsets = new ub8 [n+1];
  if (fread(strOffsets, sizeof(ub8), n+1, fpkey) != n+1){
    fprintf(stderr, "load error3 %s\n", fileName_key.c_str());
    return -1;
  }

  fclose(fpkey);
  return 0;  
}

// Hash function: http://www.burtleburtle.net/bob/c/lookup8.c
void bhash::bob_mix(ub4& a, ub4& b, ub4& c) const {
  a -= b; a -= c; a ^= (c >> 13);
  b -= c; b -= a; b ^= (a <<  8);
  c -= a; c -= b; c ^= (b >> 13);
  a -= b; a -= c; a ^= (c >> 12);
  b -= c; b -= a; b ^= (a << 16);
  c -= a; c -= b; c ^= (b >>  5);
  a -= b; a -= c; a ^= (c >>  3);
  b -= c; b -= a; b ^= (a << 10);
  c -= a; c -= b; c ^= (b >> 15);
}

void bhash::bob_mix64(ub8& a, ub8& b, ub8& c) const{
  a -= b; a -= c; a ^= (c>>43);	
  b -= c; b -= a; b ^= (a<<9);	
  c -= a; c -= b; c ^= (b>>8);	
  a -= b; a -= c; a ^= (c>>38);	
  b -= c; b -= a; b ^= (a<<23);	
  c -= a; c -= b; c ^= (b>>5);	
  a -= b; a -= c; a ^= (c>>35);	
  b -= c; b -= a; b ^= (a<<49);	
  c -= a; c -= b; c ^= (b>>11);	
  a -= b; a -= c; a ^= (c>>12);	
  b -= c; b -= a; b ^= (a<<18);	
  c -= a; c -= b; c ^= (b>>22);	
}

void bhash::bob_hash(const ub* str, const ub4 length, const ub4 init, ub4& a, ub4& b, ub4& c) const{
  ub4 len = length;
  a = b = 0x9e3779b9; // the golden ratio
  c = init;

  while (len >= 12){
    a += *(ub4*)(str+0);
    b += *(ub4*)(str+4);
    c += *(ub4*)(str+8);
    bob_mix(a, b, c);
    str += 12;
    len -= 12;
  }

  c += length;
  switch (len) {
  case 11: c += ((ub4) str[10] << 24);
  case 10: c += ((ub4) str[ 9] << 16);
  case  9: c += ((ub4) str[ 8] <<  8);
    // the first byte of c is reserved for the length
  case  8: b += ((ub4) str[ 7] << 24);
  case  7: b += ((ub4) str[ 6] << 16);
  case  6: b += ((ub4) str[ 5] <<  8);
  case  5: b += ((ub4) str[ 4]      );
  case  4: a += ((ub4) str[ 3] << 24);
  case  3: a += ((ub4) str[ 2] << 16);
  case  2: a += ((ub4) str[ 1] <<  8);
  case  1: a += str[ 0];
  }
  bob_mix(a, b, c);
}

void bhash::bob_hash64(const ub* str, const ub4 length, const ub4 init, ub8& a, ub8& b, ub8& c) const{
  ub4 len = length;
  a = b = init;
  c = 0x9e3779b97f4a7c13LL; // the golden ratio

  while (len >= 24){
    a += *(ub8*)(str);
    b += *(ub8*)(str+8);
    c += *(ub8*)(str+16);
    bob_mix64(a, b, c);
    str += 24;
    len -= 24;
  }

  c += length;
  switch(len)              /* all the case statements fall through */
    {
    case 23: c+=((ub8)str[22]<<56);
    case 22: c+=((ub8)str[21]<<48);
    case 21: c+=((ub8)str[20]<<40);
    case 20: c+=((ub8)str[19]<<32);
    case 19: c+=((ub8)str[18]<<24);
    case 18: c+=((ub8)str[17]<<16);
    case 17: c+=((ub8)str[16]<<8);
      /* the first byte of c is reserved for the length */
    case 16: b+=((ub8)str[15]<<56);
    case 15: b+=((ub8)str[14]<<48);
    case 14: b+=((ub8)str[13]<<40);
    case 13: b+=((ub8)str[12]<<32);
    case 12: b+=((ub8)str[11]<<24);
    case 11: b+=((ub8)str[10]<<16);
    case 10: b+=((ub8)str[ 9]<<8);
    case  9: b+=((ub8)str[ 8]);
    case  8: a+=((ub8)str[ 7]<<56);
    case  7: a+=((ub8)str[ 6]<<48);
    case  6: a+=((ub8)str[ 5]<<40);
    case  5: a+=((ub8)str[ 4]<<32);
    case  4: a+=((ub8)str[ 3]<<24);
    case  3: a+=((ub8)str[ 2]<<16);
    case  2: a+=((ub8)str[ 1]<<8);
    case  1: a+=((ub8)str[ 0]);
    }
  bob_mix64(a,b,c);
}

