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

#ifndef FISSION_H_
#define FISSION_H_

#include <Thread.H>
#include <Futex.H>

#include "Fusion.H"

/** Atomic Fission is one of the basic units of computation. Fission inherits a
<a href="http://gtkiostream.flatmax.org/classFutex.html">Futex</a> for waking
other atomic processes down the chain reaction after computing. Fission inherits a
<a href="http://gtkiostream.flatmax.org/classThreadedMethod.html">ThreadedMethod</a>
to run the private threadMain method, which calls your process method from your
inheriting class. The implementation would look something like this :

\code{.cpp}
class ComputeSomething : public Fission {

  // this is an example of your output data
  double data[1024];

  int process(){
    // First copy the data from chainReaction over here
    for (int i...)
      data[i]=chainReaction->data[i];
    // Now process the data
    for (int i...)
      data[i]=yourProcessingFunction(data[i]);

    // The previous two loops could be combined into one loop like so :
    for (int i...)
      data[i]=yourProcessingFunction(chainReaction->data[i]);

    // Perhaps you need to do something different for data copying and processing ?
    // This process method is where you do it.

    return 0;
  }
}
\endcode
*/
class Fission : public Futex, public ThreadedMethod {
  /** In this processing thread, wait for the prior atomic reaction to complete,
  then execute our process here.
  This thread will cease to execute if either the chainReaction is (or becomes) NULL
  or if the process method returns a value <0.
  \return NULL on exit.
  */
  virtual void *threadMain(void){
    while (true){
      if (!chainReaction) // if we have no atomic reaction defined prior to us, then exit.
        break;
      chainReaction->wait(); // Wait for the prior atomic reaction to fin. processing
      if (process()<0) // process
        break;
      wakeAll(); // Wake all of the atomic reactions waiting on this process to complete.
      if (inFusion) // If we are part of a fusion reaction, indicate completion here
        inFusion->fuse();
    }
    return NULL;
  }

protected:
  Fission *chainReaction; ///< The prior atomic reaction in the chain reaction
  Fusion *inFusion; ///< If necessary, this atom will become part of a fusion rection once processed

public:
  /** Constructor
  We start with a NULL chainReaction
  */
  Fission(){
    chainReaction=NULL;
    inFusion=NULL;
  }

  /** Process the data.
  Here you implement your process method which copies data as required from the
  chainReaction. Once you have finished computing, return <0 on error, >=0 otherwise

  \return <0 on error
  */
  virtual int process()=0;

  /** Add the chain atomic reaction - this is the reaction which we wait on
  \param chain The prior atomic reaction we are waiting on
  */
  void setChainReaction(Fission *chain){
    chainReaction=chain;
  }

  /** Add the fusion reaction which this atom will be part of upon completing process.
  \param f The Fusion atomic process
  */
  void setFusionReaction(Fusion *f){
    inFusion=f;
  }
};
#endif // FISSION_H_
