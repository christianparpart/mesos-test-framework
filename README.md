
### Building from Source

```sh
# requirements
sudo apt-get install libprotobuf-dev libboost-all-dev

cmake .
make

# run the framework scheduler
./src/my-scheduler

# the executor gets invoked automatically on the slave node(s)
```

