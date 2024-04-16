
test: ./build/catch2.o ./build/test_runner
	@echo "Running tests..."
	@./build/test_runner

test_runner: ./build/catch2.o ./build/test_runner
	@echo "test_runner built"

./build/test_runner: build/catch2.o tests/** tests/gates/* include/** src/**
	@echo "Compiling tests..."
	@g++ -g -std=c++17 -I./tests -I./include -I./external_lib -o build/test_runner tests/gates/*.cpp tests/**.cpp ./build/catch2.o src/**.cpp

./build/catch2.o:
	@echo "Compiling Catch2"
	@g++ -g -std=c++17 -I./external_lib/catch2 ./external_lib/catch2/catch_amalgamated.cpp -c -o ./build/catch2.o

clean:
	@echo "Cleaning..."
	@rm -f build/test_runner
	@rm -f ./build/**

build: ./build/libcircuitsim.a
	@echo "Building project..."

./build/libcircuitsim.a: include/** src/**
	@echo "Compiling project..."
	@cd build && ls && g++ -g -std=c++17 -I ../include -I ../include/gates -c  ../src/*.cpp
	@ar rcs build/libcircuitsim.a build/*.o
	