/*----------------------------------------------------------------------------
 INGENICO Technical Software Department
 ------------------------------------------------------------------------------
 Copyright (c) 2015, Ingenico.
 28-32 boulevard de Grenelle 75015 Paris, France.
 All rights reserved.
 This source program is the property of INGENICO Company and may not be copied
 in any form or by any means, whether in part or in whole, except under license
 expressly granted by INGENICO company
 All copies of this program, whether in part or in whole, and
 whether modified or not, must display this and all other
 embedded copyright and ownership notices in full. */

#ifndef MenuBindable_hpp_INCLUDED
#define MenuBindable_hpp_INCLUDED

/// Interface to bind event on method
class MenuBindable
{
public:
	/** Raise an event. Call the callback attached to the instance of the class. */
	virtual void raise() = 0;

	/** Destructor */
	virtual ~MenuBindable()
	{
	}
};

/// Class which links the object to the method
class MenuFunctionBind : public MenuBindable
{
public:
	/** Constructor on function
	 @param function function call back pointer */
	MenuFunctionBind(void (*function)())
	{
		m_function = function;
	}

	/** Destructor of this bind instance */
	virtual ~MenuFunctionBind()
	{
	}

	/** Raise an event. Call the callback attached to the instance of the class. */
	virtual void raise()
	{
		return m_function();
	}

protected:
	/** Method pointer to call during a raise of message */
	void (*m_function)();
};

/// Class which links the object to the method
template <class CLASS>
class MenuMethodBind : public MenuBindable
{
public:
	/** Constructor on method
	 @param object handle of the objet
	 @param method method call back pointer */
	MenuMethodBind(CLASS *object, void (CLASS::*method)())
	{
		m_object = object;
		m_method = method;
	}

	/** Destructor of this bind instance */
	virtual ~MenuMethodBind()
	{
	}

	/** Raise an event. Call the callback attached to the instance of the class. */
	virtual void raise()
	{
		(m_object->*m_method)();
	}

protected:
	/** Instance of the object */
	CLASS *m_object;

	/** Method pointer to call during a raise of message */
	void (CLASS::*m_method)();
};
#endif // MenuBindable_hpp_INCLUDED
