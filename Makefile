CXX = g++
FLAGS = -Wall -O3

hp_folder: hearn-hp.cpp
	$(CXX) $(FLAGS) -o $@ $<
