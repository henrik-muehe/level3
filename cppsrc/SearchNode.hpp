#include <cassert>
#include <iostream>
#include <httpcpp.h>
#include <fstream>
#include <sstream>
#include <fts.h>
#include <string.h>
#include <functional>
#include <stdint.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <divsufsort.h>
#include <lfs.h>
#include <unordered_set>


void for_files(const std::string& path,std::function<void (const std::string&)> callback)  {
    char *paths[] = {const_cast<char*>(path.c_str()), 0};
    FTS *tree = fts_open(paths, FTS_NOCHDIR, 0);
    if (!tree) {
        perror("fts_open");
        throw;
    }

    FTSENT *node;
    while ((node = fts_read(tree))) {
        if (node->fts_level > 0 && node->fts_name[0] == '.')
            fts_set(tree, node, FTS_SKIP);
        else if (node->fts_info & FTS_F) {
            callback(node->fts_path);
        }
    }
    if (errno) {
        perror("fts_read");
        throw;
    }

    if (fts_close(tree)) {
        perror("fts_close");
        throw;
    }
}


std::string urlDecode(const std::string &SRC) {
    std::string ret;
    char ch;
    int i, ii;
    for (i=0; i<SRC.length(); i++) {
        if (int(SRC[i])==37) {
            sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } else {
            ret+=SRC[i];
        }
    }
    return (ret);
}


struct SuffixArray {
    sauchar_t* T;
    saidx_t* SA;
    int64_t n;

    /// Constructor
    SuffixArray() : T(nullptr),SA(nullptr),n(0) {};
    /// Constructor
    SuffixArray(const std::string& str) : T(nullptr),SA(nullptr),n(str.length()) {
        build(str);
    }
    /// Build from string
    void build(const std::string& text) {
        n=text.length();

        releaseMemory();
        T = (sauchar_t *)malloc((size_t)n * sizeof(sauchar_t));
        SA = (saidx_t *)malloc((size_t)n * sizeof(saidx_t));
        assert(T && SA && "Allocation failed.");
        memcpy(T,text.c_str(),text.length());
        assert(divsufsort(T, SA, (saidx_t)text.length()) == 0);
    }
    /// Find a string
    std::vector<int64_t> find(const std::string& pattern) {
        std::vector<int64_t> res;
        saidx_t left;
        saidx_t size = sa_search(T, (saidx_t)n,(const sauchar_t *)pattern.c_str(), (saidx_t)pattern.length(),SA, (saidx_t)n, &left);
        for(saidx_t i = 0; i < size; ++i) {
            res.push_back(SA[left + i]);
        }
        return res;
    }
    /// Destructor
    ~SuffixArray() { releaseMemory(); }
    /// Release memory
    void releaseMemory() {
        return;
        free(T);
        T=nullptr;
        free(SA);
        SA=nullptr;
    }
};


struct Fingerprint {
	int64_t value;
	Fingerprint(const std::string& str) : value(0) {
		for (char c : str) {
			if (c >= 'a' && c <= 'z') value |= (1 << (c-'a'));
			else if (c >= 'A' && c <= 'Z') value |= (1 << (c-'a'+26));
			else if (c >= '0' && c <= '9') value |= (1 << (c-'a'+26+26));
		}
	}

	bool contains(const Fingerprint& other) const {
		return (other.value & value) == other.value;
	}
};

class SearchNode {
    class IsIndexedHandler : public HttpRequestHandler {
        SearchNode& sm;
    public:
        /// Constructor
        IsIndexedHandler(SearchNode& sm) : sm(sm) {}
        /// Handler
        void get(HttpRequest* const request, const vector<string>& args) {
            if (sm.indexed) {
                this->reply(request, 200, "{\"success\": \"true\"}");
            } else {
                this->reply(request, 200, "{\"success\": \"false\"}");
            }
        }
    };    

    class IndexHandler : public HttpRequestHandler {
        SearchNode& sm;
    public:
        /// Constructor
        IndexHandler(SearchNode& sm) : sm(sm) {}
        /// Handler
        void get(HttpRequest* const request, const vector<string>& args) {
            sm.index(urlDecode(args.front()));
            this->reply(request, 200, "");
        }
    };

    class QueryHandler : public HttpRequestHandler {
        SearchNode& sm;
    public:
        /// Constructor
        QueryHandler(SearchNode& sm) : sm(sm) {}
        /// Handler
        void get(HttpRequest* const request, const vector<string>& args) {
            std::stringstream r;
            r << R"({ "success": "true", "results": [)";
            bool first=true;
            for (auto e : sm.find(args.front())) {
                if (first) { first = !first; }
                else { r << ","; }
                r << "\"" << e << "\"" << "\n";
            }
            r << R"(]})";
            this->reply(request, 200, r.str());
        }
    };


    struct File {
        std::string filename;
        std::string path;
        std::vector<std::string> lines;
        std::vector<Fingerprint> fingerprints;
        std::string text;
        SuffixArray sa;
        std::vector<int64_t> linebreaks;

        File(const std::string& filename,const std::string& path) : filename(filename),path(path) {
            ifstream f(path);
            std::string line;
            while(std::getline(f,line,'\n')) {
            	lines.push_back(line);
            	fingerprints.emplace_back(line);
                text+=line + "\n";
                linebreaks.push_back(text.length());
            }
            sa.build(text);
        }

        void find(const std::string& needle, std::vector<std::string>& res) {
            auto ms = sa.find(needle);
            std::unordered_set<int64_t> usedLines;
            for (auto& m : ms) {
                auto iter=std::lower_bound(linebreaks.begin(),linebreaks.end(),m);
                std::stringstream r; 
                auto line=(std::distance(linebreaks.begin(),iter)+1);
                if (!usedLines.count(line)) {
                    r << filename << ":" << line;
                    usedLines.insert(line);
                } else { continue; }
                res.push_back(r.str());
            }
            /*Fingerprint nf(needle);
            for (uint64_t index=0; index<lines.size(); ++index) {
                if (fingerprints[index].contains(nf) && lines[index].find(needle) != std::string::npos) {
                    std::stringstream r; r << filename << ":" << index+1;
                    res.push_back(r.str());
                }
            }*/
        }
    };

    std::vector<File> files;
    bool indexed=false;

public:
    std::vector<std::string> find(const std::string& needle) {
        std::vector<std::string> res;
        for (auto& f : files) f.find(needle,res);
        return res;
    }

    void index(const std::string& path) {
        // Find all files
        for_files(path, [&](const std::string& file) {
            files.emplace_back(file.substr(path.length()+1),file);
        });
        indexed=true;
    }

    /// Start server
    void serve(int port) {
        AsyncHttpServer* server = new AsyncHttpServer(port);
        server->add_handler("^/healthcheck$", new IsIndexedHandler(*this));
        server->add_handler("^/isIndexed$", new IsIndexedHandler(*this));
        server->add_handler("^/index\\?path=(.*?)$", new IndexHandler(*this));
        server->add_handler("^/\\?q=(.*?)$", new QueryHandler(*this));
        IOLoop::instance()->start();
    }
};

