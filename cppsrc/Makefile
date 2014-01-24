all: main runner

main: main.cpp SearchNode.hpp
	g++ -std=c++11 -O3 -g -pthread  -L libs/httpcpp/lib -I libs/httpcpp -o $@ $< -lhttpcpp

runner: runner.cpp SearchNode.hpp
	g++ -std=c++11 -O3 -g -pthread  -L libs/httpcpp/lib -I libs/httpcpp -o $@ $< -lhttpcpp

httpcpp:
	mkdir -p libs; \
	cd libs; \
	rm -rf httpcpp ; \
	git clone https://github.com/henrik-muehe/httpcpp.git ; \
	cd httpcpp ; \
	make ; \
	cp lib/* ../..

clean:
	rm -f main runner