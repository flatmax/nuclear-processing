// Copyright (c) 2017 The neutron bomb processing Authors. All rights reserved.
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

class NeutronBombALSAExtPluginTest : public ALSAExternalPlugin {
		snd_pcm_format_t inFormat;
		snd_pcm_format_t outFormat;

public:
	NeutronBombALSAExtPluginTest(){
    	std::cout<<__func__<<std::endl;
		setName("NeutronBombALSAExtPluginTest");
	}

	virtual ~NeutronBombALSAExtPluginTest(){
    	std::cout<<__func__<<std::endl;
	}

	virtual int specifyHWParams(){
		int ret;
		snd_pcm_extplug_set_param_minmax(&extplug, SND_PCM_EXTPLUG_HW_CHANNELS, 1, 128);
		snd_pcm_extplug_set_slave_param_minmax(&extplug, SND_PCM_EXTPLUG_HW_CHANNELS, 1, 128);
		cout<<Hardware::formatDescription(extplug.format)<<endl;
		cout<<Hardware::formatDescription(extplug.slave_format)<<endl;

		outFormat=SND_PCM_FORMAT_FLOAT_LE;
		ret=snd_pcm_extplug_set_slave_param(&extplug, SND_PCM_EXTPLUG_HW_FORMAT, outFormat);
		cout<<ret<<endl;
		inFormat=SND_PCM_FORMAT_FLOAT_LE;
		ret=snd_pcm_extplug_set_param(&extplug, SND_PCM_EXTPLUG_HW_FORMAT, inFormat);
		cout<<ret<<endl;

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
    return 0;
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

    out.setZero();
    ch=::min(ch, slaveCh); // copy only the smallest channel count over
		out.block(0,0,out.rows(),ch)=in.block(0,0,in.rows(),ch);
  	return size;
	}
};

NeutronBombALSAExtPluginTest nBEPlugin;
extern "C" SND_PCM_PLUGIN_DEFINE_FUNC(NeutronBombALSAExtPluginTest){
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

SND_PCM_PLUGIN_SYMBOL(NeutronBombALSAExtPluginTest);
