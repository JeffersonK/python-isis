#include "_propmap.hpp"
#include "common.hpp"

namespace isis
{
namespace python
{
namespace util
{
namespace PropertyMap
{

void _setPropertyAs( isis::util::PropertyMap &base, isis::python::util::types type, const std::string &key, boost::python::api::object value )
{
	static_cast<isis::util::ValueBase::Reference &>( base.propertyValue( key.c_str() ) ) =
		( *_internal::ConvertFromPython::convert( value ) ).copyByID( static_cast<unsigned short>( type ) );
}


void _setProperty( isis::util::PropertyMap &base, const std::string &key, boost::python::api::object value )
{
	base.propertyValue( key.c_str() ) = _internal::ConvertFromPython::convert( value );
}

api::object _getProperty( const isis::util::PropertyMap &base, const std::string &key )
{
	const isis::util::PropertyValue &value = base.propertyValue( key.c_str() );

	if( value.isEmpty() )
		return api::object();
	else
		return isis::util::Singletons::get<_internal::TypesMap, 10>()[value.getTypeID()]->convert( *value );
}

bool _hasProperty( const isis::util::PropertyMap &base, const std::string &key )
{
	return base.hasProperty( key.c_str() );
}

bool _hasBranch( const isis::util::PropertyMap &base, const std::string &key )
{
	return base.hasBranch( key.c_str() );
}

isis::util::PropertyMap _branch( const isis::util::PropertyMap &base, const std::string &key )
{
	return base.branch( key.c_str() );
}

void _join ( isis::util::PropertyMap &base, const isis::data::Image &image, bool overwrite )
{
	base.join( static_cast<const isis::util::PropertyMap &>( image ), overwrite );
}

void _join ( isis::util::PropertyMap &base, const isis::util::PropertyMap &map, bool overwrite )
{
	base.join( map, overwrite );
}

bool _removeProperty( isis::util::PropertyMap &base, const std::string &path )
{
	return base.remove( path.c_str() );
}

list _getKeys ( const isis::util::PropertyMap& base )
{
	list retList;
	BOOST_FOREACH( isis::util::PropertyMap::KeyList::const_reference key, base.getKeys() ) {
		retList.append( key.c_str() );
	}
	return retList;
}

list _getMissing ( const isis::util::PropertyMap& base )
{
	list retList;
		BOOST_FOREACH( isis::util::PropertyMap::KeyList::const_reference key, base.getMissing() ) {
		retList.append( key.c_str() );
	}
	return retList;
}


dict _convertToDict ( const isis::util::PropertyMap& base )
{
	dict retDict;
	BOOST_FOREACH( isis::util::PropertyMap::FlatMap::const_reference flat, base.getFlatMap() ) {
		
		retDict[flat.first.c_str()] = isis::util::Singletons::get<_internal::TypesMap, 10>()[flat.second.getTypeID()]->convert( *flat.second );
	}
	return retDict;
}


} // end namespace PropertyMap

}
}
} // end namespace