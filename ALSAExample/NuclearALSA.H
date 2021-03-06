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

#ifndef NUCLEARALSA_H_
#define NUCLEARALSA_H_

#include "Fission.H"
#include "Fusion.H"

/** This class does no processing. It inherits an Eigen column which will hold
its output data after processing.
*/
class NuclearALSA : public Fission, public Eigen::Matrix<float, Eigen::Dynamic, 1> {
  /** This default process does nothing, assumes that audio data is manually
  placed into the output Eigen column by a different thread.
  */
  virtual int process(){
    return 0;
  }
};

/** Class to act as output data layer.
Its job is to copy the previous atomic process's data (chainReaction) into the
ALSA output audio buffer. This happens in the process method.
*/
class NuclearALSAOut : public NuclearALSA {
  unsigned int ch; ///< The channel in the out matrix
  unsigned int chCnt; ///< The slave's channel count

  float *dstAddr; ///< Points to the start of the ALSA output buffer
  snd_pcm_uframes_t size; ///< The size of the ALSA output buffer

  /** This output layer simply copies memory across to the ALSA memory buffer
  \return 0
  */
  virtual int process(){
    if (!dstAddr) // if we don't have a destination data address, exit early
      return 0;

    /// The ALSA output matrix - we are col(ch) in this matrix
    Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic> stride(1,chCnt);
    Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>, Eigen::Unaligned, Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic> >
                                            out(dstAddr, size, chCnt, stride);

    out.col(ch)=*dynamic_cast<NuclearALSA *>(chainReaction);
    return 0;
  }

public:

  /** Constructor, start with out as NULL
  */
  NuclearALSAOut(){
    dstAddr=NULL;
  }

  /** The channel in the Eigen::Map of this output
  */
  void setChannel(unsigned int whichCh){
    ch=whichCh;
  }

  /** Assign the ALSA output parameters for setting up the destination data buffer.
  \param dstAddrIn Is the destination address pointer, the start of the ALSA output audio buffer.
  \param sizeIn is the size of the ALSA buffer
  \param chCntIn is the number of output channels addressed by the output audio buffer.
  */
  void assignOutParam(float *dstAddrIn, snd_pcm_uframes_t sizeIn, unsigned int chCntIn){
    dstAddr=dstAddrIn;
    size=sizeIn;
    chCnt=chCntIn;
  }
};

/** This Fusion process is used to wait until all of the NuclearALSAOut fission processes
have completed. Once complete, we can release the ALSA processing thread to continue.
*/
class FuseALSA : public Fusion {
  // An empty process
  virtual int process(){return 0;}
};
#endif //NUCLEARALSA_H_
