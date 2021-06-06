
bin2array
----
#### description
A lightweight tool to generate source code array from binary content

#### Usage
To convert binary content (```inputfile.dat```) to C source array in ```uint8_t``` format, type:
```
bin2array --c-uint8 -f inputfile.dat > output_data.c
```
bin2array also supports ```uint16_t```, ```uint32_t```, ```uint64_t```, and ```Java byte```, ```Java char``` format. For more information, see ```binarray -h```.

### Compile
Make sure a POSIX-compatible make(1) is present in your environment, then type:
```
make
```
