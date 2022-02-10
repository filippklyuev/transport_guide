# Basic transport guide/catalogue console app.

## Program is created using c++ and is designed to build bus stops/routes map of given information (i.e. stop coordinates, distance between stops, routes etc). Two branches exist - main branch 'one stage' application, two stages branch - 'two stages' application.


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

This code will create json file with your output. Also option to print only SVG map is availible- currently it's not very user friendly,  to use it properly open main.cpp, comment and uncomment marked lines of code - request for map should be present in stat_requests. Also note that currently SVG maps are not proper representation of the given distances between stops! They are built solely with coordinates and don't take into account real distances between stops. That's why I've provided some of the examples with proper png map representations - use them to track your built routes.
