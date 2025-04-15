CXX = g++
FLAGS = -std=c++11 -Wall

.PHONY: all clean
all: hp_folder debug_hp_folder tri_hp_folder debug_tri_hp_folder no_prune_hp_folder
clean:
	rm -f hp_folder debug_hp_folder tri_hp_folder debug_tri_hp_folder no_prune_hp_folder

hp_folder: hp_folder.cpp
	$(CXX) $(FLAGS) -DUSE_PRUNING -O3 -o $@ $<

debug_hp_folder: hp_folder.cpp
	$(CXX) $(FLAGS) -DUSE_PRUNING -g -o $@ $<

no_prune_hp_folder: hp_folder.cpp
	$(CXX) $(FLAGS) -O3 -o $@ $<

tri_hp_folder: hp_folder.cpp
	$(CXX) $(FLAGS) -DUSE_PRUNING -DGRID_TRIANGULAR -O3 -o $@ $<

debug_tri_hp_folder: hp_folder.cpp
	$(CXX) $(FLAGS) -DUSE_PRUNING -DGRID_TRIANGULAR -g -o $@ $<
