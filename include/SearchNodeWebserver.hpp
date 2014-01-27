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
#include <unordered_set>

#include "Index.hpp"
#include "Utils.hpp"
#include "SuffixArray.hpp"
#include "System.hpp"


class SearchNodeWebserver {
    /// The index
    Index index;

    /// Handle true/false requests uniformly by checking whether indexing is done or not
    class IsIndexedHandler : public HttpRequestHandler {
        SearchNodeWebserver& sm;
    public:
        /// Constructor
        IsIndexedHandler(SearchNodeWebserver& sm) : sm(sm) {}
        /// Handler
        void get(HttpRequest* const request, const vector<string>& args) {
            if (sm.index.isIndexed()) {
                this->reply(request, 200, "{\"success\": \"true\"}");
            } else {
                this->reply(request, 200, "{\"success\": \"false\"}");
            }
        }
    };  

    /// Handle true/false requests uniformly by checking whether indexing is done or not
    class HealthCheckHandler : public HttpRequestHandler {
        SearchNodeWebserver& sm;
    public:
        /// Constructor
        HealthCheckHandler(SearchNodeWebserver& sm) : sm(sm) {}
        /// Handler
        void get(HttpRequest* const request, const vector<string>& args) {
            this->reply(request, 200, "{\"success\": \"true\"}");
        }
    };    

    /// Handle index calls by dispatching them to the index
    class IndexHandler : public HttpRequestHandler {
        SearchNodeWebserver& sm;
    public:
        /// Constructor
        IndexHandler(SearchNodeWebserver& sm) : sm(sm) {}
        /// Handler
        void get(HttpRequest* const request, const vector<string>& args) {
            sm.index.index(urlDecode(args.front()));
            this->reply(request, 200, "");
        }
    };

    /// Handle queries by dispatching them to the index
    class QueryHandler : public HttpRequestHandler {
        SearchNodeWebserver& sm;
    public:
        /// Constructor
        QueryHandler(SearchNodeWebserver& sm) : sm(sm) {}
        /// Handler
        void get(HttpRequest* const request, const vector<string>& args) {
            std::stringstream r;
            r << R"({ "success": "true", "results": [)";
            bool first=true;
            for (auto e : sm.index.find(args.front())) {
                if (first) { first = !first; }
                else { r << ","; }
                r << "\"" << e << "\"" << "\n";
            }
            r << R"(]})";
            this->reply(request, 200, r.str());
        }
    };

public:
    /// Start server
    void serve(int port) {
        AsyncHttpServer* server = new AsyncHttpServer(port);
        server->add_handler("^/healthcheck$", new HealthCheckHandler(*this));
        server->add_handler("^/isIndexed$", new IsIndexedHandler(*this));
        server->add_handler("^/index\\?path=(.*?)$", new IndexHandler(*this));
        server->add_handler("^/\\?q=(.*?)$", new QueryHandler(*this));
        IOLoop::instance()->start();
    }
};

