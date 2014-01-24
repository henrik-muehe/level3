#include "SearchNode.hpp"

int main(int argc,char* argv[]) {
    std::vector<std::string> args(argv,argv+argc);
    for (auto& a : args) std::cout << a << std::endl;
    int idIndex=0; for (; idIndex<args.size()-1 && args[idIndex]!="--id"; ++idIndex); ++idIndex; assert(idIndex < args.size()&&"--id arg not found");
    int id=atoi(args[idIndex].c_str());

    SearchNode sm;
    sm.serve(9090 + id);
}
