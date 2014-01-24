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
        std::string text;

        File(const std::string& filename,const std::string& path) : filename(filename),path(path) {
            ifstream f(path);
            text=std::string((std::istreambuf_iterator<char>(f)),std::istreambuf_iterator<char>());
        }

        void find(const std::string& needle, std::vector<std::string>& res) {
            std::stringstream textStream(text);
            std::string line;
            uint64_t index=0;
            while(std::getline(textStream,line,'\n')) {
                ++index;
                if (line.find(needle) != std::string::npos) {
                    std::stringstream r; r << filename << ":" << index;
                    res.push_back(r.str());
                }
            }
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

