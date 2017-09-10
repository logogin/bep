#ifndef __BEP_HPP__
#define __BEP_HPP__

#include "bhash.hpp"

//#ifdef HAVE_TR1_UNORDERED_MAP
//#include <tr1/unordered_map>
//#else
#include <map>
//#endif

namespace bep_tool{

const static size_t MAXWORKSIZE = 1048576; // 1M

using namespace std;

template <class valType> 
class bep{
  typedef map<string, valType> umap;
  typedef typename map<string, valType>::iterator umap_iterator;
  
public:
  bep() : _vals(NULL) {}
  ~bep() {
    if (_vals) delete[] _vals;
  }
 
  // build using string as a key and valType as a value
  int build(vector<string>& keys, vector<valType>& vals){
    if (keys.size() != vals.size()){
      fprintf(stderr, "keys.size():%u != vals.size():%u\n", (unsigned int)keys.size(), (unsigned int)vals.size());
      return -1;
    }
    if (_bh.build(keys) == -1){
      return -1;
    }
    if (_vals) delete[] _vals;
    _vals = new valType[vals.size()];
    for (size_t i = 0; i < vals.size(); i++){
      _vals[_bh.lookup_wocheck(keys[i])] = vals[i];
    }

    return 0;    
  }

  int build(vector<pair<string, valType> >& dic){
    vector<string> keys;
    vector<valType> vals;
    for (size_t i = 0; i < dic.size(); i++){
      keys.push_back(dic[i].first);
      vals.push_back(dic[i].second);
    }
    
    return build(keys, vals);
  }
  
  int build() {
    vector<string> keys;
    vector<valType> vals;
    for (size_t i = 0; i < _bh.size(); i++){
      string key;
      _bh.lookupKey(i, key);
      keys.push_back(key);
      vals.push_back(_vals[i]);
    }

    umap_iterator it = _work.begin(); 
    for ( ; it != _work.end(); it++){
      keys.push_back(it->first);
      vals.push_back(it->second);
    }    
    _work.clear();
    return build(keys, vals);
  }

  
  // save current perfect hash function to fileName
  // NOT save vals because we don't know how to store vals
  int save(const char* fileName, valType* & vals) {
    if (build() == -1) return -1;
    vals = _vals;
    return _bh.save(fileName);
  }
  
  // load fileName to current perfect hash function
  int load(const char* fileName, valType* & vals) {
    if (_bh.load(fileName) == -1) return -1;
    if (_vals) delete[] _vals;
    _vals = new valType [_bh.size()];
    for (size_t i = 0; i < _bh.size(); i++){
      _vals[i] = vals[i];
    }
    return 0;
  }

  // return registered val
  valType& operator[] (const string& key){
    ub4 id = 0;
    if (_bh.size() != 0 && (id = _bh.lookup(key)) != NOTFOUND){
      return _vals[id];
    }
    umap_iterator it = _work.find(key);
    if (it != _work.end()){
      return it->second;
    } else {
      if (_work.size() == MAXWORKSIZE) {
	build(); // merge _work and _bh
      }
      return _work[key];
    }
  }

  bool exist(const string& key){
    if (_bh.lookup(key) == NOTFOUND){
      if (_work.count(key) == 0) return false;
    }
    return true;
  }

  size_t size() const {
    return _bh.size() + _work.size();
  }

private:
  bool lookup(const size_t i, string& key, valType& val){
    if (i >= _bh.size()) return false;
    _bh.lookupKey(i, key);
    val = _vals[i];
    return true;
  }

  bhash _bh;      // perfect hash. key to ID
  valType* _vals;
  umap _work;
};
  
}

#endif // __BEP_HPP__
