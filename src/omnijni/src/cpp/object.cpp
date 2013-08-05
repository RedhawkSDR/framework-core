#include <omnijni/object.h>
#include <omnijni/orb.h>

namespace omnijni {
    class ObjectImpl {
    public:
        static bool isInstance (JNIEnv* env, jobject obj) {
            OnLoad(env);
            return env->IsInstanceOf(obj, cls_);
        }

        static CORBA::Object_ptr getObjectRef (JNIEnv* env, jobject obj) {
            jlong ref = env->CallLongMethod(obj, get_object_ref_);
            return reinterpret_cast<CORBA::Object_ptr>(ref);
        }

        static void OnLoad (JNIEnv* env)
        {
            if (cls_) return;

            cls_ = omnijni::loadClass(env, "omnijni.ObjectImpl");
            get_object_ref_ = env->GetMethodID(cls_, "_get_object_ref", "()J");
        }

        static void OnUnload (JNIEnv* env)
        {
        }

    private:
        static jclass cls_;
        static jmethodID get_object_ref_;
    };

    jclass ObjectImpl::cls_ = NULL;
    jmethodID ObjectImpl::get_object_ref_ = 0;
}

jclass CORBA::jni::Object::cls_ = NULL;
jmethodID CORBA::jni::Object::ctor_ = 0;

void CORBA::jni::Object::fromJObject (CORBA::Object_var& out, JNIEnv* env, jobject obj)
{
    OnLoad(env);
    if (obj == NULL) {
        out = CORBA::Object::_nil();
    } else if (omnijni::ObjectImpl::isInstance(env, obj)) {
        CORBA::Object_ptr ref = omnijni::ObjectImpl::getObjectRef(env, obj);
        out = CORBA::Object::_duplicate(ref);
    } else {
        out = omnijni::ORB::object_to_native(env, obj);
    }
}

jobject CORBA::jni::Object::toJObject (CORBA::Object_ptr in, JNIEnv* env)
{
    OnLoad(env);
    jlong ref = reinterpret_cast<jlong>(CORBA::Object::_duplicate(in));
    return env->NewObject(cls_, ctor_, ref);
}

void CORBA::jni::Object::OnLoad (JNIEnv* env)
{
    if (cls_) return;

    omnijni::ObjectImpl::OnLoad(env);
    cls_ = omnijni::loadClass(env, "omnijni.CORBAObject");
    ctor_ = env->GetMethodID(cls_, "<init>", "(J)V");
}

void CORBA::jni::Object::OnUnload (JNIEnv* env)
{
    omnijni::ObjectImpl::OnUnload(env);
}

extern "C" JNIEXPORT jboolean JNICALL Java_omnijni_ObjectImpl__1is_1a (JNIEnv* env, jclass, jlong ref, jstring jrepoId)
{
    CORBA::Object_ptr object = reinterpret_cast<CORBA::Object_ptr>(ref);
    const char* repoId = env->GetStringUTFChars(jrepoId, NULL);
    CORBA::Boolean result = object->_is_a(repoId);
    env->ReleaseStringUTFChars(jrepoId, repoId);
    return (jboolean)result;
}

extern "C" JNIEXPORT jboolean JNICALL Java_omnijni_ObjectImpl__1non_1existent (JNIEnv *, jclass, jlong ref)
{
    CORBA::Object_ptr object = reinterpret_cast<CORBA::Object_ptr>(ref);
    return (jboolean)object->_non_existent();
}

extern "C" JNIEXPORT void JNICALL Java_omnijni_ObjectImpl__1delete_1object_1ref (JNIEnv* env, jobject, jlong ref)
{
    CORBA::release(reinterpret_cast<CORBA::Object*>(ref));
}

extern "C" JNIEXPORT jlong JNICALL Java_omnijni_CORBAObject__1narrow_1object_1ref (JNIEnv* env, jobject, jlong ref)
{
    CORBA::Object_ptr object = reinterpret_cast<CORBA::Object_ptr>(ref);
    return reinterpret_cast<jlong>(CORBA::Object::_duplicate(object));
}
