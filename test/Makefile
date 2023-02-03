
all: a.out

clean:
	-rm -f a.out

a.out : test.cpp ../inc/object_mutex.hpp
	g++ -std=c++11 -g -o a.out -I../inc test.cpp
