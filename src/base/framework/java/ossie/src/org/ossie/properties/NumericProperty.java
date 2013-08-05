package org.ossie.properties;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;

abstract class NumericProperty<T extends Number> extends AbstractSimpleProperty<T> {

    public NumericProperty(String id, String name, String type, T value, Mode mode,
                           Action action, Kind[] kinds) {
        super(id, name, type, value, mode, action, kinds);
    }

    public void setValue(Number value) {
        this.value = fromNumber(value);
    }

    protected T extract(Any any) {
        try {
            return fromNumber((Number)AnyUtils.convertAny(any));
        } catch (ClassCastException ex) {
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }
    protected abstract T fromNumber(Number value);

    public boolean allocate(T capacity) {
        if (compare(this.value, capacity) < 0) {
            return false;
        }
        this.value = subtract(this.value, capacity);
        return true;
    }

    public void deallocate(T capacity) {
        this.value = add(this.value, capacity);
    }

    protected abstract int compare(T lhs, T rhs);
    protected abstract T subtract(T lhs, T rhs);
    protected abstract T add(T lhs, T rhs);
}
