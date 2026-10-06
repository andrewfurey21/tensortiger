# tensortiger


goal: implement a tinygrad uir compiler that compiles to ptx on my 3080.

maybe tileir first though.

## design choices

- use c++20
- the compiler doesn't make any heap allocs.

## links

* [All cuda docs](https://docs.nvidia.com/cuda/cuda-programming-guide/index.html)
* [Tinygrad](https://github.com/tinygrad/tinygrad)
* [Tinygrad class](https://gist.github.com/geohot/4768597d9dc536446ee2d5de1f29e89d)
* [TileIR](https://docs.nvidia.com/cuda/tile-ir/latest/sections/introduction.html)
* [Halide](https://github.com/halide/Halide)
* [TVM](https://github.com/apache/tvm/)
* [LLVM](https://mcyoung.xyz/2023/08/01/llvm-ir/)
* [Mirage](https://github.com/mirage-project/mirage)
* [Luminal](https://github.com/luminal-ai/luminal)
* [ThunderKittens](https://hamzaelshafie.bearblog.dev/dissecting-thunderkittens-anatomy-of-a-compact-dsl-for-high-performance-ai-kernels/)
* [Romou](https://www.microsoft.com/en-us/research/wp-content/uploads/2022/02/mobigpu_mobicom22_camera.pdf)
* [Egg](https://egraphs-good.github.io/)
* [MLIR](https://github.com/llvm/llvm-project/tree/main/mlir)
* [LoopStack](https://arxiv.org/abs/2205.00618)
* [TileLang](https://github.com/tile-ai/tilelang)
* [Cute Layout Algebra](https://leimao.github.io/article/CuTe-Layout-Algebra/)
* [TASO](github.com/jiazhihao/TASO)
* [PET](https://github.com/thu-pacman/PET)
* [Ansor](https://arxiv.org/pdf/2006.06762)
* [Mojo kgen](https://github.com/modular/modular/tree/main/KGEN)
* [Gluon](https://triton-lang.org/main/gluon/index.html)
* [Mosaic GPU](https://docs.jax.dev/en/latest/pallas/gpu/index.html)
* [Cutlass, Cute DSL](https://docs.nvidia.com/cutlass/latest/index.html)
* [Cute layouts](https://arxiv.org/abs/2603.02298)
* [Linear Layouts](https://arxiv.org/abs/2505.23819)
