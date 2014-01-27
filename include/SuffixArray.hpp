#pragma once

#include <divsufsort.h>
#include <lfs.h>


/// A wrapper around a c suffix array
struct SuffixArray {
	/// Text text
    std::vector<char> T;
    /// The actual suffix array
    std::vector<saidx_t> SA;

    /// Constructor
    SuffixArray() {};
    /// Constructor
    SuffixArray(const std::string& str) {
        build(str);
    }
    /// Build from std::string
	void build(const std::string& str) {
		build(str.c_str(),str.length());
	}    
	/// Build from cstring
	void build(const char* ptr,uint64_t len) {
		std::vector<char> text(len);
        memcpy(text.data(),ptr,len);
        buildZeroCopy(std::move(text));
	}
    /// Build from c string by taking over the ptr
    void buildZeroCopy(std::vector<char>&& text) {
        T=std::move(text);
        SA.resize(T.size());
        assert(divsufsort(
        	reinterpret_cast<sauchar_t*>(T.data()), 
        	SA.data(), 
        	static_cast<saidx_t>(T.size())) == 0);
    }
    /// Find a string
    std::vector<int64_t> find(const std::string& pattern) {
        std::vector<int64_t> res;
        saidx_t left;
        saidx_t size = sa_search(
        	reinterpret_cast<sauchar_t*>(T.data()), 
        	static_cast<saidx_t>(T.size()),
        	reinterpret_cast<const sauchar_t*>(pattern.c_str()), 
        	static_cast<saidx_t>(pattern.length()),
        	SA.data(),
        	static_cast<saidx_t>(T.size()), 
        	&left
        );
        for(saidx_t i = 0; i < size; ++i) {
            res.push_back(SA[left + i]);
        }
        return res;
    }
};
