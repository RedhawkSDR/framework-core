#ifndef OVERRIDABLE_SINGLETON_H_
#define OVERRIDABLE_SINGLETON_H_

template< class T >
class OverridableSingleton
{
protected:
    OverridableSingleton(){}
    
public:
    static void SetImpl( T* impl ){ impl_ = impl; }
    static T* GetImpl(){ return impl_; }
    static T* DefaultImpl()
    {
        static T default_impl;
        return &default_impl;
    }

private:
    static T* impl_;
};

template<class T>
T* OverridableSingleton<T>::impl_ = OverridableSingleton<T>::DefaultImpl();

#endif