#ifndef EVENT_DISPATCHER_H_
#define EVENT_DISPATCHER_H_

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

/**
 * EventDispatcher uses the Observer pattern to dispatch events to 1 or more
 * attached event listeners.  Listeners are registered as callback functions
 * in attach_listener() and called in sequence during dispatch().
 */
template<class MESSAGE_TYPE>
class EventDispatcher
{
public:
	typedef MESSAGE_TYPE MessageType;
	typedef boost::function< void (const MessageType&) > EventHandler;
	typedef std::vector<EventHandler> EventHandlers;

public:
	void attach_listener( const EventHandler& handler )
	{
		event_handlers_.push_back(handler);
	}

	void dispatch( const MessageType& message ) const
	{
		typename EventHandlers::const_iterator iter = event_handlers_.begin();
		typename EventHandlers::const_iterator end = event_handlers_.end();
		for( ; iter!=end; ++iter )
		{
			(*iter)(message);
		}
	}

private:
	EventHandlers event_handlers_;
};

/**
 * EventDispatcherMixin adds event dispatching semantics to child classes.
 * attach_listener() is exposed as a public method, as this is the client-
 * facing operation for event dispatching classes.  dispatch() is
 * exposed as protected, as it is expected for the event dispatching class to
 * call dispatch()  itself.
 *
 * Use the EventDispatcher class directly for cases where attach_listener()
 * and dispatch() need to be called by the same object.
 */
template<class MESSAGE_TYPE>
class EventDispatcherMixin
{
public:
	typedef typename EventDispatcher<MESSAGE_TYPE>::EventHandler EventHandler;
	typedef typename EventDispatcher<MESSAGE_TYPE>::MessageType MessageType;

protected:
	virtual ~EventDispatcherMixin(){}

public:
	void attach_listener( const EventHandler& handler )
	{
		event_dispatcher_.attach_listener(handler);
	}

protected:
	void dispatch( const MessageType& message ) const
	{
		event_dispatcher_.dispatch(message);
	}

private:
	EventDispatcher<MESSAGE_TYPE> event_dispatcher_;
};

#endif
