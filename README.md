# lynx

Useful samples of using important and popular libraries in the C++ world.

## Using `lynx`
- `format` - parsers for xml ([`boost.property_tree`](https://github.com/boostorg/property_tree)), json([`boost.json`](https://github.com/boostorg/json)), protobuf([`protobuf`](https://github.com/protocolbuffers/protobuf)) formats.
- `net`    - synchronous/asynchronous network clients, servers ([`boost.asio`](https://github.com/boostorg/asio)).
- `db`     - synchronous/asynchronous database clients ([`boost.mysql`](https://github.com/boostorg/mysql)).
- `http`   - synchronous/asynchronous http clients, servers ([`boost.beast`](https://github.com/boostorg/beast)).

## Building
Perform the following actions for building project:

```bash
./setup.sh
```

Run unit tests:

```bash
./run_tests.sh
```

## Requirements

The following tools are needed:

* [`CMake`](https://cmake.org/)
* [`Conan`](https://conan.io/)
* Compiler with C++20 support or higher 


