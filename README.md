# Basic transport guide/catalogue console app.

## Program is designed to build bus stops/routes map of given information (i.e. stop coordinates, distance between stops, routes etc). 

### Main Branch
Branch represents "one stage" type of application, meaning that program recieves input only once per one launch. The input should include basic stops/routes information and user requests **in one request**.

Program recieves json dictionary with different keys representing different data: 
	**base_requests** is an array of dictionaries of routes and stops with all the necessary information to build map and build routes (see example to understand the syntax). Render settings provides necessary info for svg formatted map to be created. Routing settings represents time of waiting for a bus (before boarding new bus and in between transfers) and bus velocity.
	**stat_requests** represent the user requests to the program. "Route" requests tries to build
the fastest route between two stops, map requests build svg map of given stops and buses, and route/stop requests are giving info on specific bus or stop.

#### COMPILATION!
Code files are proven to be compilable on Windows and Linux (Mac should be OK as well) with g++. To compile use

```
g++ -std=c++17
```

Obviously gcc should be installed for compilation:).

#### Usage!
After compilation the executable should be created. In this example my executable would be named 'transport_guide.exe' but you can name it as you want. It's highly recommended to create input json file to not copy/paste the whole json raw text to the terminal for every request. In my example this file would be called 'input.json'. To see how json file should be created I've provided you with examples folder. **IMPORTANT NOTE!!! In current version of the program json should always have all the necessary keys - _base_requests_, _stat_requests_, _render_settings_ and _routing_settings_**. After creating your json with all the necessary info and requests run the exe as follows:

```
transport_guide.exe < input.json > output.json
```

This code will create json file with your output. Also option to print only SVG map is availible- currently it's not very user friendly,  to use it properly open main.cpp, comment and uncomment marked lines of code - request for map should be present in stat_requests. Also note that currently svg map are not proper representation of the given distances between stops! They are built solely with coordinates and dont take into account real distances between stops. That's why I've provided some of the examples with proper png map representations - use them to track your built routes.

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

