#ifndef GRAPH_H
#define GRAPH_H

#include <pthread.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "minisat/minisat/core/Solver.h"
#include "minisat/minisat/core/SolverTypes.h"

std::vector<std::vector<int>> all_combinations(int N, int K);

class Vertex {
  int value;
  Vertex *pred;  // pointer to predecessor
  char color;
  std::vector<Vertex *> neighbours;  // vector of pointer to Vertex

 public:
  explicit Vertex(int value);
  ~Vertex() = default;
  friend class Graph;
};

class Graph {
  int num_vertices;
  std::vector<std::tuple<int, int>> edge_list;
  std::vector<Vertex *> vertices;  // vector of pointer to (new) Vertex

  std::map<int, Vertex *> vertex_dict;  // {int: pointer to Vertex}
  std::map<std::tuple<int, int>, std::vector<int>>
      sp_dict;  // {(int, int): int}
  std::map<int, std::vector<int>> adjacency_map;

 private:
  int find_max_degree(std::map<int, std::vector<int>> adjacency_map_copy);

 public:
  Graph(int num_vertices, const std::vector<std::tuple<int, int>> &edge_list);
  ~Graph();

  void shortest_path(int from, int to);
  void CNF_SAT_VC();
  void APPROX_VC_1();
  void APPROX_VC_2();
};

#endif  // GRAPH_H
