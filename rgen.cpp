#include <fstream>
#include <string>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <tuple>
#include <signal.h>

class Point {
public:
    float x;
    float y;

    explicit Point(float x = 0, float y = 0) {
        this->x = x;
        this->y = y;
    };

    ~Point() = default;

public:
    float cp(Point other) const {
        float ans = this->x * other.y - this->y * other.x;
        return ans;
    }

    bool operator==(const Point &other) const {
        return (this->x == other.x && this->y == other.y);
    }

    bool is_between(Point p1, Point p2) {
        float dot_prod = (this->x - p1.x) * (p2.x - p1.x) + (this->y - p1.y) * (p2.y - p1.y);
        float p1_p2_distance = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
        if (dot_prod < 0 || dot_prod > p1_p2_distance || this == &p1 || this == &p2) {
            return false;
        } else {
            return true;
        }
    }

    Point sub(Point other) const {
        Point ans = Point(this->x - other.x, this->y - other.y);
        return ans;
    }

};

bool overlap(std::tuple<Point, Point> l1, std::tuple<Point, Point> l2) {
    Point a1 = std::get<0>(l1);
    Point b1 = std::get<1>(l1);
    Point a2 = std::get<0>(l2);
    Point b2 = std::get<1>(l2);

    if (b1.sub(a1).cp(b2.sub(a2)) == 0 && ((a2.sub(a1)).cp(b1.sub(a1)) == 0)) {
        if (a1 == a2 || a1 == b2 || b1 == a2 || b1 == b2) {
            Point same_point;
            std::tuple<Point, Point> diff_points;
            if (a1 == a2) {
                same_point = a1;
                diff_points = std::make_tuple(b1, b2);
            } else if (a1 == b2) {
                same_point = a1;
                diff_points = std::make_tuple(b1, a2);
            } else if (b1 == a2) {
                same_point = b1;
                diff_points = std::make_tuple(a1, b2);
            } else {
                same_point = b1;
                diff_points = std::make_tuple(a1, a2);
            }

            if (same_point.is_between(std::get<0>(diff_points), std::get<1>(diff_points)) &&
                !(std::get<0>(diff_points) == std::get<1>(diff_points))) {
                // l1 -> l1/l2 -> l2 (not overlap with one same point) false
                return false;
            } else {
                // l1/l2 -> l1/l2 (overlap with two same point) true
                // l1 -> l2 -> l1/l2 (overlap with one same point) true
                return true;
            }
        } else {
            if (!(a1.is_between(b1, b2) || a2.is_between(b1, b2) ||
                  b1.is_between(a1, a2) || b2.is_between(a1, a2))) {
                // l1 -> l1  l2 -> l2 (not overlap with no same point) false
                return false;
            } else {
                // l1 -> l2 -> l1 -> l2 (overlap with no same point) true
                // l1 -> l2 -> l2 -> l1 (overlap with no same point) true
                return true;
            }
        }
    }
    return false;
}

bool any_overlap(std::tuple<Point, Point> l1, const std::vector<std::tuple<Point, Point>> &l2_vec) {
    for (auto l2: l2_vec) {
        if (overlap(l1, l2)) {
            return true;
        }
    }
    return false;
}


int main(int argc, char **argv) {
    int max_num_street_s = 10;
    int max_num_line_segment_n = 5;
    int max_num_wait_l = 5;
    int max_coord_value = 20;
    std::vector<std::string> all_streets;
    std::string line;

    std::string c_value;
    int c;
    int c_value_int;
    bool exit = false;

    while ((c = getopt(argc, argv, "s:n:l:c:")) != -1) {
        switch (c) {
            case 's':
                c_value = optarg;
                c_value_int = atoi(c_value.c_str());
                if (c_value_int < 2) {
                    std::cerr << "Error: option value must >= 2." << std::endl;
                    exit = true;
                } else {
                    max_num_street_s = c_value_int;
                }
                break;
            case 'n':
                c_value = optarg;
                c_value_int = atoi(c_value.c_str());
                if (c_value_int < 1) {
                    std::cerr << "Error: option value must >= 1." << std::endl;
                    exit = true;
                } else {
                    max_num_line_segment_n = c_value_int;
                }
                break;
            case 'l':
                c_value = optarg;
                c_value_int = atoi(c_value.c_str());
                if (c_value_int < 5) {
                    std::cerr << "Error: option value must >= 5." << std::endl;
                    exit = true;
                } else {
                    max_num_wait_l = c_value_int;
                }
                break;
            case 'c':
                c_value = optarg;
                c_value_int = atoi(c_value.c_str());
                if (c_value_int < 1) {
                    std::cerr << "Error: option value must >= 1." << std::endl;
                    exit = true;
                } else {
                    max_coord_value = c_value_int;
                }
                break;
            case '?':
                if (optopt == 's' || optopt == 'n' || optopt == 'l' || optopt == 'c') {
                    std::cerr << "Error: option -" << char(optopt) << " requires an argument." << std::endl;
                    exit = true;
                } else {
                    std::cerr << "Error: unknown option: " << optopt << std::endl;
                    exit = true;
                }
            default:
                return 0;
        }
    }

    if (exit) {
        kill(getpid(), SIGTERM);
    }

    std::ifstream urandom("/dev/urandom");
    int max_attempt = 25;

    while (true) {
        std::vector<std::tuple<Point, Point>> all_line_segments;
        while (!all_streets.empty()) {
            line = "rm \"" + all_streets.front() + "\"";
            std::cout << line << std::endl;
            all_streets.erase(all_streets.begin());
        }

        unsigned int num_street = 1;
        urandom.read((char *) &num_street, sizeof(int));
        num_street = 2 + num_street % (max_num_street_s - 1);

        // for each street
        for (int i = 0; i < num_street; i++) {
            unsigned int num_line_segment = 1;
            urandom.read((char *) &num_line_segment, sizeof(int));
            num_line_segment = 1 + num_line_segment % max_num_line_segment_n;

            std::vector<Point> coordinate_list;
            std::vector<std::tuple<Point, Point>> line_segment_list;
            // for each line segment
            for (int j = 0; j < num_line_segment + 1; j++) {
                int coord_attempt = 0;
                while (true) {
                    coord_attempt++;
                    if (coord_attempt > max_attempt) {
                        std::cerr << "Error: failed for 25 attempts. " << std::endl;
                        kill(getpid(), SIGTERM);
                    }

                    int x = 1;
                    urandom.read((char *) &x, sizeof(int));
                    x = x % (max_coord_value + 1);
                    int y = 1;
                    urandom.read((char *) &y, sizeof(int));
                    y = y % (max_coord_value + 1);
                    Point new_coordinate = Point(float(x), float(y));
                    std::tuple<Point, Point> new_line_segment;

                    // test whether the newly generated coordinate is valid.
                    if (coordinate_list.empty() || !(new_coordinate == coordinate_list.back())) {
                        // test whether the newly generated line segment is valid.
                        if (coordinate_list.empty()) {
                            coordinate_list.push_back(new_coordinate);
                            break;
                        } else {
                            new_line_segment = std::make_tuple(coordinate_list.back(), new_coordinate);
                            if (all_line_segments.empty() || !any_overlap(new_line_segment, all_line_segments)) {
                                coordinate_list.push_back(new_coordinate);
                                line_segment_list.push_back(new_line_segment);
                                all_line_segments.push_back(new_line_segment);
                                break;
                            }
                        }
                    }
                }
            }
            line = "add \"street " + std::to_string(i) + "\" ";
            while (!coordinate_list.empty()) {
                Point coord = coordinate_list.front();
                line += "(" + std::to_string(int(coord.x)) + "," + std::to_string(int(coord.y)) + ") ";
                coordinate_list.erase(coordinate_list.begin());
            }
            std::cout << line << std::endl;
            all_streets.push_back("street " + std::to_string(i));
        }

        std::cout << "gg" << std::endl;
        unsigned int num_wait = 1;
        urandom.read((char *) &num_wait, sizeof(int));
        num_wait = 5 + num_wait % (max_num_wait_l - 4);
        sleep(num_wait);
    }
}