#ifndef BDN_SimpleProperty_H_
#define BDN_SimpleProperty_H_


#include <bdn/Property.h>

namespace bdn
{


/** The default implementation of a Property.
	*/
template<class ValType>
class SimpleProperty : public Property<ValType>
{
public:
	~SimpleProperty()
    {
        // ensure that we unsubscribe from the bound property
		// before any of our members get destroyed
        _pBindSourceSubscription = nullptr;
    }

	ValType get() const override
	{
		MutexLock lock(_mutex);

		return _value;
	}

	void set(const ValType& val) override
	{
		bool changed = false;

		{
			MutexLock lock(_mutex);

			if(_value!=val)
			{
				_value = val;
				changed = true;
			}
		}

		_onChange.fire(*this);
	}

	
	Notifier<ReadOnlyProperty>& onChange() override
	{
		return _onChange;
	}
    
    void bind(ReadOnlyProperty<ValType>& sourceProperty) override
	{
        _pBindSourceSubscription = sourceProperty.onChange().template subscribeMember<SimpleProperty>(this, &SimpleProperty::bindSourceChanged);
        
        bindSourceChanged(sourceProperty);
    }
    
    
protected:
    virtual void bindSourceChanged(const ReadOnlyProperty& prop)
    {
        set( prop.get() );
    }
    
	Notifier<ReadOnlyProperty>	_onChange;
	P<IBase>					_pBindSourceSubscription;
};


}

#endif
