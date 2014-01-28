all: main runner

main: src/main.cpp $(wildcard include/*.hpp) libs/libdivsufsort libs/httpcpp libs/tbb
	g++ -std=c++11 -O3 -g -pthread -L libs/tbb/build/linux*release/ -L . -I libs/tbb/include -I include -I libs/libdivsufsort/include -L libs/libdivsufsort/lib/.libs -L libs/httpcpp/lib -I libs -I libs/httpcpp -o $@ $< -lhttpcpp -ldivsufsort -ltbb

runner: src/runner.cpp $(wildcard include/*.hpp) libs/libdivsufsort libs/httpcpp libs/tbb
	g++ -std=c++11 -O3 -g -pthread -L libs/tbb/build/linux*release/ -L . -I libs/tbb/include -I include -I libs/libdivsufsort/include -L libs/libdivsufsort/lib/.libs -L libs/httpcpp/lib -I libs -I libs/httpcpp -o $@ $< -lhttpcpp -ldivsufsort -ltbb

clean:
	rm -f main runner *.dSYM *.data*

libs/libdivsufsort:
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

libs/httpcpp:
	mkdir -p libs; \
	cd libs; \
	rm -rf httpcpp ; \
	git clone https://github.com/henrik-muehe/httpcpp.git ; \
	cd httpcpp ; \
	make

libs/tbb:
	mkdir -p libs; \
	cd libs; \
	rm -rf tbb; \
	wget "https://www.threadingbuildingblocks.org/sites/default/files/software_releases/source/tbb42_20131118oss_src.tgz"; \
	tar zxf tbb42_20131118oss_src.tgz ; \
	mv tbb42_20131118oss tbb ; \
	rm tbb42_20131118oss_src.tgz ; \
	cd tbb/src ; \
	make tbb -j
	#mv libs/tbb/build/linux*release/*.so* .
