all: cache

cache : cache.cpp
	g++  -o cache -g cache.cpp
clean:
	rm cache