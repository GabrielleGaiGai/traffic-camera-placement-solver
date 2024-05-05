import re
import sys

point_name = 0
point_name_dict = {}  # {(x, y): (name, set of street name)}


# parse_line(input_line) parses an input line and return command and argument, or throws an exception on error.
# parse_line: Str -> any of (ParseException, List)
def parse_line(input_line):
    input_line = input_line.strip()
    sp_list = input_line.split()

    if len(sp_list) == 0:
        raise ParseException("empty command")
    elif sp_list[0] not in ["add", "mod", "rm", "gg", "s"]:
        raise ParseException("Unknown command " + sp_list[0])
    elif sp_list[0] == "add" or sp_list[0] == "mod":
        command = sp_list[0]
        sp_list = input_line[3:].lstrip()
        sp_list = sp_list.split("\"")
        if sp_list[0] != "" or len(sp_list) < 3 or "\"".join(sp_list[1:-1]) == "":
            raise ParseException("invalid street name")
        street_name = "\"".join(sp_list[1:-1])
        coordinates = sp_list[-1]

        if '- ' in coordinates:
            raise ParseException("invalid coordinates")
        coordinates = coordinates.replace(" ", "")

        if not re.fullmatch(r'\(-?\d+,-?\d+\)(\(-?\d+,-?\d+\))+', coordinates):
            raise ParseException("invalid coordinates")
        coordinates = re.findall(r'\(-?\d+,-?\d+\)', coordinates)

        coordinates_tuple = []
        for coord in coordinates:
            coord_value = re.findall(r'-?\d+', coord)
            if len(coord_value) != 2:
                raise ParseException("invalid coordinates")
            coordinates_tuple.append((int(coord_value[0]), int(coord_value[1])))

        return [command, street_name, coordinates_tuple]

    elif sp_list[0] == "rm":
        sp_list = input_line[2:].lstrip()
        sp_list = sp_list.split("\"")
        if sp_list[0] != "" or len(sp_list) < 3 or sp_list[-1] != "" or "\"".join(sp_list[1:-1]) == "":
            raise ParseException("invalid street name")
        street_name = "\"".join(sp_list[1:-1])
        return ["rm", street_name]
    else:
        if input_line != "gg":
            raise ParseException("too many arguments")
        return ["gg"]


class ParseException(Exception):
    def __init__(self, value):
        self._value = value

    def __str__(self):
        return str(self._value)


class Vector(object):
    def __init__(self, x, y):
        self.x = float(x)
        self.y = float(y)

    def __eq__(self, other):
        return (self.x == other.x) and (self.y == other.y)

    def __repr__(self):
        return '(' + print_number(self.x) + ', ' + print_number(self.y) + ')'

    def __hash__(self):
        return hash(self.__repr__())

    # cross product
    def cp(self, other):
        return self.x * other.y - self.y * other.x

    # multiply by constant
    def multiply(self, constant):
        return Vector(constant * self.x, constant * self.y)

    # subtract
    def sub(self, other):
        return Vector(self.x - other.x, self.y - other.y)

    # add
    def add(self, other):
        return Vector(self.x + other.x, self.y + other.y)

    # distance
    def d(self, other):
        return (other.x - self.x) ** 2 + (other.y - self.y) ** 2

    # self on the line segment p1 -> p2
    def is_between(self, p1, p2):
        dot_prod = (self.x - p1.x) * (p2.x - p1.x) + (self.y - p1.y) * (p2.y - p1.y)
        if dot_prod < 0 or dot_prod > p1.d(p2):
            return False
        else:
            return True


class Point(object):
    def __init__(self, x, y, street_set):
        self.x = float(x)
        self.y = float(y)

        global point_name, point_name_dict
        if (self.x, self.y) not in point_name_dict.keys():
            self.name = "p" + str(point_name)
            point_name += 1
            point_name_dict[(self.x, self.y)] = (self.name, street_set)
        else:
            self.name = point_name_dict[(self.x, self.y)][0]
            point_name_dict[(self.x, self.y)][1].update(street_set)

    def __repr__(self):
        return self.name + ': (' + print_number(self.x) + ', ' + print_number(self.y) + ')'

    def __eq__(self, other):
        if isinstance(other, Point):
            return (self.x == other.x) and (self.x == other.x) and (self.name == other.name)
        else:
            return False

    def __hash__(self):
        return hash(self.__repr__())


class Line(object):
    def __init__(self, src, dst, street_name):
        self.src = src
        self.dst = dst
        self.street_name = street_name
        self.intersection = {}  # {street_name: set of Point}

    def __repr__(self):
        intersection_set = set()
        for street_name in self.intersection.keys():
            for intersection in self.intersection[street_name]:
                intersection_set.add((intersection.x, intersection.y))
        return '[' + str(self.src) + '-->' + str(self.dst) + ']' + ': ' + str(len(list(intersection_set)))


class Street(object):
    def __init__(self, name, coordinates):
        self.name = name
        self.coordinates = []
        self.line_segments = []

        global point_name
        point_list = []
        for coordinate in coordinates:
            p = Point(coordinate[0], coordinate[1], {name})
            self.coordinates.append(p)
            point_list.append(p)

        for i in range(len(point_list) - 1):
            line = Line(point_list[i], point_list[i + 1], name)
            self.line_segments.append(line)

    def __repr__(self):
        return self.name


class Traffic(object):
    def __init__(self):
        self.street_dict = {}

    def add(self, street_name, coordinates):
        if street_name in self.street_dict.keys():
            raise ParseException("street name already exists")
        new_street = Street(street_name, coordinates)

        global point_name_dict
        # Get all new intersections: check whether the newly added street intersects any existing street
        for old_street_name in self.street_dict.keys():
            old_street = self.street_dict[old_street_name]
            for line_segment_old in old_street.line_segments:
                for line_segment_new in new_street.line_segments:
                    intersection_list = intersect(line_segment_old, line_segment_new)
                    for intersection in intersection_list:
                        if intersection:
                            if old_street_name not in line_segment_new.intersection.keys():
                                line_segment_new.intersection[old_street_name] = {intersection}
                            else:
                                line_segment_new.intersection[old_street_name].add(intersection)
                            if new_street.name not in line_segment_old.intersection.keys():
                                line_segment_old.intersection[new_street.name] = {intersection}
                            else:
                                line_segment_old.intersection[new_street.name].add(intersection)

        self.street_dict[street_name] = new_street

    def rm(self, street_name):
        if street_name not in self.street_dict.keys():
            raise ParseException("street name does not exist")

        street = self.street_dict[street_name]
        already_checked_streets = []
        # delete current street from the intersection list of all the other street
        for line_segment in street.line_segments:
            for intersect_street in line_segment.intersection.keys():
                if intersect_street not in already_checked_streets:
                    for line in self.street_dict[intersect_street].line_segments:
                        if street_name in line.intersection.keys():
                            del line.intersection[street_name]
                already_checked_streets.append(intersect_street)

        global point_name_dict
        delete_set = set()
        for point in point_name_dict.keys():
            if street_name in list(point_name_dict[point][1]):
                point_name_dict[point][1].remove(street_name)
                if len(list(point_name_dict[point][1])) == 0:
                    delete_set.add(point)
        for point in delete_set:
            del point_name_dict[point]

        del self.street_dict[street_name]

    def mod(self, street_name, coordinates):
        self.rm(street_name)
        self.add(street_name, coordinates)

    def gg(self):
        v_dict = {}
        e_set = set()
        plot_graph = []
        for street_name in self.street_dict.keys():
            street = self.street_dict[street_name]
            coordinates_list = []
            for line_segment in street.line_segments:
                coordinates_list.append((line_segment.src.x, line_segment.src.y))
                coordinates_list.append((line_segment.dst.x, line_segment.dst.y))

                line_v_dict = {}
                for inter_street in line_segment.intersection.keys():
                    for intersection in list(line_segment.intersection[inter_street]):
                        if (intersection.x, intersection.y) != (line_segment.src.x, line_segment.src.y) and \
                                (intersection.x, intersection.y) != (line_segment.dst.x, line_segment.dst.y):
                            line_v_dict[(intersection.x, intersection.y)] = intersection
                v_dict.update(line_v_dict)
                if len(line_segment.intersection.keys()) > 0:
                    v_dict[(line_segment.src.x, line_segment.src.y)] = line_segment.src
                    v_dict[(line_segment.dst.x, line_segment.dst.y)] = line_segment.dst

                    line_v_list = sort_point(line_segment.src, list(line_v_dict.values()))
                    line_v_list.insert(0, line_segment.src)
                    line_v_list.append(line_segment.dst)
                    for i in range(len(line_v_list) - 1):
                        e_set.add((line_v_list[i].name, line_v_list[i + 1].name))
            plot_graph.append(coordinates_list)

        vertex_name_dict = {}
        vertex_name_int = 1
        for vertex in v_dict.values():
            vertex_name_dict[vertex.name] = str(vertex_name_int)
            vertex_name_int += 1

        print("V " + str(len(vertex_name_dict.keys())))
        print("E {", end="")
        e_set = list(e_set)
        for edge in e_set:
            print("<" + vertex_name_dict[edge[0]] + "," + vertex_name_dict[edge[1]] + ">", end="")
            if edge != e_set[-1]:
                print(",", end="")
        print("}")


# sort_point(src, lst) sorts the point in lst according to their distance to src.
# sort_point: Point, (list of Point) -> (list of Point)
def sort_point(src, lst):
    distance_list = []
    for point in lst:
        distance = (point.x - src.x) ** 2 + (point.y - src.y) ** 2
        distance_list.append((point, distance))
    distance_list.sort(key=lambda x: x[1])
    sorted_lst = [x[0] for x in distance_list]
    return sorted_lst


# print_number(x) returns a pretty-print string representation of a number.
#           A float number is represented by an integer, if it is whole, and up to two decimal places if it isn't.
# print_number: Int/Float -> Str
def print_number(x):
    if isinstance(x, float):
        return "{0:.2f}".format(x)
    return str(x)


# intersect(l1, l2) returns point(s) at which two line segments intersect, or None if they do not intersect.
# intersect: Line, Line -> (list of Point)
def intersect(l1, l2):
    p = Vector(l1.src.x, l1.src.y)
    r = Vector(l1.dst.x, l1.dst.y).sub(p)
    q = Vector(l2.src.x, l2.src.y)
    s = Vector(l2.dst.x, l2.dst.y).sub(q)

    # Case 1: collinear
    if r.cp(s) == 0 and ((q.sub(p)).cp(r) == 0):
        if len(list({p, q, p.add(r), q.add(s)})) < 4:
            overlap_list = list(set([x for x in [p, q, p.add(r), q.add(s)] if [p, q, p.add(r), q.add(s)].count(x) > 1]))
            # completely overlap
            if len(overlap_list) == 2:
                return [Point(overlap_list[0].x, overlap_list[0].y, {l1.street_name, l2.street_name}),
                        Point(overlap_list[1].x, overlap_list[1].y, {l1.street_name, l2.street_name})]
            else:
                a = list({p, q, p.add(r), q.add(s)} - set(overlap_list))[0]
                b = list({p, q, p.add(r), q.add(s)} - set(overlap_list))[1]
                i = overlap_list[0]
                # partially overlap with one same point
                if a.is_between(i, b):
                    return [Point(i.x, i.y, {l1.street_name, l2.street_name}),
                            Point(a.x, a.y, {l1.street_name, l2.street_name})]
                # partially overlap with one same point
                elif b.is_between(i, a):
                    return [Point(i.x, i.y, {l1.street_name, l2.street_name}),
                            Point(b.x, b.y, {l1.street_name, l2.street_name})]
                # intersect
                else:
                    return [Point(i.x, i.y, {l1.street_name, l2.street_name})]

        else:
            # partially overlap with no same point
            if p.is_between(q, q.add(s)) and (p.add(r)).is_between(q, q.add(s)):
                return [Point(p.x, p.y, {l1.street_name, l2.street_name}),
                        Point(p.add(r).x, p.add(r).y, {l1.street_name, l2.street_name})]
            # partially overlap with no same point
            elif q.is_between(p, p.add(r)) and (q.add(s)).is_between(p, p.add(r)):
                return [Point(q.x, q.y, {l1.street_name, l2.street_name}),
                        Point(q.add(s).x, q.add(s).y, {l1.street_name, l2.street_name})]
            # partially overlap with no same point
            elif p.is_between(q, q.add(s)):
                if q.is_between(p, p.add(r)):
                    return [Point(q.x, q.y, {l1.street_name, l2.street_name}),
                            Point(p.x, p.y, {l1.street_name, l2.street_name})]
                else:
                    return [Point(q.add(s).x, q.add(s).y, {l1.street_name, l2.street_name}),
                            Point(p.x, p.y, {l1.street_name, l2.street_name})]
            elif p.add(r).is_between(q, q.add(s)):
                if q.is_between(p, p.add(r)):
                    return [Point(q.x, q.y, {l1.street_name, l2.street_name}),
                            Point(p.add(r).x, p.add(r).y, {l1.street_name, l2.street_name})]
                else:
                    return [Point(q.add(s).x, q.add(s).y, {l1.street_name, l2.street_name}),
                            Point(p.add(r).x, p.add(r).y, {l1.street_name, l2.street_name})]
            # do not intersect
            else:
                return [None]

    # Case 2: parallel and do not intersect
    elif (r.cp(s) == 0) and (q.sub(p)).cp(r) != 0:
        return [None]

    # Case 3: not parallel and intersect
    elif (r.cp(s) != 0) and 0 <= (q.sub(p)).cp(s) / (r.cp(s)) <= 1 and 0 <= (p.sub(q)).cp(r) / (s.cp(r)) <= 1:
        t = (q.sub(p)).cp(s) / (r.cp(s))
        return [Point(p.add(r.multiply(t)).x, p.add(r.multiply(t)).y, {l1.street_name, l2.street_name})]

    # Case 4: not parallel and do not intersect
    else:
        return [None]


def main():
    traffic = Traffic()

    while True:
        input_line = sys.stdin.readline()
        if input_line == "":
            break

        try:
            if input_line[0] == "s" or input_line[0] == "p":
                print(input_line.strip())
            else:
                cmd_list = parse_line(input_line)
                if cmd_list[0] == "add":
                    traffic.add(cmd_list[1], cmd_list[2])
                elif cmd_list[0] == "mod":
                    traffic.mod(cmd_list[1], cmd_list[2])
                elif cmd_list[0] == "rm":
                    traffic.rm(cmd_list[1])
                else:
                    traffic.gg()
                    
            # Make sure the output is redirect to pipe immediately
            sys.stdout.flush()

        except ParseException as ex:
            sys.stderr.write("Error: {0}\n".format(ex))

    sys.exit(0)


if __name__ == "__main__":
    main()
