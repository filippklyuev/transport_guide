# transport_guide
Transport guide that provides functionality of simple buses router/map terminal app

### Two Stages branch
Branch represents "Two-Stages" type of application, where data of all the stops and routes is serialized using Protobuf and stored, and could be used later to give information on request on the "second stage". All the code is reworked to work with Protobuf. 

#### COMPILATION!
Only compilable on Linux using CMake. Protobuf should be installed.** I wasn't able to compile this version on windows because of std::filesystem that appears to work poorly on Windows (or not work at all).** Build it in another folder inside your transport_guide folder.

```
cmake .. -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=path/to/protobuf/package
cmake --build .
```

!Change 'path/to/protobuf/package' to your path to protobuf.
After that the transport_catalogue executable will be created in your build folder

#### Usage!
It's recommended to create input json file with your base requests and separate file with your stat requests.
Run:

```
./transport_catalogue make_base  < base_requests.json
./transport_catalogue process_requests < stat_requests.json > output.json
```

See example to better understand json syntax specifics for two stages app.
