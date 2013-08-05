
#ifndef PORT_H
#define PORT_H

#include "ossie/Port_impl.h"
#include <queue>
#include <list>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>

class TestComplexProps_base;
class TestComplexProps_i;

#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

#endif
