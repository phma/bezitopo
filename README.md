Bezitopo is a land surveying program using Bézier triangles. The sample data file `dat/topo0.asc` is a survey of Independence Park in Charlotte, North Carolina.

To compile, if you're not developing the program:

1. Create a subdirectory build/ inside the directory where you untarred the source code.
2. `cd build`
3. `cmake ..`
4. `make`

If you are developing the program:

1. Create a directory build/bezitopo outside the directory where you cloned the source code.
2. `cd build/bezitopo`
3. `cmake <directory where the source code is>`
4. `make`

Run the test! (`make test`) This program has shown unexpected behavior when run on a different architecture or a different operating system. If you're trying to find out why a test fails, you want to run `bezitest`, as it provides output which is suppressed by "make test".

Bezitopo is an interactive program. When you get the ? prompt, type "help" for a list of commands.
