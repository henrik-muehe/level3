all: main runner

main: src/main.cpp $(wildcard include/*.hpp)
	g++ -std=c++11 -O3 -g -pthread -I include -I libs/libdivsufsort/include -L libs/libdivsufsort/lib/.libs -L libs/httpcpp/lib -I libs -I libs/httpcpp -o $@ $< -lhttpcpp -ldivsufsort

runner: src/runner.cpp $(wildcard include/*.hpp)
	g++ -std=c++11 -O3 -g -pthread -I include -I libs/libdivsufsort/include -L libs/libdivsufsort/lib/.libs -L libs/httpcpp/lib -I libs -I libs/httpcpp -o $@ $< -lhttpcpp -ldivsufsort

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

sarray:
	mkdir -p libs; \
	cd libs; \
	rm -rf sarray ; \
	mkdir -p sarray ; \
	cd sarray ; \
	wget http://www.cs.dartmouth.edu/~doug/sarray/sarray.tar ; \
	tar xf sarray.tar ; \
	rm sarray.tar ; \

libdivsufsort:
	mkdir -p libs; \
	cd libs; \
	rm -rf libdivsufsort ; \
	wget https://libdivsufsort.googlecode.com/files/libdivsufsort-2.0.1.tar.gz ; \
	tar zxf libdivsufsort-2.0.1.tar.gz ; \
	rm libdivsufsort-2.0.1.tar.gz ; \
	mv libdivsufsort-2.0.1 libdivsufsort ; \
	cd libdivsufsort ; \
	./configure ; \
	make -j ; \
