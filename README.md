# OptimusBot

This project implements a simple bot, trading on an ETH/USD simulated market.

# Build & Run

## Windows

The code has been developed with Visual Studio so, it is composed of a solution file and two project file (one for the bot, another for the tests).
The bot can be built and run within Visual Studio 2019 or later.

The tests can be run using VS's test explorer, or by executing `.\OptimusBot.Tests.exe` from the command line within `$(SolutionDir)$(Platform)\$(Configuration)\`.

## Cross-platform

A Dockerfile is also available, which enables the bot to run on any platform.
To build the docker image, run the command `docker build -t optimusbot -f .\Dockerfile .` from the solution directory.

To run the app in a container, execute: `docker run optimusbot`

Unfortunately, I did not spend the time to investigate how the tests could be run inside a docker container...

## GitHub Actions

A GitHub Action pipeline, which builds the app using MSBuild and runs the tests, has also been setup for this repo.

# Design decisions

This ap is essentially composed of two parts: the Bot object and a set of static utility functions. 
The idea was to implement the utilities as small/stateless/testable functions.

Since the Bot itself has a mutable internal state governed by the market updates and its main observable behaviour is to print on screen, it is not covered by unit tests.

# Areas of improvement

## The trading strategy

The strategy used in the current implementations is that only prudent initial orders are placed when the bot starts up.

We could imagine more aggressive strategies, however that would imply adding periodic checks to ensure that if the market jumps, the remaining orders can be honored. The trading session would stop if not the case.

## Memory management

A virtual destructor would be required on `IDvfSimulator` to ensure that the memory allocated for the instantiated object can be cleared.

## Performance

### Multi-threading

This app is currently single-threaded and all computations are executed synchronously. 
Although the bot spends most of its time waiting for the next market update, if a performance gain would be required, using task-based programming via `std::async` would allow us to make use of multi-threading.

Note that `std::async` would be preferable to thread-based programming, via `std::thread`, since it vastly simplifies the thread management (thread exhaustion, load balancing, cross-platform...) and provides an easy way to access the return value of an asynchronous task.

### Algorithms

Implementing better algorithms is the other where performance improvements can be obtained. 

For example `OptimusBot::Utilities::ExtractBestOrder` calls `std::sort` on the order book input parameter. That would not be necessary if we could guarantee that the market simulator always returns a sorted order book. Alternatively, re-implementing that function using an algorithm iterating once over the order book and keeping track of the max bid and the min ask could also be considered.