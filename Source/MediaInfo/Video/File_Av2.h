/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef MediaInfo_Av2H
#define MediaInfo_Av2H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/File__Duplicate.h"
#include <cmath>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Av2
//***************************************************************************

class File_Av2 : public File__Analyze
{
public :
    //In
    int64u  Frame_Count_Valid{};
    bool    IsAnnexB{ true };

    //Constructor/Destructor
    File_Av2();
    ~File_Av2();

private :
    //Streams management
    void Streams_Accept() override;
    void Streams_Fill() override;
    void Streams_Finish() override;

    //Buffer - Global
    void Read_Buffer_OutOfBand() override;

    //Buffer - Per element
    void Header_Parse() override;
    void Data_Parse() override;

    //Elements
    void sequence_header_obu();
    void sequence_partition_config();
    void sequence_segment_config();
    void sequence_intra_config();
    void sequence_inter_config();
    void sequence_scc_config();
    void sequence_transform_quant_entropy_config();
    void sequence_filter_config();
    void temporal_delimiter_obu();
    void multi_stream_decoder_operation_obu();
    void multi_frame_header_obu();
    void frame_header(bool isFirst);
    void metadata_short_obu();
    void metadata_group_obu();
    void tile_group_obu(int64u obuPayloadSize);
    void layer_config_record_obu();
    void atlas_segment_info_obu();
    void operating_point_set_obu();
    void buffer_removal_timing_obu();
    void quantizer_matrix_obu();
    void film_grain_obu();
    void content_interpretation_obu();
    void padding_obu();
    void reserved_obu();
    void obu_extension_data(int64u nbBits);
    void trailing_bits(int64u nbBits);

    //Temp
    bool    sequence_header_Parsed{};
    bool    SeenFrameHeader{};
    bool    single_picture_header_flag{};
    bool    Monochrome{};

    //Helpers
    void Get_leb128(int64u& Info, const char* Name);
    void Get_uvlc(int64u& Info, const char* Name);
    void Skip_uvlc(const char* Name);
};

} //NameSpace

#endif
