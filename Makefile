CXX_FLAGS = -std=c++11 -Wall -lsfml-graphics -lsfml-window -lsfml-system

all: build run clean

build:
	@g++ -c seag.cpp
	@g++ -c example.cpp
	@g++ example.o seag.o -o example $(CXX_FLAGS)

run:
	@./example

clean:
	@rm -f example.o example