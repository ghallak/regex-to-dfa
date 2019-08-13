build:
	cd src && \
	clang++ -g -Wall -Wextra -Wpedantic -Wconversion -std=c++17 *.cpp -o regex-to-dfa && \
	mv regex-to-dfa ..

run: build
	./regex-to-dfa

clang-format:
	clang-format -style=Google -i src/*
