# Actor Framework

The main objective of this library is to use the [actor model](https://en.wikipedia.org/wiki/Actor_model) to manage **parallelism** and **concurrency** in C++. This library is heavily inspired by [Charm++](http://charmplusplus.org/) and is based on MPI.

The origin of this library is an MSc project. The actor model has been separated from the rest of the simulation and expanded to support more functionality. The main objective was to better explore the actor model and to investigate some options provided by Charm++. Despite the lack of some features, and the limited scope of the project the library remains useful with some examples provided.

## Actor Model
The actor model is a popular way to implement **parallelism** and **concurrency**. In this model, an *actor* is the primitive in charge of computation, they can only communicate using messages, which avoids the need for locks or other types of synchronization. During a simulation *actors* alternate between performing calculations, sending messages, responding to messages, or creating new actors.

The *actor model* is extremely flexible, almost all simulations and programs can be expressed using this model, however, *actors* can only modify other actors between messages, which might make it not suitable for some simulations. Finally one should note that it is not clear how complex an *actor* should be, some prefer to create a few *actors* with greater complexity, while others opt for creating more *actors* that had a lesser complexity. The number of *actors* created has an impact on performance, however, usually is ideal to opt for an intermediate point, more actors will introduce a noticeable overhead, but most implementations will suffer from load balancing problems unless we have enough *actors*.

## MPI

MPI (Message Passing Interface) is a standardized and portable message-passing library used to communicate processes in scientific applications. MPI is the base used for the current implementation of this library.

MPI offers synchronous and asynchronous messages, however, only asynchronous messages were used by this program. Traditionally, most applications are implemented only with MPI, however, research suggests that the best performance might be obtained by combining MPI and traditional multithreading, the creation of multiple threads should allow for greater parallelism, but have reduced overhead since less intra-process communications are needed.

## Charm++

[Charm++](http://charmplusplus.org/) is a programming paradigm based on C++ with the objective of facilitate concurrency in massive scientific simulation programs like [NAMD](http://www.ks.uiuc.edu/Research/namd/), [OpenAtom](http://charm.cs.illinois.edu/OpenAtom/) or [ChaNGa](http://faculty.washington.edu/trq/hpcc/tools/changa.html).

Charm++ provides a high level of abstraction that allows dividing its program into *chares* which behave similarly to *actors*. Charm++ provides multiple implementations based on MPI, UDP, Infiniband, etc. Charm++ offers multiple tools to increase performance, like load balance, *chares* migration fault tolerance, or a hybrid architecture in which a process has multiple threads, allowing greater parallelism with less control and communication overhead.

## How to use

### Design
This library contains two main classes, `Actor` and `ExecutionUnit`. Two additional minor classes, `Message` and `Logger` were also created.

The class `Logger` handles the console output, it contains a custom print function to print text indicating the actor id and execution unit responsible for the message. Several log levels are provided, allowing to show internal messages if necessary.

The class `Message` is responsible for storing the sent data. This class is based on an MPI message containing the origin and destiny actor ID. Users can also use a type to store the type of message. Apart from messages between actors we also support multiple internal messages to control the simulation.

We need to create an `ExecutionUnit` per each process. This class handles all our communications and most of our framework logic. It is responsible for creating and distributing the different actors, handling communications, killing and updating actors or finishing the simulation.

We have a master `ExecutionUnit` with additional data to handle the simulation. We have a limited load balance algorithm, sending new actors to the less occupied unit, to support this actor creation parameters need to be able to receive its arguments as a message.

Our framework uses asynchronous messages to handle the asynchronous nature of communications across actors. To avoid problems, each actor has a buffer with messages waiting to be sent.

Our execution waits until all `Actors` are completed or we force an early finish. `Actor` is the main class that the user uses to interact with the simulation. Usually, we need to create a new class for each type of `Actor`.

During the simulation, we are continuously updating each `Actor` and processing new messages. When we create a new type of `Actor` we need to override the functions `processMessage` and `update` to specify this behaviour.

### Compilation

This library provides a Makefile which contains an example of how to compile and link this library. To compile the examples provided with this library one must install MPI. Build essentials are also suggested.
```
sudo apt install mpich build-essential
```
The current Makefile will generate the corresponding executables in the current directory
```
make .
```
Finally to run each example with 4 MPI processes, one can type
```
mpirun -n 4 Build/Squirrels-Simulation.out
mpirun -n 4 Build/Simulation-example.out
```
### Documentation

We had used Doxygen to generate documentation for all functions on *docs/*

## Examples
### Simple-Example

This is a simple example in which one type of actor is sending data to another actor which receives it and shows it.

### Squirrels-Simulation

This is a complex simulation in which we simulate a population of squirrels. Squirrels are being born and killed dynamically, and we use different reductions to show a resume of the state of the simulation.

## Future work

### Early termination problems

Currently, we are not waiting to ensure that all messages are received once the simulation is completed, this causes the following error in some MPI applications.
```
tag_match.c:62   UCX  WARN  unexpected tag-receive descriptor 
0x558c43f27d80 was not matched
```

### MPI independence

Currently, the only way of communication is using MPI, with some MPI primitives still being exposed in the interface. In the future, we plan to remove all these primitives from the interface and support other ways of communication like UDP or TCP.

### Multithreading

Our actor framework is suitable for a hybrid architecture, we can create multiple threads per execution unit, to support this we might need to add additional locks to some critical sections of the execution unit.

A hybrid architecture will provide greater parallelism at the same time it will reduce the control overhead.

### Load migration

Currently, we have a simple load balance by creating new actors in the less occupied unit. It should be possible to move actors to less occupied units. The way to decide when one should do this is not trivial. Probably we should allow the users how to use this feature. We might want to provide users with the option to implement their custom load balance algorithm, similar to Charm++. At the same time, we might want to allow them to define the actual cost of each actor, instead of supposing that all of them have a similar load.

### Dynamic sent buffer and message size

MPI requires raw pointers, this causes the need to create a few buffers to store all messages. Currently, we are allocating vectors at the beginning of the program to handle this, however, a more complex data structure using a linked list of allocated pointers could remove the necessity to implement this.

At the same type, it is complicated to know the size of an MPI message beforehand, to handle this we have a maximum message size, we still only send and receive the necessary bytes, however, we are allocating unnecessary memory. This also causes an artificial limit, since our framework will be unable to handle messages of a bigger size.