These examples demonstrate how to use the M-cores on Phytec developement boards
with Phytec's MCUXpresso SDK fork https://github.com/phytec/mcux-sdk/tree/v2.13.0-phy.

### Usage

Set up the SDK using west as described [here](https://github.com/phytec/mcux-sdk/tree/v2.13.0-phy#setup).

Build the examples either one by one
```console
host:mcux-sdk/examples-phytec$ source scripts/setenv
host:mcux-sdk/examples-phytec/.../armgcc$ ./build_debug.sh
```
(there are multiple variants of ```build_*.sh``` scripts)
or all together in one shot
```console
host:mcux-sdk/examples-phytec$ source scripts/setenv
host:mcux-sdk/examples-phytec$ ./scripts/build_all.sh
```
