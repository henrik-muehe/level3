#include "SearchNode.hpp"
#include "System.hpp"
std::string random_string( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

int main(int argc,char* argv[]) {
    SearchNode sm;

    System::profile("Indexing", [&]() {
    	sm.index(argv[1]);
	});

    std::vector<std::string> words { "method" }; //{ "insured", "superfinish", "vapulatory", "eyepiece", "Anamnia", "crooner", "archgenethliac", "palisfy", "cofferlike", "Caucasian", "updeck", "ungnawn", "Mniotiltidae", "interviewable", "unbarbarize", "militation", "sonnetish", "Mocoa", "haybird", "birken", "aphlebia", "Griqua", "superincrease", "oophorectomy", "Bryanism", "macaasim", "upcity", "Rinaldo", "psycholeptic", "decumana", "crewelwork", "polarity", "shireman", "peytrel", "Amakosa", "acaleph", "Eugene", "pseudotributary", "hemachate", "periclasia", "Chrysopsis", "Chimu", "wellyard", "unlustily", "acroblast", "feastfully", "teewhaap", "gapa", "method", "literato" };
    for (auto& word : words) {
        auto res = sm.find(word);
        std::sort(res.begin(),res.end());
        int64_t hash=0;
        for (auto& a : res) {
            std::cout << a << std::endl;
            //hash += std::hash<std::string>()(a);
        }
        //std::cout << word << ": " << hash << std::endl;
    }

    /*System::profile("Find", [&]() {
	    for (int i=0; i<100; ++i) 
    		sm.find(random_string(15));
    });*/
}


/*
insured
superfinish
vapulatory
eyepiece
Anamnia
crooner
archgenethliac
palisfy
cofferlike
Caucasian
updeck
ungnawn
Mniotiltidae
interviewable
unbarbarize
militation
sonnetish
Mocoa
haybird
birken
aphlebia
Griqua
superincrease
oophorectomy
Bryanism
macaasim
upcity
Rinaldo
psycholeptic
decumana
crewelwork
polarity
shireman
peytrel
Amakosa
acaleph
Eugene
pseudotributary
hemachate
periclasia
Chrysopsis
Chimu
wellyard
unlustily
acroblast
feastfully
teewhaap
gapa
method
literato
*/