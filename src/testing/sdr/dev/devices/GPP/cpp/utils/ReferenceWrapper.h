#ifndef REFERENCE_WRAPPER_H_
#define REFERENCE_WRAPPER_H_

/**
 * ReferenceWrapper is a minimal backport of the std::reference_wrapper class
 * from C++11.  It acts as a function object by providing the () operator
 * (unlike boost::reference_wrapper).
 */
template< class REFERENCE_TYPE, class RETURN_TYPE=REFERENCE_TYPE >
class ReferenceWrapper
{
public:
	typedef REFERENCE_TYPE type;
	typedef RETURN_TYPE result_type;
	typedef void argument_type;

	explicit ReferenceWrapper( type& ref ): ref_(ref){}

	result_type operator()() const { return static_cast<result_type>(ref_); }

	type& get() const { return ref_; }

private:
	type& ref_;
};

template< class REFERENCE_TYPE >
ReferenceWrapper<REFERENCE_TYPE> MakeRef( REFERENCE_TYPE& ref )
{
	return ReferenceWrapper<REFERENCE_TYPE>(ref);
}

template< class REFERENCE_TYPE, class RETURN_TYPE >
ReferenceWrapper<REFERENCE_TYPE, RETURN_TYPE> MakeRef( REFERENCE_TYPE const& ref )
{
	return ReferenceWrapper<REFERENCE_TYPE, RETURN_TYPE>(ref);
}

template< class REFERENCE_TYPE >
ReferenceWrapper<REFERENCE_TYPE const> MakeCref( REFERENCE_TYPE const& ref )
{
	//std::cout << "MakeCref: addr=" << (size_t)((REFERENCE_TYPE const*)(&ref)) << std::endl;
	return ReferenceWrapper<REFERENCE_TYPE const>(ref);
}

template< class REFERENCE_TYPE, class RETURN_TYPE >
ReferenceWrapper<REFERENCE_TYPE const, RETURN_TYPE const> MakeCref( REFERENCE_TYPE const& ref )
{
	return ReferenceWrapper<REFERENCE_TYPE const, RETURN_TYPE const>(ref);
}

#endif
