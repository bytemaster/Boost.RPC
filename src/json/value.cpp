#include <mace/rpc/json/value.hpp>
#include <mace/rpc/json/error.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/support_utree.hpp>
#include <iomanip>

BOOST_FUSION_ADAPT_STRUCT( mace::rpc::json::detail::key_val, (std::string,key)(mace::rpc::json::value,val) )
BOOST_FUSION_ADAPT_STRUCT( mace::rpc::json::object, 
                            (std::vector<mace::rpc::json::detail::key_val>, keys) );
BOOST_FUSION_ADAPT_STRUCT( mace::rpc::json::value, 
                            (mace::rpc::json::value_variant, val) );
BOOST_FUSION_ADAPT_STRUCT( mace::rpc::json::array,  (std::vector<mace::rpc::json::value>, vals) );


namespace mace { namespace rpc { namespace json {

    namespace detail {
    using namespace boost::spirit;
    
    template<typename Iterator>
    struct json_grammar : qi::grammar<Iterator,json::value(),ascii::space_type> {
        json_grammar() : json_grammar::base_type(json_r, "json") 
        {
            using qi::lit;
            using qi::lexeme;
            using ascii::char_;
            using ascii::string;
            using namespace qi::labels;

            unesc_char.add( "\\r", '\r')("\\\\",'\\')("\\n",'\n')("\\t",'\t')("\\\"",'\"');
            unesc_str = lexeme['"' >> *(unesc_char | (char_ -'\"') ) >> '"'];
            null_s.add("null",null_t());

            null_r    %= null_s;
            json_r    %=  (double_ | bool_ | unesc_str | object_r | array_r | null_r |  empty_array_r | empty_object_r);
            empty_array_r %= ('[' >> eps >> ']');
            empty_object_r %= ('{' >> eps >> '}');
            array_r   %= ('[' >>(json_r % ',') >> ']');
            keyval_r  %= unesc_str >> ':' >> json_r;
            object_r  %= '{' >> keyval_r % ',' >> '}';
            start_tag %= json_r;
        }

       qi::symbols<char const, char const>  unesc_char;
       qi::symbols<char const, null_t>         null_s;

       qi::rule<Iterator, json::array(),    ascii::space_type> empty_array_r;
       qi::rule<Iterator, json::value(),    ascii::space_type> start_tag;
       qi::rule<Iterator, null_t(),         ascii::space_type> null_r;
       qi::rule<Iterator, std::string(),    ascii::space_type> unesc_str;
       qi::rule<Iterator, json::value(),    ascii::space_type> json_r;
       qi::rule<Iterator, json::array(),    ascii::space_type> array_r;
       qi::rule<Iterator, json::object(),   ascii::space_type> object_r;
       qi::rule<Iterator, json::object(),   ascii::space_type> empty_object_r;
       qi::rule<Iterator, json::detail::key_val(),  ascii::space_type> keyval_r;
    };
    

   /* 
    using boost::spirit::utree;
    template<typename Iterator>
    struct json_grammar : qi::grammar<Iterator,utree(),ascii::space_type> {
        json_grammar() : json_grammar::base_type(json_r, "json") 
        {
            using boost::spirit::utree;
            using qi::lit;
            using qi::lexeme;
            using ascii::char_;
            using ascii::string;
            using namespace qi::labels;

            unesc_char.add( "\\r", '\r')("\\\\",'\\')("\\n",'\n')("\\t",'\t')("\\\"",'\"');
            unesc_str = lexeme['"' >> *(unesc_char | (char_ -'\"') ) >> '"'];
            null_s.add("null",utree(utree::nil_type));

            null_r    %= null_s;
            json_r    %=  (double_ | bool_ | unesc_str | object_r | array_r | null_r);
            array_r   %= '[' >> json_r % ',' >> ']';
            keyval_r  %= unesc_str >> ':' >> json_r;
            object_r  %= '{' >> keyval_r % ',' >> '}';
        }

       qi::symbols<char const, char const>         unesc_char;
       qi::symbols<char const, utree::nil_type>    null_s;
       typedef std::pair<utree,utree>              key_val;
       typedef std::vector<key_val>                object;
       typedef std::vector<utree>                  array;

       qi::rule<Iterator, utree::nil_type(),   ascii::space_type> null_r;
       qi::rule<Iterator, utree(),             ascii::space_type> unesc_str;
       qi::rule<Iterator, utree(),             ascii::space_type> json_r;
       qi::rule<Iterator, array(),             ascii::space_type> array_r;
       qi::rule<Iterator, object(),            ascii::space_type> object_r;
       qi::rule<Iterator, key_val(),           ascii::space_type> keyval_r;
    };
    */
    

 } // namespace detail
    



struct json_value_printer : boost::static_visitor<> {
    json_value_printer( std::ostream& _os ):os(_os){}
    void operator()(std::string const& text) const {
        os << "\"" << text << '"';
    }
    void operator()(bool const& text) const {
        os << (text ? "true" : "false");
    }
    void operator()(double const& text) const {
       // os.flags( std::ios::fixed );
        os.precision(8);
        os << text;
    }
    void operator()(json::null_t const& obj) const {
        os << "null";
    }
    void operator()(json::object const& obj) const {
        os << "{";
        for( uint32_t i = 0; i < obj.keys.size(); ++i ) {
            os<<'"'<<obj.keys[i].key<<'"' << ":";
            boost::apply_visitor( json_value_printer(os), obj.keys[i].val.val);
            if( i != obj.keys.size()-1 )  
                os <<",";
        }
        os << "}";
    }
    void operator()(json::array const& text) const {
        os << "[";
        for( uint32_t i = 0; i < text.vals.size(); ++i ) {
            boost::apply_visitor( json_value_printer(os), text.vals[i].val);
            if( i != text.vals.size()-1 )  
                os <<",";
        }
        os << "]";
    }
    std::ostream& os;
};


struct json_value_pretty_printer : boost::static_visitor<>
{
    json_value_pretty_printer( std::ostream& _os, int d= 1 ):os(_os),depth(d){}
    void operator()(std::string const& text) const {
        os << "\"" << text << '"';
    }
    void operator()(bool const& text) const {
        os << (text ? "true" : "false");
    }
    void operator()(double const& text) const {
        //os.flags( std::ios::fixed );
        os.precision(8);
        os << text;
    }
    void operator()(json::null_t const& obj) const {
        os << "null";
    }
    void operator()(json::object const& obj) const {
        if( obj.keys.size() == 0 ){ os <<"{}"; return; }
        os << "{\n";
        for( uint32_t i = 0; i < obj.keys.size(); ++i ) {
            os<<std::setw(depth*4)<<""<<'"'<<obj.keys[i].key<<'"' << ":";
            boost::apply_visitor( json_value_pretty_printer(os,depth+1), obj.keys[i].val.val);
            if( i != obj.keys.size()-1 )  
                os <<",";
            os<<"\n";
        }
        os << std::setw(depth*4-4)<<"}";
    }
    void operator()(json::array const& text) const {
        if( text.vals.size() == 0 ) { os <<"[]"; return; }

        os << "[\n";
        for( uint32_t i = 0; i < text.vals.size(); ++i ) {
            os << std::setw(depth*4)<<" ";
            boost::apply_visitor( json_value_pretty_printer(os,depth+1), text.vals[i].val);
            if( i != text.vals.size()-1 )  
                os <<",";
            os<<"\n";
        }
        os << std::setw(depth*4-4)<<"]";
    }
    int           depth;
    std::ostream& os;
};




template<typename Iterator, typename Expr, typename Skip>
void parse( Iterator& itr, const Iterator& e, const Expr& g, const Skip& s, json::value& v ) {
  if( !phrase_parse(itr,e,g,s,v) ) {
    BOOST_THROW_EXCEPTION( json::parse_error() ); 
  }
}
void from_string( const std::string& j, json::value& v ) {
   std::string::const_iterator iter = j.begin();
   std::string::const_iterator end = j.end();
   typedef json::detail::json_grammar<std::string::const_iterator> gram;
   static gram g;
   json::parse( iter, end, g, boost::spirit::ascii::space, v );
}

void to_string( const value& v, std::string& j, bool pretty  ) {
  std::stringstream ss(j);
  write( ss, v, pretty );
  j = ss.str();
}


std::ostream& write(std::ostream& os, const json::value& v, bool pretty  ) {
    if( !pretty )
        boost::apply_visitor( json_value_printer(os), v.val );
    else
        boost::apply_visitor( json_value_pretty_printer(os), v.val );
    return os;
}


value::value():val(null_t()){}
value::value( const value& c ):val(c.val){}
value::value( const std::string& v ):val( v ){}
value::value( double v ):val( v ){}
value::value( bool v ):val( v ){}
value::value( const object& v ):val( v ){}
value::value( const array& v ):val( v ){}

bool value::is_object()const  { return 0 != boost::get<object>(&val);      }
bool value::is_array()const   { return 0 != boost::get<array>(&val);       }
bool value::is_string()const  { return 0 != boost::get<std::string>(&val); }
bool value::is_null()const    { return 0 != boost::get<null_t>(&val);      }
bool value::is_bool()const    { return 0 != boost::get<bool>(&val);        }
bool value::is_real()const    { return 0 != boost::get<double>(&val);      }

bool value::operator == ( const null_t& nt )const { return 0 != boost::get<null_t>(&val); }
value::operator int()const                        { return boost::get<double>(val);      }
value::operator int64_t()const                    { return boost::get<double>(val);      }
value::operator uint64_t()const                   { return boost::get<double>(val);      }
value::operator uint32_t()const                   { return boost::get<double>(val);      }
value::operator uint16_t()const                   { return boost::get<double>(val);      }
value::operator uint8_t()const                    { return boost::get<double>(val);      }
value::operator int16_t()const                    { return boost::get<double>(val);      }
value::operator int8_t()const                     { return boost::get<double>(val);      }
value::operator double()const                     { return boost::get<double>(val);      }
value::operator float()const                      { return boost::get<double>(val);      }
value::operator bool()const                       { return boost::get<bool>(val);        }
value::operator std::string()const { 
   return boost::apply_visitor( detail::cast_visitor<std::string>(), val );
}
value::operator const json::object&()const        { return boost::get<json::object>(val); }
value::operator json::object&()                   { return boost::get<json::object>(val); }
value::operator const json::array&()const         { return boost::get<json::array>(val); }
value::operator json::array&()                    { return boost::get<json::array>(val); }


value& value::operator = ( null_t v )             { val = v; }
value& value::operator = ( bool v )               { val = v; }
value& value::operator = ( double v )             { val = v; }
value& value::operator = ( int64_t v )            { val = (double)v; }
value& value::operator = ( uint64_t v )           { val = (double)v; }
value& value::operator = ( int32_t v )            { val = (double)v; }
value& value::operator = ( uint32_t v )           { val = (double)v; }
value& value::operator = ( int16_t v )            { val = (double)v; }
value& value::operator = ( uint16_t v )           { val = (double)v; }
value& value::operator = ( int8_t v )             { val = (double)v; }
value& value::operator = ( uint8_t v )            { val = (double)v; }
value& value::operator = ( const std::string& v ) { val = v; }
value& value::operator = ( const json::object& v ){ val = v; }
value& value::operator = ( const json::array& v ) { val = v; }


 value&        value::operator[]( const char* index ) {
   return boost::apply_visitor( detail::key_visitor<value&>(std::string(index),*this), val );
 }

 const value&  value::operator[]( const char* index )const {
   return boost::apply_visitor( detail::const_key_visitor<const value&>(index,*this), val );
 }

 value&        value::operator[]( const std::string& index ) {
   return boost::apply_visitor( detail::key_visitor<value&>(index,*this), val );
 }
 const value&  value::operator[]( const std::string& index )const {
   return boost::apply_visitor( detail::const_key_visitor<const value&>(index,*this), val );
 }
 value&        value::operator[]( uint32_t index ) {
   return boost::apply_visitor( detail::index_visitor<value&>(index), val );
 }
 const value&  value::operator[]( uint32_t index )const {
   return boost::apply_visitor( detail::index_visitor<const value&>(index), val );
 }

value&       object::operator[]( const std::string& index ) {
    std::vector<detail::key_val>::iterator itr = keys.begin();
    std::vector<detail::key_val>::iterator end = keys.end();
    while( itr != end ) {
      if( itr->key == index ) 
          return itr->val;
      ++itr;
    }
    keys.push_back(detail::key_val(index));
    return keys.back().val;
}
const value& object::operator[]( const std::string& index )const {
    std::vector<detail::key_val>::const_iterator itr = keys.begin();
    std::vector<detail::key_val>::const_iterator end = keys.end();
    while( itr != end ) {
      if( itr->key == index ) 
        return itr->val;
      ++itr;
    }
    BOOST_THROW_EXCEPTION( std::out_of_range( "unknown key '"+index+"'" ) );
}

bool value::contains( const std::string& key )const {
  const object* obj = boost::get<object>(&val); //*this;
  return obj && obj->contains(key);
}
bool object::contains( const std::string& key )const {
   std::vector<detail::key_val>::const_iterator itr = keys.begin();
   std::vector<detail::key_val>::const_iterator end = keys.end();
   while( itr != end ) {
     if( itr->key == key ) 
       return true;
     ++itr;
   }
   return false;
}

value& operator<<(value& jv, const value& v ) {
  return jv = v;
}
const value& operator>>(const value& jv, value& v ) {
  v = jv;
  return jv;
}

} } } // namespace mace::rpc::json

