/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Immersive Audio Model and Formats (IAMF) files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Iamf
#define MediaInfo_Iamf
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <memory>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Iamf
//***************************************************************************

class File_Iamf : public File__Analyze
{
public:
    // Constructor/Destructor
    File_Iamf();
    ~File_Iamf();

private:
    // Streams management
    void Streams_Accept();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();

    // Buffer - Global
    void Read_Buffer_OutOfBand();

    // Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    // Elements
    void ia_codec_config();
    void ia_audio_element();
    void ia_mix_presentation();
    void ia_sequence_header();
    void ParamDefinition(int64u param_definition_type);
    void ia_parameter_block();
    void ia_temporal_delimiter() {}; // Temporal Delimiter OBU has an empty payload.
    void ia_audio_frame(bool audio_substream_id_in_bitstream);

    //Temp
    int64u Frame_Count_Valid{};
    std::unique_ptr<File__Analyze> Parser_Opus;
    std::unique_ptr<File__Analyze> Parser_Flac;
    struct ParamDefinitionData{
        int64u param_definition_type{};
        int8u  param_definition_mode{};
        int64u duration{};
        int64u constant_subblock_duration{};
        int64u num_subblocks{};
    };
    std::map<int64u, ParamDefinitionData> param_definitions;
    int8u num_layers{};
    std::vector<bool> recon_gain_is_present_flag_Vec;
    size_t Frame_Count{};
    std::map<int64u, int32u> codecs;
    std::map<int64u, int64u> substreams;

    // Helpers
    void Get_leb128(int64u& Info, const char* Name);
};

} //NameSpace

#endif // !MediaInfo_Iamf
