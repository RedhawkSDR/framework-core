#ifndef __OMNIJNI_SERVANT_H__
#define __OMNIJNI_SERVANT_H__

#include <jni.h>

namespace omnijni {

    class Servant {
    public:
        Servant ():
            delegate_(NULL)
        {

        }

        void _set_delegate (JNIEnv* env, jobject delegate)
        {
            if (delegate_ != NULL) {
                env->DeleteWeakGlobalRef(delegate_);
            }
            if (delegate != NULL) {
                delegate = env->NewWeakGlobalRef(delegate);
            }
            delegate_ = delegate;
        }

    protected:
        jobject delegate_;
    };

} // namespace omnijni

#endif // __OMNIJNI_SERVANT_H__
