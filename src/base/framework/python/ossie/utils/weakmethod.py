import weakref

class WeakBoundMethod(object):
    """
    Utility class to prevent callbacks from holding a strong reference back to the original object.
    """
    def __init__(self, func):
        self.__ref = weakref.ref(func.im_self)
        self.__func = func.im_func

    def __call__(self, *args, **kwargs):
        ref = self.__ref()
        if ref is not None:
            return self.__func(ref, *args, **kwargs)
        else:
            return None

    def __eq__(self, other):
        # Convert weak reference and function into a bound method and use that
        # to check equality.
        ref = self.__ref()
        func = self.__func.__get__(ref)
        return func == other
