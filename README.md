#### ./rgen outputs random street speciﬁcations
**Command-line arguments:**
1. -s k:  where k is an integer >= 2. The number of streets should be a random integer in [2, k]. Default = 10
2. -n k:  where k is an integer >= 1. The number of line-segments in each street should be a random integer in [1, k]. Default = 5.
3. -l k: where k is an integer >= 5. rgen will wait w seconds, where w is in [5, k] before generating the output. Default = 5.
4. -c k: where k is an integer >= 1. rgen will generate (x, y) coordinates such that every x and y value is in the range [-k, k]. Default = 20.
