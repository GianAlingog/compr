# Side quest: File compressor

I'm bored and made this.

## Build

```bash
g++ -std=c++17 -O2 -o ess src/main.cpp
```

This builds a single binary named `ess`.

## Usage

Compress a file:

```bash
./ess -c path/to/input.txt
```

This writes a compressed archive next to the input file, e.g. `path/to/input.ess`.

Decompress an archive:

```bash
./ess -d path/to/input.ess
```

This recreates the original file using the name stored inside the archive.

## Background
I was reading through [CPH](https://cses.fi/book/book.pdf) for the millionth time, and similarly, I was going to once again skip over uninteresting sections. One of these was under the Greedy Algorithms section, Data Compression. Who reads the greedy section anyway?

I took a look at Huffman Coding and thought it'd be a fun project. It seems like a good way to learn more about information theory, systems design, and software engineering, as well as work with other forms of file IO and mess around with more DSA.
