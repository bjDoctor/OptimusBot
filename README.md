# OptimusBot

This project implements a simple bot, trading on an ETH/USD simulated market.

# Build & Run

## Windows

The code has been developed with Visual Studio so, it is composed of a solution file and two project file (one for the bot, another for the tests).
The bot can be build and run within Visual Studio 2019 or later.

The tests can be run using VS's test explorer, or by executing `.\OptimusBot.Tests.exe` from the command line within `$(SolutionDir)$(Platform)\$(Configuration)\`.

## Cross-platform

A Dockerfile is also available, which enables the bot to run on any platform.
To build the docker image run the command `docker build -t optimusbot -f .\Dockerfile .` from the solution directory.
To run the app in a container, execute: `docker run optimusbot`

Unfortunately, I did not spend the time to investigate how the tests could be run inside a docker container...

## GitHub Actions

A GitHub Action pipeline, which builds the app using MSBuild and runs the tests, has also been setup for this repo.