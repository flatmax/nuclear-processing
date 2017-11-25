# neutron-bomb-processing
Lockless, ordered, threaded, parallel chain reaction processing.
![Chain reaction](https://upload.wikimedia.org/wikipedia/commons/f/f0/Nuclear_fission_chain_reaction.svg "uranium chain reaction")

## What is neutron bomb processing ?

A processing methodology where one process triggers many different processes to start. It can be set up like a chain reaction,
where multiple processes can trigger multiple subsequent processes.

In the context of this project, fission processing has the following properties :
* lockless
* parallel
* ordered
* light weight
* low latency

## How can I think of it ?

Think of a chain reaction explosion where Uranium 235 triggers many other uranium atoms to split. One path of fission is dependent on only only prior computations in the fission chain. Computation is independent of other fission chains.

It is comprised of the following elements, where atoms are combined into a crystalline structure. One crystal lattice layer is synchronised to start computation at the same time. The atom's nucleus performs the computation and the neutrons ejected during fission transfer data between atoms.

### LatticeLayer - the latice layer made of atoms (or ions) which are synchronised

The LatticeLayer holds a group of atoms (computational units). The atoms are synchronised to start computation at the same time. This is accomplished by waiting on a Futex.

### Atom - the computational unit

The Atom inherits the Nucleus

### Nucleus - performs the computation

The Nucleus performs the computation.

### Neutron - the messenger

The Neutron is the messenger, it performs the data transfer between atoms.

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
