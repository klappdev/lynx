**TODO**

[cpu]
- cpu info

[concurrency]
- blocking queue, set
- thread pool (future, fiber, coroutine)
- executors
- async mutex
- async latch, barrier, semaphore
- thread_utils: name, stack_size, priority, pin_to_cpu

[collection]
- ring buffer

[memory]
- debug allocator
- numa allocator
- byte_buffer (std::string_buf)

[device]
- serial port info

[fs]
- read/write file as line(s) (c-style, c++-style, boost.asio)
- memory file(fstream + mmap)
- file watcher

[fs/linux]
- async read/write file (io_uring)

[net]
- vlan info
- ethernet info
- bluetooth info
- route table info

[net/linux]
- client/server nonblocking
- client/server async (epoll, io_uring)
- client/server chat (libev)
- parser, sniffer (libpcap)
- monitor networking (netlink)


[http]
- async client/server (boost.beast)

[db]
- async client/server (boost.mysql)

[cache]
- boost.redis
- std::mutex + std::shared_ptr<const T>
- std::atomit<std::shared_ptr<const T>>
- std::atomic<const T> + std::hazard_pointer
- std::rcu

[format]
- yaml parser (yaml-cpp, rapid-yaml)

[ftp]
- blocking client/server

[simd]
- std::simd/intrinsics sort (x86-simd-sort)
- std::simd/intrinsics parse numbers
- simd algorithm (count, sum, ...)

[grpc]
- async client/server

[crypto]
- crypt/decrypt buffers (OpenSSL)
- hashing

[compress]
- zip, gzip (boost.iostreams)


