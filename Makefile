build:
	cd src && \
	clang++ -Wall -Wextra -Wpedantic -Wconversion -std=c++17 *.cpp && \
	mv a.out ..

run: build
	./a.out

clang-format:
	clang-format -style=Google -i src/*
