# jsh-tyy - Linux Job Control Shell

This project was carried out as part of an academic project for Operating Systems 5.
It's a simple yet functional shell that supports job control.

## Features

This is a non exhaustive list of the features of our shell:

- Built-in commands: `cd`, `exit`, `jobs`, `fg`, `bg`, `kill` and `?` (environment variables are not updated, so this is the same as `echo $?`)
- Redirections: `>`, `>|`, `>>`, `2>`, `2>|`, `2>>`, `<`
- Pipelines: `|`
- Command substitution: `<()`
- Background jobs: `&`
- Job control

## Running the shell

### Prerequisites

This project requires `gcc` and `make` to be installed on your system, along with the `readline` library.
It should run on any Linux distribution, but it's sadly not `POSIX` compliant,
we use the `proc` virtual filesystem to get some pieces of information about the processes
and for named pipes, maybe one day we'll change that, should be fun.

For those of you who are interested on running the tests, you will need `valgrind` installed on your system (explained later).

### Compilation

To compile the project, execute the command

```sh
make
```

or

```sh
make jsh
```

### Execution

To run the project, execute the following command after compiling it

```sh
./jsh
```

## Testing

You can run the tests using the following commands, some may require `valgrind` to be installed and some may
take a while to run (should be less than 2 minutes).

```sh
make test-unit        # our unit tests
make test-valgrind    # our unit tests with valgrind
make test-tyy         # our tests using the professors' framework
make test-professor   # our professors' test framework
make test             # all the tests
```

## More information (in French)

You can see the project's formal specification in the [project.md](./project.md) file.

We have also written a small report on our architecture and the choices we made,
you can find it in the [ARCHITECTURE.md](./ARCHITECTURE.md) file.

## Authors

See [AUTHORS.md](./AUTHORS.md)
