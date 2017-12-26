# nuclear-processing
Lockless, ordered, threaded, parallel chain reaction processing.

See later in this document for setup, bulding and running examples

Fission|Fusion
--- | ---
![Chain reaction - fission](https://upload.wikimedia.org/wikipedia/commons/f/f0/Nuclear_fission_chain_reaction.svg "fission chain reaction") | ![Chain reaction - fusion](https://upload.wikimedia.org/wikipedia/commons/thumb/8/85/Fusion_in_the_Sun.svg/421px-Fusion_in_the_Sun.svg.png "fusion chain reaction")

## What is nuclear processing ?

A processing methodology where either one process triggers many different processes to start or many processes complete triggering one process to start. It can be set up like a fission chain reaction, where multiple processes can trigger multiple subsequent processes. It can also be set up like a fusion chain reaction, where multiple processes can complete and fuse into one process. Fusion and fission processes can be arbirarily and sequentially ordered.

In the context of this project, fission and fusion processing have the following properties :
* lockless
* parallel
* ordered
* light weight
* low latency

## How can I think of it ?

### How can I think of the fission system ?

Think of a fission chain reaction explosion where Uranium 235 triggers many other uranium atoms to split. One path of fission is dependent on only only prior computations in the fission chain. Computation is independent of other fission chains.

Each Atom waits for the prior atom in the chain reaction (the chain atom) to complete processing and signal it's completion. The Atom then
executes the process which most likely begins with a memory copy and then computation if necessary. On completion, it signals the next atoms
up the chain to begin processing.

The Fission class (process) uses a ThreadedMethod for performing the wait, process and wake loop. A Futex is used for waiting and waking.

### How can I think of the fusion system ?

A simple example is that a previous set of atomic Fission (or Fusion) reactions have completed. The atoms which result from this fission or fusion process combine and fuse into one process.

The Fusion class uses a Futex and atomic operations for signalling that multiple prior atomic processes have successfully fused.

## Examples

This repository has examples.

### ALSA audio processing examples

#### Nuclear ALSA processing

In the ALSAExample/NuclearALSAExtPluginTest.C file, an external ALSA plugin is created which gives an example of combining both nuclear fission and then nuclear fusion to process audio.

The first step breaks all input channels into seperate processing threads. The first atomic lattice of fission processing copies the input audio data into Eigen matrix columns. These atoms have an empty process which trigger the next fission process, where the audio data is copied to the ALSA output buffer.
The second step fuses all output nuclear processes together so that fusion doesn't occur until all output channels have been processed. Once fusion is complete, the process has ended and execution is passed back to the Kernel ALSA subsystem.

## setup

To setup, clone then run :
git submodule init
git submodule update
./tools/autotools.sh
./configure
make

## To run the ALSA example

Ensure you have the libasound2 and eigen3 development packages installed - then execute the configure and make from the setup section to build the ALSA plugin.
Once built, find where your alsa plugins are on your system :
find /usr/lib* -name '*libasound_module_pcm*'

Say it tells you that they are located in the /usr/lib/x86_64-linux-gnu/alsa-lib/ directory, then link the built plugin to there :
sudo ln -s `pwd`/ALSAExample/.libs/libasound_module_pcm_NuclearALSAExtPluginTest.so /usr/lib/x86_64-linux-gnu/alsa-lib

You can run the ALSA plugin with a command similar to the following :
aplay -Dnp yourFile.wav

You will need to setup your .asoundrc file similar to the following example - it uses the lfloat pcm plugins to ensure we are processing floating point in and out in our plugin :
```
pcm.np {
        type lfloat;
        slave.pcm "npin";
        slave.format FLOAT_LE
}

pcm.npin {
	type NuclearALSAExtPluginTest;
	slave.pcm "floatOut";
}

pcm.floatOut {
	type lfloat
	slave.pcm linearOut;
	slave.format S32_LE;
}

pcm.linearOut {
        type linear;
	      slave.pcm fileOut
        slave.format S32_LE
}
```

## A history of loop fission computation

Loop fission goes back decades to the development of SPICE and possibly before that to parallel computation, where computational loops
loops are broken up into smaller loops which can be executed concurrently. If you are interested, see this [Wikipedia article on
loop fission](https://en.wikipedia.org/wiki/Loop_fission_and_fusion), which gives the following very basic example :
```C
int i, a[100], b[100];
for (i = 0; i < 100; i++) {
 a[i] = 1;
 b[i] = 2;
}
// is equivalent to
int i, a[100], b[100];
for (i = 0; i < 100; i++)
 a[i] = 1;                     
for (i = 0; i < 100; i++)
 b[i] = 2;
 ```
 These two loops can clearly be executed in parallel without effecting each other. However it has no reference to ordering.
