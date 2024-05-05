# Traffic camera placement solver

This project contains two programs:
- `./rgen` that generate random add/rm/gg street commands.
- `./solver` that takes street commands as input, and output:
  	- the shortest path between any two vertices
  	- best intersections to install security cameras such that the number of cameras needed is minimized


## ./rgen
### Optional command-line arguments:
**`-s k`**  
`k` is an integer >= 2. The number of streets should be a random integer in `[2, k]`. Default = 10  
**`-n k`**  
`k` is an integer >= 1. The number of line-segments in each street should be a random integer in `[1, k]`. Default = 5.  
**`-l k`**  
`k` is an integer >= 5. `rgen` will wait w seconds, where w is in `[5, k]` before generating the output. Default = 5.  
**`-c k`**  
`k` is an integer >= 1. `rgen` will generate `(x, y)` coordinates such that every `x` and `y` value is in the range `[-k, k]`. Default = 20.  

Example:  
`./rgen -s 3 -n 2 -l 10 -c 5`  
`> add "street 0" (4,-1) (0,5)`  
`> add "street 1" (0,-2) (5,5)`  
`> add "street 2" (-3,3) (5,1)`   
`> gg`  
(wait random seconds...)  
`rm "street 0"`  
`rm "street 1"`  
`rm "street 2"` (remove old streets)  
`add "street 0" (2,4) (5,-3) `  
`add "street 1" (-3,-3) (4,5) `  
`add "street 2" (-3,3) (0,2) (-2,-3) `  
`> gg`  
(wait random seconds...)  
...  

## ./solver  
### Standard input:
**Add a street: `add "street name" (x1, y1) (x2, y2) (x3, y3) ...`**
- `add` is used to add a street. Each `(xi , yi)` is a GPS coordinate. We interpret the coordinates as a poly-line segment.
  That is, we draw a line segment from `(xi, yi)` to `(xi+1, yi+1)`. Each `xi` and `yi` is an integer.
  
Example:  
`add "Weber Street" (2,-1) (2,2) (5,5) (5,6) (3,8)`

**Modify a street: `mod "street name" (x1, y1) (x2, y2) (x3, y3) ...`**
- `mod` is used to modify the speciﬁcation of a street. Its format is the same as for add. It is a new speciﬁcation for a street you’ve speciﬁed before.

Example:  
`mod "Weber Street" (1,1) (4,8)`

**Remove a street: `rm "street name"`**
- `rm` is used to remove a street.

Example:  
`rm "Weber Street"`

**Generate a graph: `gg`**
- `gg` causes the program to output the corresponding graph, which is a list of vertices and edges. Each vertex has a ID, and an edge is expressed by `<vertex_ID, vertex_ID>`.
- There is a vertex corresponding to: (a) each intersection, and, (b) the end-point of a line segment of a street that intersects with another street.
- There is an edge between two vertices if: (a) at least one of them is an intersection, (b) both lie on the same street, and, (c) one is reachable from the other without traversing another vertex.

Example:  
`gg`  
`> V = {1: (2,2) 2: (4,2) 3: (4,4) 4: (5,5) 5: (1,4) 6: (4,7) 7: (5,6) 8: (5,8)}`  
`> E = {<1,3>, <2,3>, <3,4>, <3,6>, <7,6>, <6,5>, <9,6>, <6,8>, <6,10>}`

**Determine shortest path: `s from_vertex_ID to_vertex_ID`**
- `s` output the shortest path from the ﬁrst vertex to the second

Example:  
`s 2 10`  
`> 2 8 10`

> Note: to use the `s` command, a `gg` command must be input after any street is added/edited/removed.

**Place the traffic camera: `p method_identifier`**
- Method 0: reduction to CNF-SAT
- Method 1: pick the vertex of highest degree, add it to the output and remove all incident edges. Repeat till no edges remain.
- Method 2: pick an edge (u, v) and add both u and v to the output. Remove all edges incident to u and v. Repeat till no edges remain.
- Note that method 0 outputs the optimized results but is slow. Method 1 and 2 return the approximate of optimized results, but are fast.

Example:  
`p 0`  
`> CNF-SAT-VC: 1 3`  
`p 1`  
`> CNF-SAT-VC: 1 2 3`  
`p 2`  
`> CNF-SAT-VC: 1 2 3 4`  

> Note: to use the `p` command, a `gg` command must be input after any street is added/edited/removed.
  
