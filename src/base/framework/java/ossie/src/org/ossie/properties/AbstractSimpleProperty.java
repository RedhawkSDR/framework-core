package org.ossie.properties;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;

abstract class AbstractSimpleProperty<T> extends Property<T> {

    private final String type;

    protected AbstractSimpleProperty(String id,
                                     String name,
                                     String type,
                                     T value,
                                     Mode mode,
                                     Action action,
                                     Kind[] kinds) {
        super(id, name, value, mode, action, kinds);
        this.type = type;
    }

    @Override
    public void fromString(String str) {
        this.value = parseString(str);
    }

    @Override
    protected T fromAny_(Any any) {
        T newValue = null;
        if (!AnyUtils.isNull(any)) {
            newValue = extract(any);
        }
        return newValue;
    }

    @Override
    public Any toAny() {
        Any any = ORB.init().create_any();
        if (this.value != null) {
            insert(any, this.value);
        }
        return any;
    }

    @Override
    public String toString() {
        return this.getId() + "/" + this.getName() + " = " + this.value;
    }

    public String getType() {
        return this.type;
    }

    protected abstract T parseString(String str);
    protected abstract T extract(Any any);
    protected abstract void insert(Any any, T value);
}
