_imported from http://www-tsujii.is.s.u-tokyo.ac.jp/~hillbig/bep.htm_

# Bep: Associative Arrays for Very Large Collections

[Japanese](http://www-tsujii.is.s.u-tokyo.ac.jp/~hillbig/bep-j.htm)

## Abstract

**Bep** is a library for the associative arrays for very large collections. Associative arrays are composed of a collection of keys and a collection of values, where each key (characters) is associated with one value. In C++, STL map or hash_map are used, but it cannot manage very large collections because of its heavy memory usage. Bep uses Minimal Perfect Hash Functions and keeps the collection compactly; it requires only 3bit per key (it also requires keys to be stored. In total, it requires (the total length of keys) + (3/8 * the number of keys ) bytes). Download Bep is free software; you can redistribute it and/or modify it under the terms of the new BSD License.

## Download

**Bep** is free software; you can redistribute it and/or modify it under the terms of the new BSD License.

*   bep-0.01.tar.gz: [HTTP](http://www-tsujii.is.s.u-tokyo.ac.jp/~hillbig/software/bep-0.01.tar.gz)

## News

*   **2007-10-27**: Bep 0.01  

    *   0.01 Release

## Usage

Download source code from above link. Then type as follows,

<pre>% tar xvzf bep-0.xx.tar.gz
% cd bep-0.xx
% ./configure
% make
% make install
</pre>

If you want to use it as a library, you add "#include <bep bep.hpp="">" to your program and put "-lbep" as a compiler option.

## Sample Program

### bep_driver

<pre>% ./bep_driver wordlist index </pre>

Read a file (wordlist) containing a keyword per line and then build an associative dictionary. It checks whether it can save, load, or lookup correctly.

### bhash_driver

<pre>% ./bhash_driver wordlist index
</pre>

Read a file (wordlist) containing a keyword per line and then build a minimal perfect hash function. Next, it compares the performance between bhash::lookup_wocheck, bhash::lookup, and tr1::unordered_map.

### Example

<pre>% cat wordlist
Abraham
Abrahamn
Abramsky,
Absalom
Abson,
...
% wc wordlist
  3775712   3805231 224432799 wordlist

% ./bep_driver wordlist index
3.3642 0.8910 usec

% ./bhash_driver wordlist index
n:3775712 bn:4908423
read 3775712 keys
lookup_wo 1.713607 key:0.453850 usec
lookup    3.365828 key:0.891442 usec
tr1umap   2.279891 key:0.603831 usec
</pre>

## Interface

### bep_tool::bep (Associative Arrays)

`int bep_tool::bep::build(vector<string>& keys, vector<valType>& vals)`  
Build associative arrays from keys and vals. Return 0 if it built the arrays correctly and -1 if it failed. Input keys are not necessarily sorted. If associative arrays were already built, old arrays are discarded. This function failed if (1) keys include identical keys, or it cannot build a minimal perfect hash function. If so, please change keys by adding dummy keys.

`int bep_tool::bep::save(const char* fileName, valType*& vals)`  
Save current associative arrays to fileName. Return 0 if it succeeded and -1 if it failed.  
**NOTE**: bep::save do NOT store values because values can have arbitrary types and bep cannot know how to store them. Therefore, bep::save functions returns vals as argument. The number of returned vals can be obtained by bep::size(). The user should store vals by themselves.

`int bep_tool::bep::load(const char* fileName, valType*& vals)`  
Load associative arrays from fileName. Return 0 if it succeeded and -1 if it failed.  
**NOTE**: bep::load do NOT load values from files because values can have arbitrary types and bep cannot know how to read them. Therefore bep::load requires vals as argument. The user should load vals by themselves and set vals as an argument of bep::load. You need to keep the order of vals.

`valType& bep_tool::bep::operator [](const string& key)`  
Return the value associated by key. If key is not registred, new pair of key and values will be created (same as STL map).

`bool bep_tool::bep::exist(const string& key)`  
Check whether there is an value associated with key. Return true if it exist, and false if not.

`size_t bep_tool::bep::size() const`  
Return the number of registered keys.

### bep_tool::bhash (Minimal Perfect Hash Function)

bhash is a class for managing a minimal perfect hash function, which is used by bep. You can directly use bhash without bep. Minimal perfect hash function is a function which guarantees (1) No collision occur between registered keys (2) Hash value is between [0... # of keys -1]. You need to build an hash function before using it.

`int bep_tool::bep::build(const vector<string>& keys)</string>`  
Build a minimal perfect hash function for keys. Return 0 if it succeeded, and -1 if it failed. Keys are not necessarily sorted. It failed if there are identical keys and you need to remove such keys (by using uniq).

`int bep_tool::bhash::save(const char* fileName) const`  
Save current hash function in fileName. Return 0 if it succeeded and -1 if it failed.

`int bep_tool::bhash::load(const char* fileName);`  
Load a hash function from filename. Return 0 if it succeeded and -1 if it failed.

`bep_tool::ub4 bep_tool::bhash::lookup(const string& key) const;`  
Return the hash value for key. These hash values have no collision, and have some value between [0...bhash::size()-1]. If key is not registred one, return bep_tool::NOTFOUND.

`ub4 bep_tool::bhash::lookup_wocheck(const string& key) const;`  
Return the hash value for key. These hash values are not collisioned among registered keys and have some value between [0...bhash::size()-1]. If key is not registered one, return some value between [0...bhash::size()-1]. Since there is no key check, bhash::lookup_wocheck is faster than bhash::lookup.

`bool bep_tool::bhash::lookupKey(const size_t i, string& key) const;`  
Return true if i < bhash::size() and set its key to "key". Return false if i>=bhash::size().

`ub4 size() const;`  
Return the number of registered keys.

## Performance Test

TODO

## NOTE

BEP is optimized for bep::build which builds an associative arrays in batch manner. Bep also supports online update, but it is very slow. Please use bep::build as much as possible.

## TODO

*   Portability (Current version only work on 64bit Intel)
*   Interface
*   Performance Test
*   Write Technical details
*   Speedup, Reduce space requirements
*   Applications

## Others

Bep is named after famous hotspring spot "Beppu" where I made initial bep.

## Reference, Link

*   If you want to use complex queries (such as common prefix search), you can use other data structures.([tx](http://www-tsujii.is.s.u-tokyo.ac.jp/~hillbig/tx-j.htm) or [darts](http://chasen.org/~taku/software/darts/) )
*   Botelho, F.C., Pagh, R. and Ziviani, N. "Simple and Space-Efficient Minimal Perfect Hash Functions" 10th International Workshop on Algorithms and Data Structures (WADS07) August 2007, 139-150\. [pdf](http://homepages.dcc.ufmg.br/~nivio/papers/wads07.pdf)
*   [Darts](http://www.chasen.org/~taku/software/darts/): Trie implementation using Double Array
*   [Bob's Hash](http://www.burtleburtle.net/bob/hash/doobs.html) Hash function used in bhash.[64bit code](http://www.burtleburtle.net/bob/c/lookup8.c)

## Contact

Comments or Bug report  
[Daisuke Okanohara](http://www-tsujii.is.s.u-tokyo.ac.jp/~hillbig/atlab-j.html) (hillbig at is.s.u-tokyo.ac.jp) (Replace "at" with @)