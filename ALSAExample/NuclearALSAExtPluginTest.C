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

#include "ALSA/ALSAExternalPlugin.H"
using namespace std;
using namespace ALSA;

#include "NuclearALSA.H"

/** This class uses neutron-bomb-processing to execute lattice by lattice in parallel threads.
The implementation uses the NuclearALSA and NuclearALSAOut classes, but is more elaborate then required.
The extra elaboration shows how to chain multiple atom lattices (layers) together.

The processing is very simple. The input lattice (atom layer) is chained to one start trigger
atom. Once we trigger this atom, it triggers all of the input lattice to process.
The input lattice does nothing but receives the input ALSA audio data.
The input lattice triggers the output lattice, which copies the data received from the input
lattice over to the ALSA output audio buffer.
In this way, an elaborate method for copying the input ALSA audio data directly
to the ALSA output audio is performed through two lattices (atom layers).

Other implementations can implement signal processing in each of their layer's Fission::process methods to
do something more significant in a signal processing chain reaction !

The ::init method sets up the neutron-bomb-processing system.
It resizes the input and output atom lattices (layers) to match the input and output (slave)
channels counts.
It chains all of the atoms in the input lattice to the startTrigger atom.
It chains each output atom to a single input atom and sets its channels number.
It then starts the thread for all relevant input and output lattices.

The Futex of one Fission (the startTrigger) triggers the inChannels lattice to process.
Upon completion of the iChannels, the outChannels a triggered to execute.
The inChannels do nothing ! But their process methods could implement some form of processing.
The ALSA in data is copied directly to the inChannels during setup in the transfer method.
The outChannels are mapped to the inChannels as a wait and input data path. The outChannels
also have a pointer to the ALSA out data mapped matrix. This allows them to process
and copy the result over to the ALSA output data buffer.
The entire process is like so :
* ::transfer performs the following
* - Firstly :
* -- Zeros the output buffer
* -- copies the input audio data to the inChannels[] channel
* -- points the outChannels to the ALSA out audio matrix
*
* - Secondly :
* -- wakes the startTrigger atom, which performs an process then wakes all atoms in the waiting lattice (layer)
*/
class NuclearALSAExtPluginTest : public ALSAExternalPlugin {
		snd_pcm_format_t inFormat;
		snd_pcm_format_t outFormat;

		vector<NuclearALSA> inChannels; ///< The input data lattice
		vector<NuclearALSAOut> outChannels; ///< The output data lattice
		NuclearALSA startTrigger; ///< This atom triggers the input lattice

public:
	NuclearALSAExtPluginTest(){
    	std::cout<<__func__<<std::endl;
		setName("NuclearALSAExtPluginTest");
	}

	// virtual ~NuclearALSAExtPluginTest(){
  //   	std::cout<<__func__<<std::endl;
	// }

	virtual int specifyHWParams(){
		int ret;
		snd_pcm_extplug_set_param_minmax(&extplug, SND_PCM_EXTPLUG_HW_CHANNELS, 1, 128);
		snd_pcm_extplug_set_slave_param_minmax(&extplug, SND_PCM_EXTPLUG_HW_CHANNELS, 1, 128);
		cout<<Hardware::formatDescription(extplug.format)<<endl;
		cout<<Hardware::formatDescription(extplug.slave_format)<<endl;

		outFormat=SND_PCM_FORMAT_FLOAT_LE;
		ret=snd_pcm_extplug_set_slave_param(&extplug, SND_PCM_EXTPLUG_HW_FORMAT, outFormat);
		inFormat=SND_PCM_FORMAT_FLOAT_LE;
		ret=snd_pcm_extplug_set_param(&extplug, SND_PCM_EXTPLUG_HW_FORMAT, inFormat);

		return 0;
	}

  virtual int init(){
		cout<<__func__<<endl;
		cout<<"period size "<<getPeriodSize()<<endl;
		cout<<"extplug.rate "<<extplug.rate<<endl;
		cout<<"extplug.channels "<<extplug.channels<<endl;
		cout<<"extplug.slave_channels "<<extplug.slave_channels<<endl;
		cout<<"format "<<ALSA::Hardware::formatDescription(extplug.format)<<endl;
		cout<<"slave format "<<ALSA::Hardware::formatDescription(extplug.slave_format)<<endl;

		// set the correct channels numbers - this resizes the input and output
		// lattice of atoms to equal the channel numbers
		inChannels.resize(extplug.channels);
		outChannels.resize(extplug.slave_channels);

		// chain input to output atoms, according to the minimum number.
		for (int i=0;i<::min(extplug.channels, extplug.slave_channels);i++){
			inChannels[i].setChainReaction(&startTrigger); // the input lattice is triggered from one atom's Futex
			inChannels[i].resize(getPeriodSize(), 1);
			outChannels[i].setChainReaction(&inChannels[i]);
			outChannels[i].setChannel(i);
			outChannels[i].resize(getPeriodSize(), 1);
		}

		// create/run threads, guarding against failure
		int ret=0;
		for (int i=0;i<::min(extplug.channels, extplug.slave_channels);i++){
			ret=inChannels[i].run();
			if (!ret)
				ret=outChannels[i].run();
			if (ret)
				break;
		}
	  if (ret) // if any threads didn't create successfully, stop all threads
			for (int i=0;i<::min(extplug.channels, extplug.slave_channels);i++){
				inChannels[i].stop();
				outChannels[i].stop();
			}
    return ret;
  }

	virtual snd_pcm_sframes_t transfer(const snd_pcm_channel_area_t *dst_areas, snd_pcm_uframes_t dst_offset, const snd_pcm_channel_area_t *src_areas, snd_pcm_uframes_t src_offset, snd_pcm_uframes_t size){
		int ch=extplug.channels, slaveCh=extplug.slave_channels;
    Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic> stride(1,ch);
    Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic> strideSlave(1,ch);
		float *srcAddr=(float*)getAddress(src_areas, src_offset);
		float *dstAddr=(float*)getAddress(dst_areas, dst_offset);
		Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>, Eigen::Unaligned, Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic> >
																						in(srcAddr, size, ch, stride);
		Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>, Eigen::Unaligned, Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic> >
																						out(dstAddr, size, slaveCh, strideSlave);

		// printf("\ntransfer : in rows=%ld cols=%ld\n", in.rows(), in.cols());
		// printf("\ntransfer : out rows=%ld cols=%ld\n", out.rows(), out.cols());
		// printf("\n\ntransfer %f\n",in.col(0).maxCoeff());

		// initial setup
    out.setZero();
    ch=::min(ch, slaveCh); // copy only the smallest channel count over
		for (int i=0; i<ch; i++){
			inChannels[i].col(0)=in.col(i); // copy the input over to the first layer (here as its output)
			outChannels[i].assignOutParam(dstAddr, size, slaveCh);
		}

		// begin execution
		startTrigger.wakeAll();
		// sleep(0.03);
  	return size;
	}
};

NuclearALSAExtPluginTest nBEPlugin;
extern "C" SND_PCM_PLUGIN_DEFINE_FUNC(NuclearALSAExtPluginTest){
	std::cout<<__func__<<std::endl;
    nBEPlugin.parseConfig(name, conf, stream, mode);

	int ret=nBEPlugin.create(name, root, stream, mode);
	if (ret<0)
		return ret;

	nBEPlugin.specifyHWParams();

	*pcmp=nBEPlugin.getPCM();
	std::cout<<__func__<<" returning "<<std::endl;
    return 0;
}

SND_PCM_PLUGIN_SYMBOL(NuclearALSAExtPluginTest);
