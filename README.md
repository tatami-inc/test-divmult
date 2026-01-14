# Division or multiplication

## Overview

On many CPUs, floating-point division is much slower than the equivalent multiplication,
see [discussion here](https://stackoverflow.com/questions/4125033/floating-point-division-vs-floating-point-multiplication?noredirect=1&lq=1).
Thus, when dividing by the same denominator in a loop, one might consider replacing `x / y` with a `x * z` where `z = 1 / y` is computed outside of the loop.
This sacrifices some accuracy (due to the round-off error from computing `z`) in order to replace the per-iteration divide with a multiply.
The question is, does this actually have an impact?

## Setup

We test the speed of iterating over an array and dividing its contents by a constant.
This is a pretty common pattern in **tatami**, and while usually more operations are involved, we'll keep things simple here.
We compare to the alternative, i.e., compute the reciprocal and use it to multiply each array element within the loop.

To build, use the usual CMake process:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Results

On an Intel i7-8850H, we get:

```
$ ./build/divtest
Division:      	0.0137299 ± 1.95084e-05
Multiplication:	0.0136978 ± 3.16613e-05
```

Not much effect, it seems.
A few runs indicate that the <1% improvement is consistent, so the multiplication is faster but not by much.
I'd guess that the CPU is bottlenecked by reading the array from main memory, such that the speed of the actual operation doesn't matter.
