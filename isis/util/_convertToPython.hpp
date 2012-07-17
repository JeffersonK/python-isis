#ifndef CONVERTTOPYTHON_HPP
#define CONVERTTOPYTHON_HPP


#include <map>
#include <list>
#include "CoreUtils/common.hpp"
#include "CoreUtils/property.hpp"
#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/gregorian/gregorian_io.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <datetime.h>
#include <boost/date_time/posix_time/posix_time.hpp>


using namespace boost::python;

namespace isis
{

namespace python
{
namespace util
{
namespace _internal
{



struct PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) = 0;
};

template<bool ISNUM, typename T>
struct PyObjectGenerator : PyObjectGeneratorBase {};


//conversion for all numeric values
template<typename T>
struct PyObjectGenerator<true, T> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		return  api::object( value.as<T>() );
	}
};

template<typename T>
struct PyObjectGenerator<false, T> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		return api::object( value.as<T>() );
	}
};

//dates
template<>
struct PyObjectGenerator<false, boost::gregorian::date> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		PyDateTime_IMPORT;
		const boost::gregorian::date date = value.as<boost::gregorian::date>();
		return api::object( handle<>( borrowed( PyDate_FromDate( static_cast<int>( date.year() ),
												static_cast<int>( date.month() ),
												static_cast<int>( date.day() ) ) ) ) );


	}
};

//ptime
template<>
struct  PyObjectGenerator<false, boost::posix_time::ptime> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		PyDateTime_IMPORT;
		const boost::posix_time::ptime datetime = value.as<boost::posix_time::ptime>();
		const boost::posix_time::ptime min( boost::gregorian::min_date_time );

		if( datetime.date() != min.date() ) { // if our timestamp actually has a date different from min
			return api::object(
					   handle<>( borrowed( PyDateTime_FromDateAndTime(
											   static_cast<int>( datetime.date().year() ),
											   static_cast<int>( datetime.date().month() ),
											   static_cast<int>( datetime.date().day() ),
											   static_cast<int>( datetime.time_of_day().hours() ),
											   static_cast<int>( datetime.time_of_day().minutes() ),
											   static_cast<int>( datetime.time_of_day().seconds() ),
											   static_cast<int>( datetime.time_of_day().total_milliseconds() )
										   ) ) )
				   );
		} else {
			return api::object(
					   handle<>( borrowed( PyTime_FromTime(
											   static_cast<int>( datetime.time_of_day().hours() ),
											   static_cast<int>( datetime.time_of_day().minutes() ),
											   static_cast<int>( datetime.time_of_day().seconds() ),
											   static_cast<int>( datetime.time_of_day().total_milliseconds() )
										   ) ) )
				   );
		}
	}
};

template<>
struct PyObjectGenerator<false, isis::util::color24> : PyObjectGeneratorBase {
	virtual api::object convert ( isis::util::ValueBase &value ) {
		const isis::util::color24 cV = value.as<isis::util::color24>();
		return boost::python::make_tuple( cV.r, cV.g, cV.b );
	}
};

template<>
struct PyObjectGenerator<false, isis::util::color48> : PyObjectGeneratorBase {
	virtual api::object convert ( isis::util::ValueBase &value ) {
		const isis::util::color48 cV = value.as<isis::util::color48>();
		return boost::python::make_tuple( cV.r, cV.g, cV.b );
	}
};

//vectors
template<>
struct PyObjectGenerator<false, isis::util::ivector4> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		return api::object( value.as<isis::util::ivector4>() );
	}
};

template<>
struct PyObjectGenerator<false, isis::util::dvector3> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		return api::object( value.as<isis::util::dvector3>() );
	}
};

template<>
struct PyObjectGenerator<false, isis::util::dvector4> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		return api::object( value.as<isis::util::dvector4>() );
	}
};

template<>
struct PyObjectGenerator<false, isis::util::fvector4> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		return api::object( value.as<isis::util::fvector4>() );
	}
};

template<>
struct PyObjectGenerator<false, isis::util::fvector3> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		return api::object( value.as<isis::util::fvector3>() );
	}
};

template<>
struct PyObjectGenerator<false, isis::util::ilist> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		list retList;
		BOOST_FOREACH( isis::util::ilist::const_reference ref, value.as<isis::util::ilist>() ) {
			retList.append<int32_t>( ref );
		}
		return retList;
	}
};

template<>
struct PyObjectGenerator<false, isis::util::dlist> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		list retList;
		BOOST_FOREACH( isis::util::dlist::const_reference ref, value.as<isis::util::dlist>() ) {
			retList.append<double>( ref );
		}
		return retList;
	}
};

template<>
struct PyObjectGenerator<false, isis::util::slist> : PyObjectGeneratorBase {
	virtual api::object convert( isis::util::ValueBase &value ) {
		list retList;
		BOOST_FOREACH( isis::util::slist::const_reference ref, value.as<isis::util::slist>() ) {
			retList.append<std::string>( ref );
		}
		return retList;
	}
};

struct Generator {
	Generator( std::map<unsigned short, boost::shared_ptr<PyObjectGeneratorBase> > &tMap ) : typeMap( tMap ) {}

	template<typename T>
	void operator() ( const T & ) {
		typeMap[( unsigned short )isis::util::Value<T>::staticID] =
			boost::shared_ptr<PyObjectGeneratorBase>(
				new PyObjectGenerator<boost::is_arithmetic<T>::value, T>() );

	}
	std::map<unsigned short, boost::shared_ptr<PyObjectGeneratorBase> > &typeMap;
};


class TypesMap : public std::map<unsigned short, boost::shared_ptr<PyObjectGeneratorBase> >
{
public:
	void create() {
		boost::mpl::for_each<isis::util::_internal::types>( Generator( *this ) );
	}
};



}
}
}
} // end namespace


#endif
