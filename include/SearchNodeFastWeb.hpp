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
#include <thread>

#include "Index.hpp"
#include "Utils.hpp"
#include "SuffixArray.hpp"
#include "Timer.hpp"
#include "System.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

#include <tbb/task.h>
#include <tbb/task_group.h>





struct SearchNodeFastWeb {
    Index index;

    int listenfd,connfd,n;
    socklen_t clilen;
    pid_t     childpid;
    char mesg[1000];
    bool indexed=false;
    std::unique_ptr<std::thread> indexThread;
    tbb::task_group group;

    void serve(int port) {
        listenfd=socket(AF_INET,SOCK_STREAM,0);

        sockaddr_in servaddr,cliaddr;
        bzero(&servaddr,sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
        servaddr.sin_port=htons(port);
        bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

        listen(listenfd,1024);
        for(;;) {
            clilen=sizeof(cliaddr);
            auto connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);

            group.run([&,connfd]() {
                n = recvfrom(connfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&clilen);
                mesg[n] = 0;
                printf("RECV: %s",mesg);

                std::string reply;
                if (mesg[5]=='h') {
                    reply = "{\"success\": \"true\"}";
                } else if (mesg[5]=='i'&&mesg[6]=='n') {
                    const char* start = mesg+16;
                    const char* end = start;
                    for (; *end != ' '; ++end);
                    std::string path(start,std::distance(start,end));
                    index.index(urlDecode(path));
                    indexed=true; //unsafe
                } else if (mesg[5]=='i'&&mesg[6]=='s') {
                    if (indexed) // unsafe
                        reply = "{\"success\": \"true\"}";
                    else
                        reply = "{\"success\": \"false\"}";
                } else if (mesg[6]=='q') {
                    const char* start = mesg+8;
                    const char* end = start;
                    for (; *end != ' '; ++end);
                    std::string word(start,std::distance(start,end));

                    std::stringstream r;
                    Timer t;
                    r << R"({ "success": "true", "results": [)";
                    bool first=true;
                    for (auto e : index.find(word)) {
                        if (first) { first = !first; }
                        else { r << ","; }
                        r << "\"" << e << "\"";
                    }
                    r << R"(]})";
                    r << "\r\n";
                    //std::cout << (t.getMicro()/1000.0) << std::endl;
                    reply = r.str();

                    //std::cout << "Finding >>" << word << "<<" << std::endl;
                }

                stringstream replyStream;
                replyStream << "HTTP/1.1 200 OK\r\n";
                replyStream << "Content-Length: " << reply.length() << "\r\n";
                replyStream << "Content-Type: application/json\r\n";
                replyStream << "\r\n";
                replyStream << reply;

                //std::cout << "SENDING: " << replyStream.str() << std::endl;

                sendto(connfd,replyStream.str().c_str(),replyStream.str().length(),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
                close(connfd);
            });
        }
    }
};







/*class SearchNodeWebserver {
    /// The index
    std::vector<std::unique_ptr<Index>> indexs;

    /// Handle true/false requests uniformly by checking whether indexing is done or not
    class PingHandler : public HttpRequestHandler {
        SearchNodeWebserver& sm;
    public:
        /// Constructor
        PingHandler(SearchNodeWebserver& sm) : sm(sm) {}
        /// Handler
        void get(HttpRequest* const request, const vector<string>& args) {
            this->reply(request, 200, args.front());
        }
    };  

    /// Handle true/false requests uniformly by checking whether indexing is done or not
    class IsIndexedHandler : public HttpRequestHandler {
        SearchNodeWebserver& sm;
    public:
        /// Constructor
        IsIndexedHandler(SearchNodeWebserver& sm) : sm(sm) {}
        /// Handler
        void get(HttpRequest* const request, const vector<string>& args) {
            if (sm.isIndexed()) {
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
            for (auto& index : indexes) {
                index->index(urlDecode(args.front()));
            }
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
            Timer t;
            r << R"({ "success": "true", "results": [)";
            bool first=true;
            for (auto e : indexes.front()->find(args.front())) {
                if (first) { first = !first; }
                else { r << ","; }
                r << "\"" << e << "\"" << "\n";
            }
            r << R"(]})";
            std::cout << (t.getMicro()/1000.0) << std::endl;
            this->reply(request, 200, r.str());
        }
    };

    SearchNodeWebserver() {
        indexes.emplace_back(new Index(0));
        indexes.emplace_back(new Index(1));
        indexes.emplace_back(new Index(2));
        indexes.emplace_back(new Index(3));
    }

    bool isIndexed() {
        bool res = true;
        for (auto& index : indexes) res &= index->isIndexed();
        return res;
    }

public:
    /// Start server
    void serve(int port) {
        AsyncHttpServer* server = new AsyncHttpServer(port);
        server->add_handler("^/ping?time=(.*?)$", new PingHandler(*this));
        server->add_handler("^/healthcheck$", new HealthCheckHandler(*this));
        server->add_handler("^/isIndexed$", new IsIndexedHandler(*this));
        server->add_handler("^/index\\?path=(.*?)$", new IndexHandler(*this));
        server->add_handler("^/\\?q=(.*?)$", new QueryHandler(*this));
        IOLoop::instance()->start();
    }
};

*/