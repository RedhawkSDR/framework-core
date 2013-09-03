#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

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
