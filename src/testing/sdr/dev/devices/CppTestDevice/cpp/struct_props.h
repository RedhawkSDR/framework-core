#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/
#include <ossie/CorbaUtils.h>

struct memory_allocation_struct {
    memory_allocation_struct ()
    {
        contiguous = true;
        memory_type = 0;
    };

    static std::string getId() {
        return std::string("memory_allocation");
    };

    CORBA::Long capacity;
    bool contiguous;
    CORBA::Long memory_type;
};

inline bool operator>>= (const CORBA::Any& a, memory_allocation_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("capacity", props[idx].id)) {
            if (!(props[idx].value >>= s.capacity)) return false;
        }
        else if (!strcmp("contiguous", props[idx].id)) {
            if (!(props[idx].value >>= s.contiguous)) return false;
        }
        else if (!strcmp("memory_type", props[idx].id)) {
            if (!(props[idx].value >>= s.memory_type)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const memory_allocation_struct& s) {
    CF::Properties props;
    props.length(3);
    props[0].id = CORBA::string_dup("capacity");
    props[0].value <<= s.capacity;
    props[1].id = CORBA::string_dup("contiguous");
    props[1].value <<= s.contiguous;
    props[2].id = CORBA::string_dup("memory_type");
    props[2].value <<= s.memory_type;
    a <<= props;
};

inline bool operator== (const memory_allocation_struct& s1, const memory_allocation_struct& s2) {
    if (s1.capacity!=s2.capacity)
        return false;
    if (s1.contiguous!=s2.contiguous)
        return false;
    if (s1.memory_type!=s2.memory_type)
        return false;
    return true;
};

inline bool operator!= (const memory_allocation_struct& s1, const memory_allocation_struct& s2) {
    return !(s1==s2);
};


#endif
