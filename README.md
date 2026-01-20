# Division or multiplication

## Overview

On many CPUs, floating-point division is much slower than multiplication,
see [discussion here](https://stackoverflow.com/questions/4125033/floating-point-division-vs-floating-point-multiplication).
Thus, when dividing by the same denominator in a loop, one might consider replacing `x / y` with a `x * z` where `z = 1 / y` is computed outside of the loop.
This sacrifices some accuracy (due to the round-off error from computing `z`) in order to replace the per-iteration divide with a multiply.
The question is, does this actually have an impact?

To build, use the usual CMake process:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Basic test

We test the speed of iterating over an array and dividing its contents by a constant.
This is a pretty common pattern in **tatami**, and while usually more operations are involved, we'll keep things simple here.
We compare to the alternative, i.e., compute the reciprocal and use it to multiply each array element within the loop.

On an Intel i7-8850H, we get:

```
$ ./build/basic
Division:      	0.0137299 ± 1.95084e-05
Multiplication:	0.0136978 ± 3.16613e-05
```

On a Mac M2, we get:

```
$ ./build/basic
Division:      	0.00203902 ± 0.000123369
Multiplication:	0.00200791 ± 8.85522e-05
```

Not much effect, it seems.
I'd guess that the CPU is bottlenecked by reading the array from main memory, such that the speed of the actual operation doesn't matter.

## Log-normalization

A more robust test involves log-normalization of a count matrix in RNA-seq data.
This involves computing $\log_2(x / f + 1)$ where $x$ is the array of data and $f$ is a constant size factor.
We consider several permutations of this approach using `log1p` instead of `log2` and/or multiplying by the reciprocal
($r = f^{-1}$, $M = \log 2$, $N = (\log 2)^{-1}$).

On an Intel i7-8850H, we get:

```
$ ./build/normalize
log1p(x / f) / M:   0.150602 ± 0.00142801
log1p(x * r) * N:   0.133567 ± 0.00120476
log2(x / f + 1):    0.101818 ± 0.00119196
log2(x * r + 1):    0.0837252 ± 0.00128969
log(x / f + 1) / M: 0.0727775 ± 0.00114991
log(x * r + 1) * N: 0.062309 ± 0.00119778
```

On a Mac M2, we get:

```
$ ./build/normalize
log1p(x / f) / log(2): 0.0599988 ± 0.000203352
log1p(x * r) / log(2): 0.054169 ± 0.0001208
log2(x / f + 1):       0.0525862 ± 7.10782e-05
log2(x * r + 1):       0.045104 ± 9.04263e-05
```

Division is significantly faster here.
Now that we're doing a decent amount of work with the log-transformations, compute is now the bottleneck and the extra time spent in the divide is no longer masked.
