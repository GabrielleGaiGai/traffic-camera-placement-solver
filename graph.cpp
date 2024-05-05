#include "graph.h"

std::vector<std::vector<int>> all_combinations(int N, int K) {
  std::vector<std::vector<int>> all_comb;
  std::vector<int> comb;
  std::string bitmask(K, 1);
  bitmask.resize(N, 0);

  do {
    comb.clear();
    for (int i = 0; i < N; ++i) {
      if (bitmask[i]) {
        comb.push_back(i + 1);
      }
    }
    all_comb.push_back(comb);
  } while (std::prev_permutation(bitmask.begin(), bitmask.end()));

  return all_comb;
}

Vertex::Vertex(int value) : value(value), pred(nullptr), color('w') {}

Graph::Graph(int num_vertices,
             const std::vector<std::tuple<int, int>> &edge_list) {
  this->num_vertices = num_vertices;
  this->edge_list = edge_list;

  for (int i = 1; i <= num_vertices; i++) {
    auto *new_vertex = new Vertex(i);
    this->vertices.push_back(new_vertex);
    this->vertex_dict[i] = new_vertex;
  }

  for (auto edge : edge_list) {
    int from = std::get<0>(edge);
    int to = std::get<1>(edge);
    adjacency_map[from].push_back(to);
    adjacency_map[to].push_back(from);

    if (vertex_dict.find(from) == vertex_dict.end() ||
        vertex_dict.find(to) == vertex_dict.end()) {
      throw "Error: vertex does not exist.";
    }
    this->vertex_dict[from]->neighbours.push_back(this->vertex_dict[to]);
    this->vertex_dict[to]->neighbours.push_back(this->vertex_dict[from]);
  }
}

Graph::~Graph() {
  for (auto vertex : this->vertices) {
    delete vertex;
  }
  this->vertices.clear();
}

int Graph::find_max_degree(std::map<int, std::vector<int>> adjacency_map_copy) {
  int max_degree = -1;
  int max_degree_vertex = -1;
  for (const auto &a : adjacency_map_copy) {
    if (static_cast<int>(a.second.size()) > max_degree) {
      max_degree = static_cast<int>(a.second.size());
      max_degree_vertex = a.first;
    }
  }
  return max_degree_vertex;
}

void Graph::shortest_path(int from, int to) {
  if (this->vertex_dict.find(from) == this->vertex_dict.end() ||
      this->vertex_dict.find(to) == this->vertex_dict.end()) {
    throw "Error: vertex does not exist.";
  }

  std::tuple<int, int> query = std::make_tuple(from, to);
  if (this->sp_dict.find(query) != this->sp_dict.end()) {
    std::vector<int> sp = this->sp_dict[query];
    for (unsigned int i = 0; i < sp.size(); i++) {
      std::cout << sp[i];
      if (i != sp.size() - 1) {
        std::cout << '-';
      }
    }
    std::cout << std::endl;
  }

  Vertex *s = this->vertex_dict[from];
  for (auto vertex : this->vertices) {
    if (vertex != s) {
      vertex->color = 'w';
      vertex->pred = nullptr;
    }
  }
  s->color = 'g';
  s->pred = nullptr;
  std::vector<Vertex *> Q;
  Q.push_back(s);
  while (!Q.empty()) {
    Vertex *u = Q.front();
    Q.erase(Q.begin());
    std::vector<Vertex *> adj_list = u->neighbours;
    for (auto v : adj_list) {
      if (v->color == 'w') {
        v->color = 'g';
        v->pred = u;
        Q.push_back(v);
      }
    }
    u->color = 'b';
  }

  for (auto vertex : this->vertices) {
    if (vertex->color == 'b') {
      std::vector<int> v_from_path;
      std::vector<int> from_v_path;
      Vertex *tmp = vertex;
      v_from_path.push_back(tmp->value);
      from_v_path.insert(from_v_path.begin(), tmp->value);
      while (tmp->pred) {
        tmp = tmp->pred;
        v_from_path.push_back(tmp->value);
        from_v_path.insert(from_v_path.begin(), tmp->value);
      }
      this->sp_dict[std::make_tuple(vertex->value, from)] = v_from_path;
      this->sp_dict[std::make_tuple(from, vertex->value)] = from_v_path;
    }
  }

  if (this->sp_dict.find(query) == this->sp_dict.end()) {
    throw "Error: no path exists.";
  }

  std::vector<int> sp = this->sp_dict[query];
  for (unsigned int i = 0; i < sp.size(); i++) {
    std::cout << sp[i];
    if (i != sp.size() - 1) {
      std::cout << '-';
    }
  }
  std::cout << std::endl;
}

void Graph::CNF_SAT_VC() {
  std::vector<int> min_vc_cnf;

  for (int k = 1; k <= this->num_vertices; k++) {
    std::unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
    std::map<std::string, Minisat::Lit> atom_dict;
    std::string l_name;

    // Initiate literals: "i"=true iff "i" is in the vertex cover
    for (int i = 1; i <= this->num_vertices; i++) {
      Minisat::Lit l = Minisat::mkLit(solver->newVar());
      l_name = std::to_string(i);
      atom_dict[l_name] = l;
    }

    // Size of vertex cover <= k: "1" + "2" + ... + "num_vertex_cnf" <= k
    std::vector<std::vector<int>> all_comb =
        all_combinations(this->num_vertices, k + 1);
    for (const auto &comb : all_comb) {
      Minisat::vec<Minisat::Lit> clause;
      for (auto c : comb) {
        clause.push(~atom_dict[std::to_string(c)]);
      }
      solver->addClause(clause);
    }

    // Every edge is incident to at least one vertex in the vertex cover
    for (auto edge : this->edge_list) {
      int i = std::get<0>(edge);
      int j = std::get<1>(edge);
      Minisat::vec<Minisat::Lit> clause;
      clause.push(atom_dict[std::to_string(i)]);
      clause.push(atom_dict[std::to_string(j)]);
      solver->addClause(clause);
    }

    bool res = solver->solve();
    if (res) {
      for (int i = 1; i <= this->num_vertices; i++) {
        if (not bool(Minisat::toInt(
                solver->modelValue(atom_dict[std::to_string(i)])))) {
          min_vc_cnf.push_back(i);
        }
      }
      break;
    }
  }

  std::sort(min_vc_cnf.begin(), min_vc_cnf.end());
  std::cout << "CNF-SAT-VC: ";
  for (int i = 0; i < int(min_vc_cnf.size()); i++) {
    std::cout << min_vc_cnf[i];
    if (i != int(min_vc_cnf.size()) - 1) {
      std::cout << ",";
    }
  }
  std::cout << std::endl;
}

void Graph::APPROX_VC_1() {
  std::vector<int> min_vc_1;

  std::map<int, std::vector<int>> adjacency_map_copy = this->adjacency_map;
  while (not adjacency_map_copy.empty()) {
    // Pick a vertex of highest degree
    int max_degree_vertex = find_max_degree(adjacency_map_copy);
    // Add it to vertex cover
    min_vc_1.push_back(max_degree_vertex);
    // Throw away all edges incident on that vertex
    std::vector<int> incidents = adjacency_map_copy[max_degree_vertex];
    for (int &incident : incidents) {
      std::vector<int> *it_vertices = &adjacency_map_copy[incident];
      for (auto it = it_vertices->begin(); it != it_vertices->end();) {
        if (*it == max_degree_vertex) {
          it_vertices->erase(it);
        } else {
          it++;
        }
      }
    }
    adjacency_map_copy.erase(max_degree_vertex);
    for (auto it = adjacency_map_copy.cbegin(); it != adjacency_map_copy.cend();) {
      if (it->second.empty()) {
        it = adjacency_map_copy.erase(it);
      } else {
        it++;
      }
    }
  }

  std::sort(min_vc_1.begin(), min_vc_1.end());
  std::cout << "APPROX-VC-1: ";
  for (int i = 0; i < int(min_vc_1.size()); i++) {
    std::cout << min_vc_1[i];
    if (i != int(min_vc_1.size()) - 1) {
      std::cout << ",";
    }
  }
  std::cout << std::endl;
}

void Graph::APPROX_VC_2() {
  std::vector<int> min_vc_2;

  std::vector<std::tuple<int, int>> edge_list_copy = this->edge_list;
  std::map<int, std::vector<int>> adjacency_map_copy = this->adjacency_map;
  while (not edge_list_copy.empty()) {
    // Pick en edge (u, v)
    std::tuple<int, int> u_v = edge_list_copy.front();
    // Add u and v to vertex cover
    min_vc_2.push_back(std::get<0>(u_v));
    min_vc_2.push_back(std::get<1>(u_v));
    // Throw away all edges attached to u and v
    std::vector<int> incidents = adjacency_map_copy[std::get<0>(u_v)];
    for (int &incident : incidents) {
      std::vector<int> *it_vertices = &adjacency_map_copy[incident];
      for (auto it = it_vertices->begin(); it != it_vertices->end();) {
        if (*it == std::get<0>(u_v)) {
          it_vertices->erase(it);
        } else {
          it++;
        }
      }
    }
    incidents = adjacency_map_copy[std::get<1>(u_v)];
    for (int &incident : incidents) {
      std::vector<int> *it_vertices = &adjacency_map_copy[incident];
      for (auto it = it_vertices->begin(); it != it_vertices->end();) {
        if (*it == std::get<1>(u_v)) {
          it_vertices->erase(it);
        } else {
          it++;
        }
      }
    }

    adjacency_map_copy.erase(std::get<0>(u_v));
    adjacency_map_copy.erase(std::get<1>(u_v));
    for (auto it = adjacency_map_copy.cbegin(); it != adjacency_map_copy.cend();) {
      if (it->second.empty()) {
        it = adjacency_map_copy.erase(it);
      } else {
        it++;
      }
    }
    for (auto it = edge_list_copy.begin(); it != edge_list_copy.end();) {
      if (std::get<0>(*it) == std::get<0>(u_v) or
          std::get<0>(*it) == std::get<1>(u_v) or
          std::get<1>(*it) == std::get<0>(u_v) or
          std::get<1>(*it) == std::get<1>(u_v)) {
        edge_list_copy.erase(it);
      } else {
        it++;
      }
    }
  }

  std::sort(min_vc_2.begin(), min_vc_2.end());
  std::cout << "APPROX-VC-2: ";
  for (int i = 0; i < int(min_vc_2.size()); i++) {
    std::cout << min_vc_2[i];
    if (i != int(min_vc_2.size()) - 1) {
      std::cout << ",";
    }
  }
  std::cout << std::endl;
}
