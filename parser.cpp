#include "graph.h"

std::vector<std::tuple<int, int>> split(std::string s) {
  int from;
  int to;
  std::vector<std::tuple<int, int>> res;

  s = s.substr(1, s.length() - 2);
  while (true) {
    from = std::stoi(s.substr(1, s.find(',')));
    to = std::stoi(s.substr(s.find(',') + 1, s.find('>')));
    std::tuple<int, int> current_tuple = std::make_tuple(from, to);
    res.push_back(current_tuple);

    if (s.find('>') == s.size() - 1) {
      return res;
    } else {
      s = s.substr(s.find('>') + 2, std::string::npos);
    }
  }
}

int main() {
  std::string line;
  int num_vertex;
  std::vector<std::tuple<int, int>> edge_list;
  bool require_edge_list = false;
  bool valid_graph;
  Graph *graph;

  while (true) {
    if (std::getline(std::cin, line)) {
      std::istringstream input(line);
      std::string command;
      std::string end;

      input >> command;
      if (command == "V") {
        std::cout << line << std::endl;
        input >> num_vertex;
        input >> end;
        if (num_vertex > 1 && input.eof()) {
          require_edge_list = true;
          valid_graph = false;
        } else if (num_vertex < 2) {
          std::cerr << "Error: vertex number must > 1." << std::endl;
        } else {
          std::cerr << "Error: wrong command." << std::endl;
        }
      } else if (command == "E") {
        std::cout << line << std::endl;
        if (require_edge_list) {
          require_edge_list = false;
          std::string edge_list_str;
          input >> edge_list_str;
          input >> end;
          if (!edge_list_str.empty() && input.eof()) {
            edge_list = split(edge_list_str);
            try {
              graph = new Graph(num_vertex, edge_list);
              valid_graph = true;
            } catch (const char *error) {
              std::cerr << error << std::endl;
            }
          }
        }
      } else if (command == "s") {
        if (require_edge_list || !valid_graph) {
          std::cerr << "Error: invalid graph." << std::endl;
        } else {
          int from, to;
          input >> from;
          input >> to;
          input >> end;
          if (from && to && input.eof()) {
            try {
              graph->shortest_path(from, to);
            } catch (const char *error) {
              std::cerr << error << std::endl;
            }
          } else {
            std::cerr << "Error: wrong command." << std::endl;
          }
        }
      } else if (command == "p") {
        if (require_edge_list || !valid_graph) {
          std::cerr << "Error: invalid graph." << std::endl;
        } else {
          try {
            int method;
            input >> method;
            if (method == 0) {
              graph->CNF_SAT_VC();
            } else if (method == 1) {
              graph->APPROX_VC_1();
            } else if (method == 2) {
              graph->APPROX_VC_2();
            } else {
              std::cerr << "Error: invalid method number." << std::endl;
            }
          } catch (const std::invalid_argument &error) {
            std::cerr << error.what() << std::endl;
          }
        }
      } else if (!command.empty()) {
        std::cerr << "Error: unknown command" << std::endl;
      }
    } else if (std::cin.eof()) {
      break;
    } else {
      std::cerr << "Error: cin error" << std::endl;
      break;
    }
  }
}
