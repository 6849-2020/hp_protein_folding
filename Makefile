CXX = g++
FLAGS = -std=c++11 -Wall

.PHONY: all
all: hp_folder debug_hp_folder

hp_folder: hp_folder.cpp
	$(CXX) $(FLAGS) -O3 -o $@ $<

debug_hp_folder: hp_folder.cpp
	$(CXX) $(FLAGS) -g -o $@ $<
