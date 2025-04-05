# Adaptive Huffman Coding

### Build

```bash
$ cmake -B build -S .
$ cmake --build build
```

### Adaptive Huffman

```bash
$ ./build/adaptive_huffman_main -i [TARGET] -o [OUTPUT] -v w
$ ./build/adaptive_huffman_main -i [TARGET] -o [OUTPUT] -d -v w
```

### Scripts

Run the following commands before executing 
the python scripts to fetch the libraries

```bash
python3 -m venv venv
source venv/bin/activate
python3 -m pip install -r scripts/requirements.txt
```

- check integrity
```
chmod +x ./scripts/check_integrity.sh
./script/check_integrity.sh ./build/[huffman_main,adaptive_huffman_main] <target>
```

- plot byte frequencies
```
python3 ./scripts/plot_freq.py <target>
```
