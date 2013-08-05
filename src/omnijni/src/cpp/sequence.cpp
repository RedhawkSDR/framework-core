#include <omnijni/sequence.h>

#define ARRAY_FROM_JOBJECT(jtype, atype, seqtype, FUNC) \
    void fromJObject (seqtype& sequence, JNIEnv* env, jobject obj) { \
        if (obj == NULL) { \
            sequence.length(0); \
            return; \
        } \
        jtype##Array array = (jtype##Array)obj; \
        CORBA::ULong size = env->GetArrayLength(array); \
        sequence.length(size); \
        if (size > 0) { \
            FUNC(jtype, atype); \
        } \
    }

#define ARRAY_FROM_JOBJECT_ASSIGN(jtype, atype) \
    jtype* data = reinterpret_cast<jtype*>(env->GetPrimitiveArrayCritical(array, NULL)); \
    for (CORBA::ULong ii = 0; ii < size; ++ii) { \
        sequence[ii] = data[ii]; \
    } \
    env->ReleasePrimitiveArrayCritical(array, data, JNI_ABORT);

#define ARRAY_FROM_JOBJECT_GET_REGION(jtype, atype) \
    jtype* data = reinterpret_cast<jtype*>(&sequence[0]); \
    env->Get##atype##ArrayRegion(array, 0, size, data);

#define ARRAY_TO_JOBJECT(jtype, atype, seqtype, FUNC) \
    jtype##Array toJObject (const seqtype& sequence, JNIEnv* env) { \
        jint size = sequence.length(); \
        jtype##Array array = env->New##atype##Array(size); \
        if (size > 0) { \
            FUNC(jtype,atype); \
        } \
        return array; \
    }

#define ARRAY_TO_JOBJECT_SET_REGION(jtype, atype) \
    const jtype* data = reinterpret_cast<const jtype*>(&sequence[0]); \
    env->Set##atype##ArrayRegion(array, 0, size, data);

#define ARRAY_TO_JOBJECT_ASSIGN(jtype, atype) \
    jtype* data = reinterpret_cast<jtype*>(env->GetPrimitiveArrayCritical(array, NULL)); \
    for (jint ii = 0; ii < size; ++ii) { \
        data[ii] = sequence[ii]; \
    } \
    env->ReleasePrimitiveArrayCritical(array, data, 0);

#define ARRAY_CONVERSIONS_COPY(jtype, atype, seqtype) \
    ARRAY_TO_JOBJECT(jtype, atype, seqtype, ARRAY_TO_JOBJECT_ASSIGN); \
    ARRAY_FROM_JOBJECT(jtype, atype, seqtype, ARRAY_FROM_JOBJECT_ASSIGN);

#define ARRAY_CONVERSIONS_DIRECT(jtype, atype, seqtype) \
    ARRAY_TO_JOBJECT(jtype, atype, seqtype, ARRAY_TO_JOBJECT_SET_REGION); \
    ARRAY_FROM_JOBJECT(jtype, atype, seqtype, ARRAY_FROM_JOBJECT_GET_REGION);

namespace omnijni {
    ARRAY_CONVERSIONS_COPY(jboolean, Boolean, _CORBA_Sequence_Boolean);
    ARRAY_CONVERSIONS_COPY(jchar, Char, _CORBA_Sequence_Char);
    ARRAY_CONVERSIONS_DIRECT(jbyte, Byte, _CORBA_Sequence_Octet);
    ARRAY_CONVERSIONS_DIRECT(jshort, Short, _CORBA_Sequence<CORBA::Short>);
    ARRAY_CONVERSIONS_DIRECT(jshort, Short, _CORBA_Sequence<CORBA::UShort>);
    ARRAY_CONVERSIONS_DIRECT(jint, Int, _CORBA_Sequence<CORBA::Long>);
    ARRAY_CONVERSIONS_DIRECT(jint, Int, _CORBA_Sequence<CORBA::ULong>);
    ARRAY_CONVERSIONS_DIRECT(jlong, Long, _CORBA_Sequence<CORBA::LongLong>);
    ARRAY_CONVERSIONS_DIRECT(jlong, Long, _CORBA_Sequence<CORBA::ULongLong>);
    ARRAY_CONVERSIONS_DIRECT(jfloat, Float, _CORBA_Sequence<CORBA::Float>);
    ARRAY_CONVERSIONS_DIRECT(jdouble, Double, _CORBA_Sequence<CORBA::Double>);
}
