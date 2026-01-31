/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_AV2_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Av2.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
extern const char* Mpegv_colour_primaries(int8u colour_primaries);
extern const char* Mpegv_transfer_characteristics(int8u transfer_characteristics);
extern const char* Mpegv_matrix_coefficients(int8u matrix_coefficients);
extern const char* Mpegv_matrix_coefficients_ColorSpace(int8u matrix_coefficients);
extern const char* Avc_video_full_range[];

//---------------------------------------------------------------------------
static const char* Av2_obu_type(int8u obu_type)
{
    switch (obu_type)
    {
        case  1 : return "OBU_SEQUENCE_HEADER";
        case  2 : return "OBU_TEMPORAL_DELIMITER";
        case  3 : return "OBU_MULTI_FRAME_HEADER";
        case  4 : return "OBU_CLOSED_LOOP_KEY";
        case  5 : return "OBU_OPEN_LOOP_KEY";
        case  6 : return "OBU_LEADING_TILE_GROUP";
        case  7 : return "OBU_REGULAR_TILE_GROUP";
        case  8 : return "OBU_METADATA_SHORT";
        case  9 : return "OBU_METADATA_GROUP";
        case 10 : return "OBU_SWITCH";
        case 11 : return "OBU_LEADING_SEF";
        case 12 : return "OBU_REGULAR_SEF";
        case 13 : return "OBU_LEADING_TIP";
        case 14 : return "OBU_REGULAR_TIP";
        case 15 : return "OBU_BUFFER_REMOVAL_TIMING";
        case 16 : return "OBU_LAYER_CONFIGURATION_RECORD";
        case 17 : return "OBU_ATLAS_SEGMENT";
        case 18 : return "OBU_OPERATING_POINT_SET";
        case 19 : return "OBU_BRIDGE_FRAME";
        case 20 : return "OBU_MSDO";
        case 21 : return "OBU_RAS_FRAME";
        case 22 : return "OBU_QUANTIZATION_MATRIX";
        case 23 : return "OBU_FILM_GRAIN";
        case 24 : return "OBU_CONTENT_INTERPRETATION";
        case 25 : return "OBU_PADDING";
        default : return "";
    }
}

//---------------------------------------------------------------------------
static bool Av2_is_extensible_obu(int8u obu_type)
{
    switch (obu_type)
    {
        case  1:
        case  3:
        case 16:
        case 24:
        case 18:
        case 17: return true;
        default: return false;
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Av2::File_Av2()
{
    StreamSource = IsStream;
    FrameIsAlwaysComplete = false;
}

//---------------------------------------------------------------------------
File_Av2::~File_Av2()
{
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Av2::Streams_Accept()
{
    Fill(Stream_General, 0, General_Format, "AV2");

    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "AV2");

    if (!Frame_Count_Valid)
        Frame_Count_Valid=Config->ParseSpeed>=0.3?8:(IsSub?1:2);
}

//---------------------------------------------------------------------------
void File_Av2::Streams_Fill()
{
}

//---------------------------------------------------------------------------
void File_Av2::Streams_Finish()
{
    
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Av2::Read_Buffer_OutOfBand()
{
    //Parsing
    bool initial_presentation_delay_present;
    BS_Begin();
    Mark_1 ();
    Skip_S1(7,                                                  "version");
    Skip_S1(3,                                                  "seq_profile");
    Skip_S1(5,                                                  "seq_level_idx_0");
    Skip_SB(                                                    "seq_tier_0");
    Skip_SB(                                                    "high_bitdepth");
    Skip_SB(                                                    "twelve_bit");
    Skip_SB(                                                    "monochrome");
    Skip_SB(                                                    "chroma_subsampling_x");
    Skip_SB(                                                    "chroma_subsampling_y");
    Skip_S1(2,                                                  "chroma_sample_position");
    Skip_S1(3,                                                  "reserved");
    Get_SB (   initial_presentation_delay_present,              "initial_presentation_delay_present");
    Skip_S1(4,                                                  initial_presentation_delay_present?"initial_presentation_delay_minus_one":"reserved");
    BS_End();

    Open_Buffer_Continue(Buffer, Buffer_Size);
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Av2::Header_Parse()
{
    //Parsing
    int64u obu_size;
    if (IsAnnexB) {
        Element_DoNotShow();
        Element_End0();
        Get_leb128(obu_size,                                    "num_bytes_in_obu");
        Element_Begin0();
    }
    else {
        obu_size = Element_Size;
    }
    auto sizeof_obu_size = Element_Offset;

    Element_Name("obu_header");
    bool obu_header_extension_flag;
    int8u obu_type;
    BS_Begin();
    Get_SB  (   obu_header_extension_flag,                      "obu_header_extension_flag");
    Get_S1  (5, obu_type,                                       "obu_type");
    Skip_S1 (2,                                                 "obu_tlayer_id");
    if (obu_header_extension_flag) {
        Skip_S1 (3,                                             "obu_mlayer_id");
        Skip_S1 (5,                                             "obu_xlayer_id");
    }
    BS_End();

    FILLING_BEGIN();
    Header_Fill_Size(obu_size + sizeof_obu_size);
    Header_Fill_Code(obu_type, Av2_obu_type(obu_type));
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Av2::Data_Parse()
{
    //Probing mode in case of raw stream //TODO: better reject of bad files
    if (!IsSub && !Status[IsAccepted] && (Element_Code != 1))
    {
        Reject();
        return;
    }

    //Parsing
    Accept();
    BS_Begin();
    auto obuPayloadSize = Element_Size;
    auto bits_remain_begin = Data_BS_Remain();
    auto startPosition = 0ULL;
    bool is_tile_group = false;
    Element_Begin0();
    switch (Element_Code)
    {
    case  1: sequence_header_obu(); break;
    case  2: temporal_delimiter_obu(); break;
    case  3: multi_frame_header_obu(); break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 19: frame_header(true); break;
    case  8: metadata_short_obu(); break;
    case  9: metadata_group_obu(); break;
    case  6:
    case  7:
    case  4:
    case  5:
    case 10:
    case 21: tile_group_obu(obuPayloadSize); is_tile_group = true; break;
    case 16: layer_config_record_obu(); break;
    case 17: atlas_segment_info_obu(); break;
    case 18: operating_point_set_obu(); break;
    case 15: buffer_removal_timing_obu(); break;
    case 22: quantizer_matrix_obu(); break;
    case 23: film_grain_obu(); break;
    case 24: content_interpretation_obu(); break;
    case 25: padding_obu(); break;
    default: reserved_obu(); break;
    }
    if (Element_Code > 2 && Element_Code < 25) { // Parsing not yet implemented
        Skip_BS(Element_Size * 8,                               "Data");
        Element_End0();
        BS_End();
        return;
    }
    if (Element_Code == 1) { // Not yet completed
        Skip_BS(Data_BS_Remain(), "(Not parsed)");
        Element_End0();
        BS_End();
        return;
    }
    Element_End0();
    auto usedArith = is_tile_group;
    auto bits_remain_end = Data_BS_Remain();
    auto currentPosition = bits_remain_begin - bits_remain_end + startPosition;
    auto parsedPayloadBits = currentPosition - startPosition;
    auto remainingPayloadBits = obuPayloadSize * 8 - parsedPayloadBits;
    if (obuPayloadSize > 0 && !usedArith) {
        if (Av2_is_extensible_obu(Element_Code)) {
            bool obu_extension_flag;
            Get_SB(obu_extension_flag,                          "obu_extension_flag");
            if (obu_extension_flag)
                obu_extension_data(remainingPayloadBits - 1);
            else
                trailing_bits(remainingPayloadBits - 1);
        }
        else {
            trailing_bits(remainingPayloadBits);
        }
    }
    BS_End();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Av2::sequence_header_obu()
{
    Element_Name("sequence_header_obu");

    //Parsing
    Skip_uvlc(                                                  "seq_header_id");
    Skip_S1(5,                                                  "seq_profile_idc");
    TESTELSE_SB_GET(single_picture_header_flag,                 "single_picture_header_flag");
        // seq_lcr_id = 0 	
        // still_picture = 1
    TESTELSE_SB_ELSE(                                           "single_picture_header_flag");
        Skip_S1(3,                                              "seq_lcr_id");
        Skip_SB(                                                "still_picture");
    TESTELSE_SB_END();
    int8u seq_level_idx;
    Get_S1(5, seq_level_idx,                                    "seq_level_idx");
    if (seq_level_idx > 7 && !single_picture_header_flag) {
        Skip_SB(                                                "seq_tier");
    }
    else {
        // seq_tier = 0
    }
    int8u frame_width_bits_minus_1, frame_height_bits_minus_1;
    Get_S1(4, frame_width_bits_minus_1,                         "frame_width_bits_minus_1");
    Get_S1(4, frame_height_bits_minus_1,                        "frame_height_bits_minus_1 ");
    int32u max_frame_width_minus_1, max_frame_height_minus_1;
    Get_S4(frame_width_bits_minus_1 + 1, max_frame_width_minus_1, "max_frame_width_minus_1");
    Get_S4(frame_height_bits_minus_1 + 1, max_frame_height_minus_1, "max_frame_height_minus_1");
    TESTELSE_SB_SKIP(                                           "seq_cropping_window_present_flag");
        Skip_uvlc(                                              "seq_cropping_win_left_offset");
        Skip_uvlc(                                              "seq_cropping_win_right_offset");
        Skip_uvlc(                                              "seq_cropping_win_top_offset");
        Skip_uvlc(                                              "seq_cropping_win_bottom_offset");
    TESTELSE_SB_ELSE(                                           "seq_cropping_window_present_flag");
        // seq_cropping_win_left_offset = 0
        // seq_cropping_win_right_offset = 0
        // seq_cropping_win_top_offset = 0
        // seq_cropping_win_bottom_offset = 0
    TESTELSE_SB_END();
    Element_Begin1("color_config");
        int64u chroma_format_idc, bit_depth_idc;
        Get_uvlc(chroma_format_idc,                             "chroma_format_idc");
        Get_uvlc(bit_depth_idc,                                 "bit_depth_idc");
        Monochrome = (chroma_format_idc == 1);
    Element_End0();
    int8u max_tlayer_id, max_mlayer_id;
    if (single_picture_header_flag) {
        // decoder_model_info_present_flag = 0
        max_tlayer_id = 0;
        max_mlayer_id = 0;
        // seq_max_mlayer_cnt = 1
    }
    else {
        TESTELSE_SB_SKIP(                                       "max_display_model_info_present_flag");
            Skip_S1(4,                                          "max_initial_display_delay_minus_1");
        TESTELSE_SB_ELSE(                                       "max_display_model_info_present_flag");
            // max_initial_display_delay_minus_1 = BUFFER_POOL_MAX_SIZE - 1
        TESTELSE_SB_END();
        TEST_SB_SKIP(                                           "decoder_model_info_present_flag");
            Element_Begin1("decoder_model_info");
            Skip_S4(32,                                         "num_units_in_decoding_tick");
            Element_End0();
            TEST_SB_SKIP(                                       "max_display_model_info_present_flag");
                Element_Begin1("operating_parameters_info");
                Skip_uvlc(                                      "decoder_buffer_delay");
                Skip_uvlc(                                      "encoder_buffer_delay");
                Skip_SB(                                        "low_delay_mode_flag");
                Element_End0();
            TEST_SB_END();
        TEST_SB_END();
        Get_S1(2, max_tlayer_id,                                "max_tlayer_id");
        Get_S1(3, max_mlayer_id,                                "max_mlayer_id");
        if (max_mlayer_id > 0) {
            Skip_S4(std::ceil(std::log2(max_mlayer_id + 1)),    "seq_max_mlayer_cnt");
        }
    }
    if (max_tlayer_id > 0) {
        TEST_SB_SKIP(                                           "tlayer_dependency_present_flag");
            for (int currLayer = 1; currLayer <= max_tlayer_id; ++currLayer) {
                for (int refLayer = currLayer; refLayer >= 0; --refLayer) {
                    Skip_SB(                                    "tlayer_dependency_map");
                }
            }
        TEST_SB_END();
    }
    if (max_mlayer_id > 0) {
        TEST_SB_SKIP(                                           "mlayer_dependency_present_flag ");
            for (int currLayer = 1; currLayer <= max_mlayer_id; ++currLayer) {
                for (int refLayer = currLayer; refLayer >= 0; --refLayer) {
                    Skip_SB(                                    "mlayer_dependency_map");
                }
            }
        TEST_SB_END();
    }
    sequence_partition_config();
    sequence_segment_config();
    sequence_intra_config();
    sequence_inter_config();
    sequence_scc_config();
    sequence_transform_quant_entropy_config();
    sequence_filter_config();
    TEST_SB_SKIP(                                               "seq_tile_info_present_flag ");
        Skip_SB(                                                "allow_tile_info_change");
    TEST_SB_END();
    bool film_grain_params_present;
    Get_SB(film_grain_params_present,                           "film_grain_params_present");

    FILLING_BEGIN();
    if (!sequence_header_Parsed)
    {
        if (IsSub)
            Accept();
        Fill(Stream_Video, 0, Video_Width, max_frame_width_minus_1 + 1);
        Fill(Stream_Video, 0, Video_Height, max_frame_height_minus_1 + 1);
        Fill(Stream_Video, 0, Video_BitDepth, bit_depth_idc == 0 ? 10 : (bit_depth_idc == 1 ? 8 : 12));
        Fill(Stream_Video, 0, Video_ColorSpace, chroma_format_idc == 1 ? "Y" : "YUV");
        if (Retrieve(Stream_Video, 0, Video_ColorSpace) == __T("YUV"))
        {
            Fill(Stream_Video, 0, Video_ChromaSubsampling, chroma_format_idc == 0 ? "4:2:0" : chroma_format_idc == 2 ? "4:4:4" : chroma_format_idc == 3 ? "4:2:2" : "");
        }
        if (film_grain_params_present)
            Fill(Stream_Video, 0, Video_Format_Settings, "Film Grain Synthesis");

        sequence_header_Parsed = true;
    }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Av2::sequence_partition_config() {
    Element_Begin1("sequence_partition_config");
    TESTELSE_SB_SKIP(                                           "use_256x256_superblock");
    TESTELSE_SB_ELSE(                                           "use_256x256_superblock");
        Skip_SB(                                                "use_128x128_superblock");
    TESTELSE_SB_END();
    bool enable_sdp;
    if (Monochrome) {
        enable_sdp = 0;
    }
    else {
        Get_SB(enable_sdp,                                      "enable_sdp");
    }
    if (enable_sdp && !single_picture_header_flag) {
        Skip_SB(                                                "enable_extended_sdp");
    }
    else {
        // enable_extended_sdp = 0
    }
    TESTELSE_SB_SKIP(                                           "enable_ext_partitions");
        Skip_SB(                                                "enable_uneven_4way_partitions");
    TESTELSE_SB_ELSE(                                           "enable_ext_partitions");
        // enable_uneven_4way_partitions = 0
    TESTELSE_SB_END();
    TESTELSE_SB_SKIP(                                           "reduce_pb_aspect_ratio");
        Skip_SB(                                                "max_pb_aspect_ratio_log2_minus1");
        // MaxPbAspectRatio = 1 << (max_pb_aspect_ratio_log2_minus1 + 1)
    TESTELSE_SB_ELSE(                                           "reduce_pb_aspect_ratio");
        // MaxPbAspectRatio = 8
    TESTELSE_SB_END();
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Av2::sequence_segment_config() {
    Element_Begin1("sequence_segment_config");
    Skip_SB(                                                    "enable_ext_seg");
    // MaxSegments = enable_ext_seg ? 16 : 8
    TEST_SB_SKIP(                                               "seq_seg_info_present_flag");
        Skip_SB(                                                "seq_allow_seg_info_change");
        // ( SeqFeatureEnabled, SeqFeatureData ) = seg_info( MaxSegments )
    TEST_SB_END();
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Av2::sequence_intra_config() {
    Element_Begin1("sequence_intra_config");
    Skip_SB(                                                    "enable_dip");
    Skip_SB(                                                    "enable_intra_edge_filter");
    Skip_SB(                                                    "enable_mrls");
    Skip_SB(                                                    "enable_cfl_intra");
    if (Monochrome) {
        // cfl_ds_filter_index = 0
    }
    else {
        Skip_S1(2,                                              "cfl_ds_filter_index");
    }
    Skip_SB(                                                    "enable_mhccp");
    Skip_SB(                                                    "enable_ibp");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Av2::sequence_inter_config() {
    Element_Begin1("sequence_inter_config");
    Skip_SB(                                                    "enable_refmvbank");
    TESTELSE_SB_SKIP(                                           "disable_drl_reorder");
        // DrlReorder = DRL_REORDER_DISABLED
    TESTELSE_SB_ELSE(                                           "disable_drl_reorder");
        Skip_SB(                                                "constrain_drl_reorder");
        // DrlReorder = constrain_drl_reorder ? DRL_REORDER_CONSTRAINT: DRL_REORDER_ALWAYS
    TESTELSE_SB_END();



    Element_End0();
}

//---------------------------------------------------------------------------
void File_Av2::sequence_scc_config() {
    Element_Begin1("sequence_scc_config");

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Av2::sequence_transform_quant_entropy_config() {
    Element_Begin1("sequence_transform_quant_entropy_config");

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Av2::sequence_filter_config() {
    Element_Begin1("sequence_filter_config");

    Element_End0();
}

//---------------------------------------------------------------------------
void File_Av2::temporal_delimiter_obu()
{
    SeenFrameHeader = false;

    FILLING_BEGIN_PRECISE();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Av2::multi_stream_decoder_operation_obu()
{
    Element_Name("multi_stream_decoder_operation_obu");
}

//---------------------------------------------------------------------------
void File_Av2::multi_frame_header_obu()
{
    Element_Name("multi_frame_header_obu");
}

//---------------------------------------------------------------------------
void File_Av2::frame_header(bool isFirst)
{
    Element_Name("frame_header");
}

//---------------------------------------------------------------------------
void File_Av2::metadata_short_obu()
{
    Element_Name("metadata_short_obu");
}

//---------------------------------------------------------------------------
void File_Av2::metadata_group_obu()
{
    Element_Name("metadata_group_obu");
}

//---------------------------------------------------------------------------
void File_Av2::tile_group_obu(int64u obuPayloadSize)
{
    Element_Name("tile_group_obu");
}

//---------------------------------------------------------------------------
void File_Av2::layer_config_record_obu()
{
    Element_Name("layer_config_record_obu");
}

//---------------------------------------------------------------------------
void File_Av2::atlas_segment_info_obu()
{
    Element_Name("atlas_segment_info_obu");
}

//---------------------------------------------------------------------------
void File_Av2::operating_point_set_obu()
{
    Element_Name("operating_point_set_obu");
}

//---------------------------------------------------------------------------
void File_Av2::buffer_removal_timing_obu()
{
    Element_Name("buffer_removal_timing_obu");
}

//---------------------------------------------------------------------------
void File_Av2::quantizer_matrix_obu()
{
    Element_Name("quantizer_matrix_obu");
}

//---------------------------------------------------------------------------
void File_Av2::film_grain_obu()
{
    Element_Name("film_grain_obu");
}

//---------------------------------------------------------------------------
void File_Av2::content_interpretation_obu()
{
    Element_Name("content_interpretation_obu");
}

//---------------------------------------------------------------------------
void File_Av2::padding_obu()
{
    Element_Name("padding_obu");
    Skip_BS(Element_Size * 8,                                   "Padding");
}

//---------------------------------------------------------------------------
void File_Av2::reserved_obu()
{
    Element_Name("reserved_obu");
    Skip_BS(Element_Size * 8,                                   "Data");
}

//---------------------------------------------------------------------------
void File_Av2::obu_extension_data(int64u nbBits)
{
    Element_Begin1("obu_extension_data");
    Skip_BS(nbBits,                                             "Data");
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Av2::trailing_bits(int64u nbBits)
{
    Element_Begin1("trailing_bits");
    Mark_1(); // trailing_one_bit
    --nbBits;
    while (nbBits > 0) {
        Mark_0(); // trailing_zero_bit
        --nbBits;
    }
}

//***************************************************************************
// Helpers
//***************************************************************************

void File_Av2::Get_leb128(int64u& Info, const char* Name)
{
    Info=0;
    for (int8u i=0; i<8; i++)
    {
        if (Element_Offset>=Element_Size)
            break; // End of stream reached, not normal
        int8u leb128_byte=BigEndian2int8u(Buffer+Buffer_Offset+(size_t)Element_Offset);
        Element_Offset++;
        Info|=(static_cast<int64u>(leb128_byte&0x7f)<<(i*7));
        if (!(leb128_byte&0x80))
        {
            #if MEDIAINFO_TRACE
                if (Trace_Activated)
                {
                    Param(Name, Info, i+1);
                    Param_Info(__T("(")+Ztring::ToZtring(i+1)+__T(" bytes)"));
                }
            #endif //MEDIAINFO_TRACE
            return;
        }
    }
    Trusted_IsNot("Size is wrong");
    Info=0;
}

void File_Av2::Get_uvlc(int64u& Info, const char* Name)
{
    #define INTEGRITY(TOVALIDATE, ERRORTEXT, OFFSET) \
    if (!(TOVALIDATE)) \
    { \
        Trusted_IsNot(ERRORTEXT); \
        return; \
    } \

    auto remain_begin = BS->Remain();
    int64u leadingZeros = 0;
    while (true) {
        INTEGRITY(BS->Remain() >= 1, "Size is wrong", BS->Offset_Get())
        if (BS->GetB())
            break;
        ++leadingZeros;
    }
    if (leadingZeros >= 32) {
        Info = (1ULL << 32) - 1;
    }
    else {
        INTEGRITY(BS->Remain() >= leadingZeros, "Size is wrong", BS->Offset_Get())
        int64u value = BS->Get4(leadingZeros);
        Info = value + (1ULL << leadingZeros) - 1;
    }
    #if MEDIAINFO_TRACE
        if (Trace_Activated)
        {
            auto Bits = remain_begin - BS->Remain();
            Param(Name, Info, Bits);
            Param_Info(__T("(") + Ztring::ToZtring(Bits) + __T(" bits)"));
        }
    #endif //MEDIAINFO_TRACE
}

void File_Av2::Skip_uvlc(const char* Name)
{
    int64u temp;
    Get_uvlc(temp, Name);
}

//---------------------------------------------------------------------------
} //NameSpace

#endif //MEDIAINFO_AV2_YES
