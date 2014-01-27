#include "SearchNodeWebserver.hpp"
#include "System.hpp"
//#include <tbb/task_group.h>

//using namespace tbb;


int main(int argc,char* argv[]) {
    Index idx;

    System::profile("Indexing", [&]() {
    	idx.index(argv[1]);
	});

    int64_t hash=0;
    System::profile("Find", [&]() {
        for (int i =0; i<100; ++i) {
            std::vector<std::string> words { "insured", "superfinish", "vapulatory", "eyepiece", "Anamnia", "crooner", "archgenethliac", "palisfy", "cofferlike", "Caucasian", "updeck", "ungnawn", "Mniotiltidae", "interviewable", "unbarbarize", "militation", "sonnetish", "Mocoa", "haybird", "birken", "aphlebia", "Griqua", "superincrease", "oophorectomy", "Bryanism", "macaasim", "upcity", "Rinaldo", "psycholeptic", "decumana", "crewelwork", "polarity", "shireman", "peytrel", "Amakosa", "acaleph", "Eugene", "pseudotributary", "hemachate", "periclasia", "Chrysopsis", "Chimu", "wellyard", "unlustily", "acroblast", "feastfully", "teewhaap", "gapa", "method", "literato" };
                for (auto& word : words) {
                    auto res = idx.find(word);
                    std::sort(res.begin(),res.end());
                    for (auto& a : res) {
                        //std::cout << a << std::endl;
                        hash += std::hash<std::string>()(a);
                    }
                }
        }
    });
    std::cout << hash << std::endl; // -137712748444634040
}
