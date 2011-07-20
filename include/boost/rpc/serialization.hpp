#ifndef _BOOST_RPC_SERIALIZATION_HPP_
#define _BOOST_RPC_SERIALIZATION_HPP_
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/container/vector/vector_fwd.hpp>
#include <boost/fusion/include/vector_fwd.hpp>
#include <boost/fusion/container/generation/make_vector.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/sequence/comparison/equal_to.hpp>
#include <boost/fusion/include/equal_to.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/serialization.hpp>

namespace boost { namespace rpc {
    namespace serialization {
        template<typename Archive>
        struct item_serializer 
        {
            item_serializer(Archive& ar):ar(ar) {}

            template<typename T>
            void operator()(const T& o) const {
                ar << o;
            }
            Archive& ar;
        };

        template<typename Archive, typename V>
        Archive& serialize_fusion_vector(Archive& ar, const V& v) 
        {
            boost::fusion::for_each(v, item_serializer<Archive>(ar));
            return ar;
        }

        template<typename Archive>
        struct item_deserializer 
        {
            item_deserializer(Archive& ar):ar(ar) {}

            template<typename T>
            void operator()(T& o) const {
                ar >> o;
            }
            Archive& ar;
        };

        template<typename Archive, typename V>
        Archive& deserialize_fusion_vector(Archive& ar, V& v) 
        {
            boost::fusion::for_each(v, item_deserializer<Archive>(ar));
            return ar;
        }

    }
} }

#endif // _BOOST_RPC_SERIALIZATION_HPP_
