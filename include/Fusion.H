// Copyright (c) 2017 The nuclear processing Authors. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Flatmax Pty Ltd nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef FUSION_H_
#define FUSION_H_

#include <Thread.H>
#include <Futex.H>

/** Atomic Fusion is one of the basic units of computation. Fusion inherits a
<a href="http://gtkiostream.flatmax.org/classFutex.html">Futex</a> for waking
other atomic processes down the chain reaction after computing (process).
*/
class Fusion  : public Futex {
  volatile int fused; ///< Counter, when = 0 indicates that all atoms are fused

public:
  /** Constructor, tests to see that atomic operations are available on this arch.
  */
  Fusion(){
    if (!__atomic_always_lock_free(sizeof(fused), &fused)){
      cout<<"Fusion:: Error fused variable is not lock free"<<endl;
      exit(1);
    }
  }

  /** Process the data.
  Here you implement your process method which copies data as required from the
  chainReaction. Once you have finished computing, return <0 on error, >=0 otherwise
  \return <0 on error
  */
  virtual int process()=0;

  /** Sets the number of atoms required for fusion to copmlete.
  This must be called in a state where it isn't possible to call fuse
  \param cnt The number of atoms before fusion is complete
  */
  void setFusionAtomCount(unsigned int cnt){
    __atomic_exchange_n(&fused, cnt, __ATOMIC_RELAXED);
  }

  /** Indicate that one new atom has fused. Once all atoms have fused, wake all
  waiting atoms (atomic processes) and reset the fused count
  */
  void fuse(){
    if (__atomic_sub_fetch(&fused, 1, __ATOMIC_RELAXED)==0){ // if equal to zero
      int ret=process();
      if (ret<0)
        printf("Fusion::fuse : process error %d, continuing\n", ret);
      wakeAll(); // wake all waiting atomic processes
    }
  }

  /** Wait until all atoms are fused.
  It is better to use this method, rather then the Futex::wait method because
  we check that fused!=0 here before we wait.
  */
  void waitFused(){
    // printf("Fusion::setFusionAtomCount : fused = %d\n",__atomic_load_n(&fused, __ATOMIC_RELAXED));
    if (__atomic_load_n(&fused, __ATOMIC_RELAXED)!=0)
      wait();
  }
};
#endif // FUSION_H_
