rekheader - command line tool generating REK file header
=================================

rekheader is a small utility generating a REK file header (Fraunhofer EZRT Raw format)
for a RAW Volume Data scan without any header.

Usage:
````
  rekheader -i input.raw -o output.rek
            [-float32|-int16] -sizeX SX -sizeY SY -sizeZ SZ
            -pixelSize Microns [-sliceStep Microns]
````

Here is the main repository of the project:<br/>
https://github.com/gkv311/rekheader

## Licensing

See the [LICENSE](LICENSE.txt) file.
