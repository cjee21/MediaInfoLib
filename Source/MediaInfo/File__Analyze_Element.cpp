#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/File__Analyze_Element.h"
#include <iostream>
#include <iomanip>

namespace MediaInfoLib
{
//***************************************************************************
// Element_Node_Data
//***************************************************************************

//---------------------------------------------------------------------------
element_details::Element_Node_Data& element_details::Element_Node_Data::operator=(const Element_Node_Data& v)
{
    if (this == &v)
        return *this;

    is_empty = v.is_empty;
    type = v.type;
    if (!is_empty)
    {
        switch (type)
        {
          case element_details::Element_Node_Data::ELEMENT_NODE_STR:
          {
              size_t len = strlen(v.val.Str);
              val.Str = new char[len];
              std::memcpy(val.Str, v.val.Str, len);
              val.Str[len] = '\0';
              break;
          }
          case element_details::Element_Node_Data::ELEMENT_NODE_INT128U:
              val.i128u = new int128u;
              *val.i128u = *v.val.i128u;
              break;
          default:
              val = v.val;
              break;
        }
    }
    return *this;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(const Ztring& v)
{
    std::string tmp = v.To_UTF8();
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_STR;
    val.Str = new char[tmp.length() + 1];
    std::memcpy(val.Str, tmp.c_str(), tmp.length());
    val.Str[tmp.length()] = '\0';
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(const std::string& v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_STR;
    val.Str = new char[v.length() + 1];
    std::memcpy(val.Str, v.c_str(), v.length());
    val.Str[v.length()] = '\0';
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(bool v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_BOOL;
    val.b = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(int8u v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_INT8U;
    val.i8u = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(int8s v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_INT8S;
    val.i8s = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(int16u v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_INT16U;
    val.i16u = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(int16s v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_INT16S;
    val.i16s = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(int32u v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_INT32U;
    val.i32u = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(int32s v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_INT32S;
    val.i32s = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(int64u v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_INT64U;
    val.i64u = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(int64s v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_INT64S;
    val.i64s = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(int128u v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_INT128U;
    val.i128u = new int128u;
    *val.i128u = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(float32 v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_FLOAT32;
    val.f32 = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(float64 v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_FLOAT64;
    val.f64 = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::operator=(float80 v)
{
    is_empty = false;
    type = element_details::Element_Node_Data::ELEMENT_NODE_FLOAT80;
    val.f80 = v;
}

//---------------------------------------------------------------------------
void element_details::Element_Node_Data::clear()
{
    if (is_empty)
        return;

    switch (type)
    {
      case element_details::Element_Node_Data::ELEMENT_NODE_STR:
          if (!is_empty)
              delete [] val.Str;
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_INT128U:
          if (!is_empty)
              delete val.i128u;
          break;
      default:
          break;
    }
    is_empty = true;
}

//---------------------------------------------------------------------------
bool element_details::Element_Node_Data::empty()
{
    return is_empty;
}

//---------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const element_details::Element_Node_Data& v)
{
    switch (v.type)
    {
      case element_details::Element_Node_Data::ELEMENT_NODE_STR:
          os << v.val.Str;
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_BOOL:
          if (v.val.b)
              os << "Yes";
          else
              os << "No";
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_INT8U:
          os << Ztring::ToZtring(v.val.i8u).To_UTF8();
          if (v.format_out == element_details::Element_Node_Data::Format_Tree)
              os << " (0x" << Ztring().From_CC1(v.val.i8u).To_UTF8() << ")";
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_INT8S:
          os << Ztring::ToZtring(v.val.i8s).To_UTF8();
          if (v.format_out == element_details::Element_Node_Data::Format_Tree)
              os << " (0x" << Ztring().From_CC1(v.val.i8s).To_UTF8() << ")";
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_INT16U:
          os << Ztring::ToZtring(v.val.i16u).To_UTF8();
          if (v.format_out == element_details::Element_Node_Data::Format_Tree)
              os << " (0x" << Ztring().From_CC2(v.val.i16u).To_UTF8() << ")";
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_INT16S:
          os << Ztring::ToZtring(v.val.i16s).To_UTF8();
          if (v.format_out == element_details::Element_Node_Data::Format_Tree)
              os << " (0x" << Ztring().From_CC2(v.val.i16s).To_UTF8() << ")";
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_INT32U:
          os << Ztring::ToZtring(v.val.i32u).To_UTF8();
          if (v.format_out == element_details::Element_Node_Data::Format_Tree)
              os << " (0x" << Ztring::ToZtring(v.val.i32u, 16).To_UTF8() << ")";
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_INT32S:
          os << Ztring::ToZtring(v.val.i32s).To_UTF8();
          if (v.format_out == element_details::Element_Node_Data::Format_Tree)
              os << " (0x" << Ztring::ToZtring(v.val.i32s, 16).To_UTF8() << ")";
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_INT64U:
          os << Ztring::ToZtring(v.val.i64u).To_UTF8();
          if (v.format_out == element_details::Element_Node_Data::Format_Tree)
              os << " (0x" << Ztring::ToZtring(v.val.i64u, 16).To_UTF8() << ")";
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_INT64S:
          os << Ztring::ToZtring(v.val.i64s).To_UTF8();
          if (v.format_out == element_details::Element_Node_Data::Format_Tree)
              os << " (0x" << Ztring::ToZtring(v.val.i64s, 16).To_UTF8() << ")";
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_INT128U:
          os << Ztring::ToZtring(*v.val.i128u).To_UTF8();
          if (v.format_out == element_details::Element_Node_Data::Format_Tree)
              os << " (0x" << Ztring::ToZtring(*v.val.i128u, 16).To_UTF8() << ")";
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_FLOAT32:
          os << Ztring::ToZtring(v.val.f32, v.AfterComma).To_UTF8();
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_FLOAT64:
          os << Ztring::ToZtring(v.val.f64, v.AfterComma).To_UTF8();
          break;
      case element_details::Element_Node_Data::ELEMENT_NODE_FLOAT80:
          os << Ztring::ToZtring(v.val.f80, v.AfterComma).To_UTF8();
          break;
    }
    return os;
}


//***************************************************************************
// Element_Node
//***************************************************************************
//---------------------------------------------------------------------------
element_details::Element_Node::Element_Node()
: Pos(0), Size(0), Header_Size(0),
  Current_Child(-1), NoShow(false), OwnChildren(true), IsCat(false)
{
}
 
//---------------------------------------------------------------------------
element_details::Element_Node::Element_Node(const Element_Node& node)
{
    if (this == &node)
        return;

    Pos = node.Pos;
    Size = node.Size;
    Header_Size = node.Header_Size;
    Name = node.Name;
    Value = node.Value;
    Infos = node.Infos;
    Children = node.Children;
    Parser = node.Parser;
    Current_Child = node.Current_Child;
    NoShow = node.NoShow;
    OwnChildren = node.OwnChildren;
    IsCat = node.IsCat;
}

//---------------------------------------------------------------------------
element_details::Element_Node::~Element_Node()
{
    if (!OwnChildren)
        return;

    for (size_t i = 0; i < Children.size(); ++i)
        delete Children[i];
    Children.clear();
}

//---------------------------------------------------------------------------
void element_details::Element_Node::Init()
{
    Pos = 0;
    Size = 0;
    Header_Size = 0;
    Name.clear();
    Value.clear();
    Infos.clear();
    if (Children.size() && OwnChildren)
        for (size_t i = 0; i < Children.size(); ++i)
            delete Children[i];
    Children.clear();
    Parser.clear();
    Current_Child = -1;
    NoShow = false;
    OwnChildren = true;
    IsCat = false;
}

//---------------------------------------------------------------------------
int element_details::Element_Node::Print_Xml(std::string& Str, size_t level)
{
    std::stringstream ss;
    std::string spaces;

    if (IsCat || !Name.length())
        goto print_children;

    spaces.resize(level, ' ');
    ss << spaces;

    if (Value.empty())
        ss << "<block";
    else
        ss << "<data";

    ss << " offset=\"" << Pos << "\" name=\"" << Name << "\"";

    if (!Parser.empty())
        ss << " parser=\"" << Parser << "\"";

    for (size_t i = 0; i < Infos.size(); ++i)
    {
        ss << " info";
        if (i)
            ss << i;
        ss << "=\"" << Infos[i] << "\"";
    }

    if (!Value.empty())
    {
        Value.Set_Output_Format(Element_Node_Data::Format_Xml);
        ss << ">" << Value << "</data>";
    }
    else
        ss << " size=\"" << Size << "\">";

    ss << std::endl;

    Str += ss.str();

print_children:
    for (size_t i = 0; i < Children.size(); ++i)
        Children[i]->Print_Xml(Str, level + 4);

    if (!IsCat && Name.length())
    {
        //end tag
        if (Value.empty())
        {
            //block
            ss.str(std::string());
            ss << spaces << "</block>" << std::endl;
            Str += ss.str();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------
int element_details::Element_Node::Print_Tree_Cat(std::string& Str, size_t level)
{
    std::stringstream ss;

    std::string offset;
    ss << std::setfill('0') << std::setw(8) << std::hex << std::uppercase << Pos << std::nouppercase << std::dec;
    offset = ss.str();

    std::string spaces;
    spaces.resize(level, ' ');

    std::string ToShow;
    ToShow += "---   ";
    ToShow += Name;
    ToShow += "   ---";

    std::string minuses;
    minuses.resize(ToShow.size(), '-');

    ss << spaces << minuses << std::endl;
    ss << offset << spaces << ToShow << std::endl;
    ss << offset << spaces << minuses << std::endl;

    Str += ss.str();
    return 0;
}

//---------------------------------------------------------------------------
int element_details::Element_Node::Print_Tree(std::string& Str, size_t level)
{
    std::stringstream ss;
    std::string spaces;

    if (IsCat)
        return Print_Tree_Cat(Str, level);
    else if (!Name.length())
        goto print_children;

    ss << std::setfill('0') << std::setw(8) << std::hex << std::uppercase << Pos << std::nouppercase << std::dec;
    spaces.resize(level, ' ');
    ss << spaces;
    ss << Name;

#define NB_SPACES 40
    if (!Value.empty())
    {
        ss << ":";
        spaces.clear();
        int nb_free = NB_SPACES - ss.str().length();
        spaces.resize(nb_free > 0 ? nb_free : 1, ' ');
        Value.Set_Output_Format(Element_Node_Data::Format_Tree);
        ss << spaces << Value;
    }
#undef NB_SPACES

    if (!Parser.empty())
        ss << " - parser=\"" << Parser << "\"";

    for (size_t i = 0; i < Infos.size(); ++i)
        ss << " - " << Infos[i];

    if (Value.empty())
        ss << " (" << Size << " bytes)";

    ss << std::endl;

    Str += ss.str();

print_children:
    for (size_t i = 0; i < Children.size(); ++i)
        Children[i]->Print_Tree(Str, level + 1);

    return 0;
}

//---------------------------------------------------------------------------
int element_details::Element_Node::Print(MediaInfo_Config::trace_Format Format, std::string& Str)
{
    switch (Format)
    {
        case MediaInfo_Config::Trace_Format_Tree        : return Print_Tree(Str, 0);
        case MediaInfo_Config::Trace_Format_CSV         : break;
        case MediaInfo_Config::Trace_Format_XML         : return Print_Xml(Str, 0);
        default                                         : break;
    }
    return -1;
}

//---------------------------------------------------------------------------
void element_details::Element_Node::Add_Child(Element_Node* node)
{
    Element_Node *new_node = new Element_Node(*node);
    node->OwnChildren = false;
    Children.push_back(new_node);
}

}
