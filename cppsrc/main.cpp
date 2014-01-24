#include <cassert>
#include <iostream>
#include <httpcpp.h>


/*
            cout << "-----------------------------------" << endl;
            cout << "Handler A receives:" << endl;
            cout << "method: " << request->get_method() << endl;
            cout << "path  : " << request->get_path() << endl;
            cout << "body  : " << request->get_body() << endl;
            for (int i = 0; i < args.size(); i++) {
                cout << "arg  : " << args[i] << endl;
            }
            this->reply(request, 200, "A=>" + request->get_body());
*/


class SearchNode {
    class HealthCheckHandler : public HttpRequestHandler {
        SearchNode& sm;
    public:
        /// Constructor
        HealthCheckHandler(SearchNode& sm) : sm(sm) {}
        /// Handler
        void get(HttpRequest* const request, const vector<string>& args) {
            cout << "-----------------------------------" << endl;
            cout << "Handler A receives:" << endl;
            cout << "method: " << request->get_method() << endl;
            cout << "path  : " << request->get_path() << endl;
            cout << "body  : " << request->get_body() << endl;
            for (int i = 0; i < args.size(); i++) {
                cout << "arg  : " << args[i] << endl;
            }
            this->reply(request, 200, "{\"success\": \"false\"}");
        }
    };

public:
    /// Start server
    void serve(int port) {
        AsyncHttpServer* server = new AsyncHttpServer(port);
        server->add_handler("^/healthcheck$", new HealthCheckHandler(*this));
        server->add_handler("^/isIndexed$", new HealthCheckHandler(*this));
        server->add_handler("^/index\\?path=(.*?)$", new HealthCheckHandler(*this));
        server->add_handler("^/?q=(.*?)$", new HealthCheckHandler(*this));
        IOLoop::instance()->start();
    }
};

int main(int argc,char* argv[]) {
    std::vector<std::string> args(argv,argv+argc);
    for (auto& a : args) std::cout << a << std::endl;
    int idIndex=0; for (; idIndex<args.size()-1 && args[idIndex]!="--id"; ++idIndex); ++idIndex; assert(idIndex < args.size()&&"--id arg not found");
    int id=atoi(args[idIndex].c_str());

    SearchNode sm;
    sm.serve(9090 + id);
}
