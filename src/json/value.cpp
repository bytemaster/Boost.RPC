#include <boost/rpc/json/value.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/support_utree.hpp>

BOOST_FUSION_ADAPT_STRUCT( boost::rpc::json::detail::key_val, (std::string,key)(boost::rpc::json::value,val) )
BOOST_FUSION_ADAPT_STRUCT( boost::rpc::json::object, 
                            (std::vector<boost::rpc::json::detail::key_val>, keys) );
BOOST_FUSION_ADAPT_STRUCT( boost::rpc::json::value, 
                            (boost::rpc::json::value_variant, val) );
BOOST_FUSION_ADAPT_STRUCT( boost::rpc::json::array,  (std::vector<boost::rpc::json::value>, vals) );


namespace boost { namespace rpc { namespace json {

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
            json_r    %=  (double_ | bool_ | unesc_str | object_r | array_r | null_r);
            array_r   %= '[' >> json_r % ',' >> ']';
            keyval_r  %= unesc_str >> ':' >> json_r;
            object_r  %= '{' >> keyval_r % ',' >> '}';
            start_tag %= json_r;
        }

       qi::symbols<char const, char const>  unesc_char;
       qi::symbols<char const, null_t>         null_s;

       qi::rule<Iterator, json::value(),    ascii::space_type> start_tag;
       qi::rule<Iterator, null_t(),         ascii::space_type> null_r;
       qi::rule<Iterator, std::string(),    ascii::space_type> unesc_str;
       qi::rule<Iterator, json::value(),    ascii::space_type> json_r;
       qi::rule<Iterator, json::array(),    ascii::space_type> array_r;
       qi::rule<Iterator, json::object(),   ascii::space_type> object_r;
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
    
 template<typename Iterator, typename Expr, typename Skip>
 void parse( Iterator& itr, const Iterator& e, const Expr& g, const Skip& s, rpc::json::value& v ) {
   phrase_parse(itr,e,g,s,v); 
 }
 void read( const std::string& j, rpc::json::value& v ) {
    std::string::const_iterator iter = j.begin();
    std::string::const_iterator end = j.end();
    typedef boost::rpc::json::detail::json_grammar<std::string::const_iterator> gram;
    static gram g;
    boost::rpc::json::parse( iter, end, g, boost::spirit::ascii::space, v );
//    phrase_parse( iter, end, g, boost::spirit::ascii::space, v );
 }



struct json_value_printer : boost::static_visitor<>
{
    json_value_printer( std::ostream& _os ):os(_os){}
    void operator()(std::string const& text) const {
        os << "\"" << text << '"';
    }
    void operator()(bool const& text) const {
        os << (text ? "true" : "false");
    }
    void operator()(double const& text) const {
        os.flags( std::ios::fixed );
        os << text;
    }
    void operator()(boost::rpc::json::null_t const& obj) const {
        os << "null";
    }
    void operator()(boost::rpc::json::object const& obj) const {
        os << "{";
        for( uint32_t i = 0; i < obj.keys.size(); ++i ) {
            os<<'"'<<obj.keys[i].key<<'"' << ":";
            boost::apply_visitor( json_value_printer(os), obj.keys[i].val.val);
            if( i != obj.keys.size()-1 )  
                os <<",";
        }
        os << "}";
    }
    void operator()(boost::rpc::json::array const& text) const {
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

std::ostream& write(std::ostream& os, const boost::rpc::json::value& v ) {
    boost::apply_visitor( json_value_printer(os), v.val );
    return os;
}

 value&        value::operator[]( const std::string& index ) {
   return boost::apply_visitor( detail::key_visitor<value&>(index), val );
 }
 const value&  value::operator[]( const std::string& index )const {
   return boost::apply_visitor( detail::key_visitor<const value&>(index), val );
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


} } }

